
#include "lava/vr/lava_engine_vr.hpp"
#include "lava/ecs/lava_ecs.hpp"
#include "lava/ecs/lava_diffuse_render_system_vr.hpp"
#include "lava/ecs/lava_deferred_render_system_vr.hpp"
#include "lava/ecs/lava_pbr_render_system_vr.hpp"
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
  LavaDeferredRenderSystemVR deferred_render_system_vr{ engine };
  LavaPBRRenderSystemVR pbr_render_system_vr{ engine };

  std::shared_ptr<LavaPBRMaterial> cube_material = std::make_shared<LavaPBRMaterial>(engine, MaterialPBRProperties());
  std::shared_ptr<LavaMesh> cube_mesh = CreateCube8v(cube_material);

  std::shared_ptr<LavaPBRMaterial> basic_material_ptr = std::make_shared<LavaPBRMaterial>(engine, MaterialPBRProperties());
  MeshProperties mesh_properties = {};

  //mesh_properties.mesh_path = "../examples/assets/x-wing_cockpit.glb";
  mesh_properties.mesh_path = "../examples/assets/x-wing_cockpit_no_window.glb";
  mesh_properties.material = basic_material_ptr;

  std::shared_ptr<LavaMesh> mesh_ = std::make_shared<LavaMesh>(engine, mesh_properties);

  std::shared_ptr<LavaImage> forest_texture = std::make_shared<LavaImage>(&engine, "../examples/assets/textures/forest.png");

  std::shared_ptr<LavaPBRMaterial> terrain_material = std::make_shared<LavaPBRMaterial>(engine, MaterialPBRProperties());
  terrain_material->UpdateBaseColorImage(forest_texture);

  std::shared_ptr<LavaMesh> terrain_mesh = CreateTerrain(terrain_material,
    32, 32, 8.0f, 1.0f, 0.15f, { 20,20 });

  for (auto& data : engine.global_scene_data_vector_) {
    data.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
  }

  size_t avocado_entity;
  {
    avocado_entity = ecs_manager.createEntity();
    ecs_manager.addComponent<TransformComponent>(avocado_entity);
    ecs_manager.addComponent<RenderComponent>(avocado_entity);
    auto transform_component = ecs_manager.getComponent<TransformComponent>(avocado_entity);
    if (transform_component) {
      auto& transform = transform_component->value();
      transform.pos_ = glm::vec3(0.0f, 0.0f, -5.0f);
      transform.scale_ = glm::vec3(30.0f, 30.0f, 30.0f);
      transform.rot_ = glm::vec3(0.0f, 180.0f, 0.0f);
    }

    auto render_component = ecs_manager.getComponent<RenderComponent>(avocado_entity);
    if (render_component) {
      auto& render = render_component->value();
      render.mesh_ = mesh_;
    }
  }


   size_t light_entity = ecs_manager.createEntity();
  {
    ecs_manager.addComponent<TransformComponent>(light_entity);
    ecs_manager.addComponent<LightComponent>(light_entity);

    auto light_component = ecs_manager.getComponent<LightComponent>(light_entity);
    if (light_component) {
      auto& light = light_component->value();
      light.enabled_ = true;
      light.type_ = LIGHT_TYPE_SPOT;
      light.base_color_ = glm::vec3(0.6f, 0.6f, 0.6f);
      light.spec_color_ = glm::vec3(0.0f, 0.0f, 0.0f);
      light.cutoff_ = 85.0f;
      light.outer_cutoff_ = 90.0f;
      light.constant_att_ = 1.0f;
      light.quad_att_ = 0.112f;
      light.strength_ = 0.28f;
    }
    auto tr_component = ecs_manager.getComponent<TransformComponent>(light_entity);
    if (tr_component) {
      auto& tr = tr_component->value();
      tr.rot_ = glm::vec3(-110.05f, 0.00f, -0.5f);
      tr.pos_ = glm::vec3(0.03f, 0.06f, -1.68f);
    }
  }

  //{
  //  size_t light_entity = ecs_manager.createEntity();
  //  ecs_manager.addComponent<TransformComponent>(light_entity);
  //  ecs_manager.addComponent<LightComponent>(light_entity);

  //  auto light_component = ecs_manager.getComponent<LightComponent>(light_entity);
  //  if (light_component) {
  //    auto& light = light_component->value();
  //    light.enabled_ = true;
  //    light.type_ = LIGHT_TYPE_POINT;
  //    light.base_color_ = glm::vec3(1.0f, 1.0f, 1.0f);
  //    light.spec_color_ = glm::vec3(0.0f, 0.0f, 0.0f);
  //  }
  //  auto tr_component = ecs_manager.getComponent<TransformComponent>(light_entity);
  //  if (tr_component) {
  //    auto& tr = tr_component->value();
  //    tr.rot_ = glm::vec3(0.0f, 0.0f, 0.0f);
  //    tr.pos_ = glm::vec3(0.0f, 0.0f, 0.0f);
  //  }
  //}

  //{
  //  size_t entity = ecs_manager.createEntity();
  //  ecs_manager.addComponent<TransformComponent>(entity);
  //  ecs_manager.addComponent<RenderComponent>(entity);
  //  //ecs_manager.addComponent<UpdateComponent>(entity);

  //  auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
  //  if (transform_component) {
  //    auto& transform = transform_component->value();
  //    transform.pos_ = glm::vec3(0.0f, -10.0f, -20.0f);
  //    transform.scale_ = glm::vec3(1.0f, 1.0f, 1.0f);
  //  }

  //  auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
  //  if (render_component) {
  //    auto& render = render_component->value();
  //    render.mesh_ = terrain_mesh;
  //  }
  //}



  int count = 0;
  while (!engine.shouldClose()) {
    auto tr_component = ecs_manager.getComponent<LightComponent>(light_entity);
    if (tr_component) {
      auto& tr = tr_component->value();
      //tr.base_color_.r = 0.2f + ((cosf(count * 0.01f) * 0.5f) + 0.5f);
      //tr.cutoff_ = ((sinf(count*0.01f) * 0.5f) + 0.5f) * 90.0f;
      //tr.outer_cutoff_ = tr.cutoff_;
    }
    count++;

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
          
          //diffuse_render_system_vr.render(i,
          //  ecs_manager.getComponentList<TransformComponent>(),
          //  ecs_manager.getComponentList<RenderComponent>()
          //  );
          deferred_render_system_vr.render(i,
            ecs_manager.getComponentList<TransformComponent>(),
            ecs_manager.getComponentList<RenderComponent>(),
            ecs_manager.getComponentList<LightComponent>()
            );

          //pbr_render_system_vr.renderWithShadows(i,
          //  ecs_manager.getComponentList<TransformComponent>(),
          //  ecs_manager.getComponentList<RenderComponent>(),
          //  ecs_manager.getComponentList<LightComponent>()
          //);

          engine.releaseView(i);
        }
      }
      engine.endFrame();
    }
  }


  return 0;
}
