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
#include "lava_engine.hpp"
#include "lava_vulkan_helpers.hpp"
#include "lava_vulkan_inits.hpp"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

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
	//VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME
};

//bufferDeviceAddress


LavaEngine* loaded_engine = nullptr;

LavaEngine::LavaEngine()
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

LavaEngine::LavaEngine(unsigned int window_width, unsigned int window_height)
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

LavaEngine::~LavaEngine(){
	vkDeviceWaitIdle(device_);

	main_deletion_queue_.flush();

	for (int i = 0; i < FRAME_OVERLAP; i++) {
		vkDestroyCommandPool(device_, frames_[i].command_pool, nullptr);
		vkDestroyFence(device_, frames_[i].render_fence, nullptr);
		vkDestroySemaphore(device_, frames_[i].render_semaphore, nullptr);
		vkDestroySemaphore(device_, frames_[i].swap_chain_semaphore, nullptr);
		
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

void LavaEngine::init() {
  //Solo se puede llamar una vez a la inicializacion del motor

  initWindow();
	initVulkan();

  is_initialized_ = true;
}

void LavaEngine::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  window_ = glfwCreateWindow(window_extent_.width, window_extent_.height, "CustomEngine", nullptr, nullptr);
}

void LavaEngine::mainLoop() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();

		draw();
  }
}

