#include "Application.hpp"

int main(int /*argc*/, char ** /*argv*/)
{
  sf::RenderWindow window(sf::VideoMode(800, 600), L"ABM Project");
  ABM::Application app(window);

  app.Run();

  return 0;
}

