#ifndef ABM_COMPONENTS_HPP
#define ABM_COMPONENTS_HPP

#include <SFML/Graphics.hpp>

namespace ABM
{
// Position, velocity and view range of an Agent
struct Orientation
{
  Orientation() = default;
  Orientation(const sf::Vector2f & position, const sf::Vector2f & velocity,
              float viewRange)
    : position(position), velocity(velocity), viewRange(viewRange) { }

  sf::Vector2f position;
  sf::Vector2f velocity;
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

struct Energy
{
  Energy() = default;
  explicit Energy(std::size_t value) : value(value) { }

  std::size_t value = 0;
  static constexpr std::size_t max = 100;
};
}

#endif
