#include "engine/lava_instance.hpp"
#include "lava_vulkan_helpers.hpp"
#include "vr/lava_instance_vr.hpp"
#include "vr/lava_binding_vr.hpp"
#include "lava/openxr_common/OpenXRHelper.h"
#include "openxr/openxr_platform.h"
#include "vr/lava_helpers_vr.hpp"

LavaInstance::LavaInstance(std::vector<const char*> validation_layers){
#ifdef NDEBUG
	bool enableValidationLayers = false;
#else
	bool enableValidationLayers = true;
#endif

	//Comprobamos si las validation layers que se quieren activar están disponibles
  if (enableValidationLayers && !CheckValidationLayerSupport(validation_layers)) {
#ifndef NDEBUG
		printf("validation layers requested, but not available!");
#endif
		exit(-1);
	}

	//Primero rellenamos la informacion de la aplicacion
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "LavaEngine";
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
			static_cast<uint32_t>(validation_layers.size());
		createInfo.ppEnabledLayerNames = validation_layers.data();
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
#ifndef NDEBUG
		printf("Failed to create vulkan instance!");
#endif
		exit(-1);
	}

	setupDebugMessenger();
}


LavaInstance::LavaInstance(std::vector<const char*> validation_layers, 
	LavaInstanceVR& instance_vr, 
	LavaBindingVR& binding) {

#ifdef NDEBUG
	bool enableValidationLayers = false;
#else
	bool enableValidationLayers = true;
#endif

	//Comprobamos si las validation layers que se quieren activar están disponibles
	if (enableValidationLayers && !CheckValidationLayerSupport(validation_layers)) {
#ifndef NDEBUG
		printf("validation layers requested, but not available!");
#endif
		exit(-1);
	}

	XrGraphicsRequirementsVulkanKHR graphics_requirements{ XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR };
	
	OPENXR_CHECK_INSTANCE(
		binding.get_graphics_requirements_binding()(instance_vr.get_instance(), instance_vr.get_system_id(), &graphics_requirements),
		"Failed to get Graphics Requirements for Vulkan.",
		instance_vr.get_instance());

	VkApplicationInfo application_info;
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.pNext = nullptr;
	application_info.pApplicationName = "LavaEngineVR";
	application_info.applicationVersion = 1;
	application_info.pEngineName = "LavaEngineVR";
	application_info.engineVersion = 1;
	//application_info.apiVersion = VK_MAKE_API_VERSION(0, XR_VERSION_MAJOR(graphics_requirements.minApiVersionSupported), XR_VERSION_MINOR(graphics_requirements.minApiVersionSupported), 0);
	application_info.apiVersion = VK_API_VERSION_1_3;

	uint32_t instance_extension_count = 0;
	VULKAN_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr), "Failed to enumerate InstanceExtensionProperties.");

	std::vector<VkExtensionProperties> instance_extension_properties;
	std::vector<const char*> active_instance_extensions{};
	instance_extension_properties.resize(instance_extension_count);
	VULKAN_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extension_properties.data()), "Failed to enumerate InstanceExtensionProperties.");
	const std::vector<std::string>& openXrInstanceExtensionNames = GetInstanceExtensionsForOpenXR(instance_vr,binding);
	for (const std::string& requestExtension : openXrInstanceExtensionNames) {
		for (const VkExtensionProperties& extensionProperty : instance_extension_properties) {
			if (strcmp(requestExtension.c_str(), extensionProperty.extensionName))
				continue;
			else
				active_instance_extensions.push_back(requestExtension.c_str());
			break;
		}
	}

	VkInstanceCreateInfo instance_create_info;
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pNext = nullptr;
	instance_create_info.flags = 0;
	instance_create_info.pApplicationInfo = &application_info;

	//Activar las validation layers si se requiere
	if (enableValidationLayers) {
		//Se pasa la cantidad de validation layers asi como el puntero a ellas.
		instance_create_info.enabledLayerCount =
			static_cast<uint32_t>(validation_layers.size());
		instance_create_info.ppEnabledLayerNames = validation_layers.data();
	}
	else {
		instance_create_info.enabledLayerCount = 0;
	}

	instance_create_info.enabledExtensionCount = static_cast<uint32_t>(active_instance_extensions.size());
	instance_create_info.ppEnabledExtensionNames = active_instance_extensions.data();
	VULKAN_CHECK(vkCreateInstance(&instance_create_info, nullptr, &instance_), "Failed to create Vulkan Instance.");

}


LavaInstance::~LavaInstance(){
#ifndef NDEBUG
	DestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
#endif
	vkDestroyInstance(instance_, nullptr);
}


VkInstance LavaInstance::get_instance() const{
	return instance_;
}

void LavaInstance::setupDebugMessenger() {
#ifdef NDEBUG
	bool enableValidationLayers = false;
#else
	bool enableValidationLayers = true;
#endif
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

	if (CreateDebugUtilsMessengerEXT(get_instance(), &createInfo, nullptr,
		&debug_messenger_) != VK_SUCCESS) {
#ifndef NDEBUG
		printf("Failed to create Debug Utils Messenger");
#endif
		exit(-1);

	}
}
