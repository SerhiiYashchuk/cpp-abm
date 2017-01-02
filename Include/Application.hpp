#ifndef ABM_APPLICATION_HPP
#define ABM_APPLICATION_HPP

#include "Manager.hpp"
#include "Components.hpp"
#include "EnergySource.hpp"

#include <SFML/Graphics.hpp>

namespace ABM
{
using AgentComponents = ComponentList<PositionAndVelocity, Graphic>;

using Render = Signature<PositionAndVelocity, Graphic>;
using Movement = Signature<PositionAndVelocity>;
using AgentSignatures = SignatureList<Render, Movement>;

using AgentSettings = Settings<AgentComponents, AgentSignatures>;

class Application
{
public:
  Application(unsigned int windowWidth = 800, unsigned int windowHeight = 600,
              std::wstring title = L"Application")
    : window({ windowWidth, windowHeight }, title) { }

  void run();

private:
  void handleEvents();
  void update(float delta);
  void draw();

  void createAgents();

  sf::RenderWindow window;
  Manager<AgentSettings> agentManager;
  std::vector<EnergySource> energySources;

  static const sf::Time timePerFrame;
};
}

#endif
