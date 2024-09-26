#include "game.h"
#include "level_scene.h"
#include <SFML/Graphics.hpp>

int main() 
{
    // Create SFML window
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Angry Birds Clone", sf::Style::Fullscreen);
    window.setFramerateLimit(60);

    // Create and initialize the game
    Game game;
    game.setWindow(&window);

    // Create and set the initial scene
    auto levelOneScene = std::make_unique<LevelScene>();
    game.setScene(std::move(levelOneScene));

    // Run the game
    game.run();

    return 0;
}