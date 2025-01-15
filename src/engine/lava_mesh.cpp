#include "engine/lava_mesh.hpp"

#include "lava_types.hpp"
#include <unordered_map>
#include <filesystem>

#include "lava_vulkan_inits.hpp"
#include "engine/lava_buffer.hpp"

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

#include "engine/lava_pbr_material.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


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
  /*for (auto mesh : meshes_) {
    engine_->destroyBuffer(mesh->meshBuffers.index_buffer);
    engine_->destroyBuffer(mesh->meshBuffers.vertex_buffer);
  }*/
}

/*
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
    //printf("Failed to load glTF: %d \n", fastgltf::to_underlying(load.error()));
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

*/
bool LavaMesh::loadAsGLTF(std::filesystem::path file_path) {
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
    return false;
  }

  // Vectores globales para todos los vértices e índices del archivo
  std::vector<Vertex> combinedVertices;
  std::vector<uint32_t> combinedIndices;

  MeshAsset newmesh;
  //newmesh.name = mesh.name;
  int count_surfaces = 0;
  for (fastgltf::Mesh& mesh : gltf.meshes) {
    for (auto&& p : mesh.primitives) {
      GeoSurface newSurface;
      newSurface.start_index = static_cast<uint32_t>(combinedIndices.size());
      newSurface.count = static_cast<uint32_t>(gltf.accessors[p.indicesAccessor.value()].count);

      size_t initial_vtx = combinedVertices.size();

      // Cargar índices
      {
        fastgltf::Accessor& indexAccessor = gltf.accessors[p.indicesAccessor.value()];
        combinedIndices.reserve(combinedIndices.size() + indexAccessor.count);

        fastgltf::iterateAccessor<std::uint32_t>(gltf, indexAccessor,
          [&](std::uint32_t idx) {
            combinedIndices.push_back(idx + static_cast<uint32_t>(initial_vtx));
          });
      }

      // Cargar posiciones
      {
        fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->second];
        combinedVertices.resize(combinedVertices.size() + posAccessor.count);

        fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
          [&](glm::vec3 v, size_t index) {
            Vertex newVertex;
            newVertex.position = v;
            newVertex.normal = { 1, 0, 0 };  // Por defecto
            newVertex.color = glm::vec4{ 1.f };
            newVertex.uv_x = 0;
            newVertex.uv_y = 0;
            combinedVertices[initial_vtx + index] = newVertex;
          });
      }

      // Cargar normales
      auto normals = p.findAttribute("NORMAL");
      if (normals != p.attributes.end()) {
        fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[normals->second],
          [&](glm::vec3 v, size_t index) {
            combinedVertices[initial_vtx + index].normal = v;
          });
      }

      // Cargar UVs
      auto uv = p.findAttribute("TEXCOORD_0");
      if (uv != p.attributes.end()) {
        fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[uv->second],
          [&](glm::vec2 v, size_t index) {
            combinedVertices[initial_vtx + index].uv_x = v.x;
            combinedVertices[initial_vtx + index].uv_y = v.y;
          });
      }

      // Cargar colores
      auto colors = p.findAttribute("COLOR_0");
      if (colors != p.attributes.end()) {
        fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[colors->second],
          [&](glm::vec4 v, size_t index) {
            combinedVertices[initial_vtx + index].color = v;
          });
      }

      //newmesh.surfaces[count_surfaces] = newSurface;
      //count_surfaces++;
      newmesh.index_count += newSurface.count;
    }
  }

  // Sobrescribir colores para depuración
  constexpr bool OverrideColors = true;
  if (OverrideColors) {
    for (Vertex& vtx : combinedVertices) {
      vtx.color = glm::vec4(vtx.normal, 1.f);
    }
  }



  // Subir datos combinados al GPU
  newmesh.meshBuffers = upload(combinedIndices, combinedVertices);
  newmesh.count_surfaces = count_surfaces;


  // Agregar la malla combinada al contenedor
  //meshes_.emplace_back(std::make_shared<MeshAsset>(std::move(newmesh)));
  mesh_ = std::make_shared<MeshAsset>(std::move(newmesh));

  //Update material
  //int base_color_index = -1;
  if (gltf.materials.size() > 0) { 
    if (gltf.materials[0].pbrData.baseColorTexture.has_value()) {
      int base_color_index =  gltf.materials[0].pbrData.baseColorTexture.value().textureIndex;
      material_->base_color_ = loadImage(engine_, gltf, gltf.images[base_color_index]);
    }
  }
  //material_->base_color_ = std::make_shared<LavaImage>(engine_,gltf. );

  //  pink_color_ = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
  //default_texture_image_ = std::make_shared<LavaImage>(this, (void*)&pink_color_, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM,
  //  VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
  for (fastgltf::Image& image : gltf.images) {
    std::shared_ptr<LavaImage> img = loadImage(engine_, gltf, image);

  }


  return true;
}


bool LavaMesh::loadCustomMesh(MeshProperties prop) {
  MeshAsset newmesh;
  GeoSurface surface = { 0,static_cast<uint32_t>(prop.index.size()) };
  newmesh.meshBuffers = upload(prop.index, prop.vertex);
  //newmesh.surfaces[0] = surface;
  newmesh.count_surfaces = 1;
  newmesh.index_count = surface.count;
  mesh_ = std::make_shared<MeshAsset>(std::move(newmesh));
  //meshes_.emplace_back(std::make_shared<MeshAsset>(std::move(newmesh)));
  return true;
}

