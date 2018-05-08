#ifndef BINARY_CODEC_H
#define BINARY_CODEC_H

#include <fstream>
#include <rttr/type>
#include "binary_data.h"

namespace binary
{

enum BINARY_BASIC_TYPES
{
    CHAR,
    INT16,
    INT32,
    INT64,
    UCHAR,
    UINT16,
    UINT32,
    UINT64,
    FLOAT,
    DOUBLE,
    BOOL
};

#pragma pack(push, 1)
struct header
{
    char id_string[11] = { '\211', 'A', 'u', 'r', 'o', 'r', 'a', '\r', '\n', '\032', '\n' };
    char versionMajor = 1;
    char versionMinor = 0;
    uint64_t offsetToData = sizeof(header);
};
#pragma pack(pop)

#pragma pack(push, 1)
struct chunk_header
{
    uint64_t size; // w/o the chunk header size
    uint32_t type;
    char is_array;
    uint32_t guid;
};
#pragma pack(pop)

class binary_codec
{
public:
    uint32_t write_chunk(uint32_t type, bool is_array, unsigned char* data, size_t size, size_t count)
    {
        
    }
    template<typename T>
    void write_chunk(const T& data)
    {

    }
    template<typename T>
    void write_array_chunk()
    {

    }

    bool save(const std::string& filename)
    {
        data.write(header());
        std::ofstream file(filename, std::ios::binary | std::ios::ate);
        if(!file.is_open())
            return false;
        file.write((char*)data.ptr(), data.size());
        file.close();
        return true;
    }
private:
    binary_data data;
};

}

#endif