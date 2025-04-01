#include "lava/common/lava_shapes.hpp"
#include "lava/common/lava_types.hpp"
#include "lava/engine/lava_pbr_material.hpp"
#include "PerlinNoise.hpp"


std::shared_ptr<LavaMesh> CreateQuad(LavaEngine& engine, LavaPBRMaterial* material, float size) {

  std::vector<VertexWithTangents> quad_vertices(4);

  // Posiciones de los vértices
  quad_vertices[0].position = { -size, -size, 0.0f };
  quad_vertices[1].position = { +size, -size, 0.0f };
  quad_vertices[2].position = { +size, +size, 0.0f };
  quad_vertices[3].position = { -size, +size, 0.0f };

  glm::vec3 normal = { 0.0f, 0.0f, 1.0f };

  glm::vec2 uv[4] = {
      {0.0f, 0.0f},
      {1.0f, 0.0f},
      {1.0f, 1.0f},
      {0.0f, 1.0f} 
  };

  glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

  for (int i = 0; i < 4; ++i) {
    quad_vertices[i].normal = normal;
    quad_vertices[i].color = color;
    quad_vertices[i].uv_x = uv[i].x;
    quad_vertices[i].uv_y = uv[i].y;
  }

  std::vector<uint32_t> quad_index = {
      0, 1, 3,  
      1, 2, 3   
  };

  MeshProperties mesh_properties = {};
  mesh_properties.name = "Quad Mesh";
  mesh_properties.type = MESH_CUSTOM;
  mesh_properties.material = material;
  mesh_properties.index = quad_index;
  mesh_properties.vertex = quad_vertices;

  std::shared_ptr<LavaMesh> quad_mesh = std::make_shared<LavaMesh>(engine, mesh_properties);
  return quad_mesh;
}

