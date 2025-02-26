/**
 * @page how_to_camera Camera setup
 * 
 * The camera can be configured using a CameraComponent and a TransformComponent.
 * 
 * Both components are defined in the file lava/ecs/lava_ecs_components.hpp
 * 
 * @section section_create_camera_entity Create Camera Entity
 * 
 * First, it is necessary to create an ecs_manager instance. After that, it is possible
 * to create an entity and add the necessary components.
 * 
 * It is important to call the engine method setMainCamera so the engine can update its 
 * global camera configuration that will be used at rendering.
 * 
 * 
 * @code {.cpp}
 * #include "lava/engine/lava_engine.hpp"
 * #include "lava/ecs/lava_ecs.hpp"
 * #include "lava/ecs/lava_ecs_components.hpp"
 * @endcode
 * 
 * @code {.cpp}
 * 
 * 	LavaEngine engine;
 * 	LavaECSManager ecs_manager;
 * 
 * 	size_t camera_entity = ecs_manager.createEntity();
 * 	ecs_manager.addComponent<TransformComponent>(camera_entity);
 * 	ecs_manager.addComponent<CameraComponent>(camera_entity);
 *
 * 	auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
 * 	camera_tr.rot_ = glm::vec3(0.0f, 0.0f, 0.0f);
 * 	camera_tr.pos_ = glm::vec3(0.0f, 0.0f, 0.0f);
 * 	
 *     engine.setMainCamera(&camera_component, &camera_tr);
 * 	
 * @endcode
 * 
 * 
 * @section section_cupdate_camera_entity FlyCam
 * 
 * There is another component called UpdateComponent that can be assigned to every entity
 * Through the LavaUpdateSystem, the ecs will call the update function within every UpdateComponent.
 * 
 * @code {.cpp}
 * 
 * ecs_manager.addComponent<UpdateComponent>(camera_entity);
 * 
 * auto& update_component = ecs_manager.getComponent<UpdateComponent>(camera_entity)->value();
 * update_component.id = camera_entity;
 * upupdate_componentdate.ecs_manager = &ecs_manager;
 * update_component.update_ = [](size_t id, LavaECSManager* ecs_manager, LavaEngine& engine) {
 * 		UpdateCameraWithInput(id, ecs_manager, engine);
 * 	};
 * 
 * @endcode
 * 
 * 
 * 
 *  
 *
 * 
 *
 */