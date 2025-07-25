#ifndef __LAVA_MESH_H__
#define __LAVA_MESH_H__ 1
/**
 * @file lava_mesh.hpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Mesh header file
 * @version 0.1
 * @date 2024-10-01
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 **/

#include "lava/engine/lava_engine.hpp"
#include "fastgltf/core.hpp"

struct GPUMeshBuffers {
	std::unique_ptr<class LavaBuffer> index_buffer;
	std::unique_ptr<class LavaBuffer> vertex_buffer;
	VkDeviceAddress vertex_buffer_address;
};



struct GeoSurface {
	uint32_t start_index;
	uint32_t count;
	uint32_t material_index;
};

struct MeshAsset {
	MeshAsset() : name{ "NoName" }, count_surfaces{ 0 }, index_count{ 0 } {};
	std::string name;
	uint16_t count_surfaces;
	uint32_t index_count;
	std::vector<GeoSurface> surfaces;
	//GeoSurface surfaces[5];
	GPUMeshBuffers meshBuffers;
};

typedef enum MeshType {
	MESH_FBX,
	MESH_OBJ,
	MESH_GLTF,
	MESH_CUSTOM,
} MeshType;

struct MeshProperties {
	std::string name = "Generic Mesh";
	MeshType type = MESH_GLTF;
	std::filesystem::path mesh_path;
	std::shared_ptr<class LavaPBRMaterial>  material;
	std::vector<VertexWithTangents> vertex;
	std::vector<uint32_t> index;
};


class LavaMesh
{
public:
	LavaMesh(class LavaEngine& engine, MeshProperties prop);
	LavaMesh(class LavaEngineVR& engine, MeshProperties prop);
	~LavaMesh();

	template<typename t>
	GPUMeshBuffers upload(class LavaEngine* engine, std::span<uint32_t> indices, std::span<t> vertices);
	template<typename t>
	GPUMeshBuffers upload(class LavaEngineVR* engine, std::span<uint32_t> indices, std::span<t> vertices);

	template<typename t>
	bool loadAsGLTF(std::filesystem::path file_path);

	template<typename t>
	bool loadAsGLTFWithNodes(std::filesystem::path file_path);

	template<typename t>
	void processNode(const fastgltf::Asset& gltf,
		const fastgltf::Node& node,
		const glm::mat4& parentTransform,
		std::vector<t>& combinedVertices,
		std::vector<uint32_t>& combinedIndices,
		std::vector<GeoSurface>& surfaces,
		uint32_t& index_count,
		uint16_t& count_surfaces);

	bool loadCustomMesh(MeshProperties prop);

	LavaPBRMaterial* get_material() { return material_.get(); };
	bool isLoaded() const { return is_loaded_; }
	std::shared_ptr<MeshAsset> mesh_;
	std::shared_ptr<class LavaImage> loadImage(fastgltf::Asset& asset, fastgltf::Image& image, std::filesystem::path);

	std::vector<std::shared_ptr<LavaPBRMaterial>> materials_;

private:
	std::string name_;
	MeshType type_;
	bool is_loaded_; 
	std::shared_ptr<LavaPBRMaterial> material_;
	class LavaEngine* engine_;
	class LavaEngineVR* engine_vr_;

};




#endif // !__LAVA_MESH_H__

