/**
 * @file lava_gaussian_splat.hpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Gaussian Splat's header file
 * @version 0.1
 * @date 2025-03-04
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 */

#ifndef __LAVA_GAUSSIAN_SPLAT_H__
#define __LAVA_GAUSSIAN_SPLAT_H__ 1

struct GaussianData;

class LavaGaussianSplat {

public:

    LavaGaussianSplat();
    ~LavaGaussianSplat();

    bool importPly(class LavaEngine* engine, const std::string& plyFilename);

    size_t getNumGaussians() const { return numGaussians; }
    size_t getStride() const { return gaussianSize; }
    size_t getTotalSize() const { return getNumGaussians() * gaussianSize; }
    GaussianData* getRawDataPtr() { return data.data(); }

    std::unique_ptr<class LavaBuffer> index_buffer[26];
    std::unique_ptr<class LavaBuffer> vertex_buffer;
    VkDeviceAddress vertex_buffer_address;
private:

    bool uploadToGPU(class LavaEngine* engine);

    std::vector<GaussianData> data;

    size_t numGaussians;
    size_t gaussianSize;

    VkDeviceAddress index_buffer_address[26];
};

#endif //__LAVA_GAUSSIAN_SPLAT_H__