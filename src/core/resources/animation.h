#ifndef ANIMATION_H
#define ANIMATION_H

#include <map>
#include <asset.h>
#include <aurora/media/fbx.h>
#include <aurora/curve.h>
#include <fstream>
#include <util/animation/curve.h>

struct AnimNode
{
    std::string name;
    curve3 position;
    curve4 rotation;
    curve3 scale;
};

class AnimTrack
{
public:
    void FrameRate(float fps) { this->fps = fps; }
    float FrameRate() { return fps; }
    void Length(float l) { length = l; }
    AnimNode& operator[](const std::string& node)
    {
        AnimNode& n = animNodes[node];
        n.name = node;
        return n;
    }
    std::map<std::string, AnimNode>& GetNodes() { return animNodes; }
private:
    float fps;
    float length;
    std::map<std::string, AnimNode> animNodes;
};

class Animation
{
public:
    size_t Count() { return anims.size(); }
    AnimTrack& operator[](const std::string& anim)
    {
        return anims[anim];
    }
    std::map<std::string, AnimTrack>& GetTracks() { return anims; }
private:
    std::map<std::string, AnimTrack> anims;
};

struct AnimationReaderFBX : public asset<Animation>::reader
{
    bool operator()(const std::string& filename, Animation* animSet)
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
            //fbxReader.DumpFile(filename);
            fbxReader.ConvertCoordSys(Au::Media::FBX::OPENGL);
            
            std::vector<Au::Media::FBX::AnimationStack>& stacks =
                fbxReader.GetAnimationStacks();
            double fps = fbxReader.GetFrameRate();
            double timePerFrame = Au::Media::FBX::TimeSecond / fps;
            //animSet->FrameRate((float)fps);

            for(unsigned i = 0; i < stacks.size(); ++i)
            {
                double length = stacks[i].GetLength() / timePerFrame;
                std::string animName = stacks[i].GetName();
                {
                    // TODO: Check if fbx is made in blender, only then cut by first pipe symbol
                    size_t pipe_pos = animName.find_first_of("|");
                    if(pipe_pos != std::string::npos)
                    {
                        animName = animName.substr(pipe_pos + 1);
                    }
                }
                
                AnimTrack& anim = animSet->operator[](animName);
                anim.FrameRate((float)fps);
                anim.Length((float)length);

                //std::cout << "AnimStack " << animName << " len: " << length << std::endl;
                
                std::vector<Au::Media::FBX::SceneNode> nodes = stacks[i].GetAnimatedNodes();
                for(unsigned j = 0; j < nodes.size(); ++j)
                {
                    std::string nodeName = nodes[j].Name();
                    AnimNode& animNode = anim[nodeName];
                    float frame = 0.0f;
                    //std::cout << "  CurveNode " << nodeName << std::endl;
                    for(double t = 0.0f; t < length * timePerFrame; t += timePerFrame)
                    {
                        Au::Math::Vec3f pos = 
                            stacks[i].EvaluatePosition(nodes[j], (int64_t)t);
                        animNode.position.x[frame] = pos.x;
                        animNode.position.y[frame] = pos.y;
                        animNode.position.z[frame] = pos.z;

                        Au::Math::Quat rot = 
                            stacks[i].EvaluateRotation(nodes[j], (int64_t)t);
                        animNode.rotation.x[frame] = rot.x;
                        animNode.rotation.y[frame] = rot.y;
                        animNode.rotation.z[frame] = rot.z;
                        animNode.rotation.w[frame] = rot.w;
                        
                        Au::Math::Vec3f scale = 
                            stacks[i].EvaluateScale(nodes[j], (int64_t)t);
                        animNode.scale.x[frame] = scale.x;
                        animNode.scale.y[frame] = scale.y;
                        animNode.scale.z[frame] = scale.z;
                        
                        frame += 1.0f;
                    }
                }
            }
        }
        
        file.close();
        
        return result;
    }

private:
};

#endif
