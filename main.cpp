#include "dublin.h"
#include <SFML/Graphics.hpp>
#include <cmath>

static const float PI = 3.14159265f;

void drawTurningCircles(sf::RenderWindow& window, float x, float y, float deg, float R, sf::Color color) {
    float rad = deg * PI / 180.f;
    // perpendicular right: (sin(rad), -cos(rad)), left: (-sin(rad), cos(rad))
    float px = std::sin(rad);
    float py = -std::cos(rad);

    sf::CircleShape circle(R);
    circle.setFillColor(sf::Color::Transparent);
    circle.setOutlineColor(color);
    circle.setOutlineThickness(1.f);
    circle.setOrigin(R, R);

    circle.setPosition(x + R * px, y + R * py);
    window.draw(circle);

    circle.setPosition(x - R * px, y - R * py);
    window.draw(circle);
}

void drawArrow(sf::RenderWindow& window, float x, float y, float deg, sf::Color color) {
    float rad = deg * PI / 180.f;
    float len = 30.f;
    float ex = x + len * std::cos(rad);
    float ey = y + len * std::sin(rad);

    sf::Vertex line[] = {
        sf::Vertex(sf::Vector2f(x, y), color),
        sf::Vertex(sf::Vector2f(ex, ey), color)
    };
    window.draw(line, 2, sf::Lines);

    float headLen = 10.f;
    float headAngle = 0.4f;
    sf::ConvexShape head(3);
    head.setPoint(0, sf::Vector2f(ex, ey));
    head.setPoint(1, sf::Vector2f(ex - headLen * std::cos(rad - headAngle),
                                   ey - headLen * std::sin(rad - headAngle)));
    head.setPoint(2, sf::Vector2f(ex - headLen * std::cos(rad + headAngle),
                                   ey - headLen * std::sin(rad + headAngle)));
    head.setFillColor(color);
    window.draw(head);
}

int main() {
    Point start = {0, 0, 0.f};
    Point dest  = {0, 0, 0.f};
    bool startSet = false;
    bool destSet  = false;
    bool draggingStart = false;
    bool draggingDest  = false;

    sf::CircleShape src(4.f);
    sf::CircleShape dst(4.f);

    float R = 50.f;

    sf::RenderWindow window(sf::VideoMode(1280, 800), "Dubins Visualizer");
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                window.close();

            if (event.type == sf::Event::MouseWheelScrolled) {
                R += event.mouseWheelScroll.delta * 5.f;
                if (R < 5.f) R = 5.f;
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    start.x = event.mouseButton.x;
                    start.y = event.mouseButton.y;
                    start.deg = 0.f;
                    draggingStart = true;
                    startSet = true;
                }
                if (event.mouseButton.button == sf::Mouse::Right) {
                    dest.x = event.mouseButton.x;
                    dest.y = event.mouseButton.y;
                    dest.deg = 0.f;
                    draggingDest = true;
                    destSet = true;
                }
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left && draggingStart) {
                    float dx = event.mouseButton.x - start.x;
                    float dy = event.mouseButton.y - start.y;
                    if (std::abs(dx) > 2.f || std::abs(dy) > 2.f)
                        start.deg = std::atan2(dy, dx) * 180.f / PI;
                    draggingStart = false;
                }
                if (event.mouseButton.button == sf::Mouse::Right && draggingDest) {
                    float dx = event.mouseButton.x - dest.x;
                    float dy = event.mouseButton.y - dest.y;
                    if (std::abs(dx) > 2.f || std::abs(dy) > 2.f)
                        dest.deg = std::atan2(dy, dx) * 180.f / PI;
                    draggingDest = false;
                }
            }
        }

        sf::Vector2i mouse = sf::Mouse::getPosition(window);

        window.clear(sf::Color(30, 30, 40));

        if (startSet) {
            src.setOrigin(4.f, 4.f);
            src.setPosition(start.x, start.y);
            src.setFillColor(sf::Color::White);
            window.draw(src);

            float deg = start.deg;
            if (draggingStart) {
                float dx = mouse.x - start.x;
                float dy = mouse.y - start.y;
                if (std::abs(dx) > 2.f || std::abs(dy) > 2.f)
                    deg = std::atan2(dy, dx) * 180.f / PI;
            }
            drawArrow(window, start.x, start.y, deg, sf::Color::White);
            drawTurningCircles(window, start.x, start.y, deg, R, sf::Color::White);
        }

        if (destSet) {
            dst.setOrigin(4.f, 4.f);
            dst.setPosition(dest.x, dest.y);
            dst.setFillColor(sf::Color::Red);
            window.draw(dst);

            float deg = dest.deg;
            if (draggingDest) {
                float dx = mouse.x - dest.x;
                float dy = mouse.y - dest.y;
                if (std::abs(dx) > 2.f || std::abs(dy) > 2.f)
                    deg = std::atan2(dy, dx) * 180.f / PI;
            }
            drawArrow(window, dest.x, dest.y, deg, sf::Color::Red);
            drawTurningCircles(window, dest.x, dest.y, deg, R, sf::Color::Red);
        }

        window.display();
    }

    return 0;
}
