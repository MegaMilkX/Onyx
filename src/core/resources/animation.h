#ifndef ANIMATION_H
#define ANIMATION_H

#include <util/gfxm.h>

#include <map>
#include <asset.h>
#include <aurora/media/fbx.h>
#include <aurora/curve.h>
#include <fstream>
#include <util/animation/curve.h>

struct AnimNodePose
{
    std::string name;
    gfxm::transform transform;
};

struct AnimNode
{
    std::string name;
    curve3 position;
    curve4 rotation;
    curve3 scale;
};

struct AnimPose
{
    std::map<std::string, gfxm::mat4> poses;
};

class AnimTrack
{
public:
    class Cursor
    {
    public:
        Cursor() : track(0) {}
        Cursor(AnimTrack* t) 
        : track(t), cursor(0) 
        {}
        void operator+=(float t)
        {
            Advance(t);
        }
        void Advance(float t)
        {
            if(!track) return;
            cursor += t;
            float len = track->Length();
            if(cursor > len) cursor -= len;
        }
        void SetPose(AnimPose& p)
        {
            p = pose;
        }
        AnimPose* GetPose() 
        {
            if(!track)
                return &pose;
            for(auto& kv : track->GetNodes())
            {
                AnimNodePose np;
                np.transform.set_transform(pose.poses[kv.first]);
                if(!kv.second.position.empty()) np.transform.position(kv.second.position.at(cursor, gfxm::vec3(0, 0, 0)));
                gfxm::vec4 q = kv.second.rotation.at(cursor, gfxm::vec4(0, 0, 0, 1));
                if(!kv.second.rotation.empty()) np.transform.rotation(gfxm::quat(q.x, q.y, q.z, q.w));
                if(!kv.second.scale.empty()) np.transform.scale(kv.second.scale.at(cursor, gfxm::vec3(1, 1, 1)));
                pose.poses[kv.first] = np.transform.matrix();
            }
            return &pose;
        }
    private:
        float cursor;
        AnimTrack* track;
        AnimPose pose;
    };

    Cursor GetCursor() { return Cursor(this); }

    std::string Name() { return name; }
    void Name(const std::string& name) { this->name = name; }
    void Length(float l) { length = l; }
    float Length() { return length; }
    AnimNode& operator[](const std::string& node)
    {
        AnimNode& n = animNodes[node];
        n.name = node;
        return n;
    }
    std::map<std::string, AnimNode>& GetNodes() { return animNodes; }
private:
    std::string name;
    float length;
    std::map<std::string, AnimNode> animNodes;
};

class Animation
{
public:
    size_t Count() { return anims.size(); }
    AnimTrack* operator[](const std::string& anim)
    {
        if(anims.count(anim) == 0)
            anims[anim] = new AnimTrack();
        return anims[anim];
    }
    std::map<std::string, AnimTrack*>& GetTracks() { return anims; }
    AnimPose& GetBindPose() { return bindPose; }
    void FrameRate(float fps) { this->fps = fps; }
    float FrameRate() { return fps; }
private:
    std::map<std::string, AnimTrack*> anims;
    AnimPose bindPose;
    float fps;
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
            
            animSet->FrameRate((float)fps);
            AnimPose& pose = animSet->GetBindPose();
            for(unsigned i = 0; i < fbxReader.ModelCount(); ++i)
            {
                Au::Media::FBX::Model* fbxModel = 
                    fbxReader.GetModel(i);
                pose.poses[fbxModel->name] = *(gfxm::mat4*)&fbxModel->transform;
            }

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
                
                AnimTrack* anim = animSet->operator[](animName);
                anim->Length((float)length);
                anim->Name(animName);

                //std::cout << "AnimStack " << animName << " len: " << length << std::endl;
                
                std::vector<Au::Media::FBX::SceneNode> nodes = stacks[i].GetAnimatedNodes();
                for(unsigned j = 0; j < nodes.size(); ++j)
                {
                    if(!stacks[i].HasPositionCurve(nodes[i]) &&
                        !stacks[i].HasRotationCurve(nodes[i]) &&
                        !stacks[i].HasScaleCurve(nodes[i]))
                    {
                        continue;
                    }
                    std::string nodeName = nodes[j].Name();
                    AnimNode& animNode = anim->operator[](nodeName);
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
