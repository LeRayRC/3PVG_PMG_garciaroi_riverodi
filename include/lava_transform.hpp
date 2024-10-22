#ifndef __LAVA_TRANSFORM_H__ 
#define __LAVA_TRANSFORM_H__ 1

/**
 * @file lava_transform.hpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Transform header file
 * @version 0.1
 * @date 2024-10-01
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 **/
#include "lava_types.hpp"

class LavaTransform
{
public:
	LavaTransform();
	LavaTransform(const LavaTransform& other);
	void operator=(const LavaTransform& other);
	~LavaTransform();


	glm::vec3 get_pos() const { return pos_; }
	glm::vec3 get_rot() const { return rot_; }
	glm::vec3 get_scale() const { return scale_; }

	void set_pos(glm::vec3 new_pos) { pos_ = new_pos; }
	void set_rot(glm::vec3 new_rot) { rot_ = new_rot; }
	void set_scale(glm::vec3 new_scale) { scale_ = new_scale; }

	static glm::mat4 TranslationMatrix(glm::mat4 model, LavaTransform& transform);
	static glm::mat4 RotateMatrix(glm::mat4 model, LavaTransform& transform);
	static glm::mat4 ScaleMatrix(glm::mat4 model, LavaTransform& transform);

private:
	glm::vec3 pos_;
	glm::vec3 rot_;
	glm::vec3 scale_;
};




#endif // !__LAVA_TRANSFORM_H__ 
