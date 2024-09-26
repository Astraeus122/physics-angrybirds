#include "win_screen.h"
#include "game.h"
#include <iostream>

WinScreen::WinScreen(sf::RenderWindow& window, Game& game)
    : Menu(window), mGame(game)
{
    if (!mBackgroundTexture.loadFromFile("dependencies/sprites/win.png"))
    {
        std::cout << "Failed to load win screen background image" << std::endl;
    }
    mBackgroundSprite.setTexture(mBackgroundTexture);

    // Scale the background to fit the window
    float scaleX = window.getSize().x / static_cast<float>(mBackgroundTexture.getSize().x);
    float scaleY = window.getSize().y / static_cast<float>(mBackgroundTexture.getSize().y);
    mBackgroundSprite.setScale(scaleX, scaleY);

    mCongratulationsText.setFont(mFont);
    mCongratulationsText.setString("Congratulations!");
    mCongratulationsText.setCharacterSize(60);
    mCongratulationsText.setFillColor(sf::Color::White);
    centerText(mCongratulationsText, 200.0f);

    addButton("Main Menu", [this]() { mGame.setState(Game::GameState::MainMenu); });
    addButton("Quit", [this]() { mGame.quitGame(); });

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
}

void WinScreen::render()
{
    mWindow.draw(mBackgroundSprite);
    mWindow.draw(mCongratulationsText);
    for (const auto& button : mButtons)
    {
        mWindow.draw(button.text);
    }
}