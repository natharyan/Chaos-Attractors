#include "visualization.h"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>
#include <cstdint>

#include "attractors/lorenz.h"
#include "attractors/aizawa.h"
#include "attractors/thomas.h"
#include "attractors/halvorsen.h"
#include "attractors/sprott.h"

Visualization::Visualization(int width, int height, const std::string& title,
                             AudioPlayer& audioPlayer, const Attractor& attractor)
    : window(sf::VideoMode::getFullscreenModes()[0], title, sf::State::Fullscreen),
      scale(attractor.scale),
      offsetX(attractor.offsetX),
      offsetY(attractor.offsetY),
      angles(attractor.angles),
      offsetYs(attractor.offsetYs),
      rotationX(attractor.angles[0][0]),
      rotationY(attractor.angles[0][1]),
      rotationZ(attractor.angles[0][2]),
      audioPlayer(audioPlayer),
      font(),
      titletext(font, "", 15),
      songTitleText(font, "", 15),
      angleTextX(font, "", 15),
      angleTextY(font, "", 15),
      scaleText(font, "", 15),
      amplitudeText(font, "", 15),
      commandsText(font, "", 15),
      offsetText(font, "", 15),
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

    if (!font.openFromFile("font/RobotoMono-Regular.ttf")) {
        std::cerr << "Error loading font" << std::endl;
    }

    titletext.setCharacterSize(15);
    titletext.setFillColor(sf::Color::White);
    titletext.setString("Attractor: " + title);
    titletext.setPosition(sf::Vector2f(10.f, window.getSize().y - 170.0f));

    songTitleText.setFont(font);
    songTitleText.setCharacterSize(15);
    songTitleText.setFillColor(sf::Color::White);
    songTitleText.setPosition(sf::Vector2f(10.f, window.getSize().y - 150.0f));

    angleTextX.setFont(font);
    angleTextX.setCharacterSize(15);
    angleTextX.setFillColor(sf::Color::White);
    angleTextX.setPosition(sf::Vector2f(10.f, window.getSize().y - 130.0f));

    angleTextY.setFont(font);
    angleTextY.setCharacterSize(15);
    angleTextY.setFillColor(sf::Color::White);
    angleTextY.setPosition(sf::Vector2f(10.f, window.getSize().y - 110.0f));

    offsetText.setFont(font);
    offsetText.setCharacterSize(15);
    offsetText.setFillColor(sf::Color::White);
    offsetText.setPosition(sf::Vector2f(10.f, window.getSize().y - 90.0f));

    scaleText.setFont(font);
    scaleText.setCharacterSize(15);
    scaleText.setFillColor(sf::Color::White);
    scaleText.setPosition(sf::Vector2f(10.f, window.getSize().y - 70.0f));

    amplitudeText.setFont(font);
    amplitudeText.setCharacterSize(15);
    amplitudeText.setFillColor(sf::Color::White);
    amplitudeText.setPosition(sf::Vector2f(10.f, window.getSize().y - 50.0f));

    commandsText.setFont(font);
    commandsText.setCharacterSize(15);
    commandsText.setFillColor(sf::Color::White);
    commandsText.setPosition(sf::Vector2f(10.f, window.getSize().y - 30.0f));
    commandsText.setString("Commands: Mouse Drag(rotate along axes), T(toggle tails), Arrow Keys(change screen offset), Scroll(Change scale), Space(pause), R(reset), M(toggle menu), Q(quit)");

    window.setFramerateLimit(60);
}

