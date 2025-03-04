/** 
 * @page how_to_main_loop MainLoop
 * @ingroup how_to
 *
 * @section section_main_loop_engine Engine Main Render Loop
 * 
 * Inside the while loop, it is necessary to begin a new frame at the start of each iteration
 * then place all the render operations between begin and end frame. 
 * 
 * @code {.cpp}
 * #include "lava/engine/lava_engine.hpp"
 * @endcode
 * 
 * 
 * 
 * @code{.cpp}
 * while (!engine.shouldClose()) {
 * 
 * 	engine.beginFrame();
 * 	engine.clearWindow();
 * 
 *  //Call render operations, systems, etc..
 * 
 * 	engine.endFrame();
 * }
 * @endcode
 * 
 * @section section_main_loop_render_system Render System
 * 
 * The engine provides multiple render systems to perform draw operations over the ecs manager.
 * One of the is them LavaNormalRenderSystem that renders the normals of every entity with a TransformComponent and RenderComponent.
 * 
 * @code {.cpp}
 * #include "lava/ecs/lava_normal_render_system.hpp"
 * @endcode
 * 
 * @code {.cpp}
 * LavaEngine engine;
 * LavaECSManager ecs_manager;
 * LavaNormalRenderSystem normal_render_system{engine};
 * 
 * 
 * while (!engine.shouldClose()) {
 * 
 * 	engine.beginFrame();
 * 	engine.clearWindow();
 * 
 * 	normal_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
 * 		ecs_manager.getComponentList<RenderComponent>());
 * 
 * 	engine.endFrame();
 * }
 * @endcode
 */
