#include <cassert>

#include "EnergySource.hpp"

namespace ABM
{

/**
 * @brief C-tor
 * @param maxCapacity
 * @param currentLevel
 * @param regenerationRate
 * @param position
 */
EnergySource::EnergySource(std::size_t maxCapacity, std::size_t currentLevel,
                           std::size_t regenerationRate, sf::Vector2f position)
  : maxCapacity(maxCapacity),
    currentLevel(currentLevel),
    regenerationRate(regenerationRate)
{
  assert(currentLevel <= maxCapacity);

  updateShapeRadius();

  shape.setOrigin(shape.getRadius(), shape.getRadius());
  shape.setPosition(position);
  shape.setFillColor(sf::Color::Red);
}

/**
 * @brief Sets maximum level of energy. If current level of energy is higher
 * than the given value, it resets it to the specified maximum level
 * @param value
 */
void EnergySource::setMaxCapacity(std::size_t value)
{
  if (currentLevel > value)
  {
    currentLevel = value;
  }

  maxCapacity = value;

  updateShapeRadius();
}

/**
 * @brief Sets current level of energy
 * @param value
 */
void EnergySource::setCurrentLevel(std::size_t value)
{
  if (value > maxCapacity)
  {
    return;
  }

  updateShapeRadius();
}

/**
 * @brief Accumulates energy
 */
void EnergySource::regenerate()
{
  if (regenerationRate == 0)
  {
    return;
  }

  currentLevel = std::min(currentLevel + regenerationRate, maxCapacity);

  updateShapeRadius();
}

/**
 * @brief Changes shape's radius in proportion to the current level of energy
 */
void EnergySource::updateShapeRadius()
{
  static constexpr auto radiusDifference = maximumRadius - minimumRadius;
  const auto k = static_cast<float> (currentLevel) / maxCapacity;

  shape.setRadius(minimumRadius + radiusDifference * k);
}

}
