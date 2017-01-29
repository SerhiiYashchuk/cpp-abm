#ifndef ABM_APPLICATION_HPP
#define ABM_APPLICATION_HPP

#include <cmath>

#include "ThreadPool.hpp"
#include "Manager.hpp"
#include "Components.hpp"
#include "EnergySource.hpp"

#include <SFML/Graphics.hpp>

namespace ABM
{
using AgentComponents = ComponentList<Orientation, Energy, Destination, Graphic,
  Information>;

using Movement = Signature<Orientation, Destination>;
using Life = Signature<Energy>;
using Harvesting = Signature<Orientation, Destination, Energy>;
using InfoCollection = Signature<Orientation, Information>;
using Render = Signature<Orientation, Destination, Graphic>;
using EnergyIndication = Signature<Energy, Graphic>;
using InfoIndication = Signature<Information, Graphic>;
using AgentSignatures = SignatureList<Movement, Life, Harvesting, InfoCollection,
  Render, EnergyIndication, InfoIndication>;

using AgentSettings = Settings<AgentComponents, AgentSignatures>;

class Application
{
public:
  Application(const sf::Vector2f & worldSize = { 5000.f, 5000.f },
              const sf::Vector2u & windowSize = { 1280, 720 },
              std::wstring title = L"Application");

  void run();

  const sf::Vector2f worldSize;
  const std::size_t threadsNumber;
  static const std::size_t maxAgentsNumber = 6000;
  static const std::size_t maxSourcesNumber = 500;

private:
  class Grid
  {
  public:
    struct Cell
    {
      std::vector<std::size_t> agents;
      std::vector<std::size_t> sources;
    };

    Grid(sf::Vector2f worldSize)
      : width(static_cast<std::size_t>(std::ceil(worldSize.x / cellSize)) + 2 * offset),
        height(static_cast<std::size_t>(std::ceil(worldSize.y / cellSize)) + 2 * offset),
        cells(width, std::vector<Cell>(height)) { }

    void clearAgentsInfo() noexcept
    {
      for (auto & line : cells)
      {
        for (auto & cell : line)
        {
          cell.agents.clear();
        }
      }
    }

    void clearSourcesInfo() noexcept
    {
      for (auto & line : cells)
      {
        for (auto & cell : line)
        {
          cell.sources.clear();
        }
      }
    }

    void clear() noexcept
    {
      for (auto & line : cells)
      {
        for (auto & cell : line)
        {
          cell.agents.clear();
          cell.sources.clear();
        }
      }
    }

    std::size_t index(float coordinate) const noexcept
    {
      return static_cast<std::size_t>(coordinate) / cellSize;
    }

    sf::Vector2<std::size_t> worldToGrid(sf::Vector2f position) const noexcept
    {
      return { index(position.x), index(position.y) };
    }

    Cell & cell(sf::Vector2<std::size_t> indexes)
    {
      assert(indexes.x < width - offset);
      assert(indexes.y < height - offset);

      return cells[indexes.x + offset][indexes.y + offset];
    }

    const Cell & cell(sf::Vector2<std::size_t> indexes) const
    {
      assert(indexes.x < width - offset);
      assert(indexes.y < height - offset);

      return cells[indexes.x + offset][indexes.y + offset];
    }

  private:
    const std::size_t offset{1};
    const std::size_t cellSize{150};
    const std::size_t width;
    const std::size_t height;

    std::vector<std::vector<Cell>> cells;
  };

  void handleEvents();
  void update(float delta);
  void draw();

  void moveAgent(std::size_t index, float delta);
  void updateAgentPositionAndRotation(std::size_t index);
  void applyAgentMetabolism(std::size_t index, float delta);
  void indicateAgentEnergyLevel(std::size_t index);
  void indicateAgentKnowledge(std::size_t index);
  void lookForEnergy(std::size_t index);
  void collectInfo(std::size_t index);

  std::vector<std::size_t> findSourcesInRange(sf::Vector2f position, float range) const;
  std::vector<std::size_t> findAgentsInRange(sf::Vector2f position, float range) const;

  void createAgents();
  void createEnergySources();

  void zoomView(float factor);
  void moveView(const sf::Vector2f & offset);

  float getZoomFactor() const;

  sf::RenderWindow window;
  sf::Text statisticLabel;
  sf::Font font;

  Manager<AgentSettings> agentManager;
  std::vector<EnergySource> energySources;

  Grid grid;

  ThreadPool threadPool;
};
}

#endif
