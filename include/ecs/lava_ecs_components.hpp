#ifndef __LAVA_ECS_COMPONENTS_H__
#define __LAVA_ECS_COMPONENTS_H__ 1

#include "lava_types.hpp"
#include <glm/gtx/euler_angles.hpp>


struct RenderComponent {
  bool active_;
  std::shared_ptr<class LavaMesh> mesh_;

  RenderComponent(){
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

#endif // !__LAVA_ECS_COMPONENTS_H__

//
//
//// CODIGO DEL USUARIO
//
//struct PhyicsComponent { void accelerate_x(); };
//struct PositionComponent {};
//struct AIComponent {};
//
//
//void physicsSystem(std::vector<std::optional<PhysicsComponent>>& physics_vector) {
//  for (auto& item : physics_vector) {
//    if (!item->exists()) continue;
//    // Actualizar fisicas
//  }
//}
//
//void renderSystem(std::vector<std::optional<RenderComponent>>& rc, std::vector<std::optional<PositionComponent>>& pc) {
//  auto ri = rc.begin();
//  auto pi = pc.begin();
//  auto riend = rc.end();
//  auto piend = pc.end();
//  for (; ri != riend || pi != piend; ri++, pi++) {
//    if (!ri->exists()) continue;
//    if (!pi->exists()) continue;
//    // TODO OPENGL
//  }
//}
//
//struct WheelComponent {};
//
//int main() {
//  ECSManager ecs;
//
//  ecs.add_component_type<PhyicsComponent>();
//  ecs.add_component_type<PositionComponent>();
//  ecs.add_component_type<AIComponent>();
//  ecs.add_component_type<WheelComponent>();
//  //
//  ecs.get_component<WheelComponent>(entity);
//  ecs.get_component<AIComponent>(entity);
//
//  ecs.get_component<AIComponent>(10);
//
//
//
//
//  // NUEVA ENTIDAD
//  size_t entidadFisica = ecs.create_entity();
//
//  // 2 FORMAS DE OPERAR
//    // POR ENTIDAD acceder a algun(os) componente(s) de una entidad especifica
//  ecs.add_physics_component(entidadFisica);
//  ecs.get_physics_component(entidadFisica)->accelerate_x();
//
//
//  // A TRAVES DE SISTEMAS
//  // Aplicamos una operacion a todas las entidades que tengan  algun(os) componente(s) especificos.
//
//  physicsSystem(ecs.phy_com_list);
//}