void LavaEngine::cleanUp() {
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

void LavaEngine::initVulkan(){
	createInstance();
	//Despues de crear la instancia se configura el callback de las validation layers
	setupDebugMessenger();
	//Ahora se crea la superficie para dibujar sobre ella
	createSurface();
	//Ahora se selecciona la tarjeta grafica que cumpla con las necesidades
	pickPhysicalDevice();
	//Tras seleccionar el dispositivo fisico ahora toca crear el logico
	createLogicalDevice();
	createAllocator();
	createSwapChain();
	createImageViews();
	createCommandPool();
	createSyncObjects();
	/*
	createRenderPass();
	createGraphicsPipeline();*/
	

}
void LavaEngine::createInstance() {
	//Comprobamos si las validation layers que se quieren activar están disponibles
	if (enableValidationLayers && !CheckValidationLayerSupport(validationLayers)) {
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
	auto extensions = GetRequiredExtensions(enableValidationLayers);
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	//Se crea la instancia de Vulkan
	if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vulkan instance!");
	}
}
void LavaEngine::pickPhysicalDevice(){
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
bool LavaEngine::isDeviceSuitable(VkPhysicalDevice device){
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = FindQueueFamilies(device, surface_);

	bool extensionSupported = CheckDeviceExtensionsSupport(device, requiredDeviceExtensions);
	bool swapChainAdequate = false;

	//Es importante comprobar las propiedades disponibles de 
	//la swap chain despues de verificar que cuenta con las
	//extensiones necesarias
	//En este caso se comprueba que al menos puede representar
	//una imagen y un modo de presentacion
	if (extensionSupported) {
		SwapChainSupportDetails swapChainSupport =
			QuerySwapChainSupport(device, surface_);
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
void LavaEngine::createLogicalDevice(){
	/*
	* Se tienen que rellenar diversas estructuras, la primera
	* indica el numero de colas que queremos crear
	*/
	QueueFamilyIndices indices = FindQueueFamilies(physical_device_, surface_);
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
	
	VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address_feature_info;
	buffer_device_address_feature_info.bufferDeviceAddress = VK_TRUE;
	buffer_device_address_feature_info.bufferDeviceAddressCaptureReplay = VK_FALSE;
	buffer_device_address_feature_info.bufferDeviceAddressMultiDevice = VK_FALSE;
	buffer_device_address_feature_info.sType =
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
	buffer_device_address_feature_info.pNext = nullptr;


	VkPhysicalDeviceFeatures2 device_features{};
	device_features.sType =
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	vkGetPhysicalDeviceFeatures2(physical_device_, &device_features);
	device_features.pNext = &buffer_device_address_feature_info;
	//deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	//deviceFeatures.pNext = nullptr;


	/*
	* Ahora se rellena la estructura para crear el dispositivo logico
	*/
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	//createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
	//Al tener la extension de synchronization2 se tiene que agregar una estructura extra
	VkPhysicalDeviceSynchronization2Features sync2Info{};
	sync2Info.sType =
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
	sync2Info.synchronization2 = VK_TRUE;

	createInfo.pNext = &sync2Info;
	sync2Info.pNext = &device_features;

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
void LavaEngine::setupDebugMessenger(){
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
	createInfo.pfnUserCallback = DebugCallback;
	createInfo.pUserData = nullptr;

	if (CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr,
		&debug_messenger_) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}
void LavaEngine::createSurface(){
	if (glfwCreateWindowSurface(instance_, window_, nullptr, &surface_)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!!");
	}
}
void LavaEngine::createSwapChain(){
	SwapChainSupportDetails swapChainSupport =
		QuerySwapChainSupport(physical_device_, surface_);

	//CAREFUL 
	//En una guia recomiendan VK_FORMAT_B8G8R8A8_UNORM (https://vkguide.dev) 
	//Y en otra VK_FORMAT_B8G8R8A8_SRGV
	VkSurfaceFormatKHR surfaceFormat =
		ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode =
		ChooseSwapPresentMode(swapChainSupport.presentModes);
	
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

	QueueFamilyIndices indices = FindQueueFamilies(physical_device_, surface_);
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

	VkExtent3D draw_image_extent = {
		window_extent_.width,
		window_extent_.height,
		1
	};

	draw_image_.image_format = VK_FORMAT_R16G16B16A16_SFLOAT;
	draw_image_.image_extent = draw_image_extent;

	draw_extent_ = window_extent_;

	VkImageUsageFlags draw_image_usages{};
	draw_image_usages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	draw_image_usages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	draw_image_usages |= VK_IMAGE_USAGE_STORAGE_BIT;
	draw_image_usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateInfo rimg_info = vkinit::ImageCreateInfo(draw_image_.image_format,
		draw_image_usages, draw_image_extent);

	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	vmaCreateImage(allocator_, &rimg_info, &rimg_allocinfo, &draw_image_.image,
		&draw_image_.allocation, nullptr);

	VkImageViewCreateInfo rview_info = vkinit::ImageViewCreateInfo(draw_image_.image_format,
		draw_image_.image, VK_IMAGE_ASPECT_COLOR_BIT);
	if (vkCreateImageView(device_, &rview_info, nullptr, &draw_image_.image_view) !=
		VK_SUCCESS) {
		printf("Error creating image view!\n");
	}

	main_deletion_queue_.push_function([=]() {
		vkDestroyImageView(device_, draw_image_.image_view, nullptr);
		vmaDestroyImage(allocator_, draw_image_.image, draw_image_.allocation);
		});

}
void LavaEngine::createImageViews(){
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
void LavaEngine::createGraphicsPipeline(){}
void LavaEngine::createRenderPass(){}

void LavaEngine::createCommandPool() {
	//Primero se crean los command pool
	QueueFamilyIndices queueFamilyIndices =
		FindQueueFamilies(physical_device_,surface_);

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

void LavaEngine::createSyncObjects() {
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

void LavaEngine::draw() {
	//Esperamos a que el fence comunique que la grafica ya ha terminado de dibujar
	//El timeout esta en nanosegundos 10e-9
	if (vkWaitForFences(device_, 1, &getCurrentFrame().render_fence, true, 1000000000) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Fence timeout excedeed!");
#endif // !NDEBUG
	}
	getCurrentFrame().deletion_queue.flush();

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

	//Ahora se rellena la estructura del begin command buffer
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

	//Convertimos la imagen de dibujado a escribible
	TransitionImage(commandBuffer, draw_image_.image,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	drawBackground(commandBuffer);

	//Cambiamos tanto la imagen del swapchain como la de 
	// dibujado al mismo estado para copiar la informacion
	TransitionImage(commandBuffer,draw_image_.image ,
		VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	//Cambiamos la imagen a tipo presentable para enseñarla en la superficie
	TransitionImage(commandBuffer, swap_chain_images_[swap_chain_image_index],
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	//Copiamos la informacion
	CopyImageToImage(commandBuffer, draw_image_.image, swap_chain_images_[swap_chain_image_index],
		draw_extent_, window_extent_);

	//Se cambia la imagen al layout presentable
	TransitionImage(commandBuffer, swap_chain_images_[swap_chain_image_index],
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	//Se finalizar el command buffer
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

	//Se crea la estructura de presentacion para enviarla a la ventana de GLFW
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.pSwapchains = &swap_chain_;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &getCurrentFrame().render_semaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swap_chain_image_index;

	vkQueuePresentKHR(present_queue_, &presentInfo);

	//increase the number of frames drawn
	frame_number_++;

}

void LavaEngine::drawBackground(VkCommandBuffer command_buffer) {
	//Limpiamos la imagen con un clear color
	VkClearColorValue clearValue;
	clearValue = { {1.0f,0.0f,0.0f,0.0f} };

	//Seleccionamos un rango de la imagen sobre la que actuar
	VkImageSubresourceRange clearRange = ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

	//Aplicamos el clear color a una imagen 
	vkCmdClearColorImage(command_buffer, draw_image_.image,
		VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

}


void LavaEngine::createAllocator() {
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = physical_device_;
	allocatorInfo.device = device_;
	allocatorInfo.instance = instance_;
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	vmaCreateAllocator(&allocatorInfo, &allocator_);

	main_deletion_queue_.push_function([&]() {
		vmaDestroyAllocator(allocator_);
	});
}


