#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <fstream>
#include <aurora/gfx.h>
#include <aurora/media/fbx.h>

#include <asset.h>

#include "../util/gl_helpers.h"

struct GLAttribDesc
{
    std::string name;
    int elemCount;
    GLenum elemType;
    GLboolean normalized;
};

class MeshData
{
public:
    struct SubData
    {
        std::string name;
        unsigned int indexCount;
        unsigned int offset;
    };
    std::vector<SubData> subDataArray;
    
    void FillMesh(Au::GFX::Mesh* mesh)
    {
        std::vector<Au::AttribInfo> fmt = mesh->Format();
        
        for(unsigned i = 0; i < fmt.size(); ++i)
        {
            std::map<Au::AttribInfo, std::vector<unsigned char>>::iterator it =
                attribArrays.find(fmt[i]);
            if(it == attribArrays.end())
                continue;
            
            mesh->VertexAttribByInfo(fmt[i], it->second);
        }
        mesh->IndexData(indices);
        
        for(unsigned i = 0; i < subDataArray.size(); ++i)
        {
            SubData& subData = subDataArray[i];
            mesh->SetSubMesh(
                subData.name,
                subData.indexCount,
                subData.offset
            );
        }
    }
    
    template<typename ATTR, typename T>
    void SetAttribArray(const std::vector<T>& data)
    {
        std::vector<unsigned char> bytes(
            (unsigned char*)data.data(), 
            (unsigned char*)data.data() + data.size() * sizeof(T)
        );
        
        attribArrays[ATTR()] = bytes;
        for(unsigned i = 0; i < vaoDirty.size(); ++i)
            vaoDirty[i] = true;
    }
    
    void SetIndices(const std::vector<unsigned short>& data)
    { 
        indices = data;
        for(unsigned i = 0; i < vaoDirty.size(); ++i)
            vaoDirty[i] = true;
    }
    
    template<typename T>
    unsigned GetAttribCount()
    {
        size_t attrSz = sizeof(T);
        if(attrSz == 0)
            return 0;
        unsigned count = attribArrays[T()].size() / attrSz;
        return count;
    }
    
    template<typename T>
    T* GetAttribData()
    {
        return (T*)attribArrays[T()].data();
    }
    
    template<typename T>
    std::vector<unsigned char>& GetAttribBytes()
    {
        return attribArrays[T()];
    }
    
    std::vector<unsigned char>& GetAttribBytesByName(const std::string& name)
    {
        for(auto& kv : attribArrays)
        {
            if(kv.first.name == name)
                return kv.second;
        }
        return attribArrays[Au::Empty()];
    }
    
    std::vector<unsigned short>& GetIndices()
    {
        return indices;
    }
    
    GLuint GetVao(const std::vector<GLAttribDesc>& vertexDesc)
    {
        for(unsigned i = 0; i < vertexArrayObjects.size(); ++i)
        {
            if(_compareDesc(vertexDesc, vertexArrayObjects[i]))
            {
                if(vaoDirty[i])
                {
                    vaoDirty.erase(vaoDirty.begin() + i);
                    vertexArrayObjects[i].Cleanup();
                    vertexArrayObjects.erase(vertexArrayObjects.begin() + i);
                    break;
                }
                else
                    return vertexArrayObjects[i].GetGlName();
            }
        }
        
        GLVertexArrayObject vao;
        std::vector<GLVertexBufferDesc> desc;
        for(const GLAttribDesc& d : vertexDesc)
        {
            desc.push_back({ 
                d.name, 
                d.elemCount, 
                d.elemType, d.normalized, 
                (GLsizei)(d.elemCount * glTypeSize(d.elemType)), 
                GL_STATIC_DRAW 
            });
        }
        vao.Init(desc);
        for(const GLAttribDesc& d : vertexDesc)
        {
            vao.FillArrayBuffer(d.name, GetAttribBytesByName(d.name));
        }
        vao.FillIndexBuffer(GetIndices());
        
        vertexArrayObjects.push_back(vao);
        vaoDirty.push_back(false);
        
        return vao.GetGlName();
    }
    unsigned GetIndexCount() 
    { return indices.size(); }
    
    std::vector<GLVertexArrayObject> vertexArrayObjects;
    std::vector<bool> vaoDirty;
    std::map<Au::AttribInfo, std::vector<unsigned char>> attribArrays;
    std::vector<unsigned short> indices;
    
    bool _compareDesc(
        const std::vector<GLAttribDesc>& vertexDesc,
        const GLVertexArrayObject& vao)
    {
        const std::vector<GLVertexBufferDesc>& vaoDesc =
            vao.Desc();
        if(vertexDesc.size() != vaoDesc.size())
            return false;
        for(unsigned i = 0; i < vertexDesc.size(); ++i)
        {
            const GLAttribDesc& d1 = vertexDesc[i];
            const GLVertexBufferDesc& d2 = vaoDesc[i];

            if(d1.name != d2.name ||
                d1.elemCount != d2.size ||
                d1.elemType != d2.type ||
                d1.normalized != d2.normalized)
            {
                return false;
            }
        }
        
        return true;
    }
};

