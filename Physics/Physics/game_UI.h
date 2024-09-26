#ifndef GAME_UI_H
#define GAME_UI_H

#include <SFML/Graphics.hpp>
#include <array>
#include "projectile.h"

class GameUI
{
public:
    GameUI();
    void initialize(sf::RenderWindow* window);

    void update(int projectilesLeft, int enemiesLeft, int currentLevel);
    void render(sf::RenderWindow& window);

    void handleProjectileSelection(const sf::Vector2i& mousePosition);
    void renderProjectileSelection(sf::RenderWindow& window);
    Projectile::Type getSelectedProjectileType() const;

    bool isProjectileAvailable(Projectile::Type type) const;
    void updateProjectileAvailability(Projectile::Type type, bool available);
    void setSelectedProjectileType(Projectile::Type type);

    void handleMouseHover(const sf::Vector2i& mousePosition);
    void handleMouseClick(const sf::Vector2i& mousePosition);

    void initializeProjectileAvailability();

private:
    sf::Font mFont;
    sf::Text mProjectilesText;
    sf::Text mEnemiesText;
    sf::Text mLevelText;

    std::array<sf::Texture, 5> mProjectileTextures;
    std::array<sf::Sprite, 5> mProjectileSprites;
    std::array<sf::Text, 5> mProjectileDescriptions;
    Projectile::Type mSelectedProjectileType;
    int mHoveredProjectile;

    void createUI(sf::RenderWindow* window);
    void initializeProjectileSelection();
    void updateCurrentProjectileText();
    void loadProjectileTextures();
    sf::Text mCurrentProjectileText;   

    std::array<bool, 5> mProjectileAvailability;
};

#endif