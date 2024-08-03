#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <filesystem>
#include "helpers/matrix.h"
#include <RtAudio.h>
#include <fftw3.h>

class AizawaAttractor {
public:
    float a, b, c, d, e, f, dt;
    AizawaAttractor(float a, float b, float c, float d, float e, float f, float dt)
        : a(a), b(b), c(c), d(d), e(e), f(f), dt(dt) {}

    std::vector<float> step(const std::vector<float>& point) const {
        float x = point[0], y = point[1], z = point[2];
        float dx = ((z - b) * x - d * y) * dt;
        float dy = (d * x + (z - b) * y) * dt;
        float dz = (c + a * z - z * z * z / 3 - x * x + f * z * x * x * x) * dt;
        return {point[0] + dx, point[1] + dy, point[2] + dz};
    }
};

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
                return true;
            }
        }
        return false;
    }

    float getAmplitude() {
        size_t sampleCount = buffer.getSampleCount();
        if (sampleCount == 0) return 0.0f;

        float amplitudeSum = 0.0f;
        size_t samplePos = sound.getPlayingOffset().asSeconds() * sampleRate * 2;

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

private:
    sf::SoundBuffer buffer;
    sf::Sound sound;
    const sf::Int16* samples;
    unsigned int sampleRate;
    float currentAmplitude;
};

class Visualization {
public:
    Visualization(int width, int height, const std::string& title, AudioPlayer& audioPlayer)
        : window(sf::VideoMode::getFullscreenModes()[0], title, sf::Style::Fullscreen),
          scale(330.0f),
          offsetX(0.0f),
          offsetY(0.0f),
          angle(0),
          audioPlayer(audioPlayer) {

            if (!font.loadFromFile("font/RobotoMono-Regular.ttf")) {
                std::cerr << "Error loading font" << std::endl;
            }
            titletext.setFont(font);
            titletext.setCharacterSize(24);
            titletext.setFillColor(sf::Color::White);
            titletext.setString(title);
            titletext.setPosition(10.f, 10.f);
            window.setFramerateLimit(60);
        }

    void run(const AizawaAttractor& aizawa) {
        std::vector<std::vector<float>> points = initializePoints();
        std::vector<std::vector<sf::Vertex>> trails(points.size());
        const size_t maxTrailSize = 50;

        while (window.isOpen()) {
            handleEvents();
            float amplitude = audioPlayer.getAmplitude();
            if(amplitude > 800.0f){
                amplitude = 800.0f;
            }
            float speedFactor = aizawa.dt + 0.000007f * amplitude;
            if(speedFactor > 0.007f){
                speedFactor = 0.007f;
            }
            std::cout << "Amplitude: " << amplitude << ", SpeedFactor: " << speedFactor << std::endl;
            AizawaAttractor adjustedAizawa(aizawa.a, aizawa.b, aizawa.c, aizawa.d, aizawa.e, aizawa.f, speedFactor);
            updatePoints(adjustedAizawa, points, trails, maxTrailSize);
            std::cout << "dt: " << aizawa.dt << std::endl;
            render(points, trails);
        }
    }

private:
    sf::RenderWindow window;
    float scale;
    float offsetX, offsetY;
    float angle;
    AudioPlayer& audioPlayer;
    sf::Font font;
    sf::Text titletext;

    std::vector<std::vector<float>> initializePoints() {
        std::vector<std::vector<float>> points;
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator(seed);
        std::uniform_real_distribution<float> distribution(-0.01f, 0.01f);

        for (int i = 0; i < 1000; ++i) {
            points.push_back({
                distribution(generator),
                distribution(generator),
                distribution(generator)
            });
        }
        return points;
    }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || 
                (event.type == sf::Event::KeyPressed && (event.key.code == sf::Keyboard::Q || event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::BackSpace))) {
                window.close();
            }
        }
    }

    void updatePoints(const AizawaAttractor& aizawa, std::vector<std::vector<float>>& points, 
                      std::vector<std::vector<sf::Vertex>>& trails, size_t maxTrailSize) {
        for (size_t i = 0; i < points.size(); ++i) {
            points[i] = aizawa.step(points[i]);
            
            Matrix rotation_y(3, 3);
            rotation_y(0, 0) = cos(angle); rotation_y(0, 2) = -sin(angle);
            rotation_y(1, 1) = 1;
            rotation_y(2, 0) = sin(angle); rotation_y(2, 2) = cos(angle);

            Matrix point(3, 1);
            point(0, 0) = points[i][0];
            point(1, 0) = points[i][1];
            point(2, 0) = points[i][2];

            Matrix rotated_2d = matrix_multiplication(rotation_y, point);

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
        float normalizedAmplitude = std::min(amplitude / 8000.0f, 1.0f);
        sf::Color startColor(0, 0, 255);
        sf::Color endColor(255, 0, 0);
        return lerpColor(startColor, endColor, normalizedAmplitude);
    }

    void render(const std::vector<std::vector<float>>& points, const std::vector<std::vector<sf::Vertex>>& trails) {
        window.clear(sf::Color::Black);

        for (const auto& trail : trails) {
            window.draw(&trail[0], trail.size(), sf::PrimitiveType::LineStrip);
        }

        sf::CircleShape pointShape(1);
        for (const auto& p : points) {
            Matrix rotation_y(3, 3);
            rotation_y(0, 0) = cos(angle); rotation_y(0, 2) = -sin(angle);
            rotation_y(1, 1) = 1;
            rotation_y(2, 0) = sin(angle); rotation_y(2, 2) = cos(angle);

            Matrix point_matrix(3, 1);
            point_matrix(0, 0) = p[0];
            point_matrix(1, 0) = p[1];
            point_matrix(2, 0) = p[2];

            Matrix rotated_2d = matrix_multiplication(rotation_y, point_matrix);

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

        window.draw(titletext);
        window.display();
    }
};

int main() {
    AudioPlayer audioPlayer;
    if (!audioPlayer.loadAndPlay("audio/Clair De Lune 2009.mp3")) {
        std::cerr << "Failed to load and play audio file." << std::endl;
    }
    sf::VideoMode desktopMode = sf::VideoMode::getFullscreenModes()[0];
    AizawaAttractor aizawa(0.95f, 0.7f, 0.6f, 3.5f, 0.25f, 0.1f, 0.3f);
    Visualization vis(desktopMode.width, desktopMode.height, "Aizawa Attractor", audioPlayer);
    vis.run(aizawa);
    return 0;
}