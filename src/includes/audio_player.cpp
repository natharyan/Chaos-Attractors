#include "audio_player.h"

#include <filesystem>
#include <cmath>

AudioPlayer::AudioPlayer()
    : sound(std::nullopt), buffer(), samples(nullptr), sampleRate(0),
      currentAmplitude(0.0f), maxAmplitude(0.0f) {}

bool AudioPlayer::loadAndPlay(const std::string& path) {
    std::filesystem::path fsPath(path);
    if (fsPath.extension() == ".mp3") {
        if (buffer.loadFromFile(fsPath.string())) {
            samples = reinterpret_cast<const std::int16_t*>(buffer.getSamples());
            sampleRate = buffer.getSampleRate();
            sound.emplace(buffer);
            sound->play();
            songTitle = fsPath.filename().string();
            computeMaxAmplitude();
            return true;
        }
    }
    return false;
}

float AudioPlayer::getAmplitude() {
    size_t sampleCount = buffer.getSampleCount();
    if (sampleCount == 0) return 0.0f;

    float amplitudeSum = 0.0f;
    size_t samplePos = 0;
    if (sound) {
        samplePos = static_cast<size_t>(sound->getPlayingOffset().asSeconds() * sampleRate * 2); // stereo
    }

    for (size_t i = samplePos; i < samplePos + 2048 && i < sampleCount; ++i) {
        amplitudeSum += std::abs(samples[i]);
    }

    float amplitude = amplitudeSum / 2048.0f;
    currentAmplitude = amplitude;
    return amplitude;
}

float AudioPlayer::getCurrentAmplitude() const {
    return currentAmplitude;
}

const std::string& AudioPlayer::getSongTitle() const {
    return songTitle;
}

float AudioPlayer::getMaxAmplitude() const {
    return maxAmplitude;
}

void AudioPlayer::computeMaxAmplitude() {
    this->maxAmplitude = 0.0f;

    size_t sampleCount = buffer.getSampleCount();
    if (!samples || sampleCount == 0) return;

    for (size_t i = 0; i < sampleCount; ++i) {
        float absSample = std::abs(samples[i]);
        if (absSample > this->maxAmplitude) {
            this->maxAmplitude = absSample;
        }
    }
}
