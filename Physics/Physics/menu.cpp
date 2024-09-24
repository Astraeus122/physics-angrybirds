#include "menu.h"

Menu::Menu(sf::RenderWindow& window) : mWindow(window) {
    if (!mFont.loadFromFile("dependencies/font.ttf")) {
        // Handle font loading error
    }
}

void Menu::addButton(const std::string& text, const std::function<void()>& action) {
    Button button;
    button.text.setFont(mFont);
    button.text.setString(text);
    button.text.setCharacterSize(30);
    button.action = action;
    button.isHovered = false;
    mButtons.push_back(button);
}

void Menu::centerText(sf::Text& text, float yPosition) {
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    text.setPosition(mWindow.getSize().x / 2.0f, yPosition);

    // Set up the hitbox with increased size
    float hitboxPadding = 20.0f; // Adjust this value to increase/decrease the clickable area
    sf::FloatRect hitbox = text.getGlobalBounds();
    hitbox.left -= hitboxPadding;
    hitbox.top -= hitboxPadding;
    hitbox.width += 2 * hitboxPadding;
    hitbox.height += 2 * hitboxPadding;

    // Find the button corresponding to this text and update its hitbox
    for (auto& button : mButtons) {
        if (&button.text == &text) {
            button.hitbox = hitbox;
            break;
        }
    }
}

void Menu::updateButtonHover(const sf::Vector2f& mousePos) {
    for (auto& button : mButtons) {
        bool wasHovered = button.isHovered;
        button.isHovered = button.hitbox.contains(mousePos);

        if (button.isHovered != wasHovered) {
            if (button.isHovered) {
                button.text.setFillColor(sf::Color::Yellow);
                button.text.setScale(1.1f, 1.1f);
            }
            else {
                button.text.setFillColor(sf::Color::White);
                button.text.setScale(1.0f, 1.0f);
            }
        }
    }
}