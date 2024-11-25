#include "engine/lava_mesh.hpp"

#include "lava_types.hpp"
#include <unordered_map>
#include <filesystem>

#include "stb_image.h"

#include "lava_vulkan_inits.hpp"

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

LavaMesh::LavaMesh(LavaEngine& engine, MeshProperties prop){
  name_ = prop.name;
  engine_ = &engine;
  material_ = nullptr;
  type_ = prop.type;
  material_ = prop.material;
	switch (type_)
	{
	case MESH_FBX:
		break;
	case MESH_GLTF:
    loadAsGLTF(prop.mesh_path);
		break;
	case MESH_OBJ:
		break;
  case MESH_CUSTOM:
    loadCustomMesh(prop);
    break;
	default:
		printf("Mesh Type not provided, Select FBX, OBJ or GLTF!!");
		exit(-1);
		break;
	}

  is_loaded_ = true;
}

LavaMesh::~LavaMesh(){
  for (auto mesh : meshes_) {
    engine_->destroyBuffer(mesh->meshBuffers.index_buffer);
    engine_->destroyBuffer(mesh->meshBuffers.vertex_buffer);
  }
}

bool LavaMesh::loadAsGLTF(std::filesystem::path file_path){
  std::cout << "Loading GLTF: " << file_path << std::endl;

  fastgltf::GltfDataBuffer data;
  data.loadFromFile(file_path);

  constexpr auto gltfOptions = fastgltf::Options::LoadGLBBuffers
    | fastgltf::Options::LoadExternalBuffers;

  fastgltf::Asset gltf;
  fastgltf::Parser parser{};

  auto load = parser.loadGltfBinary(&data, file_path.parent_path(), gltfOptions);
  if (load) {
    gltf = std::move(load.get());
  }
  else {
    printf("Failed to load glTF: %d \n", fastgltf::to_underlying(load.error()));
    return false;
  }

  std::vector<std::shared_ptr<MeshAsset>> meshes;

  // use the same vectors for all meshes so that the memory doesnt reallocate as
  // often
  std::vector<uint32_t> indices;
  std::vector<Vertex> vertices;
  for (fastgltf::Mesh& mesh : gltf.meshes) {
    MeshAsset newmesh;

    //Empieza a rellenar las mallas con su nombre primero
    newmesh.name = mesh.name;

    // clear the mesh arrays each mesh, we dont want to merge them by error
    indices.clear();
    vertices.clear();

    for (auto&& p : mesh.primitives) {
      GeoSurface newSurface;
      newSurface.start_index = (uint32_t)indices.size();
      newSurface.count = (uint32_t)gltf.accessors[p.indicesAccessor.value()].count;

      size_t initial_vtx = vertices.size();

      // load indexes
      {
        fastgltf::Accessor& indexaccessor = gltf.accessors[p.indicesAccessor.value()];
        indices.reserve(indices.size() + indexaccessor.count);

        fastgltf::iterateAccessor<std::uint32_t>(gltf, indexaccessor,
          [&](std::uint32_t idx) {
            indices.push_back(idx + (uint32_t)initial_vtx);
          });
      }

      // load vertex positions
      {
        fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->second];
        vertices.resize(vertices.size() + posAccessor.count);

        fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
          [&](glm::vec3 v, size_t index) {
            Vertex newvtx;
            newvtx.position = v;
            newvtx.normal = { 1, 0, 0 };
            newvtx.color = glm::vec4{ 1.f };
            newvtx.uv_x = 0;
            newvtx.uv_y = 0;
            vertices[initial_vtx + index] = newvtx;
          });
      }

      // load vertex normals
      auto normals = p.findAttribute("NORMAL");
      if (normals != p.attributes.end()) {

        fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[(*normals).second],
          [&](glm::vec3 v, size_t index) {
            vertices[initial_vtx + index].normal = v;
          });
      }

      // load UVs
      auto uv = p.findAttribute("TEXCOORD_0");
      if (uv != p.attributes.end()) {

        fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[(*uv).second],
          [&](glm::vec2 v, size_t index) {
            vertices[initial_vtx + index].uv_x = v.x;
            vertices[initial_vtx + index].uv_y = v.y;
          });
      }

      // load vertex colors
      auto colors = p.findAttribute("COLOR_0");
      if (colors != p.attributes.end()) {

        fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*colors).second],
          [&](glm::vec4 v, size_t index) {
            vertices[initial_vtx + index].color = v;
          });
      }
      newmesh.surfaces.push_back(newSurface);
    }

    // display the vertex normals
    constexpr bool OverrideColors = true;
    if (OverrideColors) {
      for (Vertex& vtx : vertices) {
        vtx.color = glm::vec4(vtx.normal, 1.f);
      }
    }
    newmesh.meshBuffers = upload(indices, vertices);
    meshes_.emplace_back(std::make_shared<MeshAsset>(std::move(newmesh)));
  }

  return true;
}

bool LavaMesh::loadCustomMesh(MeshProperties prop) {
  MeshAsset newmesh;
  GeoSurface surface = { 0,prop.index.size() };
  newmesh.meshBuffers = upload(prop.index, prop.vertex);
  newmesh.surfaces.push_back(surface);
  meshes_.emplace_back(std::make_shared<MeshAsset>(std::move(newmesh)));
  return true;
}

GPUMeshBuffers LavaMesh::upload(std::span<uint32_t> indices, std::span<Vertex> vertices) {

    const size_t vertex_buffer_size = vertices.size() * sizeof(Vertex);
    const size_t index_buffer_size = indices.size() * sizeof(uint32_t);

    GPUMeshBuffers new_surface;

    //create vertex buffer
    new_surface.vertex_buffer = engine_->createBuffer(vertex_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY);

    //find the adress of the vertex buffer
    VkBufferDeviceAddressInfo deviceAdressInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,.buffer = new_surface.vertex_buffer.buffer };
    new_surface.vertex_buffer_address = vkGetBufferDeviceAddress(engine_->device_.get_device(), &deviceAdressInfo);

    //create index buffer
    new_surface.index_buffer = engine_->createBuffer(index_buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY);

    AllocatedBuffer staging = engine_->createBuffer(vertex_buffer_size + index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void* data = staging.allocation->GetMappedData();

    // copy vertex buffer
    memcpy(data, vertices.data(), vertex_buffer_size);
    // copy index buffer
    memcpy((char*)data + vertex_buffer_size, indices.data(), index_buffer_size);

    engine_->immediate_submit([&](VkCommandBuffer cmd) {
      VkBufferCopy vertexCopy{ 0 };
      vertexCopy.dstOffset = 0;
      vertexCopy.srcOffset = 0;
      vertexCopy.size = vertex_buffer_size;

      vkCmdCopyBuffer(cmd, staging.buffer, new_surface.vertex_buffer.buffer, 1, &vertexCopy);

      VkBufferCopy indexCopy{ 0 };
      indexCopy.dstOffset = 0;
      indexCopy.srcOffset = vertex_buffer_size;
      indexCopy.size = index_buffer_size;

      vkCmdCopyBuffer(cmd, staging.buffer, new_surface.index_buffer.buffer, 1, &indexCopy);
      });

    engine_->destroyBuffer(staging);

    return new_surface;
  
}
