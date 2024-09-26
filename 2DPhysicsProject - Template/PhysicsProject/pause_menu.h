#ifndef PAUSE_MENU_H
#define PAUSE_MENU_H

#include "menu.h"
#include <functional>

class PauseMenu : public Menu
{
public:
    PauseMenu(sf::RenderWindow& window, std::function<void(int)> callback);

    void handleEvent(const sf::Event& event) override;
    void update(sf::Time deltaTime) override;
    void render() override;

private:
    void initialize();
    sf::RectangleShape mOverlay;
    std::function<void(int)> mCallback;
};

#endif