#include "engine/lava_instance.hpp"

#include "lava_vulkan_helpers.hpp"

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
}

LavaInstance::~LavaInstance(){
	vkDestroyInstance(instance_, nullptr);
}


VkInstance LavaInstance::get_instance() const{
	return instance_;
}
