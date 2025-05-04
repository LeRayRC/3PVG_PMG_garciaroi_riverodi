#ifndef __LAVA_SESSION_H__
#define __LAVA_SESSION_H__ 1

#include <openxr/openxr.h>

class LavaSessionVR
{
public:
	LavaSessionVR(class LavaInstanceVR& instance_vr,class LavaInstance& instance, class LavaDevice& device, XrViewConfigurationType view_configuration_type);
	~LavaSessionVR();

	void set_state(XrSessionState state) {
		sessionState_ = state;
	}

	XrSessionState get_state() const{
		return sessionState_;
	}

	XrSession get_session() const {
		return session_;
	}

	XrViewConfigurationType get_configuration_view_type() {
		return view_configuration_type_;
	}

	std::vector<XrViewConfigurationView>& get_view_configuration_views() {
		return view_configuration_views;
	}

private:
	void GetConfigurationViews(XrViewConfigurationType view_configuration_type);
	
	XrSession session_ = XR_NULL_HANDLE;
	XrSessionState sessionState_ = XR_SESSION_STATE_UNKNOWN;
	std::vector<XrViewConfigurationView> view_configuration_views;
	XrViewConfigurationType view_configuration_type_ = XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM;
	LavaInstanceVR& instance_vr_;
};


#endif // !__LAVA_SESSION_H__
