#include "lava_transform.hpp"

LavaTransform::LavaTransform(){
  pos_ = { 0.0f,0.0f,0.0f };
  rot_ = { 0.0f,0.0f,0.0f };
  scale_ = { 1.0f, 1.0f,1.0f };
}

LavaTransform::~LavaTransform()
{
}

LavaTransform::LavaTransform(const LavaTransform& other) {
  pos_ = other.pos_;
  rot_ = other.rot_;
  scale_ = other.scale_;
}

void LavaTransform::operator=(const LavaTransform& other) {
  pos_ = other.pos_;
  rot_ = other.rot_;
  scale_ = other.scale_;
}


glm::mat4 LavaTransform::TranslationMatrix(glm::mat4 model, LavaTransform& transform) {
  return glm::translate(model, transform.get_pos());

}
glm::mat4 LavaTransform::RotateMatrix(glm::mat4 model, LavaTransform& transform) {
  glm::mat4 rotate;
  rotate = glm::rotate(model, transform.get_rot().x, glm::vec3(1.0f, 0.0f, 0.0f));
  rotate = glm::rotate(model, transform.get_rot().y, glm::vec3(0.0f, 1.0f, 0.0f));
  rotate = glm::rotate(model, transform.get_rot().z, glm::vec3(0.0f, 0.0f, 1.0f));
  return rotate;
}
glm::mat4 LavaTransform::ScaleMatrix(glm::mat4 model, LavaTransform& transform) {
  return glm::scale(model, transform.get_scale());
}


