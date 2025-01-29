#ifndef __LAVA_ECS_COMPONENTS_H__
#define __LAVA_ECS_COMPONENTS_H__ 1

#include "lava_types.hpp"
#include <glm/gtx/euler_angles.hpp>
#include "scripting/lava_lua_script.hpp"


struct RenderComponent {
  bool active_;
  std::shared_ptr<class LavaMesh> mesh_;

  RenderComponent(){
    active_ = true;
  }
  RenderComponent(size_t entity_id) {
    active_ = true;
  }
};

struct TransformComponent {
  glm::vec3 pos_;
  glm::vec3 rot_;
  glm::vec3 scale_;

  TransformComponent() {
    pos_ = glm::vec3(0.0f);
    rot_ = glm::vec3(0.0f);
    scale_ = glm::vec3(1.0f);
  }
  TransformComponent(size_t entity_id) {
    pos_ = glm::vec3(0.0f);
    rot_ = glm::vec3(0.0f);
    scale_ = glm::vec3(1.0f);
  }
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

struct CameraComponent {
  float fov_;
  float near_;
  float far_;
  glm::mat4 view_;

  CameraComponent() {
    fov_ = 90.0f;
    view_ = glm::mat4(1.0f);

    near_ = 10000.0f;
    far_ = 0.1f;
  }

  CameraComponent(size_t entity_id) {
    fov_ = 90.0f;
    view_ = glm::mat4(1.0f);

    near_ = 10000.0f;
    far_ = 0.1f;
  }

  void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
    const glm::vec3 w{ glm::normalize(direction) };
    const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
    const glm::vec3 v{ glm::cross(w, u) };

    view_ = glm::mat4{ 1.f };
    view_[0][0] = u.x;
    view_[1][0] = u.y;
    view_[2][0] = u.z;
    view_[0][1] = v.x;
    view_[1][1] = v.y;
    view_[2][1] = v.z;
    view_[0][2] = w.x;
    view_[1][2] = w.y;
    view_[2][2] = w.z;
    view_[3][0] = -glm::dot(u, position);
    view_[3][1] = -glm::dot(v, position);
    view_[3][2] = -glm::dot(w, position);
  }

  void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
    setViewDirection(position, target - position, up);
  }

  void setViewYXZ(glm::vec3& position, glm::vec3 rotation) {
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
    const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
    const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
    view_ = glm::mat4{ 1.f };
    view_[0][0] = u.x;
    view_[1][0] = u.y;
    view_[2][0] = u.z;
    view_[0][1] = v.x;
    view_[1][1] = v.y;
    view_[2][1] = v.z;
    view_[0][2] = w.x;
    view_[1][2] = w.y;
    view_[2][2] = w.z;
    view_[3][0] = -glm::dot(u, position);
    view_[3][1] = -glm::dot(v, position);
    view_[3][2] = -glm::dot(w, position);
  }

  void LookAt(glm::vec3& pos, glm::vec3& rot) {
    float pitch = glm::radians(rot.x); // Rotación en el eje X
    float yaw = glm::radians(rot.y);   // Rotación en el eje Y
    float roll = glm::radians(rot.z);

    glm::mat4 rotation_matrix = glm::yawPitchRoll(yaw, pitch, roll);

    // Extraer el vector de dirección desde la matriz de rotación
    glm::vec3 view_direction = glm::vec3(rotation_matrix * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));

    glm::vec3 up_vector = glm::vec3(0.0f, 1.0f, 0.0f);
    // Calculamos el punto objetivo (target)
    glm::vec3 target = pos + glm::normalize(view_direction);
    //// Generamos la matriz de vista
    view_ = glm::lookAt(pos, target, up_vector);
  }
};

struct RotateComponent {
  bool can_rotate_;
  glm::vec3 rotate_speed_;

  RotateComponent() {
    can_rotate_ = false;
    rotate_speed_ = glm::vec3(1.0f);
  }
  RotateComponent(size_t entity) {
    can_rotate_ = false;
    rotate_speed_ = glm::vec3(1.0f);
  }
};

