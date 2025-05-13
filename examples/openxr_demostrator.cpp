
#include "lava/vr/lava_engine_vr.hpp"
#include "lava/ecs/lava_ecs.hpp"
#include "lava/ecs/lava_diffuse_render_system_vr.hpp"
#include "lava/engine/lava_pbr_material.hpp"
#include "lava/engine/lava_mesh.hpp"
#include "lava/common/lava_shapes.hpp"
#include "lava/engine/lava_image.hpp"
#include "lava/ecs/lava_update_system.hpp"

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

  mesh_properties.mesh_path = "../examples/assets/spaceship_cockpit__seat.glb";
  mesh_properties.material = &basic_material;

  std::shared_ptr<LavaMesh> mesh_ = std::make_shared<LavaMesh>(engine, mesh_properties);

  std::shared_ptr<LavaImage> forest_texture = std::make_shared<LavaImage>(&engine, "../examples/assets/textures/forest.png");

  LavaPBRMaterial terrain_material(engine, MaterialPBRProperties());
  terrain_material.UpdateBaseColorImage(forest_texture);

  std::shared_ptr<LavaMesh> terrain_mesh = CreateTerrain(&terrain_material,
    32, 32, 8.0f, 1.0f, 0.15f, { 20,20 });



  size_t avocado_entity;
  {
    avocado_entity = ecs_manager.createEntity();
    ecs_manager.addComponent<TransformComponent>(avocado_entity);
    ecs_manager.addComponent<RenderComponent>(avocado_entity);
    auto transform_component = ecs_manager.getComponent<TransformComponent>(avocado_entity);
    if (transform_component) {
      auto& transform = transform_component->value();
      transform.pos_ = glm::vec3(0.0f, 0.0f, -5.0f);
      transform.scale_ = glm::vec3(3.0f, 3.0f, 3.0f);
      transform.rot_ = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    auto render_component = ecs_manager.getComponent<RenderComponent>(avocado_entity);
    if (render_component) {
      auto& render = render_component->value();
      render.mesh_ = mesh_;
    }
  }

  {
    size_t entity = ecs_manager.createEntity();
    ecs_manager.addComponent<TransformComponent>(entity);
    ecs_manager.addComponent<RenderComponent>(entity);
    //ecs_manager.addComponent<UpdateComponent>(entity);

    auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
    if (transform_component) {
      auto& transform = transform_component->value();
      transform.pos_ = glm::vec3(0.0f, -10.0f, -20.0f);
      transform.scale_ = glm::vec3(1.0f, 1.0f, 1.0f);
    }

    auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
    if (render_component) {
      auto& render = render_component->value();
      render.mesh_ = terrain_mesh;
    }
  }



  int count = 0;
  while (!engine.shouldClose()) {
    auto transform_component = ecs_manager.getComponent<TransformComponent>(avocado_entity);
    if (transform_component) {
      auto& transform = transform_component->value();
      //transform.pos_.x = cosf((++count)*0.01f);
      transform.pos_.y = 20.0f * sinf((++count)*0.01f);
      transform.rot_ = glm::vec3(0.0f);
    }


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
