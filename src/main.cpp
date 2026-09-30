#include <iostream>
#include <optional>
#include <vector>
#include <cmath>

#include <SFML/Graphics.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int FPS_LIMIT = 30;

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
std::vector<sf::Vector2f> points = {
    {100.f, 600.f},  // P0 
    {250.f, 150.f},  // P1
    {550.f, 150.f},  // P2 
    {700.f, 600.f}   // P3
};
// TODO: (Part 2) Track animation time for the square moving along the curve.
float animT = 0.f;
// TODO: (Part 3) Track the index of the control point being dragged.
int dragIndex = -1;

void handleInput(sf::Window& window, bool& shouldQuit) {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            shouldQuit = true;
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            // TODO: (Part 3) On left-click, select the closest control point
            // using mouse->position and start dragging it.
            if (mouse->button == sf::Mouse::Button::Left) {
                sf::Vector2f m(mouse->position);        
                float bestDist = 1e9f;                  
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
            if ( dragIndex != -1){
                points[dragIndex] = sf::Vector2f(mouse->position);
            }            // TODO: (Part 4) Maintain matching slopes at shared endpoints.
            // When moving point 3, move point 5 without changing its distance
            // from point 4 (point numbers here start at 1).
            
        } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            // TODO: (Part 4) '+' adds three control points; '-' removes three,
            // keeping at least four points.
        }
    }
}

void render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);
    
    // ====== ====== ======
    // TODO: (Part 1) Sample GetPoint over t in [0, 1] and connect samples using the line-drawing
    // code from your project. Draw all four control points as circles after drawing the curve.
    // ====== ====== ======
    const int SAMPLES = 50;
    sf::VertexArray curve(sf::PrimitiveType::LineStrip);

    for (int i = 0; i <= SAMPLES; ++i) {
        float t = static_cast<float>(i) / SAMPLES;
        Point2D p = getPoint(points, t);
        curve.append(sf::Vertex{p, sf::Color::White});
    }

    window.draw(curve);

    // TODO: (Part 3) Draw control handles from point 1 to 2 and point 3 to 4.
    sf::VertexArray handles(sf::PrimitiveType::Lines);
    handles.append(sf::Vertex{points[0], sf::Color::Yellow});
    handles.append(sf::Vertex{points[1], sf::Color::Yellow});
    handles.append(sf::Vertex{points[2], sf::Color::Yellow});
    handles.append(sf::Vertex{points[3], sf::Color::Yellow});
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
    animT += 0.01f;
    if (animT > 1.f) {
        animT = 0.f;
    }
    sf::RectangleShape square({20.f, 20.f});
    square.setOrigin({10.f, 10.f});        
    square.setFillColor(sf::Color::Green);
    square.setPosition(getPoint(points, animT));
    Point2D dir = getSlope(points, animT);      
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
