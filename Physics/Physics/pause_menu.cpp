#include "pause_menu.h"

PauseMenu::PauseMenu(sf::RenderWindow& window)
    : Menu(window), mCallback([](int) {}) {
    initialize();
}

PauseMenu::PauseMenu(sf::RenderWindow& window, std::function<void(int)> callback)
    : Menu(window), mCallback(callback) {
    initialize();
}

void PauseMenu::initialize() {
    mOverlay.setSize(sf::Vector2f(mWindow.getSize()));
    mOverlay.setFillColor(sf::Color(0, 0, 0, 128)); // Semi-transparent black

    addButton("Resume", [this]() { mCallback(0); });
    addButton("Main Menu", [this]() { mCallback(1); });
    addButton("Quit", [this]() { mCallback(2); });

    float yOffset = 200.0f;
    for (auto& button : mButtons) {
        centerText(button.text, yOffset);
        yOffset += 100.0f;
    }
}

void PauseMenu::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mousePos = mWindow.mapPixelToCoords(sf::Mouse::getPosition(mWindow));
        updateButtonHover(mousePos);
    }
    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f mousePos = mWindow.mapPixelToCoords(sf::Mouse::getPosition(mWindow));
        for (size_t i = 0; i < mButtons.size(); ++i) {
            if (mButtons[i].hitbox.contains(mousePos)) {
                mCallback(i);
                break;
            }
        }
    }
}

void PauseMenu::update(sf::Time deltaTime) {
    // Update logic if needed
}

void PauseMenu::render() {
    mWindow.draw(mOverlay);
    for (const auto& button : mButtons) {
        mWindow.draw(button.text);
    }
}