/**
 * @file lava_gaussian_splat.cpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Gaussian Splat's file
 * @version 0.1
 * @date 2024-10-01
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 **/

#include <algorithm>
#include <fstream>

#include "lava/gaussian/lava_gaussian_splat.hpp"
#include "lava/gaussian/lava_ply.hpp"
#include "lava/engine/lava_engine.hpp"
#include "lava/engine/lava_buffer.hpp"
#include "engine/lava_device.hpp"
#include "engine/lava_allocator.hpp"
#include "lava/jobsystem/lava_job_system.hpp"

struct GaussianData
{
    GaussianData() noexcept {}
    float posWithAlpha[4]; // center of the gaussian in object coordinates, with alpha in w
    float r_sh0[4]; // sh coeff for red channel (up to third-order)
    float g_sh0[4]; // sh coeff for green channel
    float b_sh0[4];  // sh coeff for blue channel
    float cov3_col0[3]; // 3x3 covariance matrix of the splat in object coordinates.
    int32_t padding1; //Vulkan PADDING
    float cov3_col1[3];
    int32_t padding2; //Vulkan PADDING
    float cov3_col2[3];
    int32_t padding3; //Vulkan PADDING
};

struct threadData {
    uint32_t* index;
    glm::vec3 refPos;
    glm::vec4* vecPos;
};

static float ComputeAlphaFromOpacity(float opacity)
{
    return 1.0f / (1.0f + expf(-opacity));
}

static glm::mat3 ComputeCovMatFromRotScale(float rot[4], float scale[3])
{
    glm::quat q(rot[0], rot[1], rot[2], rot[3]);
    glm::mat3 R(glm::normalize(q));
    glm::mat3 S(glm::vec3(scale[0], 0.0f, 0.0f),
        glm::vec3(0.0f, scale[1], 0.0f),
        glm::vec3(0.0f, 0.0f, scale[2]));
    return R * S * glm::transpose(S) * glm::transpose(R);
}

static inline int32_t cpuSort(const uint32_t* a, glm::vec4* vecPos, const uint32_t* b, const glm::vec3& refPos) {
    uint32_t bint = *(b);
    uint32_t aint = *(a);

    glm::vec3 point_a(vecPos[aint]);
    glm::vec3 point_b(vecPos[bint]);

    float aDist = glm::distance2(point_a, refPos);
    float bDist = glm::distance2(point_b, refPos);

    if (aDist > bDist) return -1;

    return 1;
}

static inline int32_t partition(uint32_t* array, glm::vec4* vecPos, int32_t low, int32_t high, const glm::vec3& refPos) {
    uint32_t* pivot = &array[high];
    int32_t i = (low - 1);
    for (int32_t j = low; j <= high - 1; j++) {
        if (cpuSort(pivot, vecPos, &array[j], refPos) > 0) {
            i++;
            uint32_t temp_data = array[i];
            array[i] = array[j];
            array[j] = temp_data;
        }
    }
    uint32_t temp_data = array[i + 1];
    array[i + 1] = array[high];
    array[high] = temp_data;
    return (i + 1);
}

static inline void quick_sort(uint32_t* array, glm::vec4* vecPos, int32_t low, int32_t high, const glm::vec3& refPos) {
    if (low < high) {
        int32_t pi = partition(array, vecPos, low, high, refPos);
        quick_sort(array, vecPos, low, pi - 1, refPos);
        quick_sort(array, vecPos, pi + 1, high, refPos);
    }
}

LavaGaussianSplat::LavaGaussianSplat()
{
	numGaussians = 0;
	gaussianSize = 0;
}

LavaGaussianSplat::~LavaGaussianSplat()
{
	data.clear();
}

