
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


class HyperSpaceEffectLine {
public:
  HyperSpaceEffectLine(LavaECSManager& ecs_manager, LavaEngineVR& engine, std::shared_ptr<LavaMesh> mesh_,
    glm::vec3 init_pos, float speed) :
    ecs_manager_{ ecs_manager },
    engine_{ engine },
    speed_{ speed },
    init_pos_{ init_pos },
    expansion_rate_{ 0.1f + ((float)rand() / RAND_MAX) * 0.2f } // Tasa de expansión aleatoria entre 0.1 y 0.4
  {


    entity_id = ecs_manager.createEntity();
    ecs_manager.addComponent<TransformComponent>(entity_id);
    ecs_manager.addComponent<RenderComponent>(entity_id);

    auto transform_component = ecs_manager.getComponent<TransformComponent>(entity_id);
    if (transform_component) {

      auto& transform = transform_component->value();
      transform.pos_ = init_pos;
      transform.scale_ = glm::vec3(0.01f, 0.001f, 0.001f);
      transform.rot_ = glm::vec3(0.01f, 90.0f, 0.01f);
    }

    //glm::vec3(cosf(step * i), sin(step * i), 8.0f + rand() % 5);

    auto render_component = ecs_manager.getComponent<RenderComponent>(entity_id);
    if (render_component) {
      auto& render = render_component->value();
      render.mesh_ = mesh_;
    }

    initial_direction_ = glm::normalize(glm::vec2(init_pos.x, init_pos.y));
  }
  ~HyperSpaceEffectLine() {}
public:
  void reset() {
    auto& transform_component = ecs_manager_.getComponent<TransformComponent>(entity_id)->value();
    transform_component.pos_ = init_pos_;
    //transform_component.scale_ = glm::vec3(0.01f, 0.001f, 0.001f + speed_ * 0.01f);

    // Actualizar propiedades aleatorias
    speed_ = (float)(1 + rand() % 15);
    expansion_rate_ = 0.1f + ((float)rand() / RAND_MAX) * 0.3f;
    time_alive_ = 0.0f;
  }

