#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <random>
#include <sstream>
#include <cassert>
#include <fstream>
#include <cfloat>
#include <cmath>
#include "resource.h"

// Global constants
static const int num_params = 18;
static const int iters = 800;
static const int steps_per_frame = 500;
static const double delta_per_step = 1e-5;
static const double delta_minimum = 1e-7;
static const double t_start = -3.0;
static const double t_end = 3.0;
static const bool fullscreen = false;

// Global variables
static int window_w = 1600;
static int window_h = 900;
static int window_bits = 32; // macOS typically uses 32-bit color depth
static float plot_scale = 0.25f;
static float plot_x = 0.0f;
static float plot_y = 0.0f;
static std::mt19937 rand_gen;
static sf::Font font;
static sf::Text equ_text;
static std::string equ_code;
static sf::RectangleShape equ_box;
static sf::Text t_text;
static sf::RectangleShape t_box;

static sf::Color GetRandColor(int i) {
    i += 1;
    int r = std::min(255, 50 + (i * 11909) % 256);
    int g = std::min(255, 50 + (i * 52973) % 256);
    int b = std::min(255, 50 + (i * 44111) % 256);
    return sf::Color(r, g, b, 16);
}

static sf::Vector2f ToScreen(double x, double y) {
    const float s = plot_scale * float(window_h / 2);
    const float nx = float(window_w) * 0.5f + (float(x) - plot_x) * s;
    const float ny = float(window_h) * 0.5f + (float(y) - plot_y) * s;
    return sf::Vector2f(nx, ny);
}

static void RandParams(double* params) {
    std::uniform_int_distribution<int> rand_int(0, 3);
    for (int i = 0; i < num_params; ++i) {
        const int r = rand_int(rand_gen);
        if (r == 0) {
            params[i] = 1.0f;
        } else if (r == 1) {
            params[i] = -1.0f;
        } else {
            params[i] = 0.0f;
        }
    }
}

static std::string ParamsToString(const double* params) {
    const char base27[] = "_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static_assert(num_params % 3 == 0, "Params must be a multiple of 3");
    int a = 0;
    int n = 0;
    std::string result;
    for (int i = 0; i < num_params; ++i) {
        a = a * 3 + int(params[i]) + 1;
        n += 1;
        if (n == 3) {
            result += base27[a];
            a = 0;
            n = 0;
        }
    }
    return result;
}

static void StringToParams(const std::string& str, double* params) {
    for (int i = 0; i < num_params / 3; ++i) {
        int a = 0;
        const char c = (i < str.length() ? str[i] : '_');
        if (c >= 'A' && c <= 'Z') {
            a = int(c - 'A') + 1;
        } else if (c >= 'a' && c <= 'z') {
            a = int(c - 'a') + 1;
        }
        params[i * 3 + 2] = double(a % 3) - 1.0;
        a /= 3;
        params[i * 3 + 1] = double(a % 3) - 1.0;
        a /= 3;
        params[i * 3 + 0] = double(a % 3) - 1.0;
    }
}

static sf::RectangleShape MakeBoundsShape(const sf::Text& text) {
    sf::RectangleShape blackBox;
    const sf::FloatRect textBounds = text.getGlobalBounds();
    blackBox.setPosition(textBounds.left, textBounds.top);
    blackBox.setSize(sf::Vector2f(textBounds.width, textBounds.height));
    blackBox.setFillColor(sf::Color::Black);
    return blackBox;
}

#define SIGN_OR_SKIP(i, x) \
if (params[i] != 0.0) { \
    if (isFirst) { \
        if (params[i] == -1.0) ss << "-"; \
    } else { \
        if (params[i] == -1.0) ss << " - "; \
        else ss << " + "; \
    } \
    ss << x; \
    isFirst = false; \
}
static std::string MakeEquationStr(double* params) {
    std::stringstream ss;
    bool isFirst = true;
    SIGN_OR_SKIP(0, "x\u00b2");
    SIGN_OR_SKIP(1, "y\u00b2");
    SIGN_OR_SKIP(2, "t\u00b2");
    SIGN_OR_SKIP(3, "xy");
    SIGN_OR_SKIP(4, "xt");
    SIGN_OR_SKIP(5, "yt");
    SIGN_OR_SKIP(6, "x");
    SIGN_OR_SKIP(7, "y");
    SIGN_OR_SKIP(8, "t");
    return ss.str();
}

static void ResetPlot() {
    plot_scale = 0.25f;
    plot_x = 0.0f;
    plot_y = 0.0f;
}

static void GenerateNew(sf::RenderWindow& window, double& t, double* params) {
    t = t_start;
    equ_code = ParamsToString(params);
    const std::string equation_str =
        "x' = " + MakeEquationStr(params) + "\n"
        "y' = " + MakeEquationStr(params + num_params / 2) + "\n"
        "Code: " + equ_code;
    equ_text.setCharacterSize(30);
    equ_text.setFont(font);
    equ_text.setString(equation_str);
    equ_text.setFillColor(sf::Color::White);
    equ_text.setPosition(10.0f, 10.0f);
    equ_box = MakeBoundsShape(equ_text);
    window.clear();
}

static void MakeTText(double t) {
    t_text.setCharacterSize(30);
    t_text.setFont(font);
    t_text.setString("t = " + std::to_string(t));
    t_text.setFillColor(sf::Color::White);
    t_text.setPosition(window_w - 200.0f, 10.0f);
    t_box = MakeBoundsShape(t_text);
}

