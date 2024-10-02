/**
 * @file custom_engine.hpp
 * @author ???
 * @brief Custom Engine's file
 * @version 0.1
 * @date 2024-10-01
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 */
#include "custom_engine.hpp"
#include "custom_vulkan_helpers.hpp"
#include "custom_vulkan_inits.hpp"

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> requiredDeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
	VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME
};



Engine* loaded_engine = nullptr;

Engine::Engine() 
{
	//Singleton Functionality
	assert(loaded_engine == nullptr);
	loaded_engine = this;

	is_initialized_ = false;
	frame_number_ = 0;
	stop_rendering = false;

	window_ = nullptr;
	window_extent_ = { 1280,720 };
	instance_ = VK_NULL_HANDLE;
	debug_messenger_ = VK_NULL_HANDLE;
	physical_device_ = VK_NULL_HANDLE;
	device_ = VK_NULL_HANDLE;
	graphics_queue_ = VK_NULL_HANDLE;
	present_queue_ = VK_NULL_HANDLE;
	surface_ = VK_NULL_HANDLE;
	swap_chain_ = VK_NULL_HANDLE;
	swap_chain_image_format_ = VK_FORMAT_UNDEFINED;
	render_pass_ = VK_NULL_HANDLE;
	pipeline_layout_ = VK_NULL_HANDLE;
	graphics_pipeline_ = VK_NULL_HANDLE;
}

Engine::Engine(unsigned int window_width, unsigned int window_height)
{
	//Singleton Functionality
	assert(loaded_engine == nullptr);
	loaded_engine = this;

	is_initialized_ = false;
	frame_number_ = 0;
	stop_rendering = false;

	window_ = nullptr;
	window_extent_ = { window_width,window_height };
	instance_ = VK_NULL_HANDLE;
	debug_messenger_ = VK_NULL_HANDLE;
	physical_device_ = VK_NULL_HANDLE;
	device_ = VK_NULL_HANDLE;
	graphics_queue_ = VK_NULL_HANDLE;
	present_queue_ = VK_NULL_HANDLE;
	surface_ = VK_NULL_HANDLE;
	swap_chain_ = VK_NULL_HANDLE;
	swap_chain_image_format_ = VK_FORMAT_UNDEFINED;
	render_pass_ = VK_NULL_HANDLE;
	pipeline_layout_ = VK_NULL_HANDLE;
	graphics_pipeline_ = VK_NULL_HANDLE;
}

Engine::~Engine(){
	vkDeviceWaitIdle(device_);
	for (int i = 0; i < FRAME_OVERLAP; i++) {
		vkDestroyCommandPool(device_, frames_[i].command_pool, nullptr);
	}

	for (auto imageView : swap_chain_image_views_) {
		vkDestroyImageView(device_, imageView, nullptr);
	}
	vkDestroySwapchainKHR(device_, swap_chain_, nullptr);
	vkDestroyDevice(device_, nullptr);
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
	}
	vkDestroySurfaceKHR(instance_, surface_, nullptr);
	vkDestroyInstance(instance_, nullptr);

	glfwDestroyWindow(window_);
	glfwTerminate();
}

void Engine::init() {
  //Solo se puede llamar una vez a la inicializacion del motor

  initWindow();
	initVulkan();

  is_initialized_ = true;
}

void Engine::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  window_ = glfwCreateWindow(window_extent_.width, window_extent_.height, "CustomEngine", nullptr, nullptr);
}

void Engine::mainLoop() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();

		draw();
  }
}

void Engine::cleanUp() {
	//vkDestroyPipeline(device, graphicsPipeline, nullptr);
	//vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	//vkDestroyRenderPass(device, renderPass, nullptr);
	//for (auto imageView : swap_chain_image_views_) {
	//	vkDestroyImageView(device_, imageView, nullptr);
	//}
	//vkDestroySwapchainKHR(device_, swapChain, nullptr);
	//vkDestroyDevice(device_, nullptr);
	//if (enableValidationLayers) {
	//	DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
	//}
	//vkDestroySurfaceKHR(instance_, surface_, nullptr);
	//vkDestroyInstance(instance_, nullptr);
	//
	//glfwDestroyWindow(window_);
	//glfwTerminate();
}