  void hide() {
    auto& render_component = ecs_manager_.getComponent<RenderComponent>(entity_id)->value();
    render_component.active_ = false;
  }

  
  void update() {
    auto& transform_component = ecs_manager_.getComponent<TransformComponent>(entity_id)->value();

    float dt = static_cast<float>(engine_.dt_);

    time_alive_ += dt;

    float expansion_factor = time_alive_ * expansion_rate_;

    transform_component.pos_.z += speed_ * dt;


    transform_component.pos_.x += initial_direction_.x * expansion_rate_ * dt * speed_ * 0.5f;
    transform_component.pos_.y += initial_direction_.y * expansion_rate_ * dt * speed_ * 0.5f;

    glm::vec3 movement_dir = glm::normalize(glm::vec3(
      initial_direction_.x * expansion_rate_,
      initial_direction_.y * expansion_rate_,
      speed_
    ));

    if (transform_component.pos_.z > 10.0f || glm::length(glm::vec2(transform_component.pos_.x, transform_component.pos_.y)) > 20.0f) {
      reset();
    }
  }
  LavaEngineVR& engine_;
  std::shared_ptr<LavaPBRMaterial> basic_material;
  glm::vec2 initial_direction_;
  float expansion_rate_;       // Tasa de expansión radial
  float time_alive_;
  size_t entity_id;
  float speed_;
  glm::vec3 init_pos_;
  LavaECSManager& ecs_manager_;
};


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

  std::shared_ptr<LavaPBRMaterial> hyper_space_basic_material = std::make_shared<LavaPBRMaterial>(engine, MaterialPBRProperties());
  MeshProperties hyperspace_mesh_properties = {};
  hyperspace_mesh_properties.mesh_path = "../examples/assets/hyperspace_star_wars.glb";
  hyperspace_mesh_properties.material = hyper_space_basic_material;
  std::shared_ptr<LavaMesh> hyperspace_mesh_ = std::make_shared<LavaMesh>(engine, hyperspace_mesh_properties);

  std::shared_ptr<LavaPBRMaterial> death_basic_material = std::make_shared<LavaPBRMaterial>(engine, MaterialPBRProperties());
  MeshProperties death_star_mesh_properties = {};
  death_star_mesh_properties.mesh_path = "../examples/assets/death_star.glb";
  death_star_mesh_properties.material = hyper_space_basic_material;
  std::shared_ptr<LavaMesh> death_mesh_ = std::make_shared<LavaMesh>(engine, death_star_mesh_properties);






  int num_lines = 500;
  std::vector<HyperSpaceEffectLine> hyperspace_lines;

  const float MIN_ANGLE_RAD = 0.0f * (3.14159f / 180.0f);  // 30 grados en radianes
  const float MAX_ANGLE_RAD = 180.0f * (3.14159f / 180.0f); // 150 grados en radianes
  const float ANGLE_RANGE = MAX_ANGLE_RAD - MIN_ANGLE_RAD;


  for (int i = 0; i < num_lines; i++) {
    float angle = MIN_ANGLE_RAD + (((float)rand() / RAND_MAX) * ANGLE_RANGE);

    hyperspace_lines.push_back(HyperSpaceEffectLine(ecs_manager, engine, hyperspace_mesh_,
      glm::vec3(cosf(angle), (sin(angle)) - 0.5f, -8.0f - rand() % 5), 1.0f * (float)(1 + rand() % 15))
    );
  }

  size_t avocado_entity;
  {
    avocado_entity = ecs_manager.createEntity();
    ecs_manager.addComponent<TransformComponent>(avocado_entity);
    ecs_manager.addComponent<RenderComponent>(avocado_entity);
    auto transform_component = ecs_manager.getComponent<TransformComponent>(avocado_entity);
    if (transform_component) {
      auto& transform = transform_component->value();
      transform.pos_ = glm::vec3(0.0f, 0.0f, 0.0f);
      transform.scale_ = glm::vec3(1.0f, 1.0f, 1.0f);
      transform.rot_ = glm::vec3(0.0f, 180.0f, 0.0f);
    }

    auto render_component = ecs_manager.getComponent<RenderComponent>(avocado_entity);
    if (render_component) {
      auto& render = render_component->value();
      render.mesh_ = mesh_;
    }
  }

    size_t death_star_entity;
  {
      death_star_entity = ecs_manager.createEntity();
    ecs_manager.addComponent<TransformComponent>(death_star_entity);
    ecs_manager.addComponent<RenderComponent>(death_star_entity);
    auto transform_component = ecs_manager.getComponent<TransformComponent>(death_star_entity);
    if (transform_component) {
      auto& transform = transform_component->value();
      transform.pos_ = glm::vec3(500.0f, 0.0f, -250.0f);
      transform.scale_ = glm::vec3(50.0f, 50.0f, 50.0f);
      transform.rot_ = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    auto render_component = ecs_manager.getComponent<RenderComponent>(death_star_entity);
    if (render_component) {
      auto& render = render_component->value();
      render.mesh_ = death_mesh_;
      render.active_ = false;
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


  int count = 0;
  float time_hyperspace = 5.0f;
  while (!engine.shouldClose()) {

    
    if (count > 2) {
      
      time_hyperspace -= static_cast<float>(engine.dt_);
      if (time_hyperspace < 0.0f) {
        for (auto& line : hyperspace_lines) {
          line.hide();
        }
        auto& transform_component = ecs_manager.getComponent<TransformComponent>(death_star_entity)->value();
        transform_component.pos_.z += 20.0f * static_cast<float>(engine.dt_);
        auto& render_component = ecs_manager.getComponent<RenderComponent>(death_star_entity)->value();
        render_component.active_ = true;

      }
      else {


        for (auto& line : hyperspace_lines) {
          line.update();
        }
      }


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
