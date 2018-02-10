#ifndef SOUND_CLIP_H
#define SOUND_CLIP_H

#include "../game_state.h"
#include "../resource.h"

#define STB_VORBIS_HEADER_ONLY
extern "C"{
#include <stb_vorbis.c>
}

class SoundClip
{
public:
    SoundClip()
    : buffer(0) {}
    ~SoundClip()
    {
        if(buffer)
            GameState::GetAudioMixer()->DestroyBuffer(buffer);
    }
    int Upload(void* data, size_t len, int srcSampleRate, int srcBitPerSample, int srcNChannels)
    {
        buffer = GameState::GetAudioMixer()->CreateBuffer();
        return buffer->Upload(data, len, srcSampleRate, srcBitPerSample, srcNChannels);
    }
    
    AudioBuffer* GetBuffer() { return buffer; }
private:
    AudioBuffer* buffer;
};

class SoundClipReaderOGG : public Resource<SoundClip>::Reader
{
public:
    SoundClip* operator()(const std::string& filename)
    {
        SoundClip* clip = 0;
        
        int channels = 2;
        int sampleRate = 48000;
        short* decoded;
        int len;
        len = stb_vorbis_decode_filename(filename.c_str(), &channels, &sampleRate, &decoded);
        if(len == 0)
            return clip;
        
        clip = new SoundClip();
        clip->Upload((void*)decoded, len * sizeof(short) * channels, sampleRate, 16, channels);
        
        return clip;
    }
};

#endif
