#include "lava/common/lava_engine_base.hpp"
#include "lava/engine/lava_image.hpp"
#include "lava/ecs/lava_ecs_components.hpp"
#include "engine/lava_instance.hpp"
#include "engine/lava_descriptor_manager.hpp"




LavaEngineBase::LavaEngineBase() :
	dt_{ 0.0f }
{
	
	main_camera_camera_ = nullptr;
	main_camera_transform_ = nullptr;
	is_initialized_ = true;
}

LavaEngineBase::~LavaEngineBase()
{
}

void LavaEngineBase::setMainCamera(struct CameraComponent* camera_component,
	struct TransformComponent* camera_tr) {
	main_camera_camera_ = camera_component;
	main_camera_transform_ = camera_tr;
}