bool LavaGaussianSplat::importPly(LavaEngine* engine, const std::string& plyFilename)
{
    std::ifstream plyFile(plyFilename, std::ios::binary);
    if (!plyFile.is_open())
    {
        printf("failed to open %s\n", plyFilename.c_str());
        return false;
    }

    LavaPly ply;

    {
        if (!ply.Parse(plyFile))
        {
            printf("Error parsing ply file \"%s\"\n", plyFilename.c_str());
            return false;
        }
    }

    struct
    {
        LavaPly::BinaryAttrib x, y, z;
        LavaPly::BinaryAttrib f_dc[3];
        LavaPly::BinaryAttrib f_rest[45];
        LavaPly::BinaryAttrib opacity;
        LavaPly::BinaryAttrib scale[3];
        LavaPly::BinaryAttrib rot[4];
    } props;

    {

        if (!ply.GetProperty("x", props.x) ||
            !ply.GetProperty("y", props.y) ||
            !ply.GetProperty("z", props.z))
        {
            printf("Error parsing ply file \"%s\", missing position property\n", plyFilename.c_str());
        }

        for (int i = 0; i < 3; i++)
        {
            if (!ply.GetProperty("f_dc_" + std::to_string(i), props.f_dc[i]))
            {
                printf("Error parsing ply file \"%s\", missing f_dc property\n", plyFilename.c_str());
            }
        }

        if (!ply.GetProperty("opacity", props.opacity))
        {
            printf("Error parsing ply file \"%s\", missing opacity property\n", plyFilename.c_str());
        }

        for (int i = 0; i < 3; i++)
        {
            if (!ply.GetProperty("scale_" + std::to_string(i), props.scale[i]))
            {
                printf("Error parsing ply file \"%s\", missing scale property\n", plyFilename.c_str());
            }
        }

        for (int i = 0; i < 4; i++)
        {
            if (!ply.GetProperty("rot_" + std::to_string(i), props.rot[i]))
            {
                printf("Error parsing ply file \"%s\", missing rot property\n", plyFilename.c_str());
            }
        }

    }

    {
        numGaussians = ply.GetVertexCount();
        gaussianSize = sizeof(GaussianData);
        data.resize(numGaussians);
    }

    {
        int i = 0;
        uint8_t* rawPtr = (uint8_t*)data.data();
        ply.ForEachVertex([this, &rawPtr, &i, &props](const void* plyData, size_t size)
        {
                GaussianData* basePtr = reinterpret_cast<GaussianData*>(rawPtr);
                basePtr->posWithAlpha[0] = props.x.Read<float>(plyData);
                basePtr->posWithAlpha[1] = props.y.Read<float>(plyData);
                basePtr->posWithAlpha[2] = props.z.Read<float>(plyData);
                basePtr->posWithAlpha[3] = ComputeAlphaFromOpacity(props.opacity.Read<float>(plyData));
                basePtr->r_sh0[0] = props.f_dc[0].Read<float>(plyData);
                basePtr->r_sh0[1] = 0.0f;
                basePtr->r_sh0[2] = 0.0f;
                basePtr->r_sh0[3] = 0.0f;

                basePtr->g_sh0[0] = props.f_dc[1].Read<float>(plyData);
                basePtr->g_sh0[1] = 0.0f;
                basePtr->g_sh0[2] = 0.0f;
                basePtr->g_sh0[3] = 0.0f;

                basePtr->b_sh0[0] = props.f_dc[2].Read<float>(plyData);
                basePtr->b_sh0[1] = 0.0f;
                basePtr->b_sh0[2] = 0.0f;
                basePtr->b_sh0[3] = 0.0f;

                // NOTE: scale is stored in logarithmic scale in plyFile
                float scale[3] =
                {
                    expf(props.scale[0].Read<float>(plyData)),
                    expf(props.scale[1].Read<float>(plyData)),
                    expf(props.scale[2].Read<float>(plyData))
                };
                float rot[4] =
                {
                    props.rot[0].Read<float>(plyData),
                    props.rot[1].Read<float>(plyData),
                    props.rot[2].Read<float>(plyData),
                    props.rot[3].Read<float>(plyData)
                };

                glm::mat3 v = ComputeCovMatFromRotScale(rot, scale);
                basePtr->cov3_col0[0] = v[0][0];
                basePtr->cov3_col0[1] = v[0][1];
                basePtr->cov3_col0[2] = v[0][2];
                basePtr->cov3_col1[0] = v[1][0];
                basePtr->cov3_col1[1] = v[1][1];
                basePtr->cov3_col1[2] = v[1][2];
                basePtr->cov3_col2[0] = v[2][0];
                basePtr->cov3_col2[1] = v[2][1];
                basePtr->cov3_col2[2] = v[2][2];
                i++;
                rawPtr += gaussianSize;
            });
    }

	return uploadToGPU(engine);
}

