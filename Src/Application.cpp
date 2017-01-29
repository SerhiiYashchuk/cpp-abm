#include <algorithm>

#include "Application.hpp"
#include "Utils.hpp"

namespace ABM
{
Application::Application(const sf::Vector2f & worldSize,
                         const sf::Vector2u & windowSize, std::wstring title)
  : worldSize(worldSize),
    threadsNumber(std::thread::hardware_concurrency() != 0 ?
      std::thread::hardware_concurrency() : 2),
    window({ windowSize.x, windowSize.y }, title),
    grid(worldSize),
    threadPool(threadsNumber)
{
  auto view = window.getView();
  const sf::Vector2f viewCenter = { std::max(static_cast<float>(windowSize.x), worldSize.x) * 0.5f,
                                    std::max(static_cast<float>(windowSize.y), worldSize.y) * 0.5f };

  view.setCenter(viewCenter);
  window.setView(view);

  font.loadFromFile("/usr/share/fonts/TTF/DejaVuSans.ttf");
  statisticLabel.setFont(font);
  statisticLabel.setFillColor(sf::Color::White);
  statisticLabel.setCharacterSize(15);
}

/**
 * @brief Application main loop
 */
void Application::run()
{
  sf::Clock clock;
  auto lastUpdateTime = sf::Time::Zero;

  createEnergySources();

  while (window.isOpen())
  {
    lastUpdateTime = clock.restart();

    handleEvents();
    createAgents();
    update(lastUpdateTime.asSeconds());
    agentManager.refresh();

    const auto fps = static_cast<std::size_t>(1.f / lastUpdateTime.asSeconds());

    statisticLabel.setString("FPS: " + std::to_string(fps) + "\nPopulation: " +
                             std::to_string(agentManager.getAgentsCount()));
    statisticLabel.setPosition(window.mapPixelToCoords({ 0, 0 }));
    statisticLabel.setScale({ getZoomFactor(), getZoomFactor() });

    draw();
  }
}

/**
 * @brief Handles events
 */
void Application::handleEvents()
{
  using EventType = sf::Event::EventType;

  static bool mouseLButtonDown = false;
  static sf::Vector2i lastMousePosition;
  sf::Event event;

  while (window.pollEvent(event))
  {
    switch (event.type)
    {
    case EventType::Closed:
      window.close();
      break;

    case EventType::KeyPressed:
      switch (event.key.code)
      {
      case sf::Keyboard::Add:
        zoomView(0.9f);
        break;

      case sf::Keyboard::Subtract:
        zoomView(1.1f);
        break;

      case sf::Keyboard::Left:
        moveView(sf::Vector2f{ -10.f, 0 } * getZoomFactor());
        break;

      case sf::Keyboard::Right:
        moveView(sf::Vector2f{ 10.f, 0 } * getZoomFactor());
        break;

      case sf::Keyboard::Up:
        moveView(sf::Vector2f{ 0, -10.f } * getZoomFactor());
        break;

      case sf::Keyboard::Down:
        moveView(sf::Vector2f{ 0, 10.f } * getZoomFactor());
        break;

      default:
        break;
      }

      break;

    case EventType::MouseWheelMoved:
      zoomView(1.f - 0.1f * event.mouseWheel.delta);
      break;

    // World movement
    case EventType::MouseButtonPressed:
      mouseLButtonDown = true;
      lastMousePosition = sf::Mouse::getPosition(window);
      break;

    case EventType::MouseButtonReleased:
      mouseLButtonDown = false;
      break;

    case EventType::MouseMoved:
      if (mouseLButtonDown)
      {
        const auto mousePosition = sf::Mouse::getPosition(window);
        const sf::Vector2f offset = {
          static_cast<float>(lastMousePosition.x - mousePosition.x),
          static_cast<float>(lastMousePosition.y - mousePosition.y)
        };

        moveView(offset * getZoomFactor());

        lastMousePosition = mousePosition;
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

  // Update helping grid
  grid.clearAgentsInfo();

  agentManager.forAllMatching<Movement>([this](auto index)
  {
    const auto & orientation = agentManager.getComponent<Orientation>(index);
    const auto gridPosition = grid.worldToGrid(orientation.position);

    grid.cell(gridPosition).agents.push_back(index);
  });

  const auto agentsCount = agentManager.getAgentsCount();

  // Do not process agents by groups if number of agents is relatively small
  if (agentsCount < 1000u)
  {
    // Move around the world and look for energy to consume
    agentManager.forAllMatching<Harvesting>(std::bind(& Application::lookForEnergy,
                                                         this, _1));
    // Collect information from neighbors
    agentManager.forAllMatching<InfoCollection>(std::bind(& Application::collectInfo,
                                                          this, _1));
    // Move agents
    agentManager.forAllMatching<Movement>(std::bind(& Application::moveAgent,
                                                       this, _1, delta));
    // Rotate an Agent to a direction that it's moving towards
    agentManager.forAllMatching<Render>(std::bind(& Application::updateAgentPositionAndRotation,
                                                     this, _1));
    // Reduce agent's level of energy as a cost of its action
    agentManager.forAllMatching<Life>(std::bind(& Application::applyAgentMetabolism,
                                                   this, _1, delta));
    // Change agent's fill color according to its level of energy
    //agentManager.forAllMatching<EnergyIndication>(std::bind(& Application::indicateAgentEnergyLevel,
    //                                                           this, _1));
    // Change agent's fill color according to its knowledge
    agentManager.forAllMatching<InfoIndication>(std::bind(& Application::indicateAgentKnowledge,
                                                          this, _1));
  }
  else
  {
    const auto tasksCount = threadsNumber * 2;
    const auto agentsPerTask = agentsCount / tasksCount;
    std::vector<std::future<void>> results;

    // Parallel harvesting mechanism
    // NOTE: Cannot properly parallel because EnergySource class is not thread-safe
    for (std::size_t i = 0; i < tasksCount; ++i)
    {
      const auto first = i * agentsPerTask;
      const auto last = i != tasksCount - 1 ? (i + 1) * agentsPerTask : agentsCount;

      results.emplace_back(threadPool.addTask([this, first, last]
      {
        agentManager.forGroupMatching<Harvesting>(first, last, std::bind(& Application::lookForEnergy, this, _1));
      }));
    }

    // Wait for harvestng tasks to finish
    for (auto & result : results)
    {
      result.get();
    }

    results.clear();

    // Parallel info collection
    for (std::size_t i = 0; i < tasksCount; ++i)
    {
      const auto first = i * agentsPerTask;
      const auto last = i != tasksCount - 1 ? (i + 1) * agentsPerTask : agentsCount;

      results.emplace_back(threadPool.addTask([this, first, last]
      {
        agentManager.forGroupMatching<InfoCollection>(first, last, std::bind(& Application::collectInfo, this, _1));
      }));
    }

    // Wait for harvestng tasks to finish
    for (auto & result : results)
    {
      result.get();
    }

    results.clear();

    // Parallel movement of agents
    for (std::size_t i = 0; i < tasksCount; ++i)
    {
      const auto first = i * agentsPerTask;
      const auto last = i != tasksCount - 1 ? (i + 1) * agentsPerTask : agentsCount;

      results.emplace_back(threadPool.addTask([this, first, last, delta]
      {
        agentManager.forGroupMatching<Movement>(first, last, std::bind(& Application::moveAgent, this, _1, delta));
      }));
    }

    // Wait for movement tasks to finish
    for (auto & result : results)
    {
      result.get();
    }

    results.clear();

    // Parallel visualization of agents
    for (std::size_t i = 0; i < tasksCount; ++i)
    {
      const auto first = i * agentsPerTask;
      const auto last = i != tasksCount - 1 ? (i + 1) * agentsPerTask : agentsCount;

      results.emplace_back(threadPool.addTask([this, first, last]
      {
        agentManager.forGroupMatching<Render>(first, last, std::bind(& Application::updateAgentPositionAndRotation, this, _1));
      }));
    }

    // Wait for visualization tasks to finish
    for (auto & result : results)
    {
      result.get();
    }

    results.clear();

    // Parallel life mechanism
    for (std::size_t i = 0; i < tasksCount; ++i)
    {
      const auto first = i * agentsPerTask;
      const auto last = i != tasksCount - 1 ? (i + 1) * agentsPerTask : agentsCount;

      results.emplace_back(threadPool.addTask([this, first, last, delta]
      {
        agentManager.forGroupMatching<Life>(first, last, std::bind(& Application::applyAgentMetabolism, this, _1, delta));
      }));
    }

    // Wait for life tasks to finish
    for (auto & result : results)
    {
      result.get();
    }

    results.clear();

    /*
    // Parallel energy indication
    for (std::size_t i = 0; i < tasksCount; ++i)
    {
      const auto first = i * agentsPerTask;
      const auto last = i != tasksCount - 1 ? (i + 1) * agentsPerTask : agentsCount;

      results.emplace_back(threadPool.addTask([this, first, last]
      {
        agentManager.forGroupMatching<EnergyIndication>(first, last, std::bind(& Application::indicateAgentEnergyLevel, this, _1));
      }));
    }

    // Wait for energy indication tasks to finish
    for (auto & result : results)
    {
      result.get();
    }

    results.clear();
    */

    // Parallel knowledge indication
    for (std::size_t i = 0; i < tasksCount; ++i)
    {
      const auto first = i * agentsPerTask;
      const auto last = i != tasksCount - 1 ? (i + 1) * agentsPerTask : agentsCount;

      results.emplace_back(threadPool.addTask([this, first, last]
      {
        agentManager.forGroupMatching<InfoIndication>(first, last, std::bind(& Application::indicateAgentKnowledge, this, _1));
      }));
    }

    // Wait for knowledge indication tasks to finish
    for (auto & result : results)
    {
      result.get();
    }

    results.clear();
  }

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

  agentManager.forAllMatching<Render>([this](auto index){
    const auto & graphic = agentManager.getComponent<Graphic>(index);

    window.draw(graphic.shape);
  });

  window.draw(statisticLabel);
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

  energy.value -= delta * energy.consumptionRate;

  if (energy.value < 0)
  {
    agentManager.kill(index);
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
 * @brief Application::indicateAgentKnowledge
 * @param index - index of an Agent
 */
void Application::indicateAgentKnowledge(std::size_t index)
{
  const auto & info = agentManager.getComponent<Information>(index);
  auto & graphic = agentManager.getComponent<Graphic>(index);
  const auto count = info.value.count();

  if (count == 0)
  {
    graphic.shape.setFillColor(sf::Color::Yellow);
  }
  else
  {
    graphic.shape.setFillColor(count == info.value.size() ? sf::Color::Green : sf::Color::Blue);
  }
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
  auto availableSources = findSourcesInRange(orientation.position,
                                             orientation.viewRange);
  const auto reachedDestination = orientation.position == destination.position;

  // We are interested only in sources with some minimum energy level or more
  // TODO: Make this value more reasonable, not just a constant
  const auto minimumPreferableLevel = 20.f;
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

      if (destination.position.x > worldSize.x)
      {
        destination.position.x = worldSize.x;
      }
      else if (destination.position.x < 0)
      {
        destination.position.x = 0;
      }

      if (destination.position.y > worldSize.y)
      {
        destination.position.y = worldSize.y;
      }
      else if (destination.position.y < 0)
      {
        destination.position.y = 0;
      }
    }
  }
  // Move to the source and replenish energy
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

        energy.value = std::min(500.f, energy.value + source.reset());
      }
    }
    else
    {
      destination.position = source.getPosition();
    }
  }
}

/**
 * @brief Collects information from nearby agents
 * @param index - index of an Agent
 */
void Application::collectInfo(std::size_t index)
{
  const auto & orientation = agentManager.getComponent<Orientation>(index);
  auto & info = agentManager.getComponent<Information>(index);

  const auto nearbyAgents = findAgentsInRange(orientation.position,
                                              info.shareRange);

  for (const auto i : nearbyAgents)
  {
    // NOTE: Getting info from nearby agents instead of sharing ours is thread-safe
    info.value |= agentManager.getComponent<Information>(i).value;
  }
}

/**
 * @brief Searches for Energy Sources in specific area
 * @param position - position in a world
 * @param range - range in which search is performed
 * @return Vector with indexes of found Energy Sources
 */
std::vector<std::size_t> Application::findSourcesInRange(sf::Vector2f position,
                                                         float range) const
{
  auto top = position.y - range > 0 ? position.y - range : 0;
  auto bottom = position.y + range < worldSize.y ? position.y + range : worldSize.y;
  auto left = position.x - range > 0 ? position.x - range : 0;
  auto right = position.x + range < worldSize.x ? position.x + range : worldSize.x;

  const auto topLeft = grid.worldToGrid({ left, top });
  const auto bottomRight = grid.worldToGrid({ right, bottom });

  std::vector<std::size_t> indexes;

  for (std::size_t x = topLeft.x; x < bottomRight.x; ++x)
  {
    for (std::size_t y = topLeft.y; y < bottomRight.y; ++y)
    {
      const auto & sources = grid.cell({ x, y }).sources;

      for (const auto i : sources)
      {
        const auto distance = Utils::magnitude(energySources[i].getPosition() - position);

        if (distance < range)
        {
          indexes.push_back(i);
        }
      }
    }
  }

  return indexes;
}

/**
 * @brief Searches for Agents in specific area
 * @param position - position in a world
 * @param range - range in which search is performed
 * @return Vector with indexed of found Agents
 */
std::vector<std::size_t> Application::findAgentsInRange(sf::Vector2f position,
                                                        float range) const
{
  auto top = position.y - range > 0 ? position.y - range : 0;
  auto bottom = position.y + range < worldSize.y ? position.y + range : worldSize.y;
  auto left = position.x - range > 0 ? position.x - range : 0;
  auto right = position.x + range < worldSize.x ? position.x + range : worldSize.x;

  const auto topLeft = grid.worldToGrid({ left, top });
  const auto bottomRight = grid.worldToGrid({ right, bottom });

  std::vector<std::size_t> indexes;

  for (std::size_t x = topLeft.x; x < bottomRight.x; ++x)
  {
    for (std::size_t y = topLeft.y; y < bottomRight.y; ++y)
    {
      const auto & sources = grid.cell({ x, y }).agents;

      for (const auto i : sources)
      {
        const auto & orientation = agentManager.getComponent<Orientation>(i);
        const auto distance = Utils::magnitude(orientation.position - position);

        if (distance < range)
        {
          indexes.push_back(i);
        }
      }
    }
  }

  return indexes;
}

/**
 * @brief Creates new Agents
 */
void Application::createAgents()
{
  if (agentManager.getAgentsCount() >= maxAgentsNumber)
  {
    return;
  }

  const auto groupSize = maxAgentsNumber / 20;
  const auto agentsToCreate = std::min(groupSize, maxAgentsNumber - agentManager.getAgentsCount());

  for (std::size_t i = 0; i < agentsToCreate; ++i)
  {
    const auto index = agentManager.createIndex();

    auto & orientation = agentManager.addComponent<Orientation>(index);
    auto & destination = agentManager.addComponent<Destination>(index);
    auto & info = agentManager.addComponent<Information>(index);

    agentManager.addComponent<Graphic>(index);
    agentManager.addComponent<Energy>(index, Utils::randomNumber(100.f, 300.f),
                                      Utils::randomNumber(15.f, 25.f));

    orientation.position.x = Utils::randomNumber(0.f, worldSize.x);
    orientation.position.y = Utils::randomNumber(0.f, worldSize.y);
    orientation.velocity = 300.f;
    orientation.viewRange = Utils::randomNumber(100.f, 250.f);
    destination.position = orientation.position;
    info.value = Utils::randomBitset<Information::size>(0.1);
  }
}

/**
 * @brief Creates sources of energy and places them in the world
 */
void Application::createEnergySources()
{
  energySources.reserve(maxSourcesNumber);

  for (std::size_t i = 0; i < maxSourcesNumber; ++i)
  {
    const auto maxCapacity = Utils::randomNumber(25.f, 100.f);
    const auto initialLevel = Utils::randomNumber(0.f, maxCapacity);
    const auto regenRate = Utils::randomNumber(20.f, 50.f);
    const auto position = sf::Vector2f{ Utils::randomNumber(0.f, worldSize.x),
                                        Utils::randomNumber(0.f, worldSize.y) };
    const auto gridPosition = grid.worldToGrid(position);

    energySources.emplace_back(maxCapacity, initialLevel, regenRate, position);
    grid.cell(gridPosition).sources.push_back(i);
  }
}

/**
 * @brief Zooms image in or out
 * @param factor - factor of zooming
 */
void Application::zoomView(float factor)
{
  auto view = window.getView();
  const auto windowSize = sf::Vector2f{ static_cast<float>(window.getSize().x),
                                        static_cast<float>(window.getSize().y) };
  auto newSize = view.getSize() * factor;

  if (newSize.x < windowSize.x || newSize.y < windowSize.y)
  {
    newSize = windowSize;
  }

  view.setSize(newSize);
  window.setView(view);
}

/**
 * @brief Moves image
 * @param offset - distance to move the view
 */
void Application::moveView(const sf::Vector2f & offset)
{
  auto view = window.getView();

  view.move(offset);
  window.setView(view);
}

/**
 * @brief Zoom factor getter
 * @return Current zoom factor
 */
float Application::getZoomFactor() const
{
  return window.getView().getSize().x / window.getSize().x;
}
}