GPUMeshBuffers LavaMesh::upload(std::span<uint32_t> indices, std::span<Vertex> vertices) {

    const size_t vertex_buffer_size = vertices.size() * sizeof(Vertex);
    const size_t index_buffer_size = indices.size() * sizeof(uint32_t);

    GPUMeshBuffers new_surface;

    //create vertex buffer
    new_surface.vertex_buffer = std::make_unique<LavaBuffer>(engine_->allocator_,vertex_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY);

    //find the adress of the vertex buffer
    VkBufferDeviceAddressInfo deviceAdressInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,.buffer = new_surface.vertex_buffer->buffer_.buffer };
    
    new_surface.vertex_buffer_address = vkGetBufferDeviceAddress(engine_->device_.get_device(), &deviceAdressInfo);

    //create index buffer
    new_surface.index_buffer = std::make_unique<LavaBuffer>(engine_->allocator_, index_buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY);

    LavaBuffer staging = LavaBuffer(engine_->allocator_, vertex_buffer_size + index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void* data;// = staging.get_buffer().allocation;
    vmaMapMemory(engine_->allocator_.get_allocator(), staging.get_buffer().allocation, &data);

    // copy vertex buffer
    memcpy(data, vertices.data(), vertex_buffer_size);
    // copy index buffer
    memcpy((char*)data + vertex_buffer_size, indices.data(), index_buffer_size);

    engine_->immediate_submit([&](VkCommandBuffer cmd) {
      VkBufferCopy vertexCopy{ 0 };
      vertexCopy.dstOffset = 0;
      vertexCopy.srcOffset = 0;
      vertexCopy.size = vertex_buffer_size;

      vkCmdCopyBuffer(cmd, staging.get_buffer().buffer, new_surface.vertex_buffer->get_buffer().buffer, 1, &vertexCopy);

      VkBufferCopy indexCopy{ 0 };
      indexCopy.dstOffset = 0;
      indexCopy.srcOffset = vertex_buffer_size;
      indexCopy.size = index_buffer_size;

      vkCmdCopyBuffer(cmd, staging.get_buffer().buffer, new_surface.index_buffer->get_buffer().buffer, 1, &indexCopy);
      });

    vmaUnmapMemory(engine_->allocator_.get_allocator(), staging.get_buffer().allocation);
    return new_surface;
  
}


std::shared_ptr<LavaImage> LavaMesh::loadImage(LavaEngine* engine, fastgltf::Asset& asset, fastgltf::Image& image) {
  std::shared_ptr<LavaImage> loaded_image;

  int width, height, nrChannels;

   std::visit(
        fastgltf::visitor{
            [](auto& arg) {},
            [&](fastgltf::sources::URI& filePath) {
                assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
                assert(filePath.uri.isLocalPath()); // We're only capable of loading
                // local files.

        const std::string path(filePath.uri.path().begin(),
            filePath.uri.path().end()); // Thanks C++.
        printf("loading 1\n");
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
        if (data) {
          VkExtent3D imagesize;
          imagesize.width = width;
          imagesize.height = height;
          imagesize.depth = 1;

          loaded_image = std::make_shared<LavaImage>(engine, data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, false);

          stbi_image_free(data);
        }
        },
        [&](fastgltf::sources::Vector& vector) {
            unsigned char* data = stbi_load_from_memory(vector.bytes.data(), static_cast<int>(vector.bytes.size()),
              &width, &height, &nrChannels, 4);
       printf("loading 2\n");
            if (data) {
                VkExtent3D imagesize;
                imagesize.width = width;
                imagesize.height = height;
                imagesize.depth = 1;

                loaded_image = std::make_shared<LavaImage>(engine, data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, false);

                stbi_image_free(data);
            }
        },
        [&](fastgltf::sources::BufferView& view) {
          auto& bufferView = asset.bufferViews[view.bufferViewIndex];
          auto& buffer = asset.buffers[bufferView.bufferIndex];

          std::visit(fastgltf::visitor { // We only care about VectorWithMime here, because we
          // specify LoadExternalBuffers, meaning all buffers
          // are already loaded into a vector.
          [](auto& arg) {},
          [&](fastgltf::sources::Vector& vector) {
            unsigned char* data = stbi_load_from_memory(vector.bytes.data() + bufferView.byteOffset,
              static_cast<int>(bufferView.byteLength),
              &width, &height, &nrChannels, 4);
            printf("loading 3\n");
            if (data) {
              VkExtent3D imagesize;
              imagesize.width = width;
              imagesize.height = height;
              imagesize.depth = 1;

              loaded_image = std::make_shared<LavaImage>(engine, data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, false);

              stbi_image_free(data);
            }
    } },
    buffer.data);
    },
        },
        image.data);

      // if any of the attempts to load the data failed, we havent written the image
      // so handle is null
      //if (newImage.image == VK_NULL_HANDLE) {
      //  return {};
      //}
      //else {
      //  return newImage;
      //}
      return loaded_image;
}
