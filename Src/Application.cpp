#include <functional>
#include <algorithm>

#include "Application.hpp"
#include "Utils.hpp"

namespace ABM
{
const sf::Time Application::timePerFrame = sf::seconds(1.f / 60.f);

/**
 * @brief Application main loop
 */
void Application::run()
{
  sf::Clock clock;
  auto lastUpdateTime = sf::Time::Zero;

  createEnergySources();
  createAgents();

  while (window.isOpen())
  {
    handleEvents();

    lastUpdateTime += clock.restart();

    while (lastUpdateTime > timePerFrame)
    {
      lastUpdateTime -= timePerFrame;

      handleEvents();
      update(timePerFrame.asSeconds());
      agentManager.refresh();
    }

    draw();
  }
}

/**
 * @brief Handles events
 */
void Application::handleEvents()
{
  using EventType = sf::Event::EventType;

  sf::Event event;

  while (window.pollEvent(event))
  {
    switch (event.type)
    {
    case EventType::Closed:
      window.close();
      break;

    case EventType::MouseWheelMoved:
    {
      auto view = window.getView();
      const auto zoomFactor = 1.f - 0.1f * event.mouseWheel.delta;

      view.setSize(view.getSize() * zoomFactor);

      window.setView(view);
    }
      break;

    default:
      break;
    }
  }
}

/**
 * @brief Performs logic upon game entities
 */
void Application::update(float delta)
{
  using namespace std::placeholders;

  // Move around the world and look for energy to consume
  agentManager.forAgentsMatching<Harvesting>(std::bind(& Application::lookForEnergy,
                                                       this, _1));

  // Move agents
  agentManager.forAgentsMatching<Movement>(std::bind(& Application::moveAgent,
                                                     this, _1, delta));
  // Rotate an Agent to a direction that it's moving towards
  agentManager.forAgentsMatching<Render>(std::bind(& Application::updateAgentPositionAndRotation,
                                                   this, _1));
  // Reduce agent's level of energy as a cost of its action
  //agentManager.forAgentsMatching<Life>(std::bind(& Application::applyAgentMetabolism,
  //                                               this, _1, delta));
  // Change agent's fill color according to its level of energy
  //agentManager.forAgentsMatching<EnergyIndication>(std::bind(& Application::indicateAgentEnergyLevel,
  //                                                           this, _1));

  // Udate energy sources
  for (auto & source : energySources)
  {
    source.regenerate(delta);
  }
}

/**
 * @brief Clears the window and draws a new frame
 */
void Application::draw()
{
  window.clear();

  for (const auto & source : energySources)
  {
    window.draw(source.getShape());
  }

  agentManager.forAgentsMatching<Render>([this](auto index){
    const auto & graphic = agentManager.getComponent<Graphic>(index);

    window.draw(graphic.shape);
  });

  window.display();
}

/**
 * @brief Moves an Agent
 * @param index - index of an Agent
 * @param delta - time delta that affects movement
 */
void Application::moveAgent(std::size_t index, float delta)
{
  auto & orientation = agentManager.getComponent<Orientation>(index);
  const auto & destination = agentManager.getComponent<Destination>(index);
  // TODO: Make the world independent from window size
  const auto worldSize = window.getSize();
  const auto towardsDestination = destination.position - orientation.position;
  const auto distance = Utils::magnitude(towardsDestination);
  const auto step = orientation.velocity * delta;

  if (step > distance)
  {
    orientation.position = destination.position;
  }
  else
  {
    orientation.position += Utils::normal(towardsDestination) * step;

    // The world is seamless so agents move from one side to another
    if (orientation.position.x > worldSize.x)
    {
      orientation.position.x -= worldSize.x;
    }
    else if (orientation.position.x < 0)
    {
      orientation.position.x += worldSize.x;
    }

    if (orientation.position.y > worldSize.y)
    {
      orientation.position.y -= worldSize.y;
    }
    else if (orientation.position.y < 0)
    {
      orientation.position.y += worldSize.y;
    }
  }
}

/**
 * @brief Updates position and rotation of Agent's shape
 * @param index - index of an Agent
 */
void Application::updateAgentPositionAndRotation(std::size_t index)
{
  const auto & orientation = agentManager.getComponent<Orientation>(index);
  const auto & destination = agentManager.getComponent<Destination>(index);
  auto & graphic = agentManager.getComponent<Graphic>(index);
  const auto towardsDestination = destination.position - orientation.position;

  if (Utils::magnitude(towardsDestination) > 0)
  {
    auto angle = Utils::angle({ 0.f, -1.f }, towardsDestination);

    // We need an angle in a clock-wise direction
    if (towardsDestination.x < 0)
    {
      angle = 360.f - angle;
    }

    graphic.shape.setRotation(angle);
  }

  graphic.shape.setPosition(orientation.position);
}

/**
 * @brief Reduces Agent's level of energy
 * @param index - index of an Agent
 */
void Application::applyAgentMetabolism(std::size_t index, float delta)
{
  auto & energy = agentManager.getComponent<Energy>(index);

  if (energy.value > 0)
  {
    energy.value -= delta * energy.consumptionRate;
  }
}

/**
 * @brief Visually indicates Agent's current level of energy
 * @param index - index of an Agent
 */
void Application::indicateAgentEnergyLevel(std::size_t index)
{
  const auto & energy = agentManager.getComponent<Energy>(index);
  auto & graphic = agentManager.getComponent<Graphic>(index);
  const auto shade = static_cast<float>(energy.value) / energy.max + 0.2f;
  const auto color = sf::Color(static_cast<sf::Uint8>(sf::Color::Yellow.r * shade),
                               static_cast<sf::Uint8>(sf::Color::Yellow.g * shade),
                               static_cast<sf::Uint8>(sf::Color::Yellow.b * shade));

  graphic.shape.setFillColor(color);
}

/**
 * @brief Moves an Agent towards a Source Energy in his field of view.
 * When the Agent reaches the source, he replenishes his energy level
 * @param index - index of an Agent
 */
void Application::lookForEnergy(std::size_t index)
{
  auto & orientation = agentManager.getComponent<Orientation>(index);
  auto & destination = agentManager.getComponent<Destination>(index);
  auto availableSources = findEnergySourcesInRange(orientation.position,
                                                   orientation.viewRange);
  const auto reachedDestination = orientation.position == destination.position;

  // We are interested only in sources with some minimum energy level or more
  // TODO: Make this value more reasonable, not just a constant
  const auto minimumPreferableLevel = 3.f;
  std::vector<std::size_t> preferableSources;

  for (const auto sourceIndex : availableSources)
  {
    if (energySources[sourceIndex].getCurrentLevel() >= minimumPreferableLevel)
    {
      preferableSources.push_back(sourceIndex);
    }
  }

  // In this scenario we prioritize our choice
  /*
  if (!preferableSources.empty())
  {
    availableSources = std::move(preferableSources);
  }
  */

  // In this scenario we completely ignore sources with low energy level
  availableSources = std::move(preferableSources);

  /*
  if (std::any_of(std::begin(availableSources), std::end(availableSources),
                  [this, minimumPreferableLevel](auto index)->bool
    {
      return energySources[index].getCurrentLevel() >= minimumPreferableLevel;
    }))
  {
    // FIXME: Crashes if using erase-remove idiom
    availableSources.erase(std::remove_if(std::begin(availableSources), std::end(availableSources),
                                          [this, minimumPreferableLevel](auto index)->bool
      {
        return energySources[index].getCurrentLevel() < minimumPreferableLevel;
      }));
  }
  */

  // No sources found. Move in random direction
  if (availableSources.empty())
  {
    if (reachedDestination)
    {
      destination.position = orientation.position + Utils::normal(
            Utils::randomVector(-10.f, 10.f)) * orientation.viewRange;
    }
  }
  else
  {
    const auto richestSourceItr = std::max_element(std::begin(availableSources),
                                                   std::end(availableSources),
                                                   [this](auto left, auto right)
    {
      return energySources[left].getCurrentLevel() < energySources[right].getCurrentLevel();
    });
    auto & source = energySources[*richestSourceItr];

    if (destination.position == source.getPosition())
    {
      if (reachedDestination)
      {
        auto & energy = agentManager.getComponent<Energy>(index);

        energy.value += source.reset();
      }
    }
    else
    {
      destination.position = source.getPosition();
    }
  }
}

/**
 * @brief Searches for Energy Sources in specific area
 * @param position - position in a world
 * @param range - range in which search is performed
 * @return Vector with indexes of found Energy Sources
 */
std::vector<std::size_t> Application::findEnergySourcesInRange(const sf::Vector2f & position,
                                                  float range)
{
  std::vector<std::size_t> indexes;

  for (std::size_t i = 0; i < energySources.size(); ++i)
  {
    const auto distance = Utils::magnitude(energySources[i].getPosition() - position);

    if (distance < range)
    {
      indexes.push_back(i);
    }
  }

  return indexes;
}

/**
 * @brief Creates new Agents
 */
void Application::createAgents()
{
  for (std::size_t i = 0; i < maxAgentsNumber; ++i)
  {
    const auto index = agentManager.createIndex();

    auto & orientation = agentManager.addComponent<Orientation>(index);
    auto & destination = agentManager.addComponent<Destination>(index);

    agentManager.addComponent<Graphic>(index);
    agentManager.addComponent<Energy>(index, Utils::randomNumber(150.f, 300.f));

    orientation.position.x = static_cast<float>(window.getSize().x / 2);
    orientation.position.y = static_cast<float>(window.getSize().y / 2);
    orientation.velocity = 100.f;
    orientation.viewRange = Utils::randomNumber(75.f, 150.f);
    destination.position = orientation.position;
  }
}

/**
 * @brief Creates sources of energy and places them in the world
 */
void Application::createEnergySources()
{
  const auto sourceArea = static_cast<float>(std::min(window.getSize().x,
                                                      window.getSize().y));

  energySources.reserve(maxSourcesNumber);

  for (std::size_t i = 0; i < maxSourcesNumber; ++i)
  {
    const auto maxCapacity = Utils::randomNumber(10u, 50u);
    const auto initialLevel = Utils::randomNumber(0u, maxCapacity);
    const auto regenRate = Utils::randomNumber(1u, 3u);
    const auto position = Utils::randomVector(0.f, sourceArea);

    energySources.emplace_back(maxCapacity, initialLevel, regenRate, position);
  }
}
}
