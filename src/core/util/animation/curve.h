#ifndef CURVE_H
#define CURVE_H

#include <aurora/math.h>
#include <vector>

struct keyframe
{
    keyframe(): time(0) {}
    keyframe(float time): time(time) {}
    void operator=(float value)
    { this->value = value; }
    bool operator<(const keyframe& other)
    { return time < other.time; }
    float time;
    float value;
};

class curve
{
public:
    float at(float time, float def = 0.0f)
    {
        if(keyframes.empty())
            return def;
        keyframe* k0 = 0, *k1 = 0;
        for(int i = keyframes.size() - 1; i >= 0; --i)
        {
            k0 = &keyframes[i];
            if(i == keyframes.size() - 1)
                k1 = k0;
            else
                k1 = &keyframes[i + 1];
            if(k0->time <= time)
                break;
        }
        if(k0 == 0)
            return def;
        if(k0 == k1)
            return k0->time;
        float a = k0->time;
        float b = k1->time;
        float t = (time - k0->time) / (k1->time - k0->time);
        return a + t * (b - a);
    }

    float delta(float from, float to)
    {
        if(keyframes.empty())
            return 0.0f;
        float dt = 0.0f;
        float prevValue = at(from);
        float value = at(to);

        if(to < from)
        {
            float d0 = keyframes.back().value - prevValue;
            float d1 = value - keyframes.front().value;
            dt = d0 + d1;
        }
        else
        {
            dt = value - prevValue;
        }
        return dt;
    }

    keyframe& operator[](float time)
    {
        for(unsigned i = 0; i < keyframes.size(); ++i)
        {
            if(fabsf(keyframes[i].time - time) < FLT_EPSILON)
            {
                return keyframes[i];
            }
        }
        
        keyframes.push_back(keyframe(time));
        std::sort(keyframes.begin(), keyframes.end());
        return operator[](time);
    }
private:
    std::vector<keyframe> keyframes;
};

struct curve2
{
    Au::Math::Vec2f at(float t, const Au::Math::Vec2f& def)
    {
        return Au::Math::Vec2f(x.at(t, def.x), y.at(t, def.y));
    }
    Au::Math::Vec2f delta(float from, float to)
    {
        return Au::Math::Vec2f(
            x.delta(from, to), 
            y.delta(from, to)
        );
    }
    curve x, y;
};

struct curve3
{
    Au::Math::Vec3f at(float t, const Au::Math::Vec3f& def)
    {
        return Au::Math::Vec3f(
            x.at(t, def.x), 
            y.at(t, def.y), 
            z.at(t, def.z));
    }
    Au::Math::Vec3f delta(float from, float to)
    {
        return Au::Math::Vec3f(
            x.delta(from, to),
            y.delta(from, to),
            z.delta(from, to)
        );
    }
    curve x, y, z;
};

struct curve4
{
    Au::Math::Vec4f at(float t, const Au::Math::Vec4f& def)
    {
        return Au::Math::Vec4f(
            x.at(t, def.x), 
            y.at(t, def.y), 
            z.at(t, def.z), 
            w.at(t, def.w));
    }
    Au::Math::Vec4f delta(float from, float to)
    {
        return Au::Math::Vec4f(
            x.delta(from, to),
            y.delta(from, to),
            z.delta(from, to),
            w.delta(from, to)
        );
    }
    curve x, y, z, w;
};

#endif
