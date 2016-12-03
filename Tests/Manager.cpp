#include "catch.hpp"

#include "Manager.hpp"

using namespace ABM;

using MyComponents = ComponentList<int, float, double, char>;
using Integral = Signature<int, char>;
using Float = Signature<float, double>;
using MySignatures = SignatureList<Integral, Float>;
using MySettings = Settings<MyComponents, MySignatures>;

TEST_CASE("ComponentStorage")
{
  Manager<MySettings> manager;

  SECTION("Empty manager")
  {
    REQUIRE(manager.getCapacity() == 0);
    REQUIRE(manager.getAgentsCount() == 0);
  }

  SECTION("Create some agents and then clear the manager")
  {
    for (std::size_t i = 0; i < 100u; ++i)
    {
      manager.createIndex();
    }

    REQUIRE(manager.getAgentsCount() == 0);

    manager.refresh();

    REQUIRE(manager.getAgentsCount() == 100u);
    REQUIRE(manager.getCapacity() != 0);

    manager.clear();

    REQUIRE(manager.getAgentsCount() == 0);
    REQUIRE(manager.getCapacity() != 0);
  }

  SECTION("Create a default agent")
  {
    const auto index = manager.createIndex();

    REQUIRE(index == 0);
    REQUIRE(manager.isAlive(index));
    REQUIRE_FALSE(manager.hasComponent<int>(index));
    REQUIRE_FALSE(manager.hasComponent<float>(index));
    REQUIRE_FALSE(manager.hasComponent<double>(index));
    REQUIRE_FALSE(manager.hasComponent<char>(index));
    REQUIRE_FALSE(manager.matchesSignature<Integral>(index));
    REQUIRE_FALSE(manager.matchesSignature<Float>(index));

    manager.kill(index);

    REQUIRE_FALSE(manager.isAlive(index));
  }

  SECTION("Attaching and removing components")
  {
    const auto index = manager.createIndex();

    manager.addComponent<int>(index);
    auto intComponent = manager.getComponent<int>(index);

    REQUIRE(manager.hasComponent<int>(index));
    REQUIRE((std::is_same<decltype(intComponent), int>()));

    manager.addComponent<char>(index);
    auto charComponent = manager.getComponent<char>(index);

    REQUIRE(manager.hasComponent<char>(index));
    REQUIRE((std::is_same<decltype(charComponent), char>()));

    REQUIRE(manager.matchesSignature<Integral>(index));

    manager.deleteComponent<char>(index);

    REQUIRE_FALSE(manager.hasComponent<char>(index));
    REQUIRE_FALSE(manager.matchesSignature<Integral>(index));
  }
}
