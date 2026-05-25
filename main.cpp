#include "dublin.h"
#include "dubins_paths.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

static const float PI = 3.14159265f;

static float mod2pi(float angle) {
    angle = std::fmod(angle, 2.f * PI);
    if (angle < 0.f)
        angle += 2.f * PI;
    return angle;
}

static sf::Vector2f toWorld(float lx, float ly, float thetaLineMath, float sx, float sy, float R) {
    float wx = lx * R;
    float wy = ly * R;
    float c = std::cos(thetaLineMath);
    float s = std::sin(thetaLineMath);
    float worldXMath = wx * c - wy * s;
    float worldYMath = wx * s + wy * c;
    return sf::Vector2f(sx + worldXMath, sy - worldYMath);
}

static void applySegment(SegmentType type, float len, float& x, float& y, float& theta) {
    switch (type) {
    case SegmentType::Left:
        x += std::sin(theta + len) - std::sin(theta);
        y += -std::cos(theta + len) + std::cos(theta);
        theta += len;
        break;
    case SegmentType::Right:
        x += -std::sin(theta - len) + std::sin(theta);
        y += std::cos(theta - len) - std::cos(theta);
        theta -= len;
        break;
    case SegmentType::Straight:
        x += std::cos(theta) * len;
        y += std::sin(theta) * len;
        break;
    }
}

static std::vector<sf::Vector2f> samplePath(const Path& path, Point start, Point end, float R,
                                            int stepsPerSegment = 32) {
    std::vector<sf::Vector2f> points;
    if (path.segments.size() != 3)
        return points;

    float dx = static_cast<float>(end.x - start.x);
    float dyMath = -static_cast<float>(end.y - start.y);
    float thetaLine = std::atan2(dyMath, dx);
    float alpha = mod2pi(-start.deg * PI / 180.f - thetaLine);

    float x = 0.f;
    float y = 0.f;
    float theta = alpha;
    points.push_back(toWorld(x, y, thetaLine, start.x, start.y, R));

    for (const Segment& seg : path.segments) {
        float len = static_cast<float>(seg.length);
        int steps = std::max(2, stepsPerSegment);
        for (int i = 1; i <= steps; ++i) {
            float partial = len * static_cast<float>(i) / static_cast<float>(steps);
            float sx = x;
            float sy = y;
            float st = theta;
            applySegment(seg.type, partial, sx, sy, st);
            points.push_back(toWorld(sx, sy, thetaLine, start.x, start.y, R));
        }
        applySegment(seg.type, len, x, y, theta);
    }

    return points;
}

void drawTurningCircles(sf::RenderWindow& window, float x, float y, float deg, float R,
                        sf::Color color) {
    float rad = deg * PI / 180.f;
    float px = std::sin(rad);
    float py = -std::cos(rad);

    sf::CircleShape circle(R);
    circle.setFillColor(sf::Color::Transparent);
    circle.setOutlineColor(color);
    circle.setOutlineThickness(1.f);
    circle.setOrigin({R, R});

    circle.setPosition({x + R * px, y + R * py});
    window.draw(circle);

    circle.setPosition({x - R * px, y - R * py});
    window.draw(circle);
}

