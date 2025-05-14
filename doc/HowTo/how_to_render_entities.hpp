/**
 * @page how_to_render_entities Render Entities
 * 
 * 
 * @section section_render_entities_add_components Add Render and Transform Component
 * 
 * To render an entity, it is necessary to add a TransformComponent and RenderComponent
 * 
 * First of all @link how_to_load_mesh load a mesh @endlink following its corresponding tutorial
 *
 * @code {.hpp}
 * #include "lava/engine/lava_engine.hpp"
 * #include "lava/engine/lava_mesh.hpp"
 * #include "lava/ecs/lava_ecs.hpp"
 * #include "lava/engine/lava_pbr_material.hpp"
 * #include "lava/ecs/lava_normal_render_system.hpp"
 * @endcode
 * 
 * 
 * @code {.cpp}
 *       LavaECSManager ecs_manager;
 * 
 *       LavaPBRMaterial basic_material(engine, MaterialPBRProperties());
 * 
 *       MeshProperties mesh_properties = {};
 *       mesh_properties.mesh_path = "../examples/assets/Avocado.glb";
 *       mesh_properties.material = &basic_material;
 *
 *       std::shared_ptr<LavaMesh> mesh_ = std::make_shared<LavaMesh>(engine, mesh_properties);
 * 
 *       size_t entity = ecs_manager.createEntity();
 *       ecs_manager.addComponent<TransformComponent>(entity);
 *       ecs_manager.addComponent<RenderComponent>(entity);
 *
 *       auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
 *       if (transform_component) {
 *           auto& transform = transform_component->value();
 *           transform.pos_ = glm::vec3(0.0f, 0.0f, -15.0f);
 *           transform.scale_ = glm::vec3(1.0f, 1.0f, 1.0f);
 *
 *       }
 *       auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
 *       if (render_component) {
 *           auto& render = render_component->value();
 *           render.mesh_ = mesh_;
 *       }
 * @endcode
 * 
 * 
 * @section section_render_entities_call_system Render System
 * 
 * Just call the render system that you want 
 * 
 * @code {.cpp}
 *  LavaEngine engine;
 *  LavaNormalRenderSystem normal_render_system{engine};
 * 
 *  while (!engine.shouldClose()) {
 *	    engine.beginFrame();
 *	    engine.clearWindow();
 *  
 *	    engine.renderImgui();
 *	    normal_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
 *	    	ecs_manager.getComponentList<RenderComponent>());
 *  
 *	    engine.endFrame();
 *  }
 * @endcode
 * 
 * 
 * @section section_render_entities_call_diffuse_system Diffuse Render System
 * 
 * There is also a render system that only renders the diffuse texture of every entity with transform and render components
 *  
 * @code {.cpp}
 * 
 * LavaEngine engine;
 * LavaDiffuseRenderSystem diffuse_render_system{engine};
 * while (!engine.shouldClose()) {
 *
 *	engine.beginFrame();
 *	engine.clearWindow();
 *
 *	diffuse_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
 *		ecs_manager.getComponentList<RenderComponent>());
 *
 *	ecs_render_imgui(ecs_manager, camera_entity);
 *
 *	engine.endFrame();
 * }
 * @endcode
 * 
 */