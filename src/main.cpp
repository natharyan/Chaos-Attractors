#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <filesystem>
#include "includes/matrix.h"
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

    sf::Sound sound;

private:
    sf::SoundBuffer buffer;
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
          rotationX(attractor.angles[0][0]),
          rotationY(attractor.angles[0][1]),
          rotationZ(attractor.angles[0][2]),
          audioPlayer(audioPlayer),
          xyswap(attractor.xyswap),
          isTransitioning(false), transitionFrames(0),
          attractor(attractor),
          randrange(attractor.randrange),
          spacepress(false),
          tailon(true),
          menu(true),
          isDragging(false),
          lastMousePos(0, 0),
          tailtoggle(true),
          SCROLL_WAIT_TIME(0.4f),
          MOUSE_WAIT_TIME(0.5f),
          ARROW_KEY_WAIT_TIME(0.4f) {

            if (!font.loadFromFile("font/RobotoMono-Regular.ttf")) {
                std::cerr << "Error loading font" << std::endl;
            }
            titletext.setFont(font);
            titletext.setCharacterSize(15);
            titletext.setFillColor(sf::Color::White);
            titletext.setString("Attractor: " + title);
            titletext.setPosition(10.f, window.getSize().y - 170.0f);

            songTitleText.setFont(font);
            songTitleText.setCharacterSize(15);
            songTitleText.setFillColor(sf::Color::White);
            songTitleText.setPosition(10.f, window.getSize().y - 150.0f);

            angleTextX.setFont(font);
            angleTextX.setCharacterSize(15);
            angleTextX.setFillColor(sf::Color::White);
            angleTextX.setPosition(10.f, window.getSize().y - 130.0f);

            angleTextY.setFont(font);
            angleTextY.setCharacterSize(15);
            angleTextY.setFillColor(sf::Color::White);
            angleTextY.setPosition(10.f, window.getSize().y - 110.0f);

            offsetText.setFont(font);
            offsetText.setCharacterSize(15);
            offsetText.setFillColor(sf::Color::White);
            offsetText.setPosition(10.f, window.getSize().y - 90.0f);

            scaleText.setFont(font);
            scaleText.setCharacterSize(15);
            scaleText.setFillColor(sf::Color::White);
            scaleText.setPosition(10.f, window.getSize().y - 70.0f);

            amplitudeText.setFont(font);
            amplitudeText.setCharacterSize(15);
            amplitudeText.setFillColor(sf::Color::White);
            amplitudeText.setPosition(10.f, window.getSize().y - 50.0f);

            commandsText.setFont(font);
            commandsText.setCharacterSize(15);
            commandsText.setFillColor(sf::Color::White);
            commandsText.setPosition(10.f, window.getSize().y - 30.0f);
            commandsText.setString("Commands: Mouse Drag(rotate along axes), T(toggle tails), Arrow Keys(change screen offset), Scroll(Change scale), Space(pause), R(reset), M(toggle menu), Q(quit)");

            window.setFramerateLimit(60);
        }

    void run(const Attractor& attractor) {
        std::vector<std::vector<float>> points = initializePoints();
        std::vector<std::vector<sf::Vertex>> trails(points.size());
        const size_t maxTrailSize = 80;
        if(dynamic_cast<const AizawaAttractor*>(&attractor)){
            const size_t maxTrailSize = 30;
        } else if(dynamic_cast<const SprottAttractor*>(&attractor)){
            const size_t maxTrailSize = 800;
        } else{
            const size_t maxTrailSize = 40;
        }
        while (window.isOpen()) {
            handleEvents();
            float amplitude = audioPlayer.getAmplitude();
            if(amplitude > 800.0f){
                amplitude = 800.0f;
            }
            std::unique_ptr<Attractor> adjustedattractor;
            if(!spacepress){
                if (dynamic_cast<const LorenzAttractor*>(&attractor)) {
                    float speedFactor = attractor.speedfactor(attractor.defdt, amplitude);
                    if(speedFactor > 0.008f){
                        speedFactor = 0.008f;
                    }
                    adjustedattractor = std::make_unique<LorenzAttractor>(speedFactor);
                } else if (dynamic_cast<const AizawaAttractor*>(&attractor)) {
                    float speedFactor = attractor.speedfactor(attractor.defdt, amplitude);
                    if(speedFactor > 0.1f){
                        speedFactor = 0.1f;
                    }
                    adjustedattractor = std::make_unique<AizawaAttractor>(speedFactor);
                } else if(dynamic_cast<const ThomasAttractor*>(&attractor)){
                    float speedFactor = attractor.speedfactor(attractor.defdt, amplitude);
                    if(speedFactor > 0.3f){
                        speedFactor = 0.3f;
                    }
                    std::cout << speedFactor << std::endl;
                    adjustedattractor = std::make_unique<ThomasAttractor>(speedFactor);
                } else if(dynamic_cast<const HalvorsenAttractor*>(&attractor)){
                    float speedFactor = attractor.speedfactor(attractor.defdt, amplitude);
                    if(speedFactor > 0.3f){
                        speedFactor = 0.3f;
                    }
                    adjustedattractor = std::make_unique<HalvorsenAttractor>(speedFactor);
                } else if(dynamic_cast<const SprottAttractor*>(&attractor)){
                    float speedFactor = attractor.speedfactor(attractor.defdt, amplitude);
                    if(speedFactor > 0.1f){
                        speedFactor = 0.1f;
                    }
                    adjustedattractor = std::make_unique<SprottAttractor>(speedFactor);
                }
            }else{
                if (dynamic_cast<const LorenzAttractor*>(&attractor)) {
                    adjustedattractor = std::make_unique<LorenzAttractor>(0.0f);
                } else if (dynamic_cast<const AizawaAttractor*>(&attractor)) {
                    adjustedattractor = std::make_unique<AizawaAttractor>(0.0f);
                } else if(dynamic_cast<const ThomasAttractor*>(&attractor)){
                    adjustedattractor = std::make_unique<ThomasAttractor>(0.0f);
                } else if(dynamic_cast<const HalvorsenAttractor*>(&attractor)){
                    adjustedattractor = std::make_unique<HalvorsenAttractor>(0.0f);
                } else if(dynamic_cast<const SprottAttractor*>(&attractor)){
                    adjustedattractor = std::make_unique<SprottAttractor>(0.0f);
                }
            }
            updatePoints(*adjustedattractor, points, trails, maxTrailSize);
            render(points, trails);

            songTitleText.setString("Song: " + audioPlayer.getSongTitle());
            angleTextX.setString("Rotation along X-Axis: " + std::to_string(rotationX));
            angleTextY.setString("Rotation along Y-Axis: " + std::to_string(rotationY));
            offsetText.setString("OffsetX: " + std::to_string(offsetX) + " OffsetY: " + std::to_string(offsetY));
            scaleText.setString("Scale: " + std::to_string(scale));
            amplitudeText.setString("Normalized Amplitude: " + std::to_string(std::min(audioPlayer.getCurrentAmplitude() / attractor.maxamplitude, 1.0f)).substr(0, 4));
        }
    }

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

    std::vector<std::vector<float>> initializePoints() {
        std::vector<std::vector<float>> points;
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator(seed);
        std::uniform_real_distribution<float> distribution(-randrange, randrange);
        
        if (dynamic_cast<const LorenzAttractor*>(&attractor)) {
            for (int i = 0; i < 1000; ++i) {
                float x = (i < 500) ? -0.1f : 0.1f;
                points.push_back({
                    x + distribution(generator) * 0.01f,
                    distribution(generator),
                    distribution(generator)
                });
            }
        } else if(dynamic_cast<const AizawaAttractor*>(&attractor)){
            for (int i = 0; i < 500; ++i) {
                points.push_back({
                    distribution(generator),
                    distribution(generator),
                    distribution(generator)
                });
            }
        } else if(dynamic_cast<const ThomasAttractor*>(&attractor)){
            for (int i = 0; i < 800; ++i) {
                points.push_back({
                    distribution(generator),
                    distribution(generator),
                    distribution(generator)
                });
            }
        } else if(dynamic_cast<const HalvorsenAttractor*>(&attractor)){
            for (int i = 0; i < 800; ++i) {
                points.push_back({
                    distribution(generator),
                    distribution(generator),
                    distribution(generator)
                });
            }
        } else if(dynamic_cast<const SprottAttractor*>(&attractor)){
            for (int i = 0; i < 800; ++i) {
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
        static bool isScrolled = false;
        static bool isMouseMoved = false;
        static bool isArrowKeyPressed = false;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || 
                (event.type == sf::Event::KeyPressed && 
                (event.key.code == sf::Keyboard::Q))) {
                window.close();
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isDragging = true;
                    lastMousePos = sf::Mouse::getPosition(window);
                    tailon = false;
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    isDragging = false;
                    isWaitingAfterMouseMove = true;
                    mouseTimer.restart();
                }
            } else if (event.type == sf::Event::MouseMoved) {
                if (isDragging) {
                    sf::Vector2i currentMousePos = sf::Mouse::getPosition(window);
                    sf::Vector2i delta = currentMousePos - lastMousePos;

                    rotationX -= delta.y * 0.006f;
                    rotationY -= delta.x * 0.006f;

                    lastMousePos = currentMousePos;
                    tailon = false;
                    isMouseMoved = true;
                    isWaitingAfterMouseMove = false;
                    mouseTimer.restart();
                }
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                    float zoomFactor = 1.1f;
                    if (event.mouseWheelScroll.delta > 0) {
                        scale *= zoomFactor;
                    } else {
                        scale /= zoomFactor;
                    }
                    tailon = false;
                    isScrolled = true;
                    isWaitingAfterScroll = false;
                    scrollTimer.restart();
                }
            } else if (event.type == sf::Event::KeyPressed) {
                if(event.key.code == sf::Keyboard::Space){
                    if(!spacepress){
                        spacepress = true;
                        audioPlayer.sound.pause();
                    }else{
                        spacepress = false;
                        audioPlayer.sound.play();
                    }
                } else if(event.key.code == sf::Keyboard::T){
                    tailon = !tailon;
                    tailtoggle = tailon;
                } else if(event.key.code == sf::Keyboard::Right)
                {
                    offsetX += 10.0f;
                    tailon = false;
                    isArrowKeyPressed = true;
                    arrowKeyTimer.restart();
                } else if(event.key.code == sf::Keyboard::Left)
                {
                    offsetX -= 10.0f;
                    tailon = false;
                    isArrowKeyPressed = true;
                    arrowKeyTimer.restart();
                }else if(event.key.code == sf::Keyboard::Up)
                {
                    offsetY += 10.0f;
                    tailon = false;
                    isArrowKeyPressed = true;
                    arrowKeyTimer.restart();
                }else if(event.key.code == sf::Keyboard::Down)
                {
                    offsetY -= 10.0f;
                    tailon = false;
                    isArrowKeyPressed = true;
                    arrowKeyTimer.restart();
                } else if(event.key.code == sf::Keyboard::R){
                    rotationX = angles[0][0];
                    rotationY = angles[0][1];
                    offsetX = attractor.offsetX;
                    offsetY = attractor.offsetY;
                    scale = attractor.scale;
                } else if(event.key.code == sf::Keyboard::M){
                    menu = !menu;
                }
            }
        }
        // check if scrolling has stopped
        if (isScrolled && event.type != sf::Event::MouseWheelScrolled) {
            isScrolled = false;
            isWaitingAfterScroll = true;
            scrollTimer.restart();
        }
        // check if mouse movement has stopped
        if (isMouseMoved && event.type != sf::Event::MouseMoved) {
            isMouseMoved = false;
            isWaitingAfterMouseMove = true;
            mouseTimer.restart();
        }
        // check if waiting period after last scroll has elapsed
        if (isWaitingAfterScroll && scrollTimer.getElapsedTime().asSeconds() >= SCROLL_WAIT_TIME) {
            isWaitingAfterScroll = false;
            if (tailtoggle) {
                tailon = true;
            }
        }
        // check if waiting period after last mouse move has elapsed
        if (isWaitingAfterMouseMove && mouseTimer.getElapsedTime().asSeconds() >= MOUSE_WAIT_TIME) {
            isWaitingAfterMouseMove = false;
            if (tailtoggle) {
                tailon = true;
            }
        }
        // check if waiting period after last arrow key press has elapsed
        if (isArrowKeyPressed && arrowKeyTimer.getElapsedTime().asSeconds() >= ARROW_KEY_WAIT_TIME) {
            isArrowKeyPressed = false;
            if (tailtoggle) {
                tailon = true;
            }
        }
    }

    void updatePoints(const Attractor& attractor, std::vector<std::vector<float>>& points, std::vector<std::vector<sf::Vertex>>& trails, size_t maxTrailSize) {
        if (dynamic_cast<const AizawaAttractor*>(&attractor)) {
            const size_t REALLOC_THRESHOLD = 1000; // threshold for reallocation
            const size_t REALLOC_INCREASE = 500;   // number of new elements to add during reallocation
            counter = (counter + 1) % 20;
            if(counter%20 == 0){
                unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
                std::default_random_engine generator(seed);
                std::uniform_real_distribution<float> distribution(-randrange, randrange);
                
                // check if we need to reallocate
                if (points.size() + 5 > points.capacity()) {
                    size_t newCapacity = points.capacity() + REALLOC_INCREASE;
                    points.reserve(newCapacity);
                    trails.reserve(newCapacity);
                }
                for (int i = 0; i < 5; ++i) {
                    points.push_back({
                        distribution(generator),
                        distribution(generator),
                        distribution(generator)
                    });
                    trails.push_back(std::vector<sf::Vertex>());
                }
            }

            // periodically remove excess capacity to save memory
            if (points.size() > REALLOC_THRESHOLD && points.capacity() - points.size() > REALLOC_INCREASE) {
                std::vector<std::vector<float>> temp_points(points.begin(), points.end());
                points.swap(temp_points); // swap points with temp_points which has no extra memory allocation

                std::vector<std::vector<sf::Vertex>> temp_trails(trails.begin(), trails.end());
                trails.swap(temp_trails);
            }
        }

        if(dynamic_cast<const SprottAttractor*>(&attractor)){
            rotationX += 0.0003f;
            rotationY += 0.0001f;
        }

        for (size_t i = 0; i < points.size(); ++i) {
            points[i] = attractor.step(points[i]);
            
            Matrix rotationmatrixX = Matrix(3, 3);
            Matrix rotationmatrixY = Matrix(3, 3);
            Matrix rotationmatrixZ = Matrix(3, 3);

            // Rotation around X-axis
            rotationmatrixX(0, 0) = 1;
            rotationmatrixX(1, 1) = cos(rotationX); rotationmatrixX(1, 2) = -sin(rotationX);
            rotationmatrixX(2, 1) = sin(rotationX); rotationmatrixX(2, 2) = cos(rotationX);

            // Rotation around Y-axis
            rotationmatrixY(0, 0) = cos(rotationY); rotationmatrixY(0, 2) = sin(rotationY);
            rotationmatrixY(1, 1) = 1;
            rotationmatrixY(2, 0) = -sin(rotationY); rotationmatrixY(2, 2) = cos(rotationY);

            // Rotation around Z-axis
            rotationmatrixZ(0, 0) = cos(rotationZ); rotationmatrixZ(0, 1) = -sin(rotationZ);
            rotationmatrixZ(1, 0) = sin(rotationZ); rotationmatrixZ(1, 1) = cos(rotationZ);
            rotationmatrixZ(2, 2) = 1;

            Matrix rotation = matrix_multiplication(matrix_multiplication(rotationmatrixX, rotationmatrixY), rotationmatrixZ);

            Matrix point(3, 1);
            point(0, 0) = points[i][0];
            point(1, 0) = points[i][1];
            point(2, 0) = points[i][2];

            Matrix rotated_2d = matrix_multiplication(rotation, point);

            Matrix projection_matrix(2, 3);
            projection_matrix(0, 0) = 1; projection_matrix(0, 1) = 0; projection_matrix(0, 2) = 0;
            projection_matrix(1, 0) = 0; projection_matrix(1, 1) = 1; projection_matrix(1, 2) = 0;

            Matrix projected2d = matrix_multiplication(projection_matrix, rotated_2d);

            float screenX = projected2d(0, 0) * scale + window.getSize().x / 2.0f - offsetX;
            float screenY = projected2d(1, 0) * scale + window.getSize().y / 2.0f + offsetY;

            sf::Vector2f screenPos(screenX, screenY);
            
            trails[i].push_back(sf::Vertex(screenPos, getColorForAmplitude(audioPlayer.getCurrentAmplitude())));
            if (trails[i].size() > maxTrailSize) {
                trails[i].erase(trails[i].begin());
            }
            if(dynamic_cast<const ThomasAttractor*>(&attractor)){
                for (size_t j = 0; j < trails[i].size(); ++j) {
                    float alpha = static_cast<float>(j) / trails[i].size() * 100.0f;
                    trails[i][j].color.a = static_cast<sf::Uint8>(alpha);
                }
            } else{
                for (size_t j = 0; j < trails[i].size(); ++j) {
                    float alpha = static_cast<float>(j) / trails[i].size() * 70.0f;
                    trails[i][j].color.a = static_cast<sf::Uint8>(alpha);
                }
            }
            
        }
    }

    sf::Color lerpColor(const sf::Color& start, const sf::Color& end, float t) {
        // add alpha value
        return sf::Color(
            static_cast<sf::Uint8>(start.r + t * (end.r - start.r)),
            static_cast<sf::Uint8>(start.g + t * (end.g - start.g)),
            static_cast<sf::Uint8>(start.b + t * (end.b - start.b)),
            160
        );
    }

    sf::Color getColorForAmplitude(float amplitude) {
        float normalizedAmplitude = std::min(amplitude / attractor.maxamplitude, 1.0f);
        // sf::Color startColor(115, 210, 222);
        // sf::Color endColor(216, 17, 89);
        return lerpColor(attractor.startColor, attractor.endColor, normalizedAmplitude);
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

            if(tailon){
                for (const auto& trail : trails) {
                    window.draw(&trail[0], trail.size(), sf::PrimitiveType::LineStrip);
                }
            }

            sf::CircleShape pointShape(1);
            for (const auto& p : points) {
                Matrix rotationmatrixX = Matrix(3, 3);
                Matrix rotationmatrixY = Matrix(3, 3);
                Matrix rotationmatrixZ = Matrix(3, 3);

                // Rotation around X-axis
                rotationmatrixX(0, 0) = 1;
                rotationmatrixX(1, 1) = cos(rotationX); rotationmatrixX(1, 2) = -sin(rotationX);
                rotationmatrixX(2, 1) = sin(rotationX); rotationmatrixX(2, 2) = cos(rotationX);

                // Rotation around Y-axis
                rotationmatrixY(0, 0) = cos(rotationY); rotationmatrixY(0, 2) = sin(rotationY);
                rotationmatrixY(1, 1) = 1;
                rotationmatrixY(2, 0) = -sin(rotationY); rotationmatrixY(2, 2) = cos(rotationY);

                // Rotation around Z-axis
                rotationmatrixZ(0, 0) = cos(rotationZ); rotationmatrixZ(0, 1) = -sin(rotationZ);
                rotationmatrixZ(1, 0) = sin(rotationZ); rotationmatrixZ(1, 1) = cos(rotationZ);
                rotationmatrixZ(2, 2) = 1;

                Matrix rotation = matrix_multiplication(matrix_multiplication(rotationmatrixX, rotationmatrixY), rotationmatrixZ);

                Matrix point(3, 1);
                point(0, 0) = p[0];
                point(1, 0) = p[1];
                point(2, 0) = p[2];

                Matrix rotated_2d = matrix_multiplication(rotation, point);

                Matrix projection_matrix(2, 3);
                projection_matrix(0, 0) = 1; projection_matrix(0, 1) = 0; projection_matrix(0, 2) = 0;
                projection_matrix(1, 0) = 0; projection_matrix(1, 1) = 1; projection_matrix(1, 2) = 0;
                
                Matrix projected2d = matrix_multiplication(projection_matrix, rotated_2d);

                float screenX = projected2d(0, 0) * scale + window.getSize().x / 2.0f - offsetX;
                float screenY = projected2d(1, 0) * scale + window.getSize().y / 2.0f + offsetY;

                pointShape.setPosition(screenX - pointShape.getRadius(), screenY - pointShape.getRadius());
                pointShape.setFillColor(getColorForAmplitude(audioPlayer.getCurrentAmplitude()));
                window.draw(pointShape);
            }

        }
        if(menu){
            titletext.setPosition(10.f, window.getSize().y - 170.0f);
            songTitleText.setPosition(10.f, window.getSize().y - 150.0f);
            window.draw(titletext);
            window.draw(songTitleText);
            window.draw(angleTextX);
            window.draw(angleTextY);
            window.draw(scaleText);
            window.draw(amplitudeText);
            window.draw(commandsText);
            window.draw(offsetText);
            window.display();
        } else{
            titletext.setPosition(10.f, window.getSize().y - 50.0f);
            songTitleText.setPosition(10.f, window.getSize().y - 30.0f);
            window.draw(titletext);
            window.draw(songTitleText);
            window.display();
        }
    }
};