void drawArrow(sf::RenderWindow& window, float x, float y, float deg, sf::Color color) {
    float rad = deg * PI / 180.f;
    float len = 30.f;
    float ex = x + len * std::cos(rad);
    float ey = y + len * std::sin(rad);

    sf::Vertex line[] = {
        {{x, y}, color},
        {{ex, ey}, color}
    };
    window.draw(line, 2, sf::PrimitiveType::Lines);

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

void drawPath(sf::RenderWindow& window, const std::vector<sf::Vector2f>& points, sf::Color color) {
    if (points.size() < 2)
        return;

    sf::VertexArray strip(sf::PrimitiveType::LineStrip, points.size());
    for (std::size_t i = 0; i < points.size(); ++i) {
        strip[i].position = points[i];
        strip[i].color = color;
    }
    window.draw(strip);
}

static bool loadFont(sf::Font& font) {
    const char* paths[] = {
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
    };
    for (const char* path : paths) {
        if (font.openFromFile(path))
            return true;
    }
    return false;
}

int main() {
    Point start = {0, 0, 0.f};
    Point dest  = {0, 0, 0.f};
    bool startSet = false;
    bool destSet  = false;
    bool draggingStart = false;
    bool draggingDest  = false;
    int selectedRank = 1;
    bool showAll = false;

    const sf::Color rankColors[6] = {
        sf::Color(80, 220, 120),
        sf::Color(80, 200, 240),
        sf::Color(240, 220, 80),
        sf::Color(240, 110, 220),
        sf::Color(255, 160, 60),
        sf::Color(160, 170, 255),
    };

    sf::CircleShape src(4.f);
    sf::CircleShape dst(4.f);

    int R = 50;

    sf::Font font;
    bool hasFont = loadFont(font);
    sf::Text label(font, "", 18);
    if (hasFont) {
        label.setFillColor(sf::Color::White);
    }

    const float viewW = 1280.f;
    const float viewH = 800.f;
    sf::RenderWindow window(sf::VideoMode({static_cast<unsigned>(viewW),
                                            static_cast<unsigned>(viewH)}),
                            "Dubins Visualizer");
    window.setFramerateLimit(60);
    window.setView(sf::View(sf::FloatRect({0.f, 0.f}, {viewW, viewH})));

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape)
                    window.close();

                if (key->code >= sf::Keyboard::Key::Num1 &&
                    key->code <= sf::Keyboard::Key::Num6) {
                    selectedRank = 1 + static_cast<int>(key->code) -
                                   static_cast<int>(sf::Keyboard::Key::Num1);
                }

                if (key->code == sf::Keyboard::Key::Grave) {
                    showAll = !showAll;
                }
            }

            if (const auto* scroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
                R += static_cast<int>(scroll->delta * 5.f);
                if (R < 5)
                    R = 5;
            }

            if (const auto* press = event->getIf<sf::Event::MouseButtonPressed>()) {
                sf::Vector2f wp = window.mapPixelToCoords(press->position);
                if (press->button == sf::Mouse::Button::Left) {
                    start.x = static_cast<int>(wp.x);
                    start.y = static_cast<int>(wp.y);
                    start.deg = 0.f;
                    draggingStart = true;
                    startSet = true;
                }
                if (press->button == sf::Mouse::Button::Right) {
                    dest.x = static_cast<int>(wp.x);
                    dest.y = static_cast<int>(wp.y);
                    dest.deg = 0.f;
                    draggingDest = true;
                    destSet = true;
                }
            }

            if (const auto* release = event->getIf<sf::Event::MouseButtonReleased>()) {
                sf::Vector2f wp = window.mapPixelToCoords(release->position);
                if (release->button == sf::Mouse::Button::Left && draggingStart) {
                    float dx = wp.x - start.x;
                    float dy = wp.y - start.y;
                    if (std::abs(dx) > 2.f || std::abs(dy) > 2.f)
                        start.deg = std::atan2(dy, dx) * 180.f / PI;
                    draggingStart = false;
                }
                if (release->button == sf::Mouse::Button::Right && draggingDest) {
                    float dx = wp.x - dest.x;
                    float dy = wp.y - dest.y;
                    if (std::abs(dx) > 2.f || std::abs(dy) > 2.f)
                        dest.deg = std::atan2(dy, dx) * 180.f / PI;
                    draggingDest = false;
                }
            }
        }

        sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        window.clear(sf::Color(30, 30, 40));

        float startDeg = start.deg;
        float destDeg = dest.deg;

        if (startSet) {
            if (draggingStart) {
                float dx = mouse.x - start.x;
                float dy = mouse.y - start.y;
                if (std::abs(dx) > 2.f || std::abs(dy) > 2.f)
                    startDeg = std::atan2(dy, dx) * 180.f / PI;
            }

            src.setOrigin({4.f, 4.f});
            src.setPosition({static_cast<float>(start.x), static_cast<float>(start.y)});
            src.setFillColor(sf::Color::White);
            window.draw(src);
            drawArrow(window, start.x, start.y, startDeg, sf::Color::White);
            drawTurningCircles(window, start.x, start.y, startDeg, static_cast<float>(R),
                               sf::Color(255, 255, 255, 80));
        }

        if (destSet) {
            if (draggingDest) {
                float dx = mouse.x - dest.x;
                float dy = mouse.y - dest.y;
                if (std::abs(dx) > 2.f || std::abs(dy) > 2.f)
                    destDeg = std::atan2(dy, dx) * 180.f / PI;
            }

            dst.setOrigin({4.f, 4.f});
            dst.setPosition({static_cast<float>(dest.x), static_cast<float>(dest.y)});
            dst.setFillColor(sf::Color::Red);
            window.draw(dst);
            drawArrow(window, dest.x, dest.y, destDeg, sf::Color::Red);
            drawTurningCircles(window, dest.x, dest.y, destDeg, static_cast<float>(R),
                               sf::Color(255, 80, 80, 80));
        }

        if (startSet && destSet) {
            Point liveStart = start;
            Point liveDest = dest;
            liveStart.deg = startDeg;
            liveDest.deg = destDeg;

            std::vector<RankedPath> paths = allDubinsPaths(liveStart, liveDest, R);
            if (!paths.empty()) {
                int idx = selectedRank - 1;
                if (idx >= static_cast<int>(paths.size()))
                    idx = static_cast<int>(paths.size()) - 1;
                if (idx < 0)
                    idx = 0;

                if (showAll) {
                    for (std::size_t i = 0; i < paths.size(); ++i) {
                        std::vector<sf::Vector2f> points = samplePath(
                            paths[i].path, liveStart, liveDest, static_cast<float>(R));
                        drawPath(window, points, rankColors[i % 6]);
                    }

                    if (hasFont) {
                        float yOff = 16.f;
                        for (std::size_t i = 0; i < paths.size(); ++i) {
                            std::ostringstream oss;
                            oss << (i + 1) << ". " << paths[i].name << "  "
                                << static_cast<int>(paths[i].path.length) << " px";
                            label.setString(oss.str());
                            label.setFillColor(rankColors[i % 6]);
                            sf::FloatRect bounds = label.getLocalBounds();
                            label.setPosition({viewW - bounds.size.x - 16.f, yOff});
                            window.draw(label);
                            yOff += bounds.size.y + 10.f;
                        }
                    }
                } else {
                    const RankedPath& chosen = paths[idx];
                    std::vector<sf::Vector2f> points = samplePath(
                        chosen.path, liveStart, liveDest, static_cast<float>(R));
                    drawPath(window, points, rankColors[idx % 6]);

                    if (hasFont) {
                        std::ostringstream oss;
                        oss << chosen.name << "  " << static_cast<int>(chosen.path.length)
                            << " px";
                        label.setString(oss.str());
                        label.setFillColor(rankColors[idx % 6]);
                        sf::FloatRect bounds = label.getLocalBounds();
                        label.setPosition({viewW - bounds.size.x - 16.f, 16.f});
                        window.draw(label);
                    }
                }
            }
        }

        window.display();
    }

    return 0;
}