class MeshReaderFBX : public asset<MeshData>::reader
{
public:
    bool operator()(const std::string& filename, MeshData* meshData)
    {
        bool result = false;        
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if(!file.is_open())
            return result;
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> buffer((unsigned int)size);
        if(file.read(buffer.data(), size))
        {
            result = true;
            
            Au::Media::FBX::Reader fbxReader;
            fbxReader.ReadMemory(buffer.data(), buffer.size());
            fbxReader.ConvertCoordSys(Au::Media::FBX::OPENGL);
            
            fbxReader.DumpFile(filename);
            
            int meshCount = fbxReader.MeshCount();
            std::vector<float> vertices;
            std::vector<float> normals;
            std::vector<float> uv;
            std::vector<unsigned short> indices;
            std::vector<Au::Math::Vec4f> boneIndices;
            std::vector<Au::Math::Vec4f> boneWeights;
            unsigned int indexOffset = 0;
            for(int i = 0; i < meshCount; ++i)
            {
                Au::Media::FBX::Mesh& fbxMesh = fbxReader.GetMesh(i);
                int vertexCount = fbxMesh.VertexCount();
                
                std::vector<float> v = fbxMesh.GetVertices();
                vertices.insert(vertices.end(), v.begin(), v.end());
                std::vector<float> n = fbxMesh.GetNormals(0);
                normals.insert(normals.end(), n.begin(), n.end());
                std::vector<float> u = fbxMesh.GetUV(0);
                uv.insert(uv.end(), u.begin(), u.end());
                
                MeshData::SubData subData;
                subData.offset = indices.size() * sizeof(unsigned short); // BYTE OFFSET
                
                std::vector<unsigned short> rawIndices = fbxMesh.GetIndices<unsigned short>();
                for(unsigned j = 0; j < rawIndices.size(); ++j)
                    rawIndices[j] += indexOffset;
                indices.insert(indices.end(), rawIndices.begin(), rawIndices.end());
                
                subData.name = fbxMesh.name;
                subData.indexCount = rawIndices.size();
                meshData->subDataArray.push_back(subData);
                
                Au::Media::FBX::Skin skin = fbxMesh.GetSkin();
                std::vector<Au::Math::Vec4f> tmpBoneIndices;
                std::vector<Au::Math::Vec4f> tmpBoneWeights;
                std::vector<int> boneDataCount;
                tmpBoneIndices.resize(vertexCount, Au::Math::Vec4f(0.0f, 0.0f, 0.0f, 0.0f));
                tmpBoneWeights.resize(vertexCount, Au::Math::Vec4f(0.0f, 0.0f, 0.0f, 0.0f));
                boneDataCount.resize(vertexCount, 0);
                for(unsigned j = 0; j < skin.BoneCount(); ++j)
                {
                    int64_t uidBone = skin.GetBoneUID(j);
                    Au::Media::FBX::Bone* bone = fbxReader.GetBoneByUID(uidBone);
                    if(!bone)
                        continue;
                    unsigned boneIndex = bone->Index();
                    
                    for(unsigned k = 0; k < bone->indices.size() && k < bone->weights.size(); ++k)
                    {
                        int32_t vertexIndex = bone->indices[k];
                        float weight = bone->weights[k];
                        if(weight < 0.01f)
                            continue;
                        int& dataCount = boneDataCount[vertexIndex];
                        if(dataCount > 3)
                            continue;
                        
                        tmpBoneIndices[vertexIndex][dataCount] = (float)boneIndex;
                        tmpBoneWeights[vertexIndex][dataCount] = weight;
                        
                        dataCount++;
                    }
                }
                
                //for(int j = 0; j < vertexCount; ++j)
                //    Au::Math::Normalize(tmpBoneWeights[j]);
                
                boneIndices.insert(boneIndices.end(), tmpBoneIndices.begin(), tmpBoneIndices.end());
                boneWeights.insert(boneWeights.end(), tmpBoneWeights.begin(), tmpBoneWeights.end());
                
                indexOffset = vertices.size() / 3;
            }
            
            meshData->SetAttribArray<Au::Position>(vertices);
            meshData->SetAttribArray<Au::Normal>(normals);
            meshData->SetAttribArray<Au::UV>(uv);
            meshData->SetIndices(indices);
            
            meshData->SetAttribArray<Au::BoneIndex4>(boneIndices);
            meshData->SetAttribArray<Au::BoneWeight4>(boneWeights);
        }
        
        file.close();
        
        return result;
    }
};

#endif
