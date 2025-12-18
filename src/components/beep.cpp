#include "beep.h"

#include <QAudioFormat>
#include <QAudioOutput>
#include <QBuffer>
#include <QMutex>
#include <spdlog/spdlog.h>

template <typename T>
static float amplitudeOf() {
    if constexpr (std::is_floating_point_v<T>) {
        return 1.0f;
    } else if constexpr (std::is_unsigned_v<T>) {
        using SignedT = std::make_signed_t<T>;
        return static_cast<float>(std::numeric_limits<SignedT>::max());
    } else {
        return static_cast<float>(std::numeric_limits<T>::max());
    }
}

template <typename T>
static float offsetOf() {
    if constexpr (std::is_unsigned_v<T>) {
        return 1.0f;
    } else {
        return 0.0f;
    }
}

template <typename T>
static std::vector<T> allocBeepData(int sr, float dur) {
    return std::vector<T>(static_cast<int>(dur * sr));
}

template <typename T>
static void writeBeepData(std::vector<T> &data, int sr, float at, float dur, float freq, float vol) {
    const int startSample = static_cast<int>(at * sr);
    const int numSamples = std::min(static_cast<int>(dur * sr), static_cast<int>(data.size()) - startSample);
    const float amplitude = amplitudeOf<T>();
    const float offset = offsetOf<T>();

    for (int i = 0; i < numSamples; i++) {
        const float t = static_cast<float>(i) / sr;
        const float sampleValue = std::clamp(std::sin(6.283185f * freq * t) * vol, -1.0f, 1.0f) + offset;
        data[startSample + i] += static_cast<T>(amplitude * sampleValue);
    }
}

template <typename T>
static QBuffer *genBeepData(int sr) {
    std::vector<T> data = allocBeepData<T>(sr, 0.2f);
    writeBeepData<T>(data, sr, 0.0f, 0.06f, 1500.0f, 0.3f);
    writeBeepData<T>(data, sr, 0.1f, 0.06f, 1500.0f, 0.3f);
    const auto ptr = reinterpret_cast<const char *>(data.data());
    const auto size = data.size() * sizeof(T);
    QBuffer *buffer = new QBuffer;
    buffer->setData(QByteArray(ptr, size));
    buffer->open(QIODevice::ReadOnly);
    return buffer;
}

static QBuffer *tryGenData(QAudioFormat format) {
    if (!format.isValid()) {
        spdlog::warn("Invalid audio format for beep");
        return nullptr;
    }
    if (format.channelCount() != 1) {
        spdlog::warn("Beep only supports mono audio, got {} channels", format.channelCount());
        return nullptr;
    }
    if (format.byteOrder() != QAudioFormat::LittleEndian) {
        spdlog::warn("Beep only supports little-endian audio");
        return nullptr;
    }
    if (format.codec() != "audio/pcm") {
        spdlog::warn("Beep only supports PCM audio codec");
        return nullptr;
    }
    const auto sr = format.sampleRate();
    switch (format.sampleType()) {
    case QAudioFormat::SignedInt:
        switch (format.sampleSize()) {
        case 8: return genBeepData<qint8>(sr);
        case 16: return genBeepData<qint16>(sr);
        case 32: return genBeepData<qint32>(sr);
        default: spdlog::warn("Unsupported sample size {} for SignedInt", format.sampleSize()); return nullptr;
        }
        break;
    case QAudioFormat::UnSignedInt:
        switch (format.sampleSize()) {
        case 8: return genBeepData<quint8>(sr);
        case 16: return genBeepData<quint16>(sr);
        case 32: return genBeepData<quint32>(sr);
        default: spdlog::warn("Unsupported sample size {} for UnSignedInt", format.sampleSize()); return nullptr;
        }
        break;
    case QAudioFormat::Float:
        switch (format.sampleSize()) {
        case 32: printf("AAA\n"); return genBeepData<float>(sr);
        case 64: return genBeepData<double>(sr);
        default: spdlog::warn("Unsupported sample size {} for Float", format.sampleSize()); return nullptr;
        }
        break;
    default: spdlog::warn("Unsupported sample type for beep"); return nullptr;
    }
}

static QAudioOutput *audioOutput = nullptr;
static QBuffer *buffer = nullptr;
static QMutex beepMutex;

void initBeep() {
    QMutexLocker locker(&beepMutex);
    if (audioOutput) {
        return;
    }

    QAudioDeviceInfo deviceInfo = QAudioDeviceInfo::defaultOutputDevice();
    if (deviceInfo.isNull()) {
        spdlog::warn("No audio output device available for beep");
        return;
    }

    spdlog::info("Trying to use default audio output format for beep");
    QAudioFormat format = deviceInfo.preferredFormat();
    format.setChannelCount(1);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    if (deviceInfo.isFormatSupported(format)) {
        if (buffer = tryGenData(format); buffer) {
            audioOutput = new QAudioOutput(format);
            spdlog::info("Successfully initialized beep with default audio format");
            return;
        }
    }

    spdlog::info("Trying to use nearest audio output format for beep");
    format = deviceInfo.nearestFormat(format);
    format.setChannelCount(1);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    if (deviceInfo.isFormatSupported(format)) {
        if (buffer = tryGenData(format); buffer) {
            audioOutput = new QAudioOutput(format);
            spdlog::info("Successfully initialized beep with nearest audio format");
            return;
        }
    }

    spdlog::error("Failed to initialize beep audio output");
}

// 使用 Qt 的音频输出播放扫描提示音
void playBeep() {
    if (!audioOutput || !buffer) {
        return;
    }

    buffer->seek(0);
    audioOutput->stop();
    audioOutput->start(buffer);
}
