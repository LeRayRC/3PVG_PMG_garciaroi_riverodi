/**
 * @page how_to_load_mesh Load a Mesh
 * 
 * 
 * @section section_load_mesh_file_extensions Supported file extensions
 * 
 * The engine supports to load meshes with the format gltf-binary (.glb files). 
 * 
 * It is possible to find multiple samples files from the official kronos @link https://github.com/KhronosGroup/glTF-Sample-Models  repository @endlink. 
 * 
 * 
 * 
 * 
 * @section section_load_mesh_code Showcase
 * 
 * Include the necessary headers 
 * 
 * @code {.hpp}
 *  #include "lava/engine/lava_engine.hpp"
 *  #include "lava/engine/lava_pbr_material.hpp"
 *  #include "lava/engine/lava_mesh.hpp"
 * @endcode
 * 
 * 1 - Create a material with its corresponding properties, 
 * you can leave the properties by default.
 * 
 * 2 - Fill the MeshProperties struct with the path to the glb file and the material that 
 * will be modified during the load process
 * 
 * 3 - Create the mesh itself, in this case it is created using a smart_pointer shared_ptr.  
 * 
 * @code {.cpp}
 * 
 * LavaEngine engine;
 * LavaPBRMaterial basic_material(engine, MaterialPBRProperties());
 * MeshProperties mesh_properties = {};
 * mesh_properties.mesh_path = "../examples/assets/Avocado.glb";
 * mesh_properties.material = &basic_material;
 *
 * std::shared_ptr<LavaMesh> mesh_ = std::make_shared<LavaMesh>(engine, mesh_properties);
 * @endcode
 * 
 * 
 */