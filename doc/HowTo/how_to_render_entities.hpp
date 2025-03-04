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
 * 
 */