enum LightType {
  LIGHT_TYPE_DIRECTIONAL,
  LIGHT_TYPE_POINT,
  LIGHT_TYPE_SPOT
};



struct  LightComponent {
  bool enabled_;
  LightType type_;
  glm::vec3 dir_;
  glm::vec3 base_color_;
  glm::vec3 spec_color_;
  //glm::vec3 camera_position_; //Will be present on the render system structure
  //glm::vec3 pos_;             //Will be present on the render system structure
  float linear_att_;
  float quad_att_;
  float constant_att_;
  float shininess_;
  float strength_;
  glm::vec3 spot_dir_;
  float cutoff_;
  float outer_cutoff_;

  LightComponent() {
    enabled_ = true;
    type_ = LIGHT_TYPE_DIRECTIONAL;
    dir_ = glm::vec3(0.0f, 0.0f, 0.0f);
    base_color_ = glm::vec3(0.0f, 0.0f, 0.0f);
    spec_color_ = glm::vec3(0.0f, 0.0f, 0.0f);
    spot_dir_ = glm::vec3(0.0f, 0.0f, 0.0f);
    linear_att_ = 0.0014f;
    quad_att_ = 0.00007f;
    constant_att_ = 1.0f;
    shininess_ = 90.0f;
    strength_ = 0.5f;
    cutoff_ = cosf(3.1416f * 10.0f / 180.0f);
    outer_cutoff_ = cosf(3.1416f * 30.0f / 180.0f);
  }

  LightComponent(size_t entity) {
    enabled_ = true;
    type_ = LIGHT_TYPE_DIRECTIONAL;
    dir_ = glm::vec3(0.0f, 0.0f, 0.0f);
    base_color_ = glm::vec3(0.0f, 0.0f, 0.0f);
    spec_color_ = glm::vec3(0.0f, 0.0f, 0.0f);
    spot_dir_ = glm::vec3(0.0f, 0.0f, 0.0f);
    linear_att_ = 0.0014f;
    quad_att_ = 0.00007f;
    constant_att_ = 1.0f;
    shininess_ = 90.0f;
    strength_ = 0.5f;
    cutoff_ = cosf(3.1416f * 10.0f / 180.0f);
    outer_cutoff_ = cosf(3.1416f * 30.0f / 180.0f);
  }
};


//This structure will be loaded on the pbr shader
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

  void config(LightComponent light, TransformComponent tr) {
    enabled = light.enabled_;
    type = (int)light.type_;
    pos[0] = tr.pos_.x;
    pos[1] = tr.pos_.y;
    pos[2] = tr.pos_.z;

    // Matriz de identidad
    glm::mat4 rotationMatrix = glm::mat4(1.0f);

    // Aplicar rotaciones en el orden Z, Y, X (o el orden que prefieras)
    rotationMatrix = glm::rotate(rotationMatrix, tr.rot_.z, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotación en Z
    rotationMatrix = glm::rotate(rotationMatrix, tr.rot_.y, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotación en Y
    rotationMatrix = glm::rotate(rotationMatrix, tr.rot_.x, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotación en X

    // Obtener el vector forward (tercera columna de la matriz, invertido si Z negativo es forward)
    glm::vec3 forwardVector = -glm::vec3(rotationMatrix[2]);
    forwardVector = glm::normalize(forwardVector);

    
    //float pitch = glm::radians(tr.rot_.x); // Rotación en el eje X
    //float yaw = glm::radians(tr.rot_.y);   // Rotación en el eje Y
    //float roll = glm::radians(tr.rot_.z);
    //glm::mat4 rotation_matrix = glm::yawPitchRoll(yaw, pitch, roll);
    dir[0] = forwardVector.x; //glm::vec3(rotation_matrix * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
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
    cutoff = light.cutoff_;
    outer_cutoff = light.outer_cutoff_;
  }
};


#endif // !__LAVA_ECS_COMPONENTS_H__