void Engine::initVulkan(){
	createInstance();
	//Despues de crear la instancia se configura el callback de las validation layers
	setupDebugMessenger();
	//Ahora se crea la superficie para dibujar sobre ella
	createSurface();
	//Ahora se selecciona la tarjeta grafica que cumpla con las necesidades
	pickPhysicalDevice();
	//Tras seleccionar el dispositivo fisico ahora toca crear el logico
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createCommandPool();
	createSyncObjects();
	/*
	createRenderPass();
	createGraphicsPipeline();*/
}
void Engine::createInstance() {
	//Comprobamos si las validation layers que se quieren activar están disponibles
	if (enableValidationLayers && !checkValidationLayerSupport(validationLayers)) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	//Primero rellenamos la informacion de la aplicacion
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "CustomEngine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	//Se rellena tambien la estructura para crear la 
	// instancia mas adelante. Uno de los parametros de la
	// estructura es la informacion de aplicacion creada antes
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	//Activar las validation layers si se requiere
	if (enableValidationLayers) {
		//Se pasa la cantidad de validation layers asi como el puntero a ellas.
		createInfo.enabledLayerCount =
			static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	//Obtener de GLFW las extensiones que el necesita para nuestra aplicacion
	//En el caso de que se quieran utilizar callbacks para manejar mensajes se 
	//requiere de una extension extra de modo que mediante la funcion getRequiredExtensions
	//obtenemos las extensions de glfw y añadimos la de debug si fuera necesario
	auto extensions = getRequiredExtensions(enableValidationLayers);
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	//Se crea la instancia de Vulkan
	if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vulkan instance!");
	}
}
void Engine::pickPhysicalDevice(){
	uint32_t deviceCount = 0;
	//Primero se obtienen la cantidad de GPUs disponibles en el equipo
	vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	//Ahora se meten los datos de las GPUs en el vector devices
	vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());
	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			physical_device_ = device;
		}
	}
	if (physical_device_ == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find suitable GPU");
	}
}
bool Engine::isDeviceSuitable(VkPhysicalDevice device){
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = findQueueFamilies(device, surface_);

	bool extensionSupported = checkDeviceExtensionsSupport(device, requiredDeviceExtensions);
	bool swapChainAdequate = false;

	//Es importante comprobar las propiedades disponibles de 
	//la swap chain despues de verificar que cuenta con las
	//extensiones necesarias
	//En este caso se comprueba que al menos puede representar
	//una imagen y un modo de presentacion
	if (extensionSupported) {
		SwapChainSupportDetails swapChainSupport =
			querySwapChainSupport(device, surface_);
		swapChainAdequate = !swapChainSupport.formats.empty() &&
			!swapChainSupport.presentModes.empty();
	}


	//Para que una GPU sea valida tiene que ser dedicada. Se pueden realizar las
	// comprobaciones que se quieran en base a las properties o las features
	// p.e se puede comprobar si la grafica permite usar geometry shaders
	// return deviceFeatures.geometryShader
	//return deviceProperties.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

	//No es muy importante para el inicio del motor tener esto bien configurado
	//Por conveniencia vamos a devolver true siempre y ya se cambiara
	//El tutorial recomiendo asociar una puntuacion a cada grafica del sistema 
	//para al menos seleccionar una
	// https://vulkan-tutorial.com/resources/vulkan_tutorial_en.pdf pag 60
	return indices.isComplete() && extensionSupported &&
		swapChainAdequate;
}
void Engine::createLogicalDevice(){
	/*
	* Se tienen que rellenar diversas estructuras, la primera
	* indica el numero de colas que queremos crear
	*/
	QueueFamilyIndices indices = findQueueFamilies(physical_device_, surface_);
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	float queuePriority = 1.0f;


	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	/* Tambien hay que indicar los features que se quieren
	* activar de la GPU, de momento se han dejado en blanco
	* pero se modificaran mas adelante
	*/
	VkPhysicalDeviceFeatures deviceFeatures{};

	/*
	* Ahora se rellena la estructura para crear el dispositivo logico
	*/
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();

	if (vkCreateDevice(physical_device_, &createInfo,
		nullptr, &device_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	//Index 0 hace referencia a que cola se va a utilizar de la 
	//familia seleccionada, arriba se podrian haber creado una 
	//cola de cada familia pero en este caso se ha creado una cola de 
	//dentro de la misma ya que soporta tanto comandos graficos como de presentacion
	//Tanto si fuesen iguales como diferentes, se seleccionaria la primera cola de cada familia
	// asi que el index seguiria siendo 0
	vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphics_queue_);
	vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &present_queue_);
}
void Engine::setupDebugMessenger(){
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType =
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;

	if (CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr,
		&debug_messenger_) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}