std::shared_ptr<LavaMesh> CreateCube24v(LavaEngine& engine, LavaPBRMaterial* material, float size) {

  
  std::vector<VertexWithTangents> cube_vertices(24);

  float cube_size = size;

  cube_vertices[0].position =  {+cube_size, +cube_size, +cube_size}; // 0
  cube_vertices[1].position =  {+cube_size, -cube_size, +cube_size}; // 1
  cube_vertices[2].position =  {-cube_size, -cube_size, +cube_size}; // 2
  cube_vertices[3].position =  {-cube_size, +cube_size, +cube_size}; // 3
  cube_vertices[4].position =  {+cube_size, +cube_size, -cube_size}; // 4
  cube_vertices[5].position =  {+cube_size, -cube_size, -cube_size}; // 5
  cube_vertices[6].position =  {-cube_size, -cube_size, -cube_size}; // 6
  cube_vertices[7].position =  {-cube_size, +cube_size, -cube_size}; // 7
  cube_vertices[8].position =  {+cube_size, +cube_size, +cube_size}; // 8  (0)
  cube_vertices[9].position =  {+cube_size, -cube_size, +cube_size}; // 9  (1)
  cube_vertices[10].position =  {+cube_size, -cube_size, -cube_size}; // 10 (5)
  cube_vertices[11].position =  {+cube_size, +cube_size, -cube_size}; // 11 (4)
  cube_vertices[12].position =  {-cube_size, -cube_size, +cube_size}; // 12 (2)
  cube_vertices[13].position =  {-cube_size, +cube_size, +cube_size}; // 13 (3)
  cube_vertices[14].position =  {-cube_size, +cube_size, -cube_size}; // 14 (7)
  cube_vertices[15].position =  {-cube_size, -cube_size, -cube_size}; // 15 (6)
  cube_vertices[16].position =  {+cube_size, +cube_size, +cube_size}; // 16 (0)
  cube_vertices[17].position =  {-cube_size, +cube_size, +cube_size}; // 17 (3)
  cube_vertices[18].position =  {-cube_size, +cube_size, -cube_size}; // 18 (7)
  cube_vertices[19].position =  {+cube_size, +cube_size, -cube_size}; // 19 (4)
  cube_vertices[20].position =  {+cube_size, -cube_size, +cube_size}; // 20 (1)
  cube_vertices[21].position =  {-cube_size, -cube_size, +cube_size}; // 21 (2)
  cube_vertices[22].position =  {-cube_size, -cube_size, -cube_size}; // 22 (6)
  cube_vertices[23].position =  {+cube_size, -cube_size, -cube_size}; // 23 (5)
  
  glm::vec3 normal[24] = {
      {+0.0f, +0.0f, +1.0f}, // 0
      {+0.0f, +0.0f, +1.0f}, // 1
      {+0.0f, +0.0f, +1.0f}, // 2
      {+0.0f, +0.0f, +1.0f}, // 3

      {+0.0f, +0.0f, -1.0f}, // 4
      {+0.0f, +0.0f, -1.0f}, // 5
      {+0.0f, +0.0f, -1.0f}, // 6
      {+0.0f, +0.0f, -1.0f}, // 7

      {+1.0f, +0.0f, +0.0f}, // 8  (0)
      {+1.0f, +0.0f, +0.0f}, // 9  (1)
      {+1.0f, +0.0f, +0.0f}, // 10 (5)
      {+1.0f, +0.0f, +0.0f}, // 11 (4)

      {-1.0f, +0.0f, +0.0f}, // 12 (2)
      {-1.0f, +0.0f, +0.0f}, // 13 (3)
      {-1.0f, +0.0f, +0.0f}, // 14 (7)
      {-1.0f, +0.0f, +0.0f}, // 15 (6)

      {+0.0f, +1.0f, +0.0f}, // 16 (0)
      {+0.0f, +1.0f, +0.0f}, // 17 (3)
      {+0.0f, +1.0f, +0.0f}, // 18 (7)
      {+0.0f, +1.0f, +0.0f}, // 19 (4)

      {+0.0f, -1.0f, +0.0f}, // 20 (1)
      {+0.0f, -1.0f, +0.0f}, // 21 (2)
      {+0.0f, -1.0f, +0.0f}, // 22 (6)
      {+0.0f, -1.0f, +0.0f}  // 23 (5)
  };
  glm::vec2 uv[24] = {
      {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f},
      {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f},
      {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f},
      {0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f},
      {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f},
      {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}
  };

  glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }; // Blanco

  // Rellenar el vector cube_vertices
  for (int i = 0; i < 24; ++i) {
    cube_vertices[i].normal = normal[i];
    cube_vertices[i].color = color;
    cube_vertices[i].uv_x = uv[i].x;
    cube_vertices[i].uv_y = uv[i].y;
  }


  std::vector<uint32_t> cube_index = {
        0, 1, 2, 2, 3, 0,    // Front face
        4, 5, 6, 6, 7, 4,    // Rear face
        8, 9, 10, 10, 11, 8, // Right face
        12, 13, 14, 14, 15, 12, // Left face
        16, 17, 18, 18, 19, 16, // Upper face
        20, 21, 22, 22, 23, 20  // Bottom face
  };


  MeshProperties mesh_properties = {};
  mesh_properties.name = "Cube Mesh";
  mesh_properties.type = MESH_CUSTOM;
  mesh_properties.material = material;
  mesh_properties.index = cube_index;
  mesh_properties.vertex = cube_vertices;

  std::shared_ptr<LavaMesh> cube_mesh = std::make_shared<LavaMesh>(engine, mesh_properties);
  return cube_mesh;
}