bool LavaGaussianSplat::uploadToGPU(LavaEngine* engine)
{
    uint32_t i;
    std::vector<glm::vec4> vecPos;
    vecPos.resize(numGaussians);
    uint8_t* rawPtr = (uint8_t*)data.data();
    float farLength = FLT_MIN;
    for (i = 0; i < numGaussians; i++) {
        float* posPtr = (float*)rawPtr;
        vecPos[i] = glm::vec4(posPtr[0], posPtr[1], posPtr[2], 1.0f);
        float sqLength = glm::dot(vecPos[i], vecPos[i]);
        if (sqLength > farLength) farLength = sqLength;
        rawPtr += gaussianSize;
    }
    farLength = sqrtf(farLength);
    farLength += 10.0f; // Nos alejamos un poco del punto más lejano
    const size_t vertex_buffer_size = numGaussians * gaussianSize;
    const size_t index_buffer_size = numGaussians * sizeof(uint32_t);

    //create vertex buffer
    vertex_buffer = std::make_unique<LavaBuffer>(*engine->allocator_, vertex_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    //find the adress of the vertex buffer
    VkBufferDeviceAddressInfo deviceAdressInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,.buffer = vertex_buffer->buffer_.buffer };

    vertex_buffer_address = vkGetBufferDeviceAddress(engine->device_->get_device(), &deviceAdressInfo);

    //create index buffer
    
    for (i = 0; i < 26; ++i) {
        index_buffer[i] = std::make_unique<LavaBuffer>(*engine->allocator_, index_buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY);
        //find the adress of the index buffer
        VkBufferDeviceAddressInfo deviceAdressInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,.buffer = index_buffer[i]->buffer_.buffer };

        //index_buffer_address[i] = vkGetBufferDeviceAddress(engine->device_->get_device(), &deviceAdressInfo);
    }
    
    std::unique_ptr<LavaBuffer> staging = std::make_unique<LavaBuffer>(*engine->allocator_, vertex_buffer_size + (index_buffer_size*26), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void* vData;// = staging.get_buffer().allocation;
    vmaMapMemory(engine->allocator_->get_allocator(), staging->get_buffer().allocation, &vData);

    // copy vertex buffer
    memcpy(vData, data.data(), vertex_buffer_size);

    std::vector<uint32_t> indices[26];

    {
        LavaJobSystem jobSystem;

        uint32_t array_size = (uint32_t)numGaussians - 1;
        int32_t j, k, w;
        i = 0;
        for (j = -1; j < 2; ++j) {
            for (k = -1; k < 2; ++k) {
                for (w = -1; w < 2; ++w) {
                    if (0 == j && 0 == k && 0 == w) continue;
                    indices[i].resize(numGaussians);
                    glm::vec3 refPos = farLength * glm::vec3((float)j, (float)k, (float)w);
                    threadData t_data{ indices[i].data(), refPos, vecPos.data()};
                    jobSystem.add([t_data, array_size]() {
                        
                            uint32_t fg;
                            for (fg = 0; fg < array_size; ++fg) {
                                t_data.index[fg] = fg;
                            }
                            quick_sort(t_data.index, t_data.vecPos, 0, array_size, t_data.refPos);
                        });
                    i++;
                }
            }
        }
    }

    // copy index buffer
    for (i = 0; i < 26; ++i) {
        size_t stride = vertex_buffer_size + (i * index_buffer_size);
        char* indPtr = ((char*)vData) + stride;
        memcpy(indPtr, indices[i].data(), index_buffer_size);
    }

    engine->immediate_submit([&](VkCommandBuffer cmd) {
        VkBufferCopy vertexCopy{ 0 };
        vertexCopy.dstOffset = 0;
        vertexCopy.srcOffset = 0;
        vertexCopy.size = vertex_buffer_size;

        vkCmdCopyBuffer(cmd, staging->get_buffer().buffer, vertex_buffer->get_buffer().buffer, 1, &vertexCopy);
        
        uint32_t j;
        for (j = 0; j < 26; ++j) {
            VkBufferCopy indexCopy{ 0 };
            indexCopy.dstOffset = 0;
            indexCopy.srcOffset = vertex_buffer_size + (j * index_buffer_size);
            indexCopy.size = index_buffer_size;

            vkCmdCopyBuffer(cmd, staging->get_buffer().buffer, index_buffer[j]->get_buffer().buffer, 1, &indexCopy);
        }

        });

    vmaUnmapMemory(engine->allocator_->get_allocator(), staging->get_buffer().allocation);

    return true;
}
