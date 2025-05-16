/**
 * @file lava_ply.cpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava PLY's file
 * @version 0.1
 * @date 2024-10-01
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 **/

#include "lava/gaussian/lava_ply.hpp"

#include <fstream>
#include <sstream>

static bool CheckLine(std::ifstream& plyFile, const std::string& validLine)
{
    std::string line;
    return std::getline(plyFile, line) && line == validLine;
}

static bool GetNextPlyLine(std::ifstream& plyFile, std::string& lineOut)
{
    while (std::getline(plyFile, lineOut))
    {
        // skip comment lines
        if (lineOut.find("comment", 0) != 0)
        {
            return true;
        }
    }
    return false;
}

LavaPly::LavaPly() : vertexCount(0), vertexSize(0)
{
    ;
}

bool LavaPly::Parse(std::ifstream& plyFile)
{
    if (!ParseHeader(plyFile))
    {
        return false;
    }

    // read rest of file into data ptr
    {
        AllocData(vertexCount);
        plyFile.read((char*)data.data(), vertexSize * vertexCount);
    }

    return true;
}

bool LavaPly::GetProperty(const std::string& key, BinaryAttrib& binaryAttributeOut) const
{
    auto iter = propertyMap.find(key);
    if (iter != propertyMap.end())
    {
        binaryAttributeOut = iter->second;
        return true;
    }
    return false;
}

void LavaPly::AddProperty(const std::string& key, size_t size)
{
    using PropInfoPair = std::pair<std::string, BinaryAttrib>;
    BinaryAttrib attrib;
    attrib.offset = vertexSize;
    attrib.size = size;
    propertyMap.emplace(PropInfoPair(key, attrib));
    vertexSize += size;
}

void LavaPly::AllocData(size_t numVertices)
{
    vertexCount = numVertices;
    data.resize(vertexSize * numVertices, 0);
}

void LavaPly::ForEachVertex(const VertexCallback& cb) const
{
    const uint8_t* ptr = data.data();
    for (size_t i = 0; i < vertexCount; i++)
    {
        cb(ptr, vertexSize);
        ptr += vertexSize;
    }
}

void LavaPly::ForEachVertexMut(const VertexCallbackMut& cb)
{
    uint8_t* ptr = data.data();
    for (size_t i = 0; i < vertexCount; i++)
    {
        cb(ptr, vertexSize);
        ptr += vertexSize;
    }
}

bool LavaPly::ParseHeader(std::ifstream& plyFile)
{

    // validate start of header
    std::string token1, token2, token3;

    // check header starts with "ply".
    if (!GetNextPlyLine(plyFile, token1))
    {
        printf("Unexpected error reading next line\n");
        return false;
    }
    if (token1 != "ply")
    {
        printf("Invalid ply file\n");
        return false;
    }

    // check format
    if (!GetNextPlyLine(plyFile, token1))
    {
        printf("Unexpected error reading next line\n");
        return false;
    }
    if (token1 != "format binary_little_endian 1.0" && token1 != "format binary_big_endian 1.0")
    {
        printf("Invalid ply file, expected format\n");
        return false;
    }
    if (token1 != "format binary_little_endian 1.0")
    {
        printf("Unsupported ply file, only binary_little_endian supported\n");
        return false;
    }

    // parse "element vertex {number}"
    std::string line;
    if (!GetNextPlyLine(plyFile, line))
    {
        printf("Unexpected error reading next line\n");
        return false;
    }
    std::istringstream iss(line);
    if (!((iss >> token1 >> token2 >> vertexCount) && (token1 == "element") && (token2 == "vertex")))
    {
        printf("Invalid ply file, expected \"element vertex {number}\"\n");
        return false;
    }

    // TODO: support other "element" types faces, edges etc?
    // at the moment I only care about ply files with vertex elements.

    while (true)
    {
        if (!GetNextPlyLine(plyFile, line))
        {
            printf("unexpected error reading line\n");
            return false;
        }

        if (line == "end_header")
        {
            break;
        }

        iss.str(line);
        iss.clear();
        iss >> token1 >> token2 >> token3;
        if (token1 != "property")
        {
            printf("Invalid header, expected property\n");
            return false;
        }
        if (token2 == "char" || token2 == "int8")
        {
            AddProperty(token3, sizeof(int8_t));
        }
        else if (token2 == "uchar" || token2 == "uint8")
        {
            AddProperty(token3, sizeof(uint8_t));
        }
        else if (token2 == "short" || token2 == "int16")
        {
            AddProperty(token3, sizeof(int16_t));
        }
        else if (token2 == "ushort" || token2 == "uint16")
        {
            AddProperty(token3, sizeof(uint16_t));
        }
        else if (token2 == "int" || token2 == "int32")
        {
            AddProperty(token3, sizeof(int32_t));
        }
        else if (token2 == "uint" || token2 == "uint32")
        {
            AddProperty(token3, sizeof(uint32_t));
        }
        else if (token2 == "float" || token2 == "float32")
        {
            AddProperty(token3, sizeof(float));
        }
        else if (token2 == "double" || token2 == "float64")
        {
            AddProperty(token3, sizeof(double));
        }
        else
        {
            printf("Unsupported type \"%s\" for property \"%s\"\n", token2.c_str(), token3.c_str());
            return false;
        }
    }

    return true;
}
