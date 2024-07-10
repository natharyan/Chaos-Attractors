#include <SFML/Graphics.hpp>
#include <iostream>

// give me simple 2 dimensional but beatiful plot, use the function to return the new coordinates every it is called with respect to the current coordiantes
// use a different plot from lorenz attractor

class VanDerPolOscillator{
public:
    float mu, dt;
    VanDerPolOscillator(float mu, float dt) {
        this->mu = mu;
        this->dt = dt;
    }

    sf::Vector2f VanDerPol(sf::Vector2f point) {
        float dx = (point.y)*dt;
        float dy = (mu*(1 - point.x*point.x)*point.y - point.x)*dt;
        return sf::Vector2f(point.x + dx, point.y + dy);
    }
};

class LorenzAttractor {
public:
    float sigma, rho, beta, dt;
    LorenzAttractor(float sigma, float rho, float beta, float dt) {
        this->sigma = sigma;
        this->rho = rho;
        this->beta = beta;
        this->dt = dt;
    }

    // yx, yz, xz
    sf::Vector2f Lorenz(sf::Vector2f point, int plane = 0) {
        float dx = (sigma*(point.y - point.x))*dt;
        // float dy = (point.x*(rho - point.z) - point.y)*dt;
        // float dz = (point.x*point.y - beta*point.z)*dt;
        
        // return sf::Vector2f(point.x + dx, point.y + dy);
        return sf::Vector2f(0,0);
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 800), "SFML Coordinates");
    window.setFramerateLimit(60);

    float centerX = window.getSize().x / 2;
    float centerY = window.getSize().y / 2;
    
    // create point1 in three dimensions
    sf::CircleShape point1(5);
    point1.setFillColor(sf::Color::Red);
    point1.setPosition(centerX, centerY);

    LorenzAttractor lorenz = LorenzAttractor(10,28,8/3,0.02);

    VanDerPolOscillator vanderpol = VanDerPolOscillator(2,0.02);

    sf::Vector2f oscillatorState(0.1, 0.1); // Initial state of the oscillator
    float scale = 100.0f;
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.key.code == sf::Keyboard::Escape)
            {
                window.close();
            }
        }
        oscillatorState = vanderpol.VanDerPol(oscillatorState);
        
         // Scale and center the point position
        float x = centerX + oscillatorState.x * scale;
        float y = centerY - oscillatorState.y * scale; // Subtract to invert y-axis

        point1.setPosition(x, y);
        std::cout << "x: " << oscillatorState.x << " y: " << oscillatorState.y << std::endl;

        window.clear(sf::Color::Black);

        window.draw(point1);

        window.display();
    }

    return 0;
}