/**
 * @file lava_ply.hpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava PLY's header file
 * @version 0.1
 * @date 2025-03-04
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 */

#ifndef __LAVA_PLY_H__
#define __LAVA_PLY_H__ 1

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

class LavaPly
{
public:

    struct BinaryAttrib{
        size_t size;
        size_t offset;

        template <typename T>
        T Read(const void* data)
        {
            // ADD THE OFFSET TO THE POINTER AND RETURN THE VALUE
            // IN THE ADDRESS
            return *((T*)(((uint8_t*)(data)) + offset));
        }
    };

    LavaPly();
    bool Parse(std::ifstream& plyFile);

    bool GetProperty(const std::string& key, BinaryAttrib& attributeOut) const;
    void AddProperty(const std::string& key, size_t size);
    void AllocData(size_t numVertices);

    using VertexCallback = std::function<void(const void*, size_t)>;
    void ForEachVertex(const VertexCallback& cb) const;

    using VertexCallbackMut = std::function<void(void*, size_t)>;
    void ForEachVertexMut(const VertexCallbackMut& cb);

    size_t GetVertexCount() const { return vertexCount; }

protected:
    bool ParseHeader(std::ifstream& plyFile);

    std::unordered_map<std::string, BinaryAttrib> propertyMap;
    std::vector<uint8_t> data;
    size_t vertexCount;
    size_t vertexSize;
};

#endif //__LAVA_PLY_H__