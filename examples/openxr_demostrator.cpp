
#include "lava/vr/lava_engine_vr.hpp"
#include "lava/ecs/lava_ecs.hpp"
#include "lava/ecs/lava_diffuse_render_system_vr.hpp"
#include "lava/engine/lava_pbr_material.hpp"
#include "lava/engine/lava_mesh.hpp"
#include "lava/common/lava_shapes.hpp"
#include <openxr/openxr.h>


int main(int argc, char** argv) {
  XrPosef reference_pose = { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } };

  LavaEngineVR engine(reference_pose);
  LavaECSManager ecs_manager;
  LavaDiffuseRenderSystemVR diffuse_render_system_vr{ engine };

  LavaPBRMaterial cube_material(engine, MaterialPBRProperties());
  std::shared_ptr<LavaMesh> cube_mesh = CreateCube8v(&cube_material);

  LavaPBRMaterial basic_material(engine, MaterialPBRProperties());
  MeshProperties mesh_properties = {};

  mesh_properties.mesh_path = "../examples/assets/Avocado.glb";
  mesh_properties.material = &basic_material;

  std::shared_ptr<LavaMesh> mesh_ = std::make_shared<LavaMesh>(engine, mesh_properties);


  {
    size_t entity = ecs_manager.createEntity();
    ecs_manager.addComponent<TransformComponent>(entity);
    ecs_manager.addComponent<RenderComponent>(entity);
    auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
    if (transform_component) {
      auto& transform = transform_component->value();
      transform.pos_ = glm::vec3(0.0f, 0.0f, -2.0f);
      transform.scale_ = glm::vec3(20.0f, 20.0f, 20.0f);
    }

    auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
    if (render_component) {
      auto& render = render_component->value();
      render.mesh_ = mesh_;
    }
  }


  while (!engine.shouldClose()) {
    engine.pollEvents();
    if (engine.is_session_running()) {
      engine.beginFrame();
      if (engine.is_session_active()) {
        //It is necessary to render in two views
        for (uint32_t i = 0; i < engine.get_view_count(); i++) {
          engine.prepareView(i);
          engine.updateGlobalData(i);
          //RENDER!!!!
          engine.clearWindow(i);
          
          diffuse_render_system_vr.render(i,
            ecs_manager.getComponentList<TransformComponent>(),
            ecs_manager.getComponentList<RenderComponent>()
            );
          

          engine.releaseView(i);
        }
      }
      engine.endFrame();
    }
  }


  return 0;
}
