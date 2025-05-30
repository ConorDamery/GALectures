#include <App.hpp>

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cstring>

struct SfxInstance
{
    u32 audio{ 0 };
    ma_decoder decoder{};
    bool loop{ false };
};

struct SfxAudio
{
    ma_decoder decoder{};
    f32 volume{ 1.0f };
    std::string filepath{};
};

struct SfxChannel
{
    f32 volume{ 1.0f };
    bool active{ true };
    std::vector<SfxInstance> instances;
};

struct SfxData
{
    ma_device device{};
    std::vector<SfxAudio> audios{};
    std::vector<SfxChannel> channels{};
};

static SfxData g_data;

static void sfx_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    auto* data = static_cast<SfxData*>(pDevice->pUserData);
    float* out = static_cast<float*>(pOutput);
    std::memset(out, 0, frameCount * sizeof(float) * 2);

    for (auto& channel : data->channels)
    {
        if (!channel.active) continue;

        for (auto& instance : channel.instances)
        {
            std::vector<float> tempBuffer(frameCount * 2);
            ma_uint64 framesRead;
            ma_decoder_read_pcm_frames(&instance.decoder, tempBuffer.data(), frameCount, &framesRead);

            if (framesRead == 0 && instance.loop)
            {
                ma_decoder_seek_to_pcm_frame(&instance.decoder, 0);
                ma_decoder_read_pcm_frames(&instance.decoder, tempBuffer.data(), frameCount, &framesRead);
            }

            for (ma_uint32 i = 0; i < framesRead * 2; ++i)
            {
                out[i] += tempBuffer[i] * channel.volume;
            }
        }
    }
    (void)pInput;
}

bool App::SfxInitialize()
{
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate = 48000;
    deviceConfig.dataCallback = sfx_data_callback;
    deviceConfig.pUserData = &g_data;

    if (ma_device_init(nullptr, &deviceConfig, &g_data.device) != MA_SUCCESS)
    {
        LOGE("Failed to initialize playback device.");
        return false;
    }
    ma_device_start(&g_data.device);
    return true;
}

void App::SfxShutdown()
{
    SfxReload();
    ma_device_uninit(&g_data.device);
}

void App::SfxReload()
{
    for (auto& channel : g_data.channels)
    {
        for (auto& inst : channel.instances)
        {
            ma_decoder_uninit(&inst.decoder);
        }
        channel.instances.clear();
    }

    for (auto& audio : g_data.audios)
    {
        ma_decoder_uninit(&audio.decoder);
    }

    g_data.audios.clear();
    g_data.channels.clear();
}

static SfxChannel* sfx_get_channel(u32 channel)
{
    if (channel == 0 || channel > g_data.channels.size())
    {
        LOGW("Invalid channel handle!");
        return nullptr;
    }

    return &g_data.channels[static_cast<size_t>(channel) - 1];
}

static SfxAudio* sfx_get_audio(u32 audio)
{
    if (audio == 0 || audio > g_data.audios.size())
    {
        LOGW("Invalid audio handle!");
        return nullptr;
    }

    return &g_data.audios[static_cast<size_t>(audio) - 1];
}

u32 App::SfxLoadAudio(const char* filepath)
{
    const char* path = FilePath(filepath);

    SfxAudio audio;
    audio.filepath = path;

    if (ma_decoder_init_file(path, nullptr, &audio.decoder) != MA_SUCCESS)
    {
        LOGW("Failed to load audio: %s", filepath);
        return 0;
    }

    g_data.audios.push_back(audio);
    return static_cast<u32>(g_data.audios.size());
}

void App::SfxDestroyAudio(u32 audio)
{
    auto aud = sfx_get_audio(audio);
    if (aud == nullptr) return;
    ma_decoder_uninit(&aud->decoder);
    *aud = SfxAudio{};
}

u32 App::SfxCreateChannel(f32 volume)
{
    g_data.channels.emplace_back(SfxChannel{ volume, true });
    return static_cast<u32>(g_data.channels.size());
}

void App::SfxDestroyChannel(u32 channel)
{
    auto chn = sfx_get_channel(channel);
    if (chn == nullptr)
        return;

    for (auto& inst : chn->instances)
    {
        ma_decoder_uninit(&inst.decoder);
    }

    *chn = SfxChannel{};
}

void App::SfxSetChannelVolume(u32 channel, f32 volume)
{
    auto chn = sfx_get_channel(channel);
    if (chn == nullptr)
        return;

    chn->volume = volume;
}

void App::SfxPlay(u32 audio, u32 channel, bool loop)
{
    auto aud = sfx_get_audio(audio);
    auto chn = sfx_get_channel(channel);

    if (aud == nullptr || chn == nullptr)
        return;

    SfxInstance instance;
    instance.audio = audio;

    if (ma_decoder_init_file(aud->filepath.c_str(), nullptr, &instance.decoder) != MA_SUCCESS)
    {
        LOGW("Failed to initialize decoder for playback.");
        return;
    }
    instance.loop = loop;
    chn->instances.push_back(instance);
}

void App::SfxStop(u32 audio, u32 channel)
{
    auto aud = sfx_get_audio(audio);
    auto chn = sfx_get_channel(channel);

    if (aud == nullptr || chn == nullptr)
        return;

    auto& instances = chn->instances;
    for (size_t i = 0; i < instances.size(); )
    {
        if (instances[i].audio == audio)
        {
            ma_decoder_uninit(&instances[i].decoder);
            instances.erase(instances.begin() + i);
        }
        else
        {
            ++i;
        }
    }
}