static void CreateRenderWindow(sf::RenderWindow& window) {
    // Create the window
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 8;
    settings.majorVersion = 3;
    settings.minorVersion = 0;

    const sf::VideoMode screenSize(window_w, window_h, window_bits);
    window.create(screenSize, "Chaos Equations", (fullscreen ? sf::Style::Fullscreen : sf::Style::Close), settings);
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
}

static void CenterPlot(const std::vector<sf::Vector2f>& history) {
    float min_x = FLT_MAX;
    float max_x = -FLT_MAX;
    float min_y = FLT_MAX;
    float max_y = -FLT_MAX;
    for (size_t i = 0; i < history.size(); ++i) {
        min_x = std::fmin(min_x, history[i].x);
        max_x = std::fmax(max_x, history[i].x);
        min_y = std::fmin(min_y, history[i].y);
        max_y = std::fmax(max_y, history[i].y);
    }
    max_x = std::fmin(max_x, 4.0f);
    max_y = std::fmin(max_y, 4.0f);
    min_x = std::fmax(min_x, -4.0f);
    min_y = std::fmax(min_y, -4.0f);
    plot_x = (max_x + min_x) * 0.5f;
    plot_y = (max_y + min_y) * 0.5f;
    plot_scale = 1.0f / std::fmax((max_x - min_x), (max_y - min_y));
}


int main(int argc, char *argv[]) {
  std::cout << "=========================================================" << std::endl;
  std::cout << std::endl;
  std::cout << "                      Chaos Equations" << std::endl;
  std::cout << std::endl;
  std::cout << "    These are plots of random recursive equations, which" << std::endl;
  std::cout << "often produce chaos, and results in beautiful patterns." << std::endl;
  std::cout << "For every time t, a point (x,y) is initialized to (t,t)." << std::endl;
  std::cout << "The equation is applied to the point many times, and each" << std::endl;
  std::cout << "iteration is drawn in a unique color." << std::endl;
  std::cout << std::endl;
  std::cout << "=========================================================" << std::endl;
  std::cout << std::endl;
  std::cout << "Controls:" << std::endl;
  std::cout << "      'A' - Automatic Mode (randomize equations)" << std::endl;
  std::cout << "      'R' - Repeat Mode (keep same equation)" << std::endl;
  std::cout << std::endl;
  std::cout << "      'C' - Center points" << std::endl;
  std::cout << "      'D' - Dot size Toggle" << std::endl;
  std::cout << "      'I' - Iteration Limit Toggle" << std::endl;
  std::cout << "      'T' - Trail Toggle" << std::endl;
  std::cout << std::endl;
  std::cout << "      'P' - Pause" << std::endl;
  std::cout << " 'LShift' - Slow Down" << std::endl;
  std::cout << " 'RShift' - Speed Up" << std::endl;
  std::cout << "  'Space' - Reverse" << std::endl;
  std::cout << std::endl;
  std::cout << "     'N' - New Equation (random)" << std::endl;
  std::cout << "     'L' - Load Equation" << std::endl;
  std::cout << "     'S' - Save Equation" << std::endl;
  std::cout << std::endl;

  // Setup the window
    sf::RenderWindow window;
    CreateRenderWindow(window);
    if (!font.loadFromFile("Roboto-Regular.ttf")) {
        std::cerr << "Failed to load font" << std::endl;
        return 1;
    }
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    // Initialize random number generator
    std::random_device rd;
    rand_gen.seed(rd());

    double params[num_params];
    std::uniform_real_distribution<double> rand_double(-1.0, 1.0);
    for (int i = 0; i < num_params; ++i) {
        params[i] = rand_double(rand_gen);
    }

    double t;
    GenerateNew(window, t, params);
    MakeTText(t);

    std::vector<sf::Vector2f> history;
    sf::VertexArray lines(sf::LineStrip);

    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
                if (event.key.code == sf::Keyboard::Space) {
                    ResetPlot();
                    GenerateNew(window, t, params);
                    MakeTText(t);
                    history.clear();
                    lines.clear();
                    continue;
                }
            }
        }

        for (int i = 0; i < steps_per_frame; ++i) {
            double params0[num_params];
            double x = 0;
            double y = 0;
            double t0 = t;
            for (int i = 0; i < num_params; ++i) {
                params0[i] = params[i];
            }
            for (int iter = 0; iter < iters; ++iter) {
                x += params0[0] * x * x + params0[3] * x * y + params0[4] * x * t0 + params0[6] * x + params0[8] * t0;
                y += params0[1] * y * y + params0[3] * x * y + params0[5] * y * t0 + params0[7] * y + params0[8] * t0;
                t0 += delta_per_step;
            }
            const sf::Vector2f pt = ToScreen(x, y);
            history.push_back(pt);
            lines.append(sf::Vertex(pt, GetRandColor(0)));
            if (history.size() >= 2) {
                const sf::Vector2f& p0 = history[history.size() - 2];
                const sf::Vector2f& p1 = history[history.size() - 1];
                const float d = std::sqrt((p1.x - p0.x) * (p1.x - p0.x) + (p1.y - p0.y) * (p1.y - p0.y));
                if (d >= 2) {
                    lines.append(sf::Vertex(p1, GetRandColor(0)));
                }
            }
            t += delta_per_step;
        }

        if (history.size() > 1000) {
            history.erase(history.begin(), history.begin() + history.size() - 1000);
        }

        CenterPlot(history);

        window.clear();
        window.draw(lines);
        window.draw(equ_box);
        window.draw(equ_text);
        window.draw(t_box);
        window.draw(t_text);
        window.display();
    }

    return 0;
}