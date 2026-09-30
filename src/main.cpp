#include <iostream>
#include <optional>
#include <vector>
#include <cmath>
#include <algorithm>

#include <SFML/Graphics.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int FPS_LIMIT = 30;

// Bonus: the Galaga game screen, shown at half size (1:2) in the editor
const float GAME_WIDTH = 800.f;    // window width
const float GAME_HEIGHT = 800.f;   //  window height
const float SCALE = 0.5f;          // 1:2 ratio
const sf::Vector2f BOX_SIZE{GAME_WIDTH * SCALE, GAME_HEIGHT * SCALE};
const sf::Vector2f BOX_POS{(WINDOW_WIDTH - BOX_SIZE.x) / 2.f,
                           (WINDOW_HEIGHT - BOX_SIZE.y) / 2.f};   // centred

using Point2D = sf::Vector2f;

// TODO: (Part 1) Define a function that samples a cubic Bezier curve at t in [0, 1].

Point2D getPoint(const std::vector<sf::Vector2f>& pts, float t) {
    float u = 1.f - t;
    // weights for each control point
    float w0 = u * u * u;
    float w1 = 3.f * u * u * t;
    float w2 = 3.f * u * t * t;
    float w3 = t * t * t;
    return pts[0] * w0 + pts[1] * w1 + pts[2] * w2 + pts[3] * w3;
}

// TODO: (Part 2) Define a function that returns the curve's slope at t in [0, 1].
Point2D getSlope(const std::vector<sf::Vector2f>& pts, float t) {
    //B′(t) = 3(1−t)²·(P1−P0) + 6(1−t)t·(P2−P1) + 3t²·(P3−P2)
    float u = 1.f - t;
    float w0 = 3.f*u*u;
    float w1 = 6.f*u*t;
    float w2 = 3*t*t;
    return w0*(pts[1]-pts[0]) + w1*(pts[2]-pts[1]) + w2*(pts[3]-pts[2])/* w0 * (P1 - P0) + ... */;    
}

// TODO: (Part 1) Store four control points for the curve.
// Bonus: a list of separate curves; each one is its own chain of points
std::vector<std::vector<sf::Vector2f>> curves = {
    {
        {100.f, 600.f},  // P0
        {250.f, 150.f},  // P1
        {550.f, 150.f},  // P2
        {700.f, 600.f}   // P3
    }
};
int activeCurve = 0;   // which curve is editing
// TODO: (Part 2) Track animation time for the square moving along the curve.
float animT = 0.f;
// TODO: (Part 3) Track the index of the control point being dragged.
int dragIndex = -1;

void handleInput(sf::Window& window, bool& shouldQuit) {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        auto& points = curves[activeCurve];
        if (event->is<sf::Event::Closed>()) {
            window.close();
            shouldQuit = true;
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            // TODO: (Part 3) On left-click, select the closest control point
            // using mouse->position and start dragging it.
            if (mouse->button == sf::Mouse::Button::Left) {
                sf::Vector2f m(mouse->position);   
                dragIndex = -1;     
                float bestDist = 15.f;                  
                for (int i = 0; i < (int)points.size(); ++i) {
                    float dx = m.x - points[i].x;
                    float dy = m.y - points[i].y;
                    float dist = std::sqrt(dx*dx + dy*dy);
                    if (dist<bestDist) {
                        bestDist = dist;
                        dragIndex = i;
                     }
                 }
            }
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonReleased>()) {
            // TODO: (Part 3) On left-button release, stop dragging.
            if (mouse->button == sf::Mouse::Button::Left) {
                dragIndex = -1;
             }
        } else if (const auto* mouse = event->getIf<sf::Event::MouseMoved>()) {
            // TODO: (Part 3) Move the selected control point to mouse->position.
            if (dragIndex != -1) {
                points[dragIndex] = sf::Vector2f(mouse->position);

                // (Part 4) keep the joint smooth: find the handle on the other side
                int i = dragIndex;
                int joint = -1, partner = -1;
                if (i % 3 == 2 && i + 2 < (int)points.size()) {   // handle BEFORE a joint
                    joint = i + 1;
                    partner = i + 2;
                } else if (i % 3 == 1 && i - 2 >= 0) {            // handle AFTER a joint
                    joint = i - 1;
                    partner = i - 2;
                }

                if (partner != -1) {
                    sf::Vector2f dir = points[joint] - points[i];              // dragged -> joint
                    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                    sf::Vector2f old = points[partner] - points[joint];
                    float dist = std::sqrt(old.x * old.x + old.y * old.y);     // partner's old distance
                    if (len > 0.f) {
                        points[partner] = points[joint] + (dir / len) * dist;  // same line, same distance
                    }
                }
            }          
            // TODO: (Part 4) Maintain matching slopes at shared endpoints.
            // When moving point 3, move point 5 without changing its distance
            // from point 4 (point numbers here start at 1).
            
        } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            // TODO: (Part 4) '+' adds three control points; '-' removes three,
            // keeping at least four points.
            if (key->code == sf::Keyboard::Key::Equal || key->code == sf::Keyboard::Key::Add) {
                sf::Vector2f last = points[points.size() - 1];   
                sf::Vector2f prev = points[points.size() - 2];   
                sf::Vector2f center{WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f};

                sf::Vector2f a = last + (last - prev);    
                sf::Vector2f c = center * 2.f - last;   
                sf::Vector2f b = (a + c) / 2.f;           

                // keep all three inside the window
                for (sf::Vector2f* p : {&a, &b, &c}) {
                    p->x = std::clamp(p->x, 20.f, WINDOW_WIDTH - 20.f);
                    p->y = std::clamp(p->y, 20.f, WINDOW_HEIGHT - 20.f);
                }
                points.push_back(a);
                points.push_back(b);
                points.push_back(c);
            } else if (key->code == sf::Keyboard::Key::Hyphen || key->code == sf::Keyboard::Key::Subtract) {
                if (points.size() >= 7) {            
                    points.resize(points.size() - 3);
                    dragIndex = -1;                  
                }
            } else if (key->code == sf::Keyboard::Key::E) {
                // Bonus: export points as C++ code, in game coordinates
                std::cout << "std::vector<sf::Vector2f> path = {\n";
                for (const auto& p : points) {
                    sf::Vector2f g = (p - BOX_POS) / SCALE;   // editor -> game
                    std::cout << "    {" << std::round(g.x) << ".f, " << std::round(g.y) << ".f},\n";
                }
                std::cout << "};\n";
            } else if (key->code == sf::Keyboard::Key::N) {
                // Bonus: start a new separate curve
                curves.push_back({
                    {150.f, 150.f},
                    {300.f, 50.f},
                    {500.f, 50.f},
                    {650.f, 150.f}
                });
                activeCurve = (int)curves.size() - 1;   // switch to the new one
                dragIndex = -1;
                animT = 0.f;
            } else if (key->code == sf::Keyboard::Key::Tab) {
                // Bonus: switch to the next curve (wraps around)
                activeCurve = (activeCurve + 1) % (int)curves.size();
                dragIndex = -1;
                animT = 0.f;
            }
        }
    }
}

