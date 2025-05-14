#ifndef __LAVA_INSTANCE_VR_H__
#define __LAVA_INSTANCE_VR_H__ 1

#include "openxr/openxr.h"
#include "lava/common/lava_global_helpers.hpp"
//#include "lava/openxr_common/GraphicsAPI.h"

class LavaInstanceVR
{
public:
	LavaInstanceVR();
	~LavaInstanceVR();

	XrInstance get_instance() {
		return instance_;
	}

	XrSystemId get_system_id() {
		return system_id_;
	}

private:
	void CreateInstance();
	void CreateDebugMessenger();
	void GetInstanceProperties();
	void GetSystemID();





private:
	XrInstance instance_;
	XrDebugUtilsMessengerEXT debug_utils_messenger;
	std::vector<std::string> instance_extensions_ = {};
	std::vector<std::string> api_layers = {};


	std::vector<const char*> active_api_Layers = {};
	std::vector<const char*> active_instance_extensions = {};
	GraphicsAPI_Type api_type_;

	XrFormFactor form_factor_ = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
	XrSystemId system_id_ = {};
	XrSystemProperties system_properties_ = { XR_TYPE_SYSTEM_PROPERTIES };
};



#endif // !__LAVA_INSTANCE_VR_H__
