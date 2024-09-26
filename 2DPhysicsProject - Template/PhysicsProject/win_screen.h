#ifndef WIN_SCREEN_H
#define WIN_SCREEN_H

#include "menu.h"

class Game;  // Forward declaration

class WinScreen : public Menu
{
public:
    WinScreen(sf::RenderWindow& window, Game& game);

    void handleEvent(const sf::Event& event) override;
    void update(sf::Time deltaTime) override;
    void render() override;

private:
    sf::Sprite mBackgroundSprite;
    sf::Texture mBackgroundTexture;
    sf::Text mCongratulationsText;
    Game& mGame;
};

#endif