int main() {
    AudioPlayer audioPlayer;
    std::string attractorchoice;
    std::unique_ptr<Attractor> attractor;
    std::string title;
    sf::VideoMode desktopMode = sf::VideoMode::getFullscreenModes()[0];
    std::cout << std::endl << "==== Chaos Attractor Music Visualizer ====" << std::endl;
    std::cout << "Available Attractors:" << std::endl;
    std::cout << "1. Thomas" << std::endl;
    std::cout << "2. Halvorsen" << std::endl;
    std::cout << "3. Sprott" << std::endl;
    std::cout << "4. Aizawa" << std::endl;
    std::cout << "5. Lorenz" << std::endl;
    std::cout << "Enter the name of an attractor: ";
    std::cin >> attractorchoice;

    if(attractorchoice == "Lorenz") {
        attractor = std::make_unique<LorenzAttractor>(lorenz_defdt);
        title = "Lorenz Attractor";
    } else if(attractorchoice == "Aizawa") {
        attractor = std::make_unique<AizawaAttractor>(aizawa_defdt);
        title = "Aizawa Attractor";
    } else if(attractorchoice == "Thomas") {
        attractor = std::make_unique<ThomasAttractor>(thomas_defdt);
        title = "Thomas Attractor";
    } else if(attractorchoice == "Halvorsen") {
        attractor = std::make_unique<HalvorsenAttractor>(halvorsen_defdt);
        title = "Halvorsen Attractor";
    } else if(attractorchoice == "Sprott") {
        attractor = std::make_unique<SprottAttractor>(sprott_defdt);
        title = "Sprott Attractor";
    } else {
        std::cout << "Invalid attractor choice. Please try again.";
        return 1;
    }

    if (!audioPlayer.loadAndPlay(attractor->defaultaudio)) {
        std::cerr << "Error loading audio" << std::endl;
        return 1;
    }

    Visualization vis(desktopMode.width, desktopMode.height, title, audioPlayer, *attractor);
    vis.run(*attractor);
    audioPlayer.sound.stop();

    return 0;
}