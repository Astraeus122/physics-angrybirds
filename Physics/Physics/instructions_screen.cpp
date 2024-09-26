#include "instructions_screen.h"
#include <iostream>

InstructionsScreen::InstructionsScreen(sf::RenderWindow& window, std::function<void()> backCallback)
    : Menu(window), mBackCallback(backCallback) 
{
    if (!mBackgroundTexture.loadFromFile("dependencies/sprites/menu.png")) 
    {
        std::cout << "Failed to load how to play background image" << std::endl;
    }
    mBackgroundSprite.setTexture(mBackgroundTexture);

    // Scale the background to fit the window
    float scaleX = window.getSize().x / static_cast<float>(mBackgroundTexture.getSize().x);
    float scaleY = window.getSize().y / static_cast<float>(mBackgroundTexture.getSize().y);
    mBackgroundSprite.setScale(scaleX, scaleY);

    // Set up instructions text
    mInstructionsText.setFont(mFont);
    mInstructionsText.setString
    (
        "How to Play:\n\n"
        "1. Click and drag the slingshot to aim\n"
        "2. Release to launch the fireball\n"
        "3. Destroy all the enemies and if you want to, the blocks\n"
        "4. Click and hover on the UI sprites in the top left to change projectile\n"
        "5. Press 'Esc' to pause the game\n"
        "6. Please give good grade"
    );
    mInstructionsText.setCharacterSize(30);
    mInstructionsText.setFillColor(sf::Color::White);
    mInstructionsText.setPosition(50, 50);

    // Set up back button
    mBackButtonText.setFont(mFont);
    mBackButtonText.setString("Back");
    mBackButtonText.setCharacterSize(30);
    mBackButtonText.setFillColor(sf::Color::White);
    mBackButtonText.setPosition(50, window.getSize().y - 80);
}

void InstructionsScreen::handleEvent(const sf::Event& event) 
{
    if (event.type == sf::Event::MouseMoved) 
    {
        sf::Vector2f mousePos = mWindow.mapPixelToCoords(sf::Mouse::getPosition(mWindow));
        updateButtonHover(mousePos);
    }
    if (event.type == sf::Event::MouseButtonPressed) 
    {
        sf::Vector2f mousePos = mWindow.mapPixelToCoords(sf::Mouse::getPosition(mWindow));
        if (mBackButtonText.getGlobalBounds().contains(mousePos)) 
        {
            mBackCallback();
        }
    }
}

void InstructionsScreen::update(sf::Time deltaTime) 
{
}

void InstructionsScreen::render() 
{
    mWindow.draw(mBackgroundSprite);
    mWindow.draw(mInstructionsText);
    mWindow.draw(mBackButtonText);
}