#pragma once

#include <SFML/Audio.hpp>
#include <string>
#include <optional>
#include <cstdint>

class AudioPlayer {
public:
    AudioPlayer();

    bool loadAndPlay(const std::string& path);

    float getAmplitude();
    float getCurrentAmplitude() const;
    const std::string& getSongTitle() const;
    float getMaxAmplitude() const;

    std::optional<sf::Sound> sound;

private:
    sf::SoundBuffer buffer;
    const std::int16_t* samples;
    unsigned int sampleRate;
    float currentAmplitude;
    std::string songTitle;
    float maxAmplitude;

    void computeMaxAmplitude();
};
