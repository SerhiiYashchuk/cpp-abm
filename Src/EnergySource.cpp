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
 * @param delta - time delta
 */
void EnergySource::regenerate(float delta)
{
  currentLevel = std::min(currentLevel + regenerationRate * delta, maxCapacity);

  updateShapeRadius();
}

/**
 * @brief Resets current level of energy to zero
 * @return Level of energy before reset
 */
float EnergySource::reset()
{
  const auto energy = currentLevel;

  setCurrentLevel(0);

  return energy;
}

/**
 * @brief Changes shape's radius in proportion to the current level of energy
 */
void EnergySource::updateShapeRadius()
{
  static constexpr auto radiusDifference = maximumRadius - minimumRadius;
  const auto k = static_cast<float> (currentLevel) / maxCapacity;

  shape.setRadius(minimumRadius + radiusDifference * k);
  shape.setOrigin(shape.getRadius(), shape.getRadius());
}

}
