#ifndef __LAVA_ECS_COMPONENTS_H__
#define __LAVA_ECS_COMPONENTS_H__ 1

#include "lava/common/lava_types.hpp"
#include <glm/gtx/euler_angles.hpp>
#include "lava/scripting/lava_lua_script.hpp"
//#include "lava/common/lava_global_helpers.hpp"
#include "lava/engine/lava_buffer.hpp"
#include "lava/ecs/lava_ecs.hpp"


struct RenderComponent {
  bool active_ = true;
  std::shared_ptr<class LavaMesh> mesh_;
};

struct TransformComponent {
  glm::vec3 pos_ = glm::vec3(0.0f);
  glm::vec3 rot_ = glm::vec3(0.0f);
  glm::vec3 scale_ = glm::vec3(1.0f);
};

struct LuaScriptComponent {
  std::unique_ptr<LavaLuaScript> script_;
  std::string lua_script_path;
  size_t entity;

  LuaScriptComponent() {
    script_ = std::make_unique<LavaLuaScript>();
    entity = -1;
    script_->set_int_variable("entity_id", -1);
  }
  LuaScriptComponent(size_t entity_id) {
    script_ = std::make_unique<LavaLuaScript>();
    entity = entity_id;
    script_->set_int_variable("ENTITY_ID", entity);
  }

  void set_lua_script_path(std::string& path ) {
    lua_script_path = path;
  }
};

enum CameraType {
  CameraType_Perspective,
  CameraType_Orthographic
};

struct CameraComponent {
  CameraType type_ = CameraType_Perspective;
  float fov_ = 90.0f;
  float near_ = 10000.0f;
  float far_ = 0.1f;
  float size_ = 5;
  glm::mat4 view_ = glm::mat4(1.0f);
};

struct RotateComponent {
  bool can_rotate_ = false;
  glm::vec3 rotate_speed_ = glm::vec3(1.0f);
};

enum LightType {
  LIGHT_TYPE_DIRECTIONAL,
  LIGHT_TYPE_POINT,
  LIGHT_TYPE_SPOT
};

struct  LightComponent {
  bool enabled_;
  LightType type_ = LIGHT_TYPE_DIRECTIONAL;
  LightType allocated_type_ = LIGHT_TYPE_DIRECTIONAL;
  glm::vec3 dir_ = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 base_color_ = glm::vec3(1.0f, 1.0f, 1.0f);
  glm::vec3 spec_color_ = glm::vec3(1.0f, 0.0f, 0.0f);
  float linear_att_ = 0.0014f;
  float quad_att_ = 0.00007f;
  float constant_att_ = 1.0f;
  float shininess_ = 90.0f;
  float strength_ = 0.5f;
  glm::vec3 spot_dir_ = glm::vec3(0.0f, 0.0f, 0.0f);
  float cutoff_ = 10.0f;
  float outer_cutoff_ = 10.0f;
  glm::mat4 viewproj_;

  bool allocated_ = false;
  std::unique_ptr<LavaBuffer> light_data_buffer_;
  std::unique_ptr<LavaBuffer> light_viewproj_buffer_;
  VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;
};


struct LightShaderStruct {
  float pos[3];
  int enabled;

  float dir[3];
  int type;

  float diff_color[3];
  float quad_att;

  float spec_color[3];
  float linear_att;

  float spot_dir[3];
  float constant_att;

  float shininess;
  float strength;
  float cutoff;
  float outer_cutoff;

  LightShaderStruct() {
    enabled = 1;
    type = 0;
    pos[0] = 0.0f;
    pos[1] = 0.0f;
    pos[2] = 0.0f;
    dir[0] = 0.0f;
    dir[1] = 0.0f;
    dir[2] = 0.0f;
    diff_color[0] = 0.0f;
    diff_color[1] = 0.0f;
    diff_color[2] = 0.0f;
    spec_color[0] = 0.0f;
    spec_color[1] = 0.0f;
    spec_color[2] = 0.0f;
    spot_dir[0] = 0.0f;
    spot_dir[1] = 0.0f;
    spot_dir[2] = 0.0f;
    linear_att = 0.0014f;
    quad_att = 0.00007f;
    constant_att = 1.0f;
    shininess = 90.0f;
    strength = 0.5f;
    cutoff = cosf(3.1416f * 10.0f / 180.0f);
    outer_cutoff = cosf(3.1416f * 30.0f / 180.0f);
  }

  void config(LightComponent& light, TransformComponent& tr) {
    enabled = light.enabled_;
    type = (int)light.type_;
    pos[0] = tr.pos_.x;
    pos[1] = tr.pos_.y;
    pos[2] = tr.pos_.z;

    // Matriz de identidad
    glm::mat4 rotationMatrix = glm::mat4(1.0f);

    // Aplicar rotaciones en el orden Z, Y, X (o el orden que prefieras)
    rotationMatrix = glm::rotate(rotationMatrix, tr.rot_.z, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotaci�n en Z
    rotationMatrix = glm::rotate(rotationMatrix, tr.rot_.y, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotaci�n en Y
    rotationMatrix = glm::rotate(rotationMatrix, tr.rot_.x, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotaci�n en X

    // Obtener el vector forward (tercera columna de la matriz, invertido si Z negativo es forward)
    glm::vec3 forwardVector = glm::vec3(rotationMatrix[2]);
    forwardVector = glm::normalize(forwardVector);

    dir[0] = forwardVector.x;
    dir[1] = forwardVector.y;
    dir[2] = forwardVector.z;

    diff_color[0] = light.base_color_.x;
    diff_color[1] = light.base_color_.y;
    diff_color[2] = light.base_color_.z;

    spec_color[0] = light.spec_color_.x;
    spec_color[1] = light.spec_color_.y;
    spec_color[2] = light.spec_color_.z;
    linear_att = light.linear_att_;
    quad_att = light.quad_att_;
    constant_att = light.constant_att_;
    shininess = light.shininess_;
    strength = light.strength_;
    spot_dir[0] = light.spot_dir_.x;
    spot_dir[1] = light.spot_dir_.y;
    spot_dir[2] = light.spot_dir_.z;
    cutoff = cosf(glm::radians(light.cutoff_));
    outer_cutoff = cosf(glm::radians(light.outer_cutoff_));
  }
};

/**
 * @brief Update component useful for gameplay purpouses
 * 
 */
struct UpdateComponent {
  /** Ecs manager to retrieve other components */
  class LavaECSManager* ecs_manager; 
  /** entity id that holds the component */
  size_t id; 
  /** function that will be called every frame by the LavaUpdateSystem */
  std::function<void(size_t id, LavaECSManager* ecs_manager, class LavaEngine& engine)> update_; 
};



#endif // !__LAVA_ECS_COMPONENTS_H__


