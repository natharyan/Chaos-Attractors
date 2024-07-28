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
          scale(15.0f),
          offsetX(100.0f),
          offsetY(0.0f),
          angle(M_PI / 2) {
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
    float angle;

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
            
            trails[i].push_back(sf::Vertex(screenPos, sf::Color::Cyan));
            if (trails[i].size() > maxTrailSize) {
                trails[i].erase(trails[i].begin());
            }

            for (size_t j = 0; j < trails[i].size(); ++j) {
                float alpha = static_cast<float>(j) / trails[i].size() * 255.0f;
                trails[i][j].color.a = static_cast<sf::Uint8>(alpha);
            }
        }
    }

    void render(const std::vector<std::vector<float>>& points, const std::vector<std::vector<sf::Vertex>>& trails) {
        window.clear(sf::Color::Black);
        
        for (const auto& trail : trails) {
            window.draw(&trail[0], trail.size(), sf::PrimitiveType::LineStrip);
        }
        
        sf::CircleShape point(1);
        point.setFillColor(sf::Color(0, 255, 255, 200));
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

            point.setPosition(screenX - point.getRadius(), screenY - point.getRadius());
            window.draw(point);
        }
        
        window.display();
    }
};

int main() {
    LorenzAttractor lorenz(10.0f, 28.0f, 8.0f / 3.0f, 0.004f);
    Visualization vis(1920, 1080, "Lorenz Attractor");
    vis.run(lorenz);
    return 0;
}