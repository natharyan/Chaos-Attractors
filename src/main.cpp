#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <filesystem>
#include "includes/matrix.h"
#include <RtAudio.h>
#include <fftw3.h>
#include "includes/attractors/attractors.h"
#include "includes/attractors/base_attractor.h"
#include <string>


class AudioPlayer {
public:
    AudioPlayer() : sound(), buffer(), samples(), sampleRate(0), currentAmplitude(0.0f) {}

    bool loadAndPlay(const std::string& path) {
        std::__fs::filesystem::path fsPath(path);
        if (fsPath.extension() == ".mp3") {
            if (buffer.loadFromFile(fsPath.string())) {
                samples = buffer.getSamples();
                sampleRate = buffer.getSampleRate();
                sound.setBuffer(buffer);
                sound.play();
                songTitle = fsPath.filename().string();
                computeMaxAmplitude();
                return true;
            }
        }
        return false;
    }

    float getAmplitude() {
        size_t sampleCount = buffer.getSampleCount();
        if (sampleCount == 0) return 0.0f;

        float amplitudeSum = 0.0f;
        size_t samplePos = sound.getPlayingOffset().asSeconds() * sampleRate * 2; // 2 channels (stereo)

        for (size_t i = samplePos; i < samplePos + 2048 && i < sampleCount; ++i) {
            amplitudeSum += std::abs(samples[i]);
        }

        float amplitude = amplitudeSum / 2048.0f;
        currentAmplitude = amplitude;
        return amplitude;
    }

    float getCurrentAmplitude() const {
        return currentAmplitude;
    }

    const std::string& getSongTitle() const {
        return songTitle;
    }

    float getMaxAmplitude() const {
        return maxAmplitude;
    }

private:
    sf::SoundBuffer buffer;
    sf::Sound sound;
    const sf::Int16* samples;
    unsigned int sampleRate;
    float currentAmplitude;
    std::string songTitle;
    float maxAmplitude;

    void computeMaxAmplitude() {
        float maxAmplitude = 0.0f;

        size_t sampleCount = buffer.getSampleCount();

        for (size_t i = 0; i < sampleCount; ++i) {
            float absSample = std::abs(samples[i]);
            if (absSample > maxAmplitude) {
                maxAmplitude = absSample;
            }
        }
    }
};

class Visualization {
public:
    Visualization(int width, int height, const std::string& title, AudioPlayer& audioPlayer, const Attractor& attractor)
        : window(sf::VideoMode::getFullscreenModes()[0], title, sf::Style::Fullscreen),
          scale(attractor.scale),
          offsetX(attractor.offsetX),
          offsetY(attractor.offsetY),
          angles(attractor.angles),
          offsetYs(attractor.offsetYs),
          angle(attractor.angles[0]),
          audioPlayer(audioPlayer),
          xyswap(attractor.xyswap),
          isTransitioning(false), transitionFrames(0),
          attractor(attractor),
          randrange(attractor.randrange),
          lastadjustedangle(attractor.angles[0]) {

            if (!font.loadFromFile("font/RobotoMono-Regular.ttf")) {
                std::cerr << "Error loading font" << std::endl;
            }
            titletext.setFont(font);
            titletext.setCharacterSize(15);
            titletext.setFillColor(sf::Color::White);
            titletext.setString("Attractor: " + title);
            titletext.setPosition(10.f, window.getSize().y - 90.0f);

            songTitleText.setFont(font);
            songTitleText.setCharacterSize(15);
            songTitleText.setFillColor(sf::Color::White);
            songTitleText.setPosition(10.f, window.getSize().y - 70.0f);

            angleText.setFont(font);
            angleText.setCharacterSize(15);
            angleText.setFillColor(sf::Color::White);
            angleText.setPosition(10.f, window.getSize().y - 50.0f);

            scaleText.setFont(font);
            scaleText.setCharacterSize(15);
            scaleText.setFillColor(sf::Color::White);
            scaleText.setPosition(10.f, window.getSize().y - 30.0f);

            window.setFramerateLimit(60);
        }

