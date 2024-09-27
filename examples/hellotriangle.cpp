#include "hellotriangle.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>

#include "custom_vulkan_helpers.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGTH = 600;

//Muchas de las validation layers incluidas en el sdk de vuelkan se encuentran
//reunidas(bundle) en una layer que se llama VK_LAYER_KHRONOS_validation
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif


class HelloTriangleApp {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}
private:
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	

	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(WIDTH, HEIGTH, "Vulkan", nullptr, nullptr);
	}

	void createInstance() {
		//Comprobamos si las validation layers que se quieren activar están disponibles
		if (enableValidationLayers && !checkValidationLayerSupport(validationLayers)) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		//Primero rellenamos la informacion de la aplicacion
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
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
		}else {
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
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vulkan instance!");
		}

	}
	void initVulkan() {
		createInstance();
		//Despues de crear la instancia se configura el callback de las validation layers
		if (enableValidationLayers) {
			setupDebugMessenger();
		}
		//Ahora se selecciona la tarjeta grafica que cumpla con las necesidades
		pickPhysicalDevice();
		//Tras seleccionar el dispositivo fisico ahora toca crear el logico
		createLogicalDevice();
	}
	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}
	void cleanUp() {
		vkDestroyDevice(device, nullptr);
		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		//Primero se obtienen la cantidad de GPUs disponibles en el equipo
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		//Ahora se meten los datos de las GPUs en el vector devices
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
			}
		}
		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find suitable GPU");
		}
	}

	void createLogicalDevice() {
		/*
		* Se tienen que rellenar diversas estructuras, la primera
		* indica el numero de colas que queremos crear
		*/
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;
		
		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

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
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;

		if (vkCreateDevice(physicalDevice, &createInfo,
			nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}
		//En el queue index se pone 0 porque se va a crear solo
		// una cola dentro de la familia de colas referentes a 
		// operaciones graficas
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	}

	//El objetivo de este metodo es el de configurar un callback que
	//se ejecutará cuando un validation layer falle.

	void setupDebugMessenger() {

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

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
			&debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}


};

int main() {
	HelloTriangleApp app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}