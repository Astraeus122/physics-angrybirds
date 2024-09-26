#ifndef MENU_H
#define MENU_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>

class Menu
{
public:
    Menu(sf::RenderWindow& window);
    virtual ~Menu() = default;

    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(sf::Time deltaTime) = 0;
    virtual void render() = 0;

protected:
    sf::RenderWindow& mWindow;
    sf::Font mFont;

    struct Button
    {
        sf::Text text;
        sf::FloatRect hitbox;
        std::function<void()> action;
        bool isHovered;
    };

    std::vector<Button> mButtons;

    void addButton(const std::string& text, const std::function<void()>& action);
    void centerText(sf::Text& text, float yPosition);
    void updateButtonHover(const sf::Vector2f& mousePos);
};

#endif