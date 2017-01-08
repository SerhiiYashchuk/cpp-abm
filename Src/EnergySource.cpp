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
EnergySource::EnergySource(float maxCapacity, float currentLevel,
                           float regenerationRate, sf::Vector2f position)
  : currentLevel(currentLevel),
    regenerationRate(regenerationRate),
    maxCapacity(maxCapacity)
{
  assert(maxCapacity > 0);
  assert(currentLevel <= maxCapacity);
  assert(regenerationRate > 0);

  updateShapeRadius();

  shape.setOrigin(shape.getRadius(), shape.getRadius());
  shape.setPosition(position);
  shape.setFillColor(sf::Color::Red);
}

/**
 * @brief Sets current level of energy
 * @param value
 */
void EnergySource::setCurrentLevel(float value)
{
  assert(value >= 0);
  assert(value <= maxCapacity);

  currentLevel = value;

  updateShapeRadius();
}

/**
 * @brief Accumulates energy
 */
void EnergySource::regenerate()
{
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
