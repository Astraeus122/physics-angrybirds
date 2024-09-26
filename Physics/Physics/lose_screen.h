#ifndef LOSE_SCREEN_H
#define LOSE_SCREEN_H

#include "menu.h"

class Game;  // Forward declaration

class LoseScreen : public Menu
{
public:
    LoseScreen(sf::RenderWindow& window, Game& game);

    void handleEvent(const sf::Event& event) override;
    void update(sf::Time deltaTime) override;
    void render() override;

private:
    sf::Sprite mBackgroundSprite;
    sf::Texture mBackgroundTexture;
    Game& mGame;
};

#endif