#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "menu.h"
#include <functional>

class MainMenu : public Menu {
public:
    MainMenu(sf::RenderWindow& window);
    MainMenu(sf::RenderWindow& window, std::function<void(int)> callback);

    void handleEvent(const sf::Event& event) override;
    void update(sf::Time deltaTime) override;
    void render() override;

private:
    void initialize();
    sf::Sprite mBackgroundSprite;
    sf::Texture mBackgroundTexture;
    sf::Text mTitleText;
    std::function<void(int)> mCallback;
};

#endif