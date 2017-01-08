#ifndef ABM_ENERGY_SOURCE_HPP
#define ABM_ENERGY_SOURCE_HPP

#include <SFML/Graphics.hpp>

namespace ABM
{
class EnergySource
{
public:
  EnergySource(float maxCapacity) : EnergySource(maxCapacity, 0, 1.f) { }
  EnergySource(float maxCapacity, float currentLevel,
               float regenerationRate = 1.f,
               sf::Vector2f position = sf::Vector2f(0, 0));

  float getRegenerationRate() const { return regenerationRate; }
  float getMaxCapacity() const { return maxCapacity; }

  float getCurrentLevel() const { return currentLevel; }
  void setCurrentLevel(float value);

  sf::Vector2f getPosition() const { return shape.getPosition(); }
  void setPosition(sf::Vector2f value) { shape.setPosition(value); }

  const auto & getShape() const { return shape; }

  void regenerate();
  float reset();

protected:

private:
  void updateShapeRadius();

  float currentLevel = 0;
  const float regenerationRate = 1.f;
  const float maxCapacity = 0;
  sf::CircleShape shape;

  static constexpr auto minimumRadius = 5.f;
  static constexpr auto maximumRadius = 30.f;
};
}

#endif
