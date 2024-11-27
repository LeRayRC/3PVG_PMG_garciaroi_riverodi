#ifndef __LAVA_ECS_COMPONENTS_H__
#define __LAVA_ECS_COMPONENTS_H__ 1

#include "lava_types.hpp"


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

  void LookAt(glm::vec3 target, glm::vec3 pos) {
    //glm::vec3 viewDirection = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f)); // Dirección en la que mira
    //glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f); // Vector "arriba"

    //// Calculamos el punto objetivo (target)
    //glm::vec3 target = cameraPosition + viewDirection;

    //// Generamos la matriz de vista
    //view_ = glm::lookAt(cameraPosition, target, upVector);
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
