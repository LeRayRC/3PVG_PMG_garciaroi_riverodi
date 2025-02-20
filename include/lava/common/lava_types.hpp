#ifndef __LAVA_TYPES_
#define __LAVA_TYPES_ 1

struct AllocatedImage {
	VkImage image;
	VkImageView image_view;
	VmaAllocation allocation;
	VkExtent3D image_extent;
	VkFormat image_format;
};

struct Vertex {
	glm::vec3 position;
	float uv_x;
	glm::vec3 normal;
	float uv_y;
	glm::vec4 color;
};

struct VertexWithTangents {
	glm::vec3 position;
	float uv_x;
	glm::vec3 normal;
	float uv_y;
	glm::vec4 color;
	glm::vec3 tangent_;
	float padding1;
	glm::vec3 bitangent_;
	float padding2;
};

#endif // ! __LAVA_CUSTOM_TYPES_
