#ifndef ABM_UTILS_HPP
#define ABM_UTILS_HPP

#include <type_traits>
#include <random>
#include <cmath>
#include <bitset>
#include <cassert>

#include <SFML/System/Vector2.hpp>

constexpr auto PI = 3.1415926535897f;

namespace ABM
{
namespace Utils
{
/**
 * @brief Calculates magnitude of a given vector
 */
auto magnitude(const sf::Vector2f & vector)
{
  return std::sqrt(vector.x * vector.x + vector.y * vector.y);
}

/**
 * @brief Calculates product of two given vectors
 */
auto product(const sf::Vector2f & v1, const sf::Vector2f & v2)
{
  return v1.x * v2.x + v1.y * v2.y;
}

/**
 * @brief Calculates normalized vector for a given one
 */
auto normal(const sf::Vector2f & vector)
{
  const auto mag = magnitude(vector);

  return sf::Vector2f{ vector.x / mag, vector.y / mag };
}

/**
 * @brief Calculates angle between two given vectors
 */
auto angle(const sf::Vector2f & v1, const sf::Vector2f & v2)
{
  const auto normalV1 = normal(v1);
  const auto normalV2 = normal(v2);

  return std::acos(product(normalV1, normalV2)) * 180.f / PI;
}

/**
 * @brief Partial specialization of randomNumber for integral types
 */
template<typename T>
T randomNumber(T minValue, T maxValue, std::true_type)
{
  static std::random_device randomDevice;
  static std::mt19937 generator{ randomDevice() };
  std::uniform_int_distribution<T> distr{ minValue, maxValue };

  return distr(generator);
}

/**
 * @brief Partial specialization of randomNumber for floatin point types
 */
template<typename T>
T randomNumber(T minValue, T maxValue, std::false_type)
{
  assert(minValue <= maxValue);

  static std::random_device randomDevice;
  static std::mt19937 generator{ randomDevice() };
  std::uniform_real_distribution<T> distr{ minValue, maxValue };

  return distr(generator);
}

/**
 * @brief Generates random number in range of two given values
 */
template<typename T>
T randomNumber(T minValue, T maxValue)
{
  static_assert(std::is_arithmetic<T>(), "T has to be arithmetic");

  return randomNumber(minValue, maxValue, std::is_integral<T>());
}

/**
 * @brief Generates random sf::Vector with values in range of two given numbers
 */
template<typename T>
sf::Vector2<T> randomVector(T minValue, T maxValue)
{
  return { randomNumber(minValue, maxValue), randomNumber(minValue, maxValue) };
}

/**
 * @brief Generates a bitset with randomly set bits
 */
template<std::size_t size>
std::bitset<size> randomBitset(double probability = 0.5)
{
  std::bitset<size> bitset;

  static std::random_device randomDevice;
  static std::mt19937 generator{ randomDevice() };
  std::bernoulli_distribution distr{ probability };

  for (std::size_t i = 0; i < size; ++i)
  {
    bitset[i] = distr(generator);
  }

  return bitset;
}
}
}

#endif
