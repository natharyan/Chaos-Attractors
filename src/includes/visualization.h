#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

#include "audio_player.h"
#include "attractors/base_attractor.h"
#include "attractors/attractors.h"
#include "matrix.h"

class Visualization {
public:
    Visualization(int width, int height, const std::string& title,
                  AudioPlayer& audioPlayer, const Attractor& attractor);

    void run(const Attractor& attractor);

private:
    sf::RenderWindow window;
    float scale;
    float offsetX, offsetY;
    float angle;
    std::vector<std::array<float, 3>> angles;
    std::array<float, 4> offsetYs;
    AudioPlayer& audioPlayer;
    sf::Font font;
    sf::Text titletext;
    sf::Text songTitleText;
    sf::Text angleTextX;
    sf::Text angleTextY;
    sf::Text scaleText;
    sf::Text amplitudeText;
    sf::Text commandsText;
    sf::Text offsetText;
    bool isTransitioning;
    int transitionFrames;
    bool xyswap;
    const Attractor& attractor;
    float randrange;
    bool spacepress;
    bool tailon;
    bool menu;
    int counter = 0;
    float rotationX, rotationY, rotationZ;
    bool isDragging;
    sf::Vector2i lastMousePos;
    bool tailtoggle;
    sf::Clock scrollTimer;
    bool isWaitingAfterScroll;
    const float SCROLL_WAIT_TIME;
    sf::Clock mouseTimer;
    bool isWaitingAfterMouseMove;
    const float MOUSE_WAIT_TIME;
    sf::Clock arrowKeyTimer;
    const float ARROW_KEY_WAIT_TIME;

    std::vector<std::vector<float>> initializePoints();
    bool isAngleInList(float value, const std::array<float, 4> list);
    void handleEvents();
    void updatePoints(const Attractor& attractor, std::vector<std::vector<float>>& points,
                      std::vector<std::vector<sf::Vertex>>& trails, size_t maxTrailSize);
    sf::Color lerpColor(const sf::Color& start, const sf::Color& end, float t);
    sf::Color getColorForAmplitude(float amplitude);
    void render(const std::vector<std::vector<float>>& points,
                const std::vector<std::vector<sf::Vertex>>& trails);
};