    void run(const Attractor& attractor) {
        std::vector<std::vector<float>> points = initializePoints();
        std::vector<std::vector<sf::Vertex>> trails(points.size());
        const size_t maxTrailSize = 30;

        while (window.isOpen()) {
            handleEvents();
            float amplitude = audioPlayer.getAmplitude();
            if(amplitude > 800.0f){
                amplitude = 800.0f;
            }
            float speedFactor = attractor.speedfactor(attractor.defdt, amplitude);
            if(speedFactor > 0.007f){
                speedFactor = 0.007f;
            }
            // std::cout << "Amplitude: " << amplitude << ", SpeedFactor: " << speedFactor << std::endl;
            std::unique_ptr<Attractor> adjustedattractor;
            if (dynamic_cast<const LorenzAttractor*>(&attractor)) {
                adjustedattractor = std::make_unique<LorenzAttractor>(speedFactor);
            } else if (dynamic_cast<const AizawaAttractor*>(&attractor)) {
                adjustedattractor = std::make_unique<AizawaAttractor>(speedFactor);
            }
            updatePoints(*adjustedattractor, points, trails, maxTrailSize);
            render(points, trails);

            songTitleText.setString("Now Playing: " + audioPlayer.getSongTitle());
            angleText.setString("Rotation: " + std::to_string(angle));
            scaleText.setString("Scale: " + std::to_string(scale));
        }
    }

private:
    sf::RenderWindow window;
    float scale;
    float offsetX, offsetY;
    float angle;
    std::array<float, 4> angles;
    std::array<float, 4> offsetYs;
    AudioPlayer& audioPlayer;
    sf::Font font;
    sf::Text titletext;
    sf::Text songTitleText;
    sf::Text angleText;
    sf::Text scaleText;
    bool isTransitioning;
    int transitionFrames;
    bool xyswap;
    const Attractor& attractor;
    float randrange;
    float lastadjustedangle;

