#ifndef ABM_APPLICATION_HPP
#define ABM_APPLICATION_HPP

#include "ThreadPool.hpp"
#include "Manager.hpp"
#include "Components.hpp"
#include "EnergySource.hpp"

#include <SFML/Graphics.hpp>

namespace ABM
{
using AgentComponents = ComponentList<Orientation, Energy, Destination, Graphic>;

using Movement = Signature<Orientation, Destination>;
using Life = Signature<Energy>;
using Harvesting = Signature<Orientation, Destination, Energy>;
using Render = Signature<Orientation, Destination, Graphic>;
using EnergyIndication = Signature<Energy, Graphic>;
using AgentSignatures = SignatureList<Movement, Life, Harvesting, Render,
  EnergyIndication>;

using AgentSettings = Settings<AgentComponents, AgentSignatures>;

class Application
{
public:
  Application(const sf::Vector2f & worldSize = { 800.f, 600.f },
              const sf::Vector2u & windowSize = { 800, 600 },
              std::wstring title = L"Application");

  void run();

  const sf::Vector2f worldSize;
  const std::size_t threadsNumber;
  static const std::size_t maxAgentsNumber = 25;
  static const std::size_t maxSourcesNumber = 10;

private:
  void handleEvents();
  void update(float delta);
  void draw();

  void moveAgent(std::size_t index, float delta);
  void updateAgentPositionAndRotation(std::size_t index);
  void applyAgentMetabolism(std::size_t index, float delta);
  void indicateAgentEnergyLevel(std::size_t index);
  void lookForEnergy(std::size_t index);

  std::vector<std::size_t> findEnergySourcesInRange(const sf::Vector2f & position,
                                                    float range);

  void createAgents();
  void createEnergySources();

  void zoomView(float factor);
  void moveView(const sf::Vector2f & offset);

  float getZoomFactor() const;

  sf::RenderWindow window;
  Manager<AgentSettings> agentManager;
  std::vector<EnergySource> energySources;
  ThreadPool threadPool;

  static const sf::Time timePerFrame;
};
}

#endif