void render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);
    auto& points = curves[activeCurve];
    //BONUS  
    sf::RectangleShape gameBox(BOX_SIZE);
    gameBox.setPosition(BOX_POS);
    gameBox.setFillColor(sf::Color::Transparent);          // just an outline
    gameBox.setOutlineColor(sf::Color(100, 100, 100));     // grey
    gameBox.setOutlineThickness(2.f);
    window.draw(gameBox);
    // ====== ====== ======
    // TODO: (Part 1) Sample GetPoint over t in [0, 1] and connect samples using the line-drawing
    // code from your project. Draw all four control points as circles after drawing the curve.
    // ====== ====== ======
    const int SAMPLES = 50;
    sf::VertexArray curve(sf::PrimitiveType::LineStrip);

    for (int s = 0; s + 3 < (int)points.size(); s += 3) {
        // copy the 4 points for this curve
        std::vector<sf::Vector2f> seg(points.begin() + s, points.begin() + s + 4);
        for (int i = 0; i <= SAMPLES; ++i) {
            float t = static_cast<float>(i) / SAMPLES;
            Point2D p = getPoint(seg, t); 
            curve.append(sf::Vertex{p, sf::Color::White});
        }
    }
    window.draw(curve);

    // TODO: (Part 3) Draw control handles from point 1 to 2 and point 3 to 4.
    sf::VertexArray handles(sf::PrimitiveType::Lines);
    for (int s = 0; s + 3 < (int)points.size(); s += 3) {
    handles.append(sf::Vertex{points[s], sf::Color::Yellow});
    handles.append(sf::Vertex{points[s+1], sf::Color::Yellow});
    handles.append(sf::Vertex{points[s+2], sf::Color::Yellow});
    handles.append(sf::Vertex{points[s+3], sf::Color::Yellow});
    }
    window.draw(handles);
    
    const float RADIUS = 6.f;
    for (const auto& p : points) {
        sf::CircleShape c(RADIUS);
        c.setOrigin({RADIUS, RADIUS});
        c.setFillColor(sf::Color::Red);
        c.setPosition(p);
        window.draw(c);
    }

    // ====== ====== ======
    // TODO: (Part 2) Draw a small square moving repeatedly along the curve.
    // Use GetSlope to orient it to the curve at each time step.
    // ====== ====== ======
    int numCurves = ((int)points.size() - 1) / 3;   // 4 pts -> 1, 7 -> 2, 10 -> 3
    animT += 0.01f;
    if (animT >= numCurves) {
        animT = 0.f;
    }

    int curveIdx = (int)animT;              // which curve we're on
    float localT = animT - curveIdx;        // how far along that curve (0 to 1)
    std::vector<sf::Vector2f> seg(points.begin() + curveIdx * 3,
                                  points.begin() + curveIdx * 3 + 4);


    sf::RectangleShape square({20.f, 20.f});
    square.setOrigin({10.f, 10.f});        
    square.setFillColor(sf::Color::Green);
    square.setPosition(getPoint(seg, localT));
    Point2D dir = getSlope(seg, localT);   
    float angle = std::atan2(dir.y, dir.x);     
    square.setRotation(sf::radians(angle));    
    window.draw(square);
    // ====== ====== ======

    // TODO: (Part 4) Draw all connected cubic Bezier segments and their handles.
    // ====== ====== ======

    // ====== ====== ======
    // TODO: (Bonus) Support multiple curves, a Galaga screen overlay at a 1:2 ratio, and exporting
    // curve points as C++ code for Project 1b.
    // ====== ====== ======

    window.display();
}

int main() {
    sf::RenderWindow window;

    try {
        std::cout << "Controls: drag = move point, +/- = add/remove curve, E = export\n";
        // Initialize window
        window.create(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Bezier Curve Editor");
        window.setPosition({100, 50});
        window.setFramerateLimit(FPS_LIMIT);
        // Prevent key repeats.
        window.setKeyRepeatEnabled(false);

        bool shouldQuit = false;
        // Main game loop
        while (window.isOpen()) {
            handleInput(window, shouldQuit);
            if (shouldQuit) {
                break;
            }
            render(window);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