    std::vector<std::vector<float>> initializePoints() {
        std::vector<std::vector<float>> points;
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator(seed);
        std::uniform_real_distribution<float> distribution(-randrange, randrange);
        
        if (dynamic_cast<const LorenzAttractor*>(&attractor)) {
            for (int i = 0; i < 500; ++i) {
                float x = (i < 250) ? -0.1f : 0.1f;
                points.push_back({
                    x + distribution(generator) * 0.01f,
                    distribution(generator),
                    distribution(generator)
                });
            }
        } else if (dynamic_cast<const AizawaAttractor*>(&attractor)) {
            for (int i = 0; i < 1000; ++i) {
                points.push_back({
                    distribution(generator),
                    distribution(generator),
                    distribution(generator)
                });
            }
        }
        return points;
    }

bool isAngleInList(float value, const std::array<float, 4> list) {
    for (float item : list) {
        if (std::abs(item - value) < 0.001f) {
            return true;
        }
    }
    return false;
}

void handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed || 
            (event.type == sf::Event::KeyPressed && 
            (event.key.code == sf::Keyboard::Q || event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::BackSpace))) {
            window.close();
        } else if (event.type == sf::Event::KeyPressed) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                    if (isAngleInList(angle, angles)) {
                        // Move to the next angle in the sequence
                        for (size_t i = 0; i < angles.size(); ++i) {
                            if (angles[i] == angle) {
                                size_t nextIndex = (i + 1) % angles.size();
                                angle = angles[nextIndex];
                                offsetY = offsetYs[nextIndex];
                                break;
                            }
                        }
                    } else {
                        // Move to the closest larger angle
                        size_t closestIndex = 0;
                        float minDifference = 2 * M_PI;
                        for (size_t i = 0; i < angles.size(); ++i) {
                            float diff = angles[i] - angle;
                            if (diff > 0 && diff < minDifference) {
                                minDifference = diff;
                                closestIndex = i;
                            }
                        }
                        angle = angles[closestIndex];
                        offsetY = offsetYs[closestIndex];
                    }
                    isTransitioning = true;
                    transitionFrames = 30;
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                    if (isAngleInList(angle, angles)) {
                        // Move to the previous angle in the sequence
                        for (size_t i = 0; i < angles.size(); ++i) {
                            if (angles[i] == angle) {
                                size_t prevIndex = (i - 1 + angles.size()) % angles.size();
                                angle = angles[prevIndex];
                                offsetY = offsetYs[prevIndex];
                                break;
                            }
                        }
                    } else {
                        // Move to the closest smaller angle
                        size_t closestIndex = 0;
                        float minDifference = 2 * M_PI;
                        for (size_t i = 0; i < angles.size(); ++i) {
                            float diff = angle - angles[i];
                            if (diff > 0 && diff < minDifference) {
                                minDifference = diff;
                                closestIndex = i;
                            }
                        }
                        angle = angles[closestIndex];
                        offsetY = offsetYs[closestIndex];
                    }
                    isTransitioning = true;
                    transitionFrames = 30;
                }
            } else {
                if (event.key.code == sf::Keyboard::Right && !sf::Keyboard::isKeyPressed(sf::Keyboard::A) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                    angle = std::fmod(angle + 0.002f, 2 * M_PI);
                    if (angle < 0) {
                        angle += 2 * M_PI;
                    }
                    // Move to the closest smaller angle
                    size_t closestIndex = 0;
                    float minDifference = 2 * M_PI;
                    for (size_t i = 0; i < angles.size(); ++i) {
                        float diff = angle - angles[i];
                        if (diff > 0 && diff < minDifference) {
                            minDifference = diff;
                            closestIndex = i;
                        }
                    }
                    offsetY = offsetYs[closestIndex];
                }

                if (event.key.code == sf::Keyboard::Left && !sf::Keyboard::isKeyPressed(sf::Keyboard::A) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                    angle = std::fmod(angle - 0.002f, 2 * M_PI);
                    if (angle < 0) {
                        angle += 2 * M_PI;
                    }
                    // Move to the closest smaller angle
                    size_t closestIndex = 0;
                    float minDifference = 2 * M_PI;
                    for (size_t i = 0; i < angles.size(); ++i) {
                        float diff = angle - angles[i];
                        if (diff > 0 && diff < minDifference) {
                            minDifference = diff;
                            closestIndex = i;
                        }
                    }
                    offsetY = offsetYs[closestIndex];
                }
            }
            if(event.key.code == sf::Keyboard::Up){
                scale += 0.1f;
                offsetY += 2.2f;
            }
            if(event.key.code == sf::Keyboard::Down){
                scale -= 0.1f;
                offsetY -= 2.2f;
            }
        }
    }
}

    void updatePoints(const Attractor& attractor, std::vector<std::vector<float>>& points, 
                      std::vector<std::vector<sf::Vertex>>& trails, size_t maxTrailSize) {
        for (size_t i = 0; i < points.size(); ++i) {
            points[i] = attractor.step(points[i]);
            
            Matrix rotation = Matrix(3, 3);
            if (xyswap) {
                // Rotate around y-axis
                rotation(0, 0) = cos(angle); rotation(0, 2) = -sin(angle);
                rotation(1, 1) = 1;
                rotation(2, 0) = sin(angle); rotation(2, 2) = cos(angle);
            }else {
                // Rotate around x-axis
                rotation(0, 0) = 1;
                rotation(1, 1) = cos(angle); rotation(1, 2) = -sin(angle);
                rotation(2, 1) = sin(angle); rotation(2, 2) = cos(angle);
            }

            Matrix point(3, 1);
            point(0, 0) = points[i][0];
            point(1, 0) = points[i][1];
            point(2, 0) = points[i][2];

            Matrix rotated_2d = matrix_multiplication(rotation, point);

            Matrix projection_matrix(2, 3);
            projection_matrix(0, 0) = 1; projection_matrix(0, 1) = 0; projection_matrix(0, 2) = 0;
            projection_matrix(1, 0) = 0; projection_matrix(1, 1) = 1; projection_matrix(1, 2) = 0;

            Matrix projected2d = matrix_multiplication(projection_matrix, rotated_2d);

            float screenX = projected2d(0, 0) * scale + window.getSize().x / 2.0f + offsetX;
            float screenY = projected2d(1, 0) * scale + window.getSize().y / 2.0f + offsetY;

            sf::Vector2f screenPos(screenX, screenY);
            
            trails[i].push_back(sf::Vertex(screenPos, getColorForAmplitude(audioPlayer.getCurrentAmplitude())));
            if (trails[i].size() > maxTrailSize) {
                trails[i].erase(trails[i].begin());
            }

            for (size_t j = 0; j < trails[i].size(); ++j) {
                float alpha = static_cast<float>(j) / trails[i].size() * 255.0f;
                trails[i][j].color.a = static_cast<sf::Uint8>(alpha);
            }
        }
    }

    sf::Color lerpColor(const sf::Color& start, const sf::Color& end, float t) {
        return sf::Color(
            static_cast<sf::Uint8>(start.r + t * (end.r - start.r)),
            static_cast<sf::Uint8>(start.g + t * (end.g - start.g)),
            static_cast<sf::Uint8>(start.b + t * (end.b - start.b))
        );
    }

    sf::Color getColorForAmplitude(float amplitude) {
        float normalizedAmplitude = std::min(amplitude / attractor.maxamplitude, 1.0f);
        // sf::Color startColor(0, 0, 255);
        // sf::Color endColor(255, 0, 0);
        sf::Color startColor(100, 223, 223);
        sf::Color endColor(116, 0, 184);
        return lerpColor(startColor, endColor, normalizedAmplitude);
    }

    void render(const std::vector<std::vector<float>>& points, const std::vector<std::vector<sf::Vertex>>& trails) {
        if (isTransitioning) {
            window.clear(sf::Color::Black);
            transitionFrames--;
            if (transitionFrames <= 0) {
                isTransitioning = false;
            }
        } else {
            window.clear(sf::Color::Black);

            for (const auto& trail : trails) {
                window.draw(&trail[0], trail.size(), sf::PrimitiveType::LineStrip);
            }

            sf::CircleShape pointShape(1);
            for (const auto& p : points) {
                Matrix rotation = Matrix(3, 3);
                if (xyswap) {
                    // Rotate around y-axis
                    rotation(0, 0) = cos(angle); rotation(0, 2) = -sin(angle);
                    rotation(1, 1) = 1;
                    rotation(2, 0) = sin(angle); rotation(2, 2) = cos(angle);
                }else {
                    // Rotate around x-axis
                    rotation(0, 0) = 1;
                    rotation(1, 1) = cos(angle); rotation(1, 2) = -sin(angle);
                    rotation(2, 1) = sin(angle); rotation(2, 2) = cos(angle);
                }

                Matrix point(3, 1);
                point(0, 0) = p[0];
                point(1, 0) = p[1];
                point(2, 0) = p[2];

                Matrix rotated_2d = matrix_multiplication(rotation, point);

                Matrix projection_matrix(2, 3);
                projection_matrix(0, 0) = 1; projection_matrix(0, 1) = 0; projection_matrix(0, 2) = 0;
                projection_matrix(1, 0) = 0; projection_matrix(1, 1) = 1; projection_matrix(1, 2) = 0;
                
                Matrix projected2d = matrix_multiplication(projection_matrix, rotated_2d);

                float screenX = projected2d(0, 0) * scale + window.getSize().x / 2.0f + offsetX;
                float screenY = projected2d(1, 0) * scale + window.getSize().y / 2.0f + offsetY;

                pointShape.setPosition(screenX - pointShape.getRadius(), screenY - pointShape.getRadius());
                pointShape.setFillColor(getColorForAmplitude(audioPlayer.getCurrentAmplitude()));
                window.draw(pointShape);
            }

        }
        window.draw(titletext);
        window.draw(songTitleText);
        window.draw(angleText);
        window.draw(scaleText);
        window.display();
    }
};

int main() {
    AudioPlayer audioPlayer;
    sf::VideoMode desktopMode = sf::VideoMode::getFullscreenModes()[0];
    std::cout << "Attractors:" << std::endl;
    std::cout << "1. Lorenz" << std::endl;
    std::cout << "2. Aizawa" << std::endl;
    std::cout << "Enter Attractor: ";
    std::string attractorchoice;
    std::cin >> attractorchoice;
    std::unique_ptr<Attractor> attractor;
    std::string title;

    if(attractorchoice == "Lorenz") {
        attractor = std::make_unique<LorenzAttractor>(lorenz_defdt);
        title = "Lorenz Attractor";
    } else if(attractorchoice == "Aizawa") {
        attractor = std::make_unique<AizawaAttractor>(aizawa_defdt);
        title = "Aizawa Attractor";
    } else {
        std::cerr << "Invalid attractor choice." << std::endl;
        return 1;
    }

    if (!audioPlayer.loadAndPlay(attractor->defaultaudio)) {
        std::cerr << "Error loading audio" << std::endl;
        return 1;
    }

    Visualization vis(desktopMode.width, desktopMode.height, title, audioPlayer, *attractor);
    vis.run(*attractor);

    return 0;
}