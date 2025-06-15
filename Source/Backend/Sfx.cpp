#include <App.hpp>

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <iostream>
#include <algorithm>
#include <cstring>
#include <atomic>

using namespace GASandbox;

using fReadSample = bool (*)(f32&);
using fIsCallbackBound = bool (*)();

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
    string filepath{};
};

struct SfxChannel
{
    f32 volume{ 1.0f };
    bool active{ true };
    list<SfxInstance> instances{};

    SfxChannel() = default;
    SfxChannel(f32 volume, bool active)
        : volume(volume), active(active)
    {}
};

struct SfxGlobal
{
    ma_device device{};
    list<SfxAudio> audios{};
    list<SfxChannel> channels{};

    list<f32> buffer{};
    size_type capacity{ 0 };
    std::atomic<size_type> tail{ 0 };
    std::atomic<size_type> head{ 0 };
    std::atomic<bool> callbackBound{ false };

    fReadSample freadSample{ nullptr };
    fIsCallbackBound fisCallbackBound{ nullptr };

    f64 sampleTimeAccum{ 0 };
};
static SfxGlobal g;

static void sfx_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    auto* data = static_cast<SfxGlobal*>(pDevice->pUserData);
    f32* out = static_cast<f32*>(pOutput);
    std::memset(out, 0, frameCount * sizeof(f32) * 2);

    for (auto& channel : data->channels)
    {
        if (!channel.active) continue;

        for (auto& instance : channel.instances)
        {
            list<f32> tempBuffer(frameCount * 2);
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

    if (data->fisCallbackBound())
    {
        for (ma_uint32 i = 0; i < frameCount; ++i)
        {
            f32 scriptSample = 0.0f;
            if (!data->freadSample(scriptSample))
            {
                scriptSample = 0.0f; // Underrun
            }

            out[i * 2 + 0] += scriptSample * 0.5f;
            out[i * 2 + 1] += scriptSample * 0.5f;
        }
    }
    
    (void)pInput;
}

bool App::SfxInitialize(const sAppConfig& config)
{
    g.capacity = (size_type)48000*2 + 1;
    g.buffer.resize(g.capacity);
    g.freadSample = App::SfxReadSample;
    g.fisCallbackBound = App::SfxIsCallbackBound;

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate = 48000;
    deviceConfig.dataCallback = sfx_data_callback;
    deviceConfig.pUserData = &g;

    if (ma_device_init(nullptr, &deviceConfig, &g.device) != MA_SUCCESS)
    {
        LOGE("Failed to initialize playback device.");
        return false;
    }
    ma_device_start(&g.device);
    return true;
}

void App::SfxShutdown()
{
    SfxReload();
    ma_device_uninit(&g.device);
}

void App::SfxUpdate(f64 dt)
{
    if (!SfxIsCallbackBound())
        return;

    constexpr f64 sampleRate = 48000.0;
    constexpr f64 sampleDelta = 1.0 / sampleRate;

    g.sampleTimeAccum += dt;

    // Cap to avoid runaway accumulation
    constexpr f64 maxAccum = 0.25; // 250 ms = safe max
    if (g.sampleTimeAccum > maxAccum)
        g.sampleTimeAccum = maxAccum;

    while (g.sampleTimeAccum >= sampleDelta)
    {
        f32 sample = CodeAudio(sampleRate, sampleDelta);
        SfxWriteSample(sample);
        g.sampleTimeAccum -= sampleDelta;
    }
}

bool App::SfxWriteSample(f32 sample)
{
    size_type head = g.head.load(std::memory_order_relaxed);
    size_type tail = g.tail.load(std::memory_order_acquire);

    size_type nextHead = (head + 1) % g.capacity;
    if (nextHead == tail)
    {
        // Buffer full
        return false;
    }

    g.buffer[head] = sample;
    g.head.store(nextHead, std::memory_order_release);
    return true;
}

bool App::SfxReadSample(f32& sampleOut)
{
    size_type head = g.head.load(std::memory_order_acquire);
    size_type tail = g.tail.load(std::memory_order_relaxed);

    if (tail == head)
    {
        // Buffer empty
        return false;
    }

    sampleOut = g.buffer[tail];
    size_type nextTail = (tail + 1) % g.capacity;
    g.tail.store(nextTail, std::memory_order_release);
    return true;
}

size_type App::SfxSampleCount()
{
    if (g.head >= g.tail)
        return g.head - g.tail;
    return g.capacity - g.tail + g.head;
}

void App::SfxClearSamples()
{
    g.head = g.tail.load();
}

void App::SfxReload()
{
    SfxUnbindCallback();
    SfxClearSamples();

    for (auto& channel : g.channels)
    {
        for (auto& inst : channel.instances)
        {
            ma_decoder_uninit(&inst.decoder);
        }
        channel.instances.clear();
    }

    for (auto& audio : g.audios)
    {
        ma_decoder_uninit(&audio.decoder);
    }

    g.audios.clear();
    g.channels.clear();

    // Audio API
    CodeBindMethod("app", "App", true, "sfxBindCallback()",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 1);
            SfxBindCallback();
        });

    CodeBindMethod("app", "App", true, "sfxUnbindCallback()",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 1);
            SfxUnbindCallback();
        });

    CodeBindMethod("app", "App", true, "sfxIsCallbackBound",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 1);
            CodeSetSlotBool(vm, SfxIsCallbackBound(), 0);
        });

    CodeBindMethod("app", "App", true, "sfxLoadAudio(_)",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 1);
            u32 audio = SfxLoadAudio(CodeGetSlotString(vm, 1));
            CodeSetSlotUInt(vm, 0, audio);
        });

    CodeBindMethod("app", "App", true, "sfxDestroyAudio(_)",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 1);
            SfxDestroyAudio(CodeGetSlotUInt(vm, 1));
        });

    CodeBindMethod("app", "App", true, "sfxCreateChannel(_)",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 1);
            u32 channel = SfxCreateChannel(CodeGetSlotFloat(vm, 1));
            CodeSetSlotUInt(vm, 0, channel);
        });

    CodeBindMethod("app", "App", true, "sfxDestroyChannel(_)",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 1);
            SfxDestroyChannel(CodeGetSlotUInt(vm, 1));
        });

    CodeBindMethod("app", "App", true, "sfxSetChannelVolume(_,_)",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 2);
            SfxSetChannelVolume(CodeGetSlotUInt(vm, 1), CodeGetSlotFloat(vm, 2));
        });

    CodeBindMethod("app", "App", true, "sfxPlay(_,_,_)",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 3);
            SfxPlay(CodeGetSlotUInt(vm, 1), CodeGetSlotUInt(vm, 2), CodeGetSlotBool(vm, 3));
        });

    CodeBindMethod("app", "App", true, "sfxStop(_,_)",
        [](sCodeVM* vm)
        {
            CodeEnsureSlots(vm, 2);
            SfxStop(CodeGetSlotUInt(vm, 1), CodeGetSlotUInt(vm, 2));
        });
}