void Visualization::run(const Attractor& attractor) {
    std::vector<std::vector<float>> points = initializePoints();
    std::vector<std::vector<sf::Vertex>> trails(points.size());
    const size_t maxTrailSize = 80;
    if (dynamic_cast<const AizawaAttractor*>(&attractor)) {
        const size_t maxTrailSize = 30;
    } else if (dynamic_cast<const SprottAttractor*>(&attractor)) {
        const size_t maxTrailSize = 800;
    } else if (dynamic_cast<const LorenzAttractor*>(&attractor)) {
        const size_t maxTrailSize = 20;
    } else {
        const size_t maxTrailSize = 40;
    }

    while (window.isOpen()) {
        handleEvents();
        float amplitude = audioPlayer.getAmplitude();
        if (amplitude > 800.0f) {
            amplitude = 800.0f;
        }

        std::unique_ptr<Attractor> adjustedattractor;
        if (!spacepress) {
            if (dynamic_cast<const LorenzAttractor*>(&attractor)) {
                float speedFactor = attractor.speedfactor(attractor.defdt, amplitude);
                if (speedFactor > 0.008f) speedFactor = 0.008f;
                adjustedattractor = std::make_unique<LorenzAttractor>(speedFactor);
            } else if (dynamic_cast<const AizawaAttractor*>(&attractor)) {
                float speedFactor = attractor.speedfactor(attractor.defdt, amplitude);
                if (speedFactor > 0.1f) speedFactor = 0.1f;
                adjustedattractor = std::make_unique<AizawaAttractor>(speedFactor);
            } else if (dynamic_cast<const ThomasAttractor*>(&attractor)) {
                float speedFactor = attractor.speedfactor(attractor.defdt, amplitude);
                if (speedFactor > 0.3f) speedFactor = 0.3f;
                std::cout << speedFactor << std::endl;
                adjustedattractor = std::make_unique<ThomasAttractor>(speedFactor);
            } else if (dynamic_cast<const HalvorsenAttractor*>(&attractor)) {
                float speedFactor = attractor.speedfactor(attractor.defdt, amplitude);
                if (speedFactor > 0.3f) speedFactor = 0.3f;
                adjustedattractor = std::make_unique<HalvorsenAttractor>(speedFactor);
            } else if (dynamic_cast<const SprottAttractor*>(&attractor)) {
                float speedFactor = attractor.speedfactor(attractor.defdt, amplitude);
                if (speedFactor > 0.1f) speedFactor = 0.1f;
                adjustedattractor = std::make_unique<SprottAttractor>(speedFactor);
            }
        } else {
            if (dynamic_cast<const LorenzAttractor*>(&attractor)) {
                adjustedattractor = std::make_unique<LorenzAttractor>(0.0f);
            } else if (dynamic_cast<const AizawaAttractor*>(&attractor)) {
                adjustedattractor = std::make_unique<AizawaAttractor>(0.0f);
            } else if (dynamic_cast<const ThomasAttractor*>(&attractor)) {
                adjustedattractor = std::make_unique<ThomasAttractor>(0.0f);
            } else if (dynamic_cast<const HalvorsenAttractor*>(&attractor)) {
                adjustedattractor = std::make_unique<HalvorsenAttractor>(0.0f);
            } else if (dynamic_cast<const SprottAttractor*>(&attractor)) {
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
        amplitudeText.setString("Normalized Amplitude: " +
            std::to_string(std::min(audioPlayer.getCurrentAmplitude() / attractor.maxamplitude, 1.0f)).substr(0, 4));
    }
}

std::vector<std::vector<float>> Visualization::initializePoints() {
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
    } else if (dynamic_cast<const AizawaAttractor*>(&attractor)) {
        for (int i = 0; i < 200; ++i) {
            points.push_back({ distribution(generator), distribution(generator), distribution(generator) });
        }
    } else if (dynamic_cast<const ThomasAttractor*>(&attractor)) {
        for (int i = 0; i < 800; ++i) {
            points.push_back({ distribution(generator), distribution(generator), distribution(generator) });
        }
    } else if (dynamic_cast<const HalvorsenAttractor*>(&attractor)) {
        for (int i = 0; i < 800; ++i) {
            points.push_back({ distribution(generator), distribution(generator), distribution(generator) });
        }
    } else if (dynamic_cast<const SprottAttractor*>(&attractor)) {
        for (int i = 0; i < 800; ++i) {
            points.push_back({ distribution(generator), distribution(generator), distribution(generator) });
        }
    }
    return points;
}

bool Visualization::isAngleInList(float value, const std::array<float, 4> list) {
    for (float item : list) {
        if (std::abs(item - value) < 0.001f) {
            return true;
        }
    }
    return false;
}

void Visualization::handleEvents() {
    static bool isScrolled = false;
    static bool isMouseMoved = false;
    static bool isArrowKeyPressed = false;

    bool anyWheelEvent = false;
    bool anyMouseMoveEvent = false;
    bool anyArrowKeyEvent = false;

    while (auto eventOpt = window.pollEvent()) {
        const auto& event = *eventOpt;
        if (event.is<sf::Event::Closed>()) {
            window.close();
        } else if (event.is<sf::Event::KeyPressed>()) {
            const auto* key = event.getIf<sf::Event::KeyPressed>();
            if (!key) continue;
            anyArrowKeyEvent = true;
            if (key->code == sf::Keyboard::Key::Q) {
                window.close();
            } else if (key->code == sf::Keyboard::Key::Space) {
                if (!spacepress) {
                    spacepress = true;
                    if (audioPlayer.sound) audioPlayer.sound->pause();
                } else {
                    spacepress = false;
                    if (audioPlayer.sound) audioPlayer.sound->play();
                }
            } else if (key->code == sf::Keyboard::Key::T) {
                tailon = !tailon;
                tailtoggle = tailon;
            } else if (key->code == sf::Keyboard::Key::Right) {
                offsetX += 10.0f;
                tailon = false;
                isArrowKeyPressed = true;
                arrowKeyTimer.restart();
            } else if (key->code == sf::Keyboard::Key::Left) {
                offsetX -= 10.0f;
                tailon = false;
                isArrowKeyPressed = true;
                arrowKeyTimer.restart();
            } else if (key->code == sf::Keyboard::Key::Up) {
                offsetY += 10.0f;
                tailon = false;
                isArrowKeyPressed = true;
                arrowKeyTimer.restart();
            } else if (key->code == sf::Keyboard::Key::Down) {
                offsetY -= 10.0f;
                tailon = false;
                isArrowKeyPressed = true;
                arrowKeyTimer.restart();
            } else if (key->code == sf::Keyboard::Key::R) {
                rotationX = angles[0][0];
                rotationY = angles[0][1];
                offsetX = attractor.offsetX;
                offsetY = attractor.offsetY;
                scale = attractor.scale;
            } else if (key->code == sf::Keyboard::Key::M) {
                menu = !menu;
            }
        } else if (event.is<sf::Event::MouseButtonPressed>()) {
            const auto* mb = event.getIf<sf::Event::MouseButtonPressed>();
            if (!mb) continue;
            if (mb->button == sf::Mouse::Button::Left) {
                isDragging = true;
                lastMousePos = mb->position;
                tailon = false;
            }
        } else if (event.is<sf::Event::MouseButtonReleased>()) {
            const auto* mb = event.getIf<sf::Event::MouseButtonReleased>();
            if (!mb) continue;
            if (mb->button == sf::Mouse::Button::Left) {
                isDragging = false;
                isWaitingAfterMouseMove = true;
                mouseTimer.restart();
            }
        } else if (event.is<sf::Event::MouseMoved>()) {
            const auto* mm = event.getIf<sf::Event::MouseMoved>();
            if (!mm) continue;
            if (isDragging) {
                sf::Vector2i currentMousePos = mm->position;
                sf::Vector2i delta = currentMousePos - lastMousePos;

                rotationX -= delta.y * 0.006f;
                rotationY -= delta.x * 0.006f;

                lastMousePos = currentMousePos;
                tailon = false;
                isMouseMoved = true;
                isWaitingAfterMouseMove = false;
                mouseTimer.restart();
            }
            anyMouseMoveEvent = true;
        } else if (event.is<sf::Event::MouseWheelScrolled>()) {
            const auto* mw = event.getIf<sf::Event::MouseWheelScrolled>();
            if (!mw) continue;
            if (mw->wheel == sf::Mouse::Wheel::Vertical) {
                float zoomFactor = 1.1f;
                if (mw->delta > 0) {
                    scale *= zoomFactor;
                } else {
                    scale /= zoomFactor;
                }
                tailon = false;
                isScrolled = true;
                isWaitingAfterScroll = false;
                scrollTimer.restart();
            }
            anyWheelEvent = true;
        }
    }

    if (isScrolled && !anyWheelEvent) {
        isScrolled = false;
        isWaitingAfterScroll = true;
        scrollTimer.restart();
    }
    if (isMouseMoved && !anyMouseMoveEvent) {
        isMouseMoved = false;
        isWaitingAfterMouseMove = true;
        mouseTimer.restart();
    }
    if (isWaitingAfterScroll && scrollTimer.getElapsedTime().asSeconds() >= SCROLL_WAIT_TIME) {
        isWaitingAfterScroll = false;
        if (tailtoggle) tailon = true;
    }
    if (isWaitingAfterMouseMove && mouseTimer.getElapsedTime().asSeconds() >= MOUSE_WAIT_TIME) {
        isWaitingAfterMouseMove = false;
        if (tailtoggle) tailon = true;
    }
    if (isArrowKeyPressed && arrowKeyTimer.getElapsedTime().asSeconds() >= ARROW_KEY_WAIT_TIME) {
        isArrowKeyPressed = false;
        if (tailtoggle) tailon = true;
    }
}

void Visualization::updatePoints(const Attractor& attractor, std::vector<std::vector<float>>& points,
                                  std::vector<std::vector<sf::Vertex>>& trails, size_t maxTrailSize) {
    if (dynamic_cast<const AizawaAttractor*>(&attractor)) {
        const size_t REALLOC_THRESHOLD = 1000;
        const size_t REALLOC_INCREASE = 500;
        counter = (counter + 1) % 40;
        if (counter % 40 == 0) {
            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            std::default_random_engine generator(seed);
            std::uniform_real_distribution<float> distribution(-10 * randrange, 10 * randrange);

            if (points.size() + 10 > points.capacity()) {
                size_t newCapacity = points.capacity() + REALLOC_INCREASE;
                points.reserve(newCapacity);
                trails.reserve(newCapacity);
            }
            for (int i = 0; i < 10; ++i) {
                points.push_back({ distribution(generator), distribution(generator), distribution(generator) });
                trails.push_back(std::vector<sf::Vertex>());
            }
        }

        if (points.size() > REALLOC_THRESHOLD && points.capacity() - points.size() > REALLOC_INCREASE) {
            std::vector<std::vector<float>> temp_points(points.begin(), points.end());
            points.swap(temp_points);
            std::vector<std::vector<sf::Vertex>> temp_trails(trails.begin(), trails.end());
            trails.swap(temp_trails);
        }
    }

    if (dynamic_cast<const SprottAttractor*>(&attractor)) {
        rotationX += 0.0003f;
        rotationY += 0.0001f;
    }

    for (size_t i = 0; i < points.size(); ++i) {
        points[i] = attractor.step(points[i]);

        Matrix rotationmatrixX = Matrix(3, 3);
        Matrix rotationmatrixY = Matrix(3, 3);
        Matrix rotationmatrixZ = Matrix(3, 3);

        rotationmatrixX(0, 0) = 1;
        rotationmatrixX(1, 1) = cos(rotationX); rotationmatrixX(1, 2) = -sin(rotationX);
        rotationmatrixX(2, 1) = sin(rotationX); rotationmatrixX(2, 2) = cos(rotationX);

        rotationmatrixY(0, 0) = cos(rotationY); rotationmatrixY(0, 2) = sin(rotationY);
        rotationmatrixY(1, 1) = 1;
        rotationmatrixY(2, 0) = -sin(rotationY); rotationmatrixY(2, 2) = cos(rotationY);

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

        trails[i].push_back(sf::Vertex{screenPos, getColorForAmplitude(audioPlayer.getCurrentAmplitude())});
        if (trails[i].size() > maxTrailSize) {
            trails[i].erase(trails[i].begin());
        }

        if (dynamic_cast<const ThomasAttractor*>(&attractor)) {
            for (size_t j = 0; j < trails[i].size(); ++j) {
                float alpha = static_cast<float>(j) / trails[i].size() * 100.0f;
                trails[i][j].color.a = static_cast<std::uint8_t>(alpha);
            }
        } else {
            for (size_t j = 0; j < trails[i].size(); ++j) {
                float alpha = static_cast<float>(j) / trails[i].size() * 70.0f;
                trails[i][j].color.a = static_cast<std::uint8_t>(alpha);
            }
        }
    }
}

sf::Color Visualization::lerpColor(const sf::Color& start, const sf::Color& end, float t) {
    return sf::Color(
        static_cast<std::uint8_t>(start.r + t * (end.r - start.r)),
        static_cast<std::uint8_t>(start.g + t * (end.g - start.g)),
        static_cast<std::uint8_t>(start.b + t * (end.b - start.b)),
        static_cast<std::uint8_t>(160)
    );
}

sf::Color Visualization::getColorForAmplitude(float amplitude) {
    float normalizedAmplitude = std::min(amplitude / attractor.maxamplitude, 1.0f);
    return lerpColor(attractor.startColor, attractor.endColor, normalizedAmplitude);
}

void Visualization::render(const std::vector<std::vector<float>>& points,
                            const std::vector<std::vector<sf::Vertex>>& trails) {
    if (isTransitioning) {
        window.clear(sf::Color::Black);
        transitionFrames--;
        if (transitionFrames <= 0) {
            isTransitioning = false;
        }
    } else {
        window.clear(sf::Color::Black);

        if (tailon) {
            for (const auto& trail : trails) {
                window.draw(&trail[0], trail.size(), sf::PrimitiveType::LineStrip);
            }
        }

        sf::CircleShape pointShape(1);
        for (const auto& p : points) {
            Matrix rotationmatrixX = Matrix(3, 3);
            Matrix rotationmatrixY = Matrix(3, 3);
            Matrix rotationmatrixZ = Matrix(3, 3);

            rotationmatrixX(0, 0) = 1;
            rotationmatrixX(1, 1) = cos(rotationX); rotationmatrixX(1, 2) = -sin(rotationX);
            rotationmatrixX(2, 1) = sin(rotationX); rotationmatrixX(2, 2) = cos(rotationX);

            rotationmatrixY(0, 0) = cos(rotationY); rotationmatrixY(0, 2) = sin(rotationY);
            rotationmatrixY(1, 1) = 1;
            rotationmatrixY(2, 0) = -sin(rotationY); rotationmatrixY(2, 2) = cos(rotationY);

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

            pointShape.setPosition(sf::Vector2f(screenX - pointShape.getRadius(), screenY - pointShape.getRadius()));
            pointShape.setFillColor(getColorForAmplitude(audioPlayer.getCurrentAmplitude()));
            window.draw(pointShape);
        }
    }

    if (menu) {
        titletext.setPosition(sf::Vector2f(10.f, window.getSize().y - 170.0f));
        songTitleText.setPosition(sf::Vector2f(10.f, window.getSize().y - 150.0f));
        window.draw(titletext);
        window.draw(songTitleText);
        window.draw(angleTextX);
        window.draw(angleTextY);
        window.draw(scaleText);
        window.draw(amplitudeText);
        window.draw(commandsText);
        window.draw(offsetText);
        window.display();
    } else {
        window.display();
    }
}
