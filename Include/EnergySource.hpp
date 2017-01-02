#ifndef ABM_ENERGY_SOURCE_HPP
#define ABM_ENERGY_SOURCE_HPP

#include <SFML/Graphics.hpp>

namespace ABM
{
class EnergySource
{
public:
  EnergySource() : EnergySource(0, 0, 0) {}
  EnergySource(std::size_t maxCapacity, std::size_t currentLevel,
               std::size_t regenerationRate,
               sf::Vector2f position = sf::Vector2f(0, 0));

  std::size_t getMaxCapacity() const { return maxCapacity; }
  void setMaxCapacity(std::size_t value);

  std::size_t getCurrentLevel() const { return currentLevel; }
  void setCurrentLevel(std::size_t value);

  std::size_t getRegenerationRate() const { return regenerationRate; }
  void setRegenerationRate(std::size_t value) { regenerationRate = value; }

  sf::Vector2f getPosition() const { return shape.getPosition(); }
  void setPosition(sf::Vector2f value) { shape.setPosition(value); }

  const auto & getShape() const { return shape; }

  void regenerate();

protected:

private:
  void updateShapeRadius();

  std::size_t maxCapacity = 0;
  std::size_t currentLevel = 0;
  std::size_t regenerationRate = 0;
  sf::CircleShape shape;

  static constexpr auto minimumRadius = 5.f;
  static constexpr auto maximumRadius = 30.f;
};
}

#endif
