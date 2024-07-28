#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include "helpers/matrix.h"

class LorenzAttractor {
public:
    float sigma, rho, beta, dt;
    LorenzAttractor(float sigma, float rho, float beta, float dt)
        : sigma(sigma), rho(rho), beta(beta), dt(dt) {}

    std::vector<float> step(const std::vector<float>& point) const {
        float dx = sigma * (point[1] - point[0]) * dt;
        float dy = (point[0] * (rho - point[2]) - point[1]) * dt;
        float dz = (point[0] * point[1] - beta * point[2]) * dt;
        return {point[0] + dx, point[1] + dy, point[2] + dz};
    }
};

class Visualization {
public:
    Visualization(int width, int height, const std::string& title)
        : window(sf::VideoMode(width, height), title),
          scale(14.0f),
          offsetX(0.0f),
          offsetY(25.0f) {
        window.setFramerateLimit(60);
    }

    void run(const LorenzAttractor& lorenz) {
        std::vector<std::vector<float>> points = initializePoints();
        std::vector<std::vector<sf::Vertex>> trails(points.size());
        const size_t maxTrailSize = 50;

        while (window.isOpen()) {
            handleEvents();
            updatePoints(lorenz, points, trails, maxTrailSize);
            render(points, trails);
        }
    }

private:
    sf::RenderWindow window;
    float scale;
    float offsetX, offsetY;

    std::vector<std::vector<float>> initializePoints() {
        std::vector<std::vector<float>> points;
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator(seed);
        std::uniform_real_distribution<float> distribution(-0.2f, 0.2f);

        for (int i = 0; i < 500; ++i) {
            float x = (i < 250) ? -0.1f : 0.1f;
            points.push_back({
                x + distribution(generator) * 0.01f,
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
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Q)) {
                window.close();
            }
        }
    }

    void updatePoints(const LorenzAttractor& lorenz, std::vector<std::vector<float>>& points, 
                      std::vector<std::vector<sf::Vertex>>& trails, size_t maxTrailSize) {
        for (size_t i = 0; i < points.size(); ++i) {
            points[i] = lorenz.step(points[i]);
            // get scaled coordinates
            sf::Vector2f screenPos = worldToScreen(points[i][0], points[i][2]);
            
            trails[i].push_back(sf::Vertex(screenPos, sf::Color::Cyan));
            if (trails[i].size() > maxTrailSize) {
                trails[i].erase(trails[i].begin());
            }

            // Fade the trail
            for (size_t j = 0; j < trails[i].size(); ++j) {
                float alpha = static_cast<float>(j) / trails[i].size() * 255.0f;
                trails[i][j].color.a = static_cast<sf::Uint8>(alpha);
            }
        }
    }

    void render(const std::vector<std::vector<float>>& points, const std::vector<std::vector<sf::Vertex>>& trails) {
        window.clear(sf::Color::Black);
        
        // Draw trails
        for (const auto& trail : trails) {
            window.draw(&trail[0], trail.size(), sf::PrimitiveType::LineStrip);
        }
        
        // Draw current points
        sf::CircleShape point(1);
        point.setFillColor(sf::Color(0, 255, 255, 200));
        for (const auto& p : points) {
            sf::Vector2f screenPos = worldToScreen(p[0], p[2]);
            point.setPosition(screenPos - sf::Vector2f(point.getRadius(), point.getRadius()));
            window.draw(point);
        }
        
        window.display();
    }

    sf::Vector2f worldToScreen(float x, float y) const {
        float screenX = (x + offsetX) * scale + window.getSize().x / 2.0f;
        float screenY = (-y + offsetY) * scale + window.getSize().y / 2.0f;
        return sf::Vector2f(screenX, screenY);
    }
};

int main() {
    LorenzAttractor lorenz(10.0f, 28.0f, 8.0f / 3.0f, 0.004f);
    Visualization vis(1080, 800, "Lorenz Attractor");
    vis.run(lorenz);
    return 0;
}