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

#include "dynamic_ptr.hpp"

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
	VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
};

Engine* Engine::loaded_engine = nullptr;

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
	swapChain = VK_NULL_HANDLE;
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
	swapChain = VK_NULL_HANDLE;
	swap_chain_image_format_ = VK_FORMAT_UNDEFINED;
	render_pass_ = VK_NULL_HANDLE;
	pipeline_layout_ = VK_NULL_HANDLE;
	graphics_pipeline_ = VK_NULL_HANDLE;
}

Engine::~Engine(){

	for (auto imageView : swap_chain_image_views_) {
		vkDestroyImageView(device_, imageView, nullptr);
	}
	vkDestroySwapchainKHR(device_, swapChain, nullptr);
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

  DynamicPtr<Engine> a;
  a.UpdatePointer(this);
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
	return true;
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
	//En el queue index se pone 0 porque se va a crear solo
	// una cola dentro de la familia de colas referentes a 
	// operaciones graficas
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
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

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
		nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swapchain!!");
	}
	vkGetSwapchainImagesKHR(device_, swapChain, &imageCount, nullptr);
	swap_chain_images_.resize(imageCount);
	vkGetSwapchainImagesKHR(device_, swapChain, &imageCount, swap_chain_images_.data());
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


