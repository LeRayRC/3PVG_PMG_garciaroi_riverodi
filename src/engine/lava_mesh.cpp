#include "lava/engine/lava_mesh.hpp"

#include "lava/common/lava_types.hpp"
#include <unordered_map>
#include <filesystem>

#include "lava_vulkan_inits.hpp"
#include "lava/engine/lava_buffer.hpp"

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/util.hpp>

#include "lava/engine/lava_pbr_material.hpp"
#include "engine/lava_allocator.hpp"
#include "lava/engine/lava_image.hpp"

#include "lava/vr/lava_engine_vr.hpp"
#include "lava/engine/lava_pbr_material.hpp"

#include "stb_image.h"



LavaMesh::LavaMesh(LavaEngine& engine, MeshProperties prop){
  name_ = prop.name;
  engine_ = &engine;
  engine_vr_ = nullptr;
  material_ = nullptr;
  type_ = prop.type;
  material_ = prop.material;
	switch (type_)
	{
	case MESH_FBX:
		break;
	case MESH_GLTF:
        //NEED FIX: SHOULD ALSO BE POSIBLE TO CALL WITHOUT TANGENTS (ONLY VERTEX)
    loadAsGLTFWithNodes<VertexWithTangents>(prop.mesh_path);
    material_->UpdateDescriptorSet();
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

LavaMesh::LavaMesh(LavaEngineVR& engine, MeshProperties prop) {
  name_ = prop.name;
  engine_ = nullptr;
  engine_vr_ = &engine;
  material_ = nullptr;
  type_ = prop.type;
  material_ = prop.material;
  switch (type_)
  {
  case MESH_FBX:
    break;
  case MESH_GLTF:
    //NEED FIX: SHOULD ALSO BE POSIBLE TO CALL WITHOUT TANGENTS (ONLY VERTEX)
    loadAsGLTF<VertexWithTangents>(prop.mesh_path);
    material_->UpdateDescriptorSet();
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
}

template<typename t>
bool LavaMesh::loadAsGLTFWithNodes(std::filesystem::path file_path) {
  std::cout << "Loading GLTF: " << file_path << std::endl;

  const auto root_path = file_path.parent_path();

  fastgltf::GltfDataBuffer data;
  data.loadFromFile(file_path);

  auto extension = file_path.extension().string();
  std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

  constexpr auto gltfOptions = fastgltf::Options::LoadGLBBuffers
    | fastgltf::Options::LoadExternalBuffers;// | fastgltf::Options::LoadExternalImages;

  fastgltf::Asset gltf;
  fastgltf::Parser parser{};

  if (extension == ".glb") {
    auto result = parser.loadGltfBinary(&data, file_path.parent_path(), gltfOptions);
    if (result) {
      gltf = std::move(result.get());
    }
  }
  else if (extension == ".gltf") {
    auto result = parser.loadGltf(&data, file_path.parent_path(), gltfOptions);
    if (result) {
      gltf = std::move(result.get());
    }
  }
  else {
    std::cerr << "File format no supported, engine supports gltf or glb: " << extension << std::endl;
    return false;
  }



  std::vector<t> combinedVertices;
  std::vector<uint32_t> combinedIndices;
  std::vector<GeoSurface> surfaces;

  MeshAsset newmesh;
  int count_surfaces = 0;

  materials_.clear();

  if (!gltf.materials.empty()) {
    for (size_t i = 0; i < gltf.materials.size(); ++i) {
      const fastgltf::Material& gltfMat = gltf.materials[i];

      // Crear un nuevo material para cada material GLTF
      std::shared_ptr<LavaPBRMaterial> mat;
      if (engine_) {
        mat = std::make_shared<LavaPBRMaterial>(*engine_, MaterialPBRProperties()/* o *engine_vr_ según corresponda */);
      }
      else {
        mat = std::make_shared<LavaPBRMaterial>(*engine_vr_, MaterialPBRProperties() /* o *engine_vr_ según corresponda */);

      }

      // Configurar propiedades básicas
      mat->uniform_properties.metallic_factor_ = gltfMat.pbrData.metallicFactor;
      mat->uniform_properties.roughness_factor_ = gltfMat.pbrData.roughnessFactor;
      mat->uniform_properties.opacity_mask_ = gltfMat.alphaCutoff;

      if (gltfMat.specular.get() != nullptr) {
        mat->uniform_properties.specular_factor_ = gltfMat.specular->specularFactor;
      }

      // Cargar texturas
      if (gltfMat.pbrData.baseColorTexture.has_value()) {
        int base_color_index = (int)gltfMat.pbrData.baseColorTexture.value().textureIndex;
        mat->base_color_ = loadImage(gltf, gltf.images[base_color_index], root_path);
      }
      else {
        if (engine_) {
          mat->base_color_ = engine_->default_texture_image_black;
        }
        else {
          mat->base_color_ = engine_vr_->default_texture_image_black;
        }
      }

      if (gltfMat.normalTexture.has_value()) {
        int normal_index = (int)gltfMat.normalTexture.value().textureIndex;
        mat->normal_ = loadImage(gltf, gltf.images[normal_index], root_path);
        mat->uniform_properties.use_normal_ = 1.0f;
      }

      if (gltfMat.pbrData.metallicRoughnessTexture.has_value()) {
        int mt_rg_index = (int)gltfMat.pbrData.metallicRoughnessTexture.value().textureIndex;
        mat->metallic_roughness_ = loadImage(gltf, gltf.images[mt_rg_index], root_path);
      }

      // Actualizar descriptor set para el material
      mat->UpdateDescriptorSet();

      // Agregar a la lista de materiales
      materials_.push_back(mat);
    }
  }



  // Iniciar el procesamiento desde la escena raíz
  if (!gltf.scenes.empty()) {
    size_t sceneIndex = 0;
    // Usa la escena por defecto si está disponible
    if (gltf.defaultScene.has_value()) {
      sceneIndex = gltf.defaultScene.value();
    }

    const fastgltf::Scene& scene = gltf.scenes[sceneIndex];

    // Matriz de identidad como transformación inicial
    glm::mat4 rootTransform(1.0f);

    // Procesar todos los nodos de la escena raíz
    for (const auto& nodeIndex : scene.nodeIndices) {
      const fastgltf::Node& node = gltf.nodes[nodeIndex];
      processNode(gltf, node, rootTransform, combinedVertices, combinedIndices,
        surfaces,newmesh.index_count, newmesh.count_surfaces);
      
    }

    newmesh.surfaces = surfaces;
  }

  // Subir datos combinados al GPU
  if (engine_) {
    newmesh.meshBuffers = upload<t>(engine_, combinedIndices, combinedVertices);
  }
  else {
    newmesh.meshBuffers = upload<t>(engine_vr_, combinedIndices, combinedVertices);

  }
  newmesh.count_surfaces = count_surfaces;

  // Agregar la malla combinada al contenedor
  //meshes_.emplace_back(std::make_shared<MeshAsset>(std::move(newmesh)));
  mesh_ = std::make_shared<MeshAsset>(std::move(newmesh));

  //Update material
  //int base_color_index = -1;

  /*
  if (gltf.materials.size() > 0) {
    material_->uniform_properties.metallic_factor_ = gltf.materials[0].pbrData.metallicFactor;
    material_->uniform_properties.roughness_factor_ = gltf.materials[0].pbrData.roughnessFactor;
    material_->uniform_properties.opacity_mask_ = gltf.materials[0].alphaCutoff;

    if (gltf.materials[0].specular.get() != nullptr) {
      material_->uniform_properties.specular_factor_ = gltf.materials[0].specular->specularFactor;
    }
    if (gltf.materials[0].pbrData.baseColorTexture.has_value()) {
      int base_color_index = (int)gltf.materials[0].pbrData.baseColorTexture.value().textureIndex;
      material_->base_color_ = loadImage(gltf, gltf.images[base_color_index], root_path);
    }


    if (gltf.materials[0].normalTexture.has_value()) {
      int normal_index = (int)gltf.materials[0].normalTexture.value().textureIndex;
      material_->normal_ = loadImage(gltf, gltf.images[normal_index], root_path);
      material_->uniform_properties.use_normal_ = 1.0f;
      constexpr bool calc_tangents = sizeof(t) == sizeof(VertexWithTangents);
      if (calc_tangents) {

        glm::vec3 edge1;
        glm::vec3 edge2;
        glm::vec2 deltaUV1;
        glm::vec2 deltaUV2;
        glm::vec3 tangent;
        glm::vec3 bitangent;
        for (int i = 0; i < combinedIndices.size() - 2; i += 3) {
          //Calculate Tangent an Bitangent
          edge1 = combinedVertices[combinedIndices[i + 1]].position - combinedVertices[combinedIndices[i]].position;
          edge2 = combinedVertices[combinedIndices[i + 2]].position - combinedVertices[combinedIndices[i]].position;
          deltaUV1.x = combinedVertices[combinedIndices[i + 1]].uv_x - combinedVertices[combinedIndices[i]].uv_x;
          deltaUV1.y = combinedVertices[combinedIndices[i + 1]].uv_y - combinedVertices[combinedIndices[i]].uv_y;
          deltaUV2.x = combinedVertices[combinedIndices[i + 2]].uv_x - combinedVertices[combinedIndices[i]].uv_x;;
          deltaUV2.y = combinedVertices[combinedIndices[i + 1]].uv_y - combinedVertices[combinedIndices[i]].uv_y;

          float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

          tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
          tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
          tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

          bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
          bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
          bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

          char* vertex_pointer = (char*)(&combinedVertices[combinedIndices[i]]); // collect pointer to first vertex

          glm::vec3* tangent_ptr = (glm::vec3*)(vertex_pointer + offsetof(VertexWithTangents, tangent_));
          glm::vec3* bitangent_ptr = (glm::vec3*)(vertex_pointer + offsetof(VertexWithTangents, bitangent_));
          *tangent_ptr = tangent;
          *bitangent_ptr = bitangent;

          vertex_pointer = (char*)(&combinedVertices[combinedIndices[i + 1]]); // collect pointer to second vertex

          tangent_ptr = (glm::vec3*)(vertex_pointer + offsetof(VertexWithTangents, tangent_));
          bitangent_ptr = (glm::vec3*)(vertex_pointer + offsetof(VertexWithTangents, bitangent_));
          *tangent_ptr = tangent;
          *bitangent_ptr = bitangent;

          vertex_pointer = (char*)(&combinedVertices[combinedIndices[i + 2]]); // collect pointer to third vertex

          tangent_ptr = (glm::vec3*)(vertex_pointer + offsetof(VertexWithTangents, tangent_));
          bitangent_ptr = (glm::vec3*)(vertex_pointer + offsetof(VertexWithTangents, bitangent_));
          *tangent_ptr = tangent;
          *bitangent_ptr = bitangent;
        }
      }
    }

    if (gltf.materials[0].pbrData.metallicRoughnessTexture.has_value()) {
      int mt_rg_index = (int)gltf.materials[0].pbrData.metallicRoughnessTexture.value().textureIndex;
      material_->metallic_roughness_ = loadImage(gltf, gltf.images[mt_rg_index], root_path);
    }
  }

  */
  return true;
}


template<typename t>
bool LavaMesh::loadAsGLTF(std::filesystem::path file_path) {
  std::cout << "Loading GLTF: " << file_path << std::endl;

  const auto root_path = file_path.parent_path();

  fastgltf::GltfDataBuffer data;
  data.loadFromFile(file_path);

  auto extension = file_path.extension().string();
  std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

  constexpr auto gltfOptions = fastgltf::Options::LoadGLBBuffers
    | fastgltf::Options::LoadExternalBuffers;// | fastgltf::Options::LoadExternalImages;

  fastgltf::Asset gltf;
  fastgltf::Parser parser{};

  if (extension == ".glb") {
    auto result = parser.loadGltfBinary(&data, file_path.parent_path(), gltfOptions);
    if (result) {
      gltf = std::move(result.get());
    }
  }
  else if (extension == ".gltf") {
    auto result = parser.loadGltf(&data, file_path.parent_path(), gltfOptions);
    if (result) {
      gltf = std::move(result.get());
    }
  }
  else {
    std::cerr << "File format no supported, engine supports gltf or glb: " << extension << std::endl;
    return false;
  }

 

  // Vectores globales para todos los v�rtices e �ndices del archivo
  std::vector<t> combinedVertices;
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

      // Cargar �ndices
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
            t newVertex;
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

  // Sobrescribir colores para depuraci�n
  constexpr bool OverrideColors = true;
  if (OverrideColors) {
    for (t& vtx : combinedVertices) {
      vtx.color = glm::vec4(vtx.normal, 1.f);
    }
  }



  // Subir datos combinados al GPU
  if (engine_) {
    newmesh.meshBuffers = upload<t>(engine_,combinedIndices, combinedVertices);

  }
  else {
    newmesh.meshBuffers = upload<t>(engine_vr_,combinedIndices, combinedVertices);

  }
  newmesh.count_surfaces = count_surfaces;

  // Agregar la malla combinada al contenedor
  //meshes_.emplace_back(std::make_shared<MeshAsset>(std::move(newmesh)));
  mesh_ = std::make_shared<MeshAsset>(std::move(newmesh));

  //Update material
  //int base_color_index = -1;
  if (gltf.materials.size() > 0) {
    material_->uniform_properties.metallic_factor_ = gltf.materials[0].pbrData.metallicFactor;
    material_->uniform_properties.roughness_factor_ = gltf.materials[0].pbrData.roughnessFactor;
    material_->uniform_properties.opacity_mask_ = gltf.materials[0].alphaCutoff;
    
    if (gltf.materials[0].specular.get() != nullptr) {
        material_->uniform_properties.specular_factor_ = gltf.materials[0].specular->specularFactor;
    }
    if (gltf.materials[0].pbrData.baseColorTexture.has_value()) {
        int base_color_index = (int)gltf.materials[0].pbrData.baseColorTexture.value().textureIndex;
        material_->base_color_ = loadImage(gltf, gltf.images[base_color_index], root_path);
    }
    
    if (gltf.materials[0].normalTexture.has_value()) {
        int normal_index = (int)gltf.materials[0].normalTexture.value().textureIndex;
        material_->normal_ = loadImage(gltf, gltf.images[normal_index], root_path);
        material_->uniform_properties.use_normal_ = 1.0f;
        constexpr bool calc_tangents = sizeof(t) == sizeof(VertexWithTangents);
        if (calc_tangents) {

            glm::vec3 edge1;
            glm::vec3 edge2;
            glm::vec2 deltaUV1;
            glm::vec2 deltaUV2;
            glm::vec3 tangent;
            glm::vec3 bitangent;
            for (int i = 0; i < combinedIndices.size() - 2; i += 3) {
                //Calculate Tangent an Bitangent
                edge1 = combinedVertices[combinedIndices[i+1]].position - combinedVertices[combinedIndices[i]].position;
                edge2 = combinedVertices[combinedIndices[i + 2]].position - combinedVertices[combinedIndices[i]].position;
                deltaUV1.x = combinedVertices[combinedIndices[i + 1]].uv_x - combinedVertices[combinedIndices[i]].uv_x;
                deltaUV1.y = combinedVertices[combinedIndices[i + 1]].uv_y - combinedVertices[combinedIndices[i]].uv_y;
                deltaUV2.x = combinedVertices[combinedIndices[i + 2]].uv_x - combinedVertices[combinedIndices[i]].uv_x;;
                deltaUV2.y = combinedVertices[combinedIndices[i + 1]].uv_y - combinedVertices[combinedIndices[i]].uv_y;

                float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

                tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

                bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

                char* vertex_pointer = (char*)(&combinedVertices[combinedIndices[i]]); // collect pointer to first vertex

                glm::vec3* tangent_ptr = (glm::vec3*)(vertex_pointer + offsetof(VertexWithTangents, tangent_));
                glm::vec3* bitangent_ptr = (glm::vec3*)(vertex_pointer + offsetof(VertexWithTangents, bitangent_));
                *tangent_ptr = tangent;
                *bitangent_ptr = bitangent;

                vertex_pointer = (char*)(&combinedVertices[combinedIndices[i + 1]]); // collect pointer to second vertex

                tangent_ptr = (glm::vec3*)(vertex_pointer + offsetof(VertexWithTangents, tangent_));
                bitangent_ptr = (glm::vec3*)(vertex_pointer + offsetof(VertexWithTangents, bitangent_));
                *tangent_ptr = tangent;
                *bitangent_ptr = bitangent;

                vertex_pointer = (char*)(&combinedVertices[combinedIndices[i + 2]]); // collect pointer to third vertex

                tangent_ptr = (glm::vec3*)(vertex_pointer + offsetof(VertexWithTangents, tangent_));
                bitangent_ptr = (glm::vec3*)(vertex_pointer + offsetof(VertexWithTangents, bitangent_));
                *tangent_ptr = tangent;
                *bitangent_ptr = bitangent;
            }
        }
    }

    if (gltf.materials[0].pbrData.metallicRoughnessTexture.has_value()) {
        int mt_rg_index = (int)gltf.materials[0].pbrData.metallicRoughnessTexture.value().textureIndex;
        material_->metallic_roughness_ = loadImage(gltf, gltf.images[mt_rg_index], root_path);
    }
  }

  return true;
}


bool LavaMesh::loadCustomMesh(MeshProperties prop) {
  MeshAsset newmesh;
  GeoSurface surface = { 0,static_cast<uint32_t>(prop.index.size()) };
  
  if (engine_) {
    newmesh.meshBuffers = upload<VertexWithTangents>(engine_,prop.index, prop.vertex);
  }
  else {
    newmesh.meshBuffers = upload<VertexWithTangents>(engine_vr_,prop.index, prop.vertex);
  }
  //newmesh.surfaces[0] = surface;
  newmesh.count_surfaces = 1;
  newmesh.index_count = surface.count;
  mesh_ = std::make_shared<MeshAsset>(std::move(newmesh));
  //meshes_.emplace_back(std::make_shared<MeshAsset>(std::move(newmesh)));
  return true;
}

template<typename t>
GPUMeshBuffers LavaMesh::upload(LavaEngine* engine, std::span<uint32_t> indices, std::span<t> vertices) {

    const size_t vertex_buffer_size = vertices.size() * sizeof(t);
    const size_t index_buffer_size = indices.size() * sizeof(uint32_t);

    GPUMeshBuffers new_surface;

    //create vertex buffer
    new_surface.vertex_buffer = std::make_unique<LavaBuffer>(*engine->allocator_,vertex_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY);

    //find the adress of the vertex buffer
    VkBufferDeviceAddressInfo deviceAdressInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,.buffer = new_surface.vertex_buffer->buffer_.buffer };
    
    new_surface.vertex_buffer_address = vkGetBufferDeviceAddress(engine->device_->get_device(), &deviceAdressInfo);

    //create index buffer
    new_surface.index_buffer = std::make_unique<LavaBuffer>(*engine->allocator_, index_buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY);

    LavaBuffer staging = LavaBuffer(*engine->allocator_, vertex_buffer_size + index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void* data;// = staging.get_buffer().allocation;
    vmaMapMemory(engine->allocator_->get_allocator(), staging.get_buffer().allocation, &data);

    // copy vertex buffer
    memcpy(data, vertices.data(), vertex_buffer_size);
    // copy index buffer
    memcpy((char*)data + vertex_buffer_size, indices.data(), index_buffer_size);

    engine->immediate_submit([&](VkCommandBuffer cmd) {
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

    vmaUnmapMemory(engine->allocator_->get_allocator(), staging.get_buffer().allocation);
    return new_surface;
  
}

template<typename t>
GPUMeshBuffers LavaMesh::upload(LavaEngineVR* engine, std::span<uint32_t> indices, std::span<t> vertices) {

  const size_t vertex_buffer_size = vertices.size() * sizeof(t);
  const size_t index_buffer_size = indices.size() * sizeof(uint32_t);

  GPUMeshBuffers new_surface;

  //create vertex buffer
  new_surface.vertex_buffer = std::make_unique<LavaBuffer>(*engine->allocator_, vertex_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VMA_MEMORY_USAGE_GPU_ONLY);

  //find the adress of the vertex buffer
  VkBufferDeviceAddressInfo deviceAdressInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,.buffer = new_surface.vertex_buffer->buffer_.buffer };

  new_surface.vertex_buffer_address = vkGetBufferDeviceAddress(engine->device_->get_device(), &deviceAdressInfo);

  //create index buffer
  new_surface.index_buffer = std::make_unique<LavaBuffer>(*engine->allocator_, index_buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VMA_MEMORY_USAGE_GPU_ONLY);

  LavaBuffer staging = LavaBuffer(*engine->allocator_, vertex_buffer_size + index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

  void* data;// = staging.get_buffer().allocation;
  vmaMapMemory(engine->allocator_->get_allocator(), staging.get_buffer().allocation, &data);

  // copy vertex buffer
  memcpy(data, vertices.data(), vertex_buffer_size);
  // copy index buffer
  memcpy((char*)data + vertex_buffer_size, indices.data(), index_buffer_size);

  engine->immediate_submit([&](VkCommandBuffer cmd) {
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

  vmaUnmapMemory(engine->allocator_->get_allocator(), staging.get_buffer().allocation);
  return new_surface;

}


std::shared_ptr<LavaImage> LavaMesh::loadImage(fastgltf::Asset& asset, fastgltf::Image& image, std::filesystem::path root_path) {
  std::shared_ptr<LavaImage> loaded_image;

  int width, height, nrChannels;

  std::visit(
      fastgltf::visitor{
          [](auto &arg) {},
          [&](fastgltf::sources::URI &filePath) {
            assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
            assert(filePath.uri.isLocalPath());   // We're only capable of loading
                                                  // local files.

            const std::string path(filePath.uri.path().begin(),
                                   filePath.uri.path().end()); // Thanks C++.

            std::filesystem::path full_path = root_path / std::filesystem::path(filePath.uri.path().begin(), filePath.uri.path().end());

            std::string path_str = full_path.string();
            
            unsigned char *data = stbi_load(path_str.c_str(), &width, &height, &nrChannels, 4);
            if (data) {
              VkExtent3D imagesize;
              imagesize.width = width;
              imagesize.height = height;
              imagesize.depth = 1;

              if (engine_) {
                loaded_image = std::make_shared<LavaImage>(engine_, data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, false);
              }
              else {
                loaded_image = std::make_shared<LavaImage>(engine_vr_, data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, false);
              }

              stbi_image_free(data);
            }
          },
          [&](fastgltf::sources::Vector &vector) {
            unsigned char *data = stbi_load_from_memory(vector.bytes.data(), static_cast<int>(vector.bytes.size()),
                                                        &width, &height, &nrChannels, 4);
            printf("loading 2\n");
            if (data) {
              VkExtent3D imagesize;
              imagesize.width = width;
              imagesize.height = height;
              imagesize.depth = 1;

              if (engine_) {
                loaded_image = std::make_shared<LavaImage>(engine_, data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, false);
              }
              else {
                loaded_image = std::make_shared<LavaImage>(engine_vr_, data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, false);
              }

              stbi_image_free(data);
            }
          },
          [&](fastgltf::sources::BufferView &view) {
            auto &bufferView = asset.bufferViews[view.bufferViewIndex];
            auto &buffer = asset.buffers[bufferView.bufferIndex];

            std::visit(fastgltf::visitor{// We only care about VectorWithMime here, because we
                                         // specify LoadExternalBuffers, meaning all buffers
                                         // are already loaded into a vector.
                                         [](auto &arg) {},
                                         [&](fastgltf::sources::Array &vector) {
                                           unsigned char *data = stbi_load_from_memory(vector.bytes.data() + bufferView.byteOffset,
                                                                                       static_cast<int>(bufferView.byteLength),
                                                                                       &width, &height, &nrChannels, 4);
                                           if (data) {
                                             VkExtent3D imagesize;
                                             imagesize.width = width;
                                             imagesize.height = height;
                                             imagesize.depth = 1;
                                             if (engine_) {
                                              loaded_image = std::make_shared<LavaImage>(engine_, data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, false);
                                             }
                                             else {
                                               loaded_image = std::make_shared<LavaImage>(engine_vr_, data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, false);
                                             }
                                             stbi_image_free(data);
                                           }
                                         }},
                       buffer.data);
          },
      },
      image.data);
  return loaded_image;
}


template<typename t>
void LavaMesh::processNode(const fastgltf::Asset& gltf,
  const fastgltf::Node& node,
  const glm::mat4& parentTransform,
  std::vector<t>& combinedVertices,
  std::vector<uint32_t>& combinedIndices,
  std::vector<GeoSurface>& surfaces,        // NUEVO: parámetro para almacenar las superficies
  uint32_t& index_count,
  uint16_t& count_surfaces) {

  // Calcular la transformación de este nodo
  glm::mat4 nodeTransform = parentTransform;
  printf("%s\n", node.name.c_str());
  // Verificar qué tipo de transformación está presente
  if (std::holds_alternative<fastgltf::Node::TransformMatrix>(node.transform)) {
    const auto& matrix = std::get<fastgltf::Node::TransformMatrix>(node.transform);
    glm::mat4 localTransform(
      matrix[0], matrix[1], matrix[2], matrix[3],
      matrix[4], matrix[5], matrix[6], matrix[7],
      matrix[8], matrix[9], matrix[10], matrix[11],
      matrix[12], matrix[13], matrix[14], matrix[15]
    );
    nodeTransform = nodeTransform * localTransform;
  }
  else if (std::holds_alternative<fastgltf::TRS>(node.transform)) {
    const auto& trs = std::get<fastgltf::TRS>(node.transform);

    glm::vec3 translation(trs.translation[0], trs.translation[1], trs.translation[2]);
    glm::quat rotation(trs.rotation[3], trs.rotation[0], trs.rotation[1], trs.rotation[2]); // w, x, y, z
    glm::vec3 scale(trs.scale[0], trs.scale[1], trs.scale[2]);

    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);
    glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

    glm::mat4 localTransform = translationMatrix * rotationMatrix * scaleMatrix;
    nodeTransform = nodeTransform * localTransform;
  }

  // Procesar la malla asociada al nodo actual, si existe
  if (node.meshIndex.has_value()) {
    const fastgltf::Mesh& mesh = gltf.meshes[node.meshIndex.value()];
    size_t initial_vtx = combinedVertices.size();

    for (const auto& p : mesh.primitives) {
      GeoSurface newSurface;
      newSurface.start_index = static_cast<uint32_t>(combinedIndices.size());
      newSurface.count = static_cast<uint32_t>(gltf.accessors[p.indicesAccessor.value()].count);

      // NUEVO: Asignar el material correspondiente a la superficie
      if (p.materialIndex.has_value()) {
        newSurface.material_index = static_cast<uint32_t>(p.materialIndex.value());
      }
      else {
        newSurface.material_index = 0; // Material por defecto si no se especifica
      }

      // NUEVO: Añadir la superficie al vector de superficies
      surfaces.push_back(newSurface);

      // Actualizar el contador de índices
      index_count += newSurface.count;

      // Cargar índices
      {
        const fastgltf::Accessor& indexAccessor = gltf.accessors[p.indicesAccessor.value()];
        combinedIndices.reserve(combinedIndices.size() + indexAccessor.count);

        fastgltf::iterateAccessor<std::uint32_t>(gltf, indexAccessor,
          [&](std::uint32_t idx) {
            combinedIndices.push_back(idx + static_cast<uint32_t>(initial_vtx));
          });
      }

      // Cargar posiciones
      {
        auto posIt = p.findAttribute("POSITION");
        if (posIt != p.attributes.end()) {
          const fastgltf::Accessor& posAccessor = gltf.accessors[posIt->second];
          size_t start_index = combinedVertices.size();
          combinedVertices.resize(combinedVertices.size() + posAccessor.count);

          fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
            [&](glm::vec3 v, size_t index) {
              t newVertex;

              // Aplicar la transformación del nodo a la posición
              glm::vec4 transformedPos = nodeTransform * glm::vec4(v, 1.0f);
              newVertex.position = glm::vec3(transformedPos);

              newVertex.normal = { 1, 0, 0 };  // Por defecto
              newVertex.color = glm::vec4{ 1.f };
              newVertex.uv_x = 0;
              newVertex.uv_y = 0;
              combinedVertices[start_index + index] = newVertex;
            });
        }
      }

      // Cargar normales
      auto normals = p.findAttribute("NORMAL");
      if (normals != p.attributes.end()) {
        const fastgltf::Accessor& normalAccessor = gltf.accessors[normals->second];

        fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, normalAccessor,
          [&](glm::vec3 v, size_t index) {
            // Transformar la normal con la matriz de rotación (ignorando escala y traslación)
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(nodeTransform)));
            glm::vec3 transformedNormal = normalMatrix * v;
            combinedVertices[initial_vtx + index].normal = glm::normalize(transformedNormal);
          });
      }

      // Cargar UVs
      auto uv = p.findAttribute("TEXCOORD_0");
      if (uv != p.attributes.end()) {
        const fastgltf::Accessor& uvAccessor = gltf.accessors[uv->second];

        fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, uvAccessor,
          [&](glm::vec2 v, size_t index) {
            combinedVertices[initial_vtx + index].uv_x = v.x;
            combinedVertices[initial_vtx + index].uv_y = v.y;
          });
      }

      // Cargar colores
      auto colors = p.findAttribute("COLOR_0");
      if (colors != p.attributes.end()) {
        const fastgltf::Accessor& colorAccessor = gltf.accessors[colors->second];

        fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, colorAccessor,
          [&](glm::vec4 v, size_t index) {
            combinedVertices[initial_vtx + index].color = v;
          });
      }

      // Incrementar contador de superficies
      count_surfaces++;
    }
  }

  // Procesar recursivamente los nodos hijos
  for (const auto& childIndex : node.children) {
    const fastgltf::Node& childNode = gltf.nodes[childIndex];
    processNode(gltf, childNode, nodeTransform, combinedVertices, combinedIndices, surfaces, index_count, count_surfaces);
  }
}
