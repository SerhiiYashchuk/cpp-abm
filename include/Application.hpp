#ifndef ABM_APPLICATION_HPP
#define ABM_APPLICATION_HPP

#include <SFML/Graphics.hpp>

namespace ABM
{
class Application
{
public:
  Application() = delete;
  Application(sf::RenderWindow & renderWindow) : window(renderWindow) { }
  ~Application() = default;

  void Run();

protected:
  void HandleEvents();
  void Update(sf::Time delta);
  void Draw();

  sf::RenderWindow & window;

private:
  static const sf::Time timePerFrame;
};
}

#endif