static SfxChannel* sfx_get_channel(u32 channel)
{
    if (channel == 0 || channel > g.channels.size())
    {
        LOGW("Invalid channel handle!");
        return nullptr;
    }

    return &g.channels[static_cast<size_type>(channel) - 1];
}

static SfxAudio* sfx_get_audio(u32 audio)
{
    if (audio == 0 || audio > g.audios.size())
    {
        LOGW("Invalid audio handle!");
        return nullptr;
    }

    return &g.audios[static_cast<size_type>(audio) - 1];
}

void App::SfxBindCallback()
{
    SfxClearSamples();
    g.callbackBound.store(true, std::memory_order_release);
}

void App::SfxUnbindCallback()
{
    g.callbackBound.store(false, std::memory_order_release);
}

bool App::SfxIsCallbackBound()
{
    return g.callbackBound.load(std::memory_order_acquire);
}

u32 App::SfxLoadAudio(cstring filepath)
{
    cstring path = FilePath(filepath);

    SfxAudio audio;
    audio.filepath = path;

    if (ma_decoder_init_file(path, nullptr, &audio.decoder) != MA_SUCCESS)
    {
        LOGW("Failed to load audio: %s", filepath);
        return 0;
    }

    g.audios.push_back(audio);
    return static_cast<u32>(g.audios.size());
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
    g.channels.emplace_back(SfxChannel{ volume, true });
    return static_cast<u32>(g.channels.size());
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
    for (size_type i = 0; i < instances.size(); )
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