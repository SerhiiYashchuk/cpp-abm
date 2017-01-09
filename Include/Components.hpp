#ifndef ABM_COMPONENTS_HPP
#define ABM_COMPONENTS_HPP

#include <SFML/Graphics.hpp>

namespace ABM
{
struct Destination
{
  Destination() = default;
  explicit Destination(const sf::Vector2f & position) : position(position) { }

  sf::Vector2f position;
};

// Position, velocity and view range of an Agent
struct Orientation
{
  Orientation() = default;
  Orientation(const sf::Vector2f & position, float velocity, float viewRange)
    : position(position), velocity(velocity), viewRange(viewRange) { }

  sf::Vector2f position;
  float velocity;
  float viewRange = 0;
};

// Graphical representation of an Agent
struct Graphic
{
  Graphic()
  {
    const float width = 20.f;
    const float height = 40.f;

    // Triangle shape with a yellow fill
    shape.setPointCount(3);
    shape.setPoint(0, { width / 2, 0.f });
    shape.setPoint(1, { 0.f, height });
    shape.setPoint(2, { width, height });

    shape.setFillColor(sf::Color::Yellow);

    shape.setOrigin(width / 2, height / 2);
  }

  sf::ConvexShape shape;
};

// Life energy of an Agent
struct Energy
{
  Energy() = default;
  explicit Energy(float value, float consumptionRate = 1.f)
    : value(value), consumptionRate(consumptionRate) { }

  float value = 0;
  float consumptionRate = 1.f;
  static constexpr float max = 100.f;
};
}

#endif
