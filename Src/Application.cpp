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

    default:
      break;
    }
  }
}

/**
 * @brief Performs logic upon game entities
 */
void Application::update(float /*delta*/)
{
  // Rotate an Agent to a direction that it's moving towards
  agentManager.forAgentsMatching<Render>([this](auto index){
    const auto & posAndVel = agentManager.getComponent<PositionAndVelocity>(index);
    auto & graphic = agentManager.getComponent<Graphic>(index);

    auto angle = Utils::angle({ 0.f, -1.f }, posAndVel.velocity);

    // We need an angle in a clock-wise direction
    if (posAndVel.velocity.x < 0)
    {
      angle += 180.f;
    }

    graphic.shape.setPosition(posAndVel.position);
    graphic.shape.setRotation(angle);
  });
}

/**
 * @brief Clears the window and draws a new frame
 */
void Application::draw()
{
  window.clear();

  agentManager.forAgentsMatching<Render>([this](auto index){
    const auto & graphic = agentManager.getComponent<Graphic>(index);

    window.draw(graphic.shape);
  });

  for (const auto & source : energySources)
  {
    window.draw(source.getShape());
  }

  window.display();
}

/**
 * @brief Creates new Agents
 */
void Application::createAgents()
{
  const auto & index = agentManager.createIndex();

  auto & posAndVel = agentManager.addComponent<PositionAndVelocity>(index);
  agentManager.addComponent<Graphic>(index);

  posAndVel.position.x = static_cast<float>(window.getSize().x / 2);
  posAndVel.position.y = static_cast<float>(window.getSize().y / 2);
  posAndVel.velocity = { -1.f, 0.f };
}
}
