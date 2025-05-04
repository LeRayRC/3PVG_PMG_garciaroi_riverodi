#ifndef __LAVA_ENGINE_BASE_H__
#define __LAVA_ENGINE_BASE_H__ 1

// push constants for our mesh object draws
struct GPUDrawPushConstants {
	glm::mat4 world_matrix;
	VkDeviceAddress vertex_buffer;
};

struct GlobalSceneData {
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;
	glm::vec3 ambientColor;
	int gbuffer_render_selected;
	glm::vec3 cameraPos;
	int padding2;
};

enum class RenderingBackend {
	GLFW,
	OpenXR
};

class LavaEngineBase
{
public:
	LavaEngineBase();
	~LavaEngineBase();

	

	bool is_initialized_ = false;
	GlobalSceneData global_scene_data_;



	void setMainCamera(struct CameraComponent* camera_component,
		struct TransformComponent* camera_tr);

	struct CameraComponent* main_camera_camera_;
	struct TransformComponent* main_camera_transform_;
	double dt_;

	std::chrono::steady_clock::time_point chrono_now_;
	std::chrono::steady_clock::time_point chrono_last_update_;

	virtual bool shouldClose() = 0;
	virtual void beginFrame() = 0;
	virtual void endFrame() = 0;
	virtual void clearWindow() = 0;
	virtual void pollEvents() = 0;
	virtual void updateMainCamera() = 0;

	VkInstance get_instance() const;
private:

};




#endif // !__LAVA_ENGINE_BASE_H__
