#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "menu.h"
#include <functional>

class InstructionsScreen : public Menu {
public:
    InstructionsScreen(sf::RenderWindow& window, std::function<void()> backCallback);

    void handleEvent(const sf::Event& event) override;
    void update(sf::Time deltaTime) override;
    void render() override;

private:
    sf::Sprite mBackgroundSprite;
    sf::Texture mBackgroundTexture;
    sf::Text mInstructionsText;
    sf::Text mBackButtonText;
    std::function<void()> mBackCallback;
};

#endif