void Engine::createSurface(){
	if (glfwCreateWindowSurface(instance_, window_, nullptr, &surface_)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!!");
	}
}
void Engine::createSwapChain(){
	SwapChainSupportDetails swapChainSupport =
		querySwapChainSupport(physical_device_, surface_);

	//CAREFUL 
	//En una guia recomiendan VK_FORMAT_B8G8R8A8_UNORM (https://vkguide.dev) 
	//Y en otra VK_FORMAT_B8G8R8A8_SRGV (se ha seleccionado esta por defecto)
	VkSurfaceFormatKHR surfaceFormat =
		chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode =
		chooseSwapPresentMode(swapChainSupport.presentModes);
	
	//VkExtent2D extent =
	//	chooseSwapExtent(swapChainSupport.capabilities, window_);
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount >
		swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface_;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = window_extent_;
	createInfo.imageArrayLayers = 1;
	//VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
	//VK_IMAGE_USAGE_TRANSFER_DST_BIT util para postprocesos
	createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physical_device_, surface_);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(),
		indices.presentFamily.value() };
	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	if (vkCreateSwapchainKHR(device_, &createInfo,
		nullptr, &swap_chain_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swapchain!!");
	}
	vkGetSwapchainImagesKHR(device_, swap_chain_, &imageCount, nullptr);
	swap_chain_images_.resize(imageCount);
	vkGetSwapchainImagesKHR(device_, swap_chain_, &imageCount, swap_chain_images_.data());
	swap_chain_image_format_ = surfaceFormat.format;
}
void Engine::createImageViews(){
	swap_chain_image_views_.resize(swap_chain_images_.size());
	for (size_t i = 0; i < swap_chain_images_.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swap_chain_images_[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swap_chain_image_format_;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device_, &createInfo,
			nullptr, &swap_chain_image_views_[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image view!");
		}
	}
}
void Engine::createGraphicsPipeline(){}
void Engine::createRenderPass(){}

void Engine::createCommandPool() {
	//Primero se crean los command pool
	QueueFamilyIndices queueFamilyIndices =
		findQueueFamilies(physical_device_,surface_);

	VkCommandPoolCreateInfo commandPoolInfo{};
	commandPoolInfo.sType =
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex =
		queueFamilyIndices.graphicsFamily.value();

	for (int i = 0; i < FRAME_OVERLAP; i++) {
		if (vkCreateCommandPool(device_, &commandPoolInfo, nullptr, 
			&frames_[i].command_pool) != VK_SUCCESS) {
			exit(-1);
		}
		VkCommandBufferAllocateInfo commandAllocInfo{};
		commandAllocInfo.sType =
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandAllocInfo.pNext = nullptr;
		commandAllocInfo.commandPool = frames_[i].command_pool;
		commandAllocInfo.commandBufferCount = 1;
		commandAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		
		//Se reserva los command buffer correspondientes
		if (vkAllocateCommandBuffers(device_, &commandAllocInfo, 
			&frames_[i].main_command_buffer) != VK_SUCCESS) {
			exit(-1);
		}

	}
}

void Engine::createSyncObjects() {
	//1 fence para avisar a la CPU cuando un frame ya se ha dibujado
	//1 semaforo para la comunicacion con el swapchain y otro para
	//los comandos de dibujado
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType =
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = nullptr;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType =
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreInfo.pNext = nullptr;
	
	for (int i = 0; i < FRAME_OVERLAP; i++) {
		if (vkCreateFence(device_, &fenceInfo, nullptr, &frames_[i].render_fence) != VK_SUCCESS) {
			exit(-1);
		}
		if (vkCreateSemaphore(device_,&semaphoreInfo,nullptr,&frames_[i].render_semaphore) != VK_SUCCESS) {
			exit(-1);
		}

		if (vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &frames_[i].swap_chain_semaphore) != VK_SUCCESS) {
			exit(-1);
		}
	}
}

void Engine::draw() {
	//Esperamos a que el fence comunique que la grafica ya ha terminado de dibujar
	//El timeout esta en nanosegundos 10e-9
	if (vkWaitForFences(device_, 1, &getCurrentFrame().render_fence, true, 1000000000) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Fence timeout excedeed!");
#endif // !NDEBUG
	}
	//Reseteamos el fence
	if (vkResetFences(device_, 1, &getCurrentFrame().render_fence) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Fence restart failed!");
#endif // !NDEBUG
	}

	//Solicitamos una imagen del swap chain
	uint32_t swap_chain_image_index;
	if (vkAcquireNextImageKHR(device_, swap_chain_, 1000000000,
		getCurrentFrame().swap_chain_semaphore, nullptr, &swap_chain_image_index) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Swapchain image not retrieved!");
#endif // !NDEBUG
	}

	//Se resetea e inicia el command buffer del frame actual
	VkCommandBuffer commandBuffer = getCurrentFrame().main_command_buffer;
	if (vkResetCommandBuffer(commandBuffer, 0) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Resseting commandbuffer failed!");
#endif // !NDEBUG
	}

	//Ahora se rellena la estrcutura del begin command buffer
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType =
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
#ifndef NDEBUG
			printf("Begin commandbuffer failed!");
#endif // !NDEBUG
	}

	//Convertimos la imagen del swapchain en escribible
	transitionImage(commandBuffer, swap_chain_images_[swap_chain_image_index],
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	VkClearColorValue clearValue;
	float flash = std::abs(std::sin(frame_number_ / 120.f));
	clearValue = { {0.0f,0.0f,flash,0.0f}};

	VkImageSubresourceRange clearRange = imageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

	vkCmdClearColorImage(commandBuffer, swap_chain_images_[swap_chain_image_index],
		VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

	transitionImage(commandBuffer, swap_chain_images_[swap_chain_image_index],
		VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("End commandbuffer failed!");
#endif // !NDEBUG
	}

	VkCommandBufferSubmitInfo commandSubmitInfo = vkinit::CommandBufferSubmitInfo(commandBuffer);
	VkSemaphoreSubmitInfo waitInfo = vkinit::SemaphoreSubmitInfo(
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, getCurrentFrame().swap_chain_semaphore);
	VkSemaphoreSubmitInfo signalInfo = vkinit::SemaphoreSubmitInfo(
		VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, getCurrentFrame().render_semaphore);

	VkSubmitInfo2 submit = vkinit::SubmitInfo(&commandSubmitInfo, &signalInfo, &waitInfo);
	vkQueueSubmit2(graphics_queue_, 1, &submit, getCurrentFrame().render_fence);


	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.pSwapchains = &swap_chain_;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &getCurrentFrame().render_semaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swap_chain_image_index;

	vkQueuePresentKHR(graphics_queue_, &presentInfo);

	//increase the number of frames drawn
	frame_number_++;

}

void Engine::transitionImage(VkCommandBuffer cmd, VkImage image, 
	VkImageLayout currentLayout, VkImageLayout newLayout) {

	VkImageMemoryBarrier2 imageBarrier{};
	imageBarrier.sType =
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	imageBarrier.pNext = nullptr;
	imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
	imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

	imageBarrier.oldLayout = currentLayout;
	imageBarrier.newLayout = newLayout;

	VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ?
		VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	imageBarrier.subresourceRange = imageSubresourceRange(aspectMask);
	imageBarrier.image = image;

	VkDependencyInfo depInfo{};
	depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	depInfo.pNext = nullptr;
	depInfo.imageMemoryBarrierCount = 1;
	depInfo.pImageMemoryBarriers = &imageBarrier;

	vkCmdPipelineBarrier2(cmd, &depInfo);
}

VkImageSubresourceRange  Engine::imageSubresourceRange(VkImageAspectFlags aspectMask) {
	VkImageSubresourceRange subImage{};
	subImage.aspectMask = aspectMask;
	subImage.baseMipLevel = 0;
	subImage.levelCount = VK_REMAINING_MIP_LEVELS;
	subImage.baseArrayLayer = 0;
	subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

	return subImage;
}




