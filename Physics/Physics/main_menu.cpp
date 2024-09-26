#include "main_menu.h"
#include <iostream>

MainMenu::MainMenu(sf::RenderWindow& window)
    : Menu(window), mCallback([](int) {}) 
{
    initialize();
}

MainMenu::MainMenu(sf::RenderWindow& window, std::function<void(int)> callback)
    : Menu(window), mCallback(callback)
{
    initialize();
}

void MainMenu::initialize()
{
    if (!mBackgroundTexture.loadFromFile("dependencies/sprites/menu.png")) 
    {
        std::cout << "Failed to load menu background image" << std::endl;
    }
    mBackgroundSprite.setTexture(mBackgroundTexture);

    // Scale the background to fit the window
    float scaleX = mWindow.getSize().x / static_cast<float>(mBackgroundTexture.getSize().x);
    float scaleY = mWindow.getSize().y / static_cast<float>(mBackgroundTexture.getSize().y);
    mBackgroundSprite.setScale(scaleX, scaleY);

    // Set up title text
    mTitleText.setFont(mFont);
    mTitleText.setString("Angry Birds Remake");
    mTitleText.setCharacterSize(60);
    mTitleText.setFillColor(sf::Color::White);
    centerText(mTitleText, 100.0f);

    // Add buttons with callbacks
    addButton("Start Game", [this]() { mCallback(0); });
    addButton("How to Play", [this]() { mCallback(1); });
    addButton("Quit", [this]() { mCallback(2); });

    float yOffset = 400.0f; 
    for (auto& button : mButtons)
    {
        centerText(button.text, yOffset);
        yOffset += 100.0f;
    }
}

void MainMenu::handleEvent(const sf::Event& event) 
{
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f mousePos = mWindow.mapPixelToCoords(sf::Mouse::getPosition(mWindow));
        updateButtonHover(mousePos);
    }
    if (event.type == sf::Event::MouseButtonPressed) 
    {
        sf::Vector2f mousePos = mWindow.mapPixelToCoords(sf::Mouse::getPosition(mWindow));
        for (size_t i = 0; i < mButtons.size(); ++i) 
        {
            if (mButtons[i].hitbox.contains(mousePos))
            {
                mCallback(i);
                break;
            }
        }
    }
}

void MainMenu::update(sf::Time deltaTime)
{
}

void MainMenu::render()
{
    mWindow.draw(mBackgroundSprite);
    mWindow.draw(mTitleText);
    for (const auto& button : mButtons)
    {
        mWindow.draw(button.text);
    }
}