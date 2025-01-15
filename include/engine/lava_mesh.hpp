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

#include "engine/lava_material.hpp"
#include "lava_engine.hpp"
#include "lava_types.hpp"
#include "lava_transform.hpp"
#include "fastgltf/core.hpp"


class LavaMesh
{
public:
	LavaMesh(class LavaEngine& engine, MeshProperties prop);
	~LavaMesh();

	GPUMeshBuffers upload(std::span<uint32_t> indices, std::span<Vertex> vertices);
	bool loadAsGLTF(std::filesystem::path file_path);
	bool loadCustomMesh(MeshProperties prop);

	LavaPBRMaterial* get_material() { return material_; };
	bool isLoaded() const { return is_loaded_; }
	//std::vector<std::shared_ptr<MeshAsset>> meshes_;
	std::shared_ptr<MeshAsset> mesh_;
	LavaTransform& get_transform() { return transform_; }
	std::shared_ptr<class LavaImage> loadImage(LavaEngine* engine, fastgltf::Asset& asset, fastgltf::Image& image);


private:
	std::string name_;
	MeshType type_;
	bool is_loaded_; 
	class LavaPBRMaterial* material_;
	class LavaEngine* engine_;
	class LavaTransform transform_;

};




#endif // !__LAVA_MESH_H__

