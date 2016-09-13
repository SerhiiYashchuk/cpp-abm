#include "Application.hpp"

namespace ABM
{
const sf::Time Application::timePerFrame = sf::seconds(1.f / 60.f);

/**
 * @brief Application main loop
 */
void Application::Run()
{
  sf::Clock clock;
  auto lastUpdateTime = sf::Time::Zero;

  while (window.isOpen())
  {
    HandleEvents();

    lastUpdateTime += clock.restart();

    while (lastUpdateTime > timePerFrame)
    {
      lastUpdateTime -= timePerFrame;

      HandleEvents();
      Update(timePerFrame);
    }

    Draw();
  }
}

/**
 * @brief Handles events
 */
void Application::HandleEvents()
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
void Application::Update(sf::Time /*delta*/)
{

}

/**
 * @brief Clears the window and draws a new frame
 */
void Application::Draw()
{
  window.clear(sf::Color::Black);

  window.display();
}
}
