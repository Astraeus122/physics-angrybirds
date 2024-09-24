#include "win_screen.h"
#include <iostream>

WinScreen::WinScreen(sf::RenderWindow& window) : Menu(window) 
{
    if (!mBackgroundTexture.loadFromFile("dependencies/sprites/win.png")) 
    {
        std::cerr << "Failed to load win screen background image!" << std::endl;
    }
    mBackgroundSprite.setTexture(mBackgroundTexture);

    // Scale the background to fit the window
    float scaleX = window.getSize().x / static_cast<float>(mBackgroundTexture.getSize().x);
    float scaleY = window.getSize().y / static_cast<float>(mBackgroundTexture.getSize().y);
    mBackgroundSprite.setScale(scaleX, scaleY);

    addButton("Next Level", []() { /* Go to next level action */ });
    addButton("Main Menu", []() { /* Return to main menu action */ });
    addButton("Quit", []() { /* Quit game action */ });

    float yOffset = 400.0f;
    for (auto& button : mButtons) 
    {
        centerText(button.text, yOffset);
        yOffset += 100.0f;
    }
}

void WinScreen::handleEvent(const sf::Event& event) 
{
    if (event.type == sf::Event::MouseButtonPressed) 
    {
        sf::Vector2f mousePos = mWindow.mapPixelToCoords(sf::Mouse::getPosition(mWindow));
        for (auto& button : mButtons) 
        {
            if (button.text.getGlobalBounds().contains(mousePos))
            {
                button.action();
                break;
            }
        }
    }
}

void WinScreen::update(sf::Time deltaTime)
{
    // Update logic if needed
}

void WinScreen::render() 
{
    mWindow.draw(mBackgroundSprite);
    for (const auto& button : mButtons)
    {
        mWindow.draw(button.text);
    }
}