std::shared_ptr<LavaMesh> CreateCube8v(LavaEngine& engine, LavaPBRMaterial* material, float size) {
  float cube_size = size;

  std::vector<VertexWithTangents> cube_vertices(8); // 8 vértices para el cubo

  cube_vertices[0].position = { +cube_size, -cube_size, cube_size }; // 0
  cube_vertices[1].position = { -cube_size, -cube_size, cube_size }; // 1
  cube_vertices[2].position = { -cube_size, +cube_size, cube_size }; // 2
  cube_vertices[3].position = { +cube_size, +cube_size, cube_size }; // 3
  cube_vertices[4].position = { +cube_size, -cube_size, -cube_size }; // 4
  cube_vertices[5].position = { -cube_size, -cube_size, -cube_size }; // 5
  cube_vertices[6].position = { -cube_size, +cube_size, -cube_size }; // 6
  cube_vertices[7].position = { +cube_size, +cube_size, -cube_size }; // 7

  glm::vec3 normal[8] = {
      {+0.57f, -0.57f, +0.57f}, // 0
      {-0.57f, -0.57f, +0.57f}, // 1
      {-0.57f, +0.57f, +0.57f}, // 2
      {+0.57f, +0.57f, +0.57f}, // 3
      {+0.57f, -0.57f, -0.57f}, // 4
      {-0.57f, -0.57f, -0.57f}, // 5
      {-0.57f, +0.57f, -0.57f}, // 6
      {+0.57f, +0.57f, -0.57f}  // 7
  };

  glm::vec2 uv[8] = {
      {1.0f, 0.0f}, // 0
      {0.0f, 0.0f}, // 1
      {0.0f, 1.0f}, // 2
      {1.0f, 1.0f}, // 3
      {1.0f, 0.0f}, // 4
      {0.0f, 0.0f}, // 5
      {0.0f, 1.0f}, // 6
      {1.0f, 1.0f}  // 7
  };

  // Definir el color de los vértices del cubo
  glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }; // Blanco

  // Rellenar el vector cube_vertices con normales, colores y UVs
  for (int i = 0; i < 8; ++i) {
    cube_vertices[i].normal = normal[i];
    cube_vertices[i].color = color;
    cube_vertices[i].uv_x = uv[i].x;
    cube_vertices[i].uv_y = uv[i].y;
  }

  // Definir los índices de los triángulos del cubo (12 triángulos = 6 caras * 2 triángulos por cara)
  std::vector<uint32_t> cube_index =
  {
    // Cara frontal
    1, 0, 3,
    1, 3, 2,

    // Cara derecha
    0, 1, 5,
    5, 4, 0,

    // Cara trasera
    6, 7, 4,
    4, 5, 6,

    // Cara izquierda
    2, 3, 7,
    7, 6, 2,

    // Cara superior
    6, 5, 1,
    1, 2, 6,

    // Cara inferior
    3, 0, 4,
    4, 7, 3
  };

  // Configurar las propiedades de la malla
  MeshProperties mesh_properties = {};
  mesh_properties.name = "Cube Mesh";
  mesh_properties.type = MESH_CUSTOM;
  mesh_properties.material = material;
  mesh_properties.index = cube_index;
  mesh_properties.vertex = cube_vertices;

  // Crear y devolver la malla del cubo
  std::shared_ptr<LavaMesh> cube_mesh = std::make_shared<LavaMesh>(engine, mesh_properties);
  return cube_mesh;
}


std::shared_ptr<LavaMesh> CreateSphere(LavaEngine& engine, 
  LavaPBRMaterial* material, 
  float sphere_size, 
  int num_heights, 
  int num_revs) 
{
  int total_vertices = (num_heights + 1) * (num_revs + 1);
  std::vector<VertexWithTangents> sphere_vertices(total_vertices);
  int total_indices = num_heights * num_revs * 6;
  std::vector<uint32_t> sphere_indices(total_indices);

  float alpha = 3.14f / num_heights; // Incremento en la latitud (eje Y)
  float omega = 6.28f / num_revs;    // Incremento en la longitud (eje XZ)

  for (int i = 0; i <= num_heights; ++i) {
    for (int j = 0; j <= num_revs; ++j) {
      int index = j + i * (num_revs + 1);

      // Calcular la posición del vértice
      if (j == num_revs) {
        // Cerrar la esfera en la longitud (360° = 0°)
        sphere_vertices[index].position = {
            cosf(0) * cosf(i * alpha - 1.57f) * sphere_size,
            sinf(alpha * i - 1.57f) * sphere_size,
            sinf(0) * cosf(i * alpha - 1.57f) * sphere_size
        };
      }
      else {
        // Posición general del vértice
        sphere_vertices[index].position = {
            cosf(omega * j) * cosf(i * alpha - 1.57f) * sphere_size,
            sinf(alpha * i - 1.57f) * sphere_size,
            sinf(omega * j) * cosf(i * alpha - 1.57f) * sphere_size
        };
      }

      // Calcular las coordenadas UV
      sphere_vertices[index].uv_x = static_cast<float>(j) / num_revs;
      sphere_vertices[index].uv_y = static_cast<float>(i) / num_heights;

      // Calcular la normal (normalizada)
      sphere_vertices[index].normal = glm::normalize(sphere_vertices[index].position);
    }
  }


  for (int i = 0; i < num_heights; ++i) {
    for (int j = 0; j < num_revs; ++j) {
      int base_index = (i * 6 * num_revs) + (j * 6);

      // Primer triángulo
      sphere_indices[base_index] = ((num_revs + 1) * i) + j + 1;
      sphere_indices[base_index + 1] = ((num_revs + 1) * i) + j;
      sphere_indices[base_index + 2] = ((num_revs + 1) * i) + j + num_revs + 1;

      // Segundo triángulo
      sphere_indices[base_index + 3] = ((num_revs + 1) * i) + j + num_revs + 1;
      sphere_indices[base_index + 4] = ((num_revs + 1) * i) + j + num_revs + 2;
      sphere_indices[base_index + 5] = ((num_revs + 1) * i) + j + 1;
    }
  }

  // Crear y devolver la malla de la esfera
  MeshProperties mesh_properties = {};
  mesh_properties.name = "Sphere Mesh";
  mesh_properties.type = MESH_CUSTOM;
  mesh_properties.material = material;
  mesh_properties.vertex = sphere_vertices;
  mesh_properties.index = sphere_indices;

  // Nota: Aquí deberías generar también los índices (order) para los triángulos de la esfera.
  // Esto no está incluido en este código, pero puedes adaptarlo del código original.

  std::shared_ptr<LavaMesh> sphere_mesh = std::make_shared<LavaMesh>(engine, mesh_properties);
  return sphere_mesh;
}


