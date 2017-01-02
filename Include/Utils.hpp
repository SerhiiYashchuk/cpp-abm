#ifndef ABM_UTILS_HPP
#define ABM_UTILS_HPP

#include <cmath>
#include <SFML/System/Vector2.hpp>

constexpr auto PI = 3.1415926535897f;

namespace ABM
{
namespace Utils
{
auto magnitude(const sf::Vector2f & vector)
{
  return std::sqrt(vector.x * vector.x + vector.y * vector.y);
}

auto product(const sf::Vector2f & v1, const sf::Vector2f & v2)
{
  return v1.x * v2.x + v1.y * v2.y;
}

auto angle(const sf::Vector2f & v1, const sf::Vector2f & v2)
{
  return std::acos(product(v1, v2) / magnitude(v1) * magnitude(v2)) * 180 / PI;
}
}
}

#endif