std::shared_ptr<LavaMesh> CreateTerrain(
  LavaEngine& engine,
  LavaPBRMaterial* material,
  int num_cols,                // Número de columnas (por defecto: 64)
  int num_rows,                // Número de filas (por defecto: 64)
  float height_mult,        // Multiplicador de altura (por defecto: 10.0)
  float size,               // Tamaño de cada celda del terreno (por defecto: 1.0)
  float smoothness,         // Suavidad del ruido (por defecto: 0.1)
  glm::vec2 tilling, // Escala de las UVs (por defecto: {1.0, 1.0})
  bool is_centered          // Centrar el terreno en el origen (por defecto: true)
) {
  // Calcular el número total de vértices
  int total_vertices = (num_cols + 1) * (num_rows + 1);
  std::vector<VertexWithTangents> terrain_vertices(total_vertices);

  // Calcular el número total de índices (6 por cada cuadrado, 2 triángulos por cuadrado)
  int total_indices = num_cols * num_rows * 6;
  std::vector<uint32_t> terrain_indices(total_indices);

  siv::PerlinNoise pn; 
  

  // Generar los vértices, normales y UVs
  for (int i = 0; i <= num_rows; ++i) {
    for (int j = 0; j <= num_cols; ++j) {
      int index = j + i * (num_cols + 1);

      // Calcular la posición del vértice
      float x = size * j;
      float z = size * i;
      float y = (float)pn.noise2D(x * smoothness, z * smoothness) * height_mult;

      if (is_centered) {
        x -= (size * num_cols) / 2.0f;
        z -= (size * num_rows) / 2.0f;
      }

      terrain_vertices[index].position = { x, y, z };

      // Calcular las coordenadas UV
      terrain_vertices[index].uv_x = (static_cast<float>(j) / num_cols) * tilling.x;
      terrain_vertices[index].uv_y = (static_cast<float>(i) / num_rows) * tilling.y;
    }
  }

  // Calcular las normales
  for (int i = 0; i <= num_rows; ++i) {
    for (int j = 0; j <= num_cols; ++j) {
      int index = j + i * (num_cols + 1);

      // Vectores para calcular la normal
      glm::vec3 up_vector = { 0.0f, 0.0f, 0.0f };
      glm::vec3 right_vector = { 0.0f, 0.0f, 0.0f };
      glm::vec3 left_vector = { 0.0f, 0.0f, 0.0f };
      glm::vec3 bottom_vector = { 0.0f, 0.0f, 0.0f };

      // Calcular los vectores de los vecinos
      if (i < num_rows) {
        up_vector = terrain_vertices[j + (i + 1) * (num_cols + 1)].position - terrain_vertices[index].position;
      }
      if (i > 0) {
        bottom_vector = terrain_vertices[j + (i - 1) * (num_cols + 1)].position - terrain_vertices[index].position;
      }
      if (j < num_cols) {
        right_vector = terrain_vertices[(j + 1) + i * (num_cols + 1)].position - terrain_vertices[index].position;
      }
      if (j > 0) {
        left_vector = terrain_vertices[(j - 1) + i * (num_cols + 1)].position - terrain_vertices[index].position;
      }

      // Normalizar los vectores
      up_vector = glm::normalize(up_vector);
      bottom_vector = glm::normalize(bottom_vector);
      right_vector = glm::normalize(right_vector);
      left_vector = glm::normalize(left_vector);

      // Calcular las normales cruzadas
      glm::vec3 cross_right_up = glm::cross(right_vector, up_vector);
      glm::vec3 cross_up_left = glm::cross(up_vector, left_vector);
      glm::vec3 cross_left_bottom = glm::cross(left_vector, bottom_vector);
      glm::vec3 cross_bottom_right = glm::cross(bottom_vector, right_vector);

      // Sumar las normales cruzadas y normalizar el resultado
      glm::vec3 normal = glm::normalize(cross_right_up + cross_up_left + cross_left_bottom + cross_bottom_right);

      // Asignar la normal al vértice
      terrain_vertices[index].normal = -1.0f * normal;
      //terrain_vertices[index].normal = glm::vec3(0.0f,1.0f,0.0f);

    }
  }

  // Generar los índices de los triángulos
  unsigned int* order_elements_pointer = terrain_indices.data();
  for (int i = 0; i < num_rows; ++i) {
    for (int j = 0; j < num_cols; ++j) {
      int base_index = (i * 6 * num_cols) + (j * 6);

      // Primer triángulo
      order_elements_pointer[base_index] = ((num_cols + 1) * i) + j + 1;
      order_elements_pointer[base_index + 1] = ((num_cols + 1) * i) + j;
      order_elements_pointer[base_index + 2] = ((num_cols + 1) * i) + j + num_cols + 1;

      // Segundo triángulo
      order_elements_pointer[base_index + 3] = ((num_cols + 1) * i) + j + num_cols + 1;
      order_elements_pointer[base_index + 4] = ((num_cols + 1) * i) + j + num_cols + 2;
      order_elements_pointer[base_index + 5] = ((num_cols + 1) * i) + j + 1;
    }
  }

  // Configurar las propiedades de la malla
  MeshProperties mesh_properties = {};
  mesh_properties.name = "Terrain Mesh";
  mesh_properties.type = MESH_CUSTOM;
  mesh_properties.material = material;
  mesh_properties.vertex = terrain_vertices;
  mesh_properties.index = terrain_indices;

  // Crear y devolver la malla del terreno
  std::shared_ptr<LavaMesh> terrain_mesh = std::make_shared<LavaMesh>(engine, mesh_properties);
  return terrain_mesh;
}