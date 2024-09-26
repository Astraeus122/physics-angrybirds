#include "game_UI.h"
#include <iostream>

GameUI::GameUI() : mSelectedProjectileType(Projectile::Type::Standard), mHoveredProjectile(-1)
{
    initializeProjectileAvailability();
}

void GameUI::initialize(sf::RenderWindow* window)
{
    if (!mFont.loadFromFile("dependencies/font.ttf"))
    {
        std::cout << "Failed to load font" << std::endl;
    }

    loadProjectileTextures();
    createUI(window);
    initializeProjectileSelection();
}

void GameUI::loadProjectileTextures()
{
    mProjectileTextures[0].loadFromFile("dependencies/sprites/standard.png");
    mProjectileTextures[1].loadFromFile("dependencies/sprites/bouncy.png");
    mProjectileTextures[2].loadFromFile("dependencies/sprites/explosive.png");
    mProjectileTextures[3].loadFromFile("dependencies/sprites/heavy.png");
    mProjectileTextures[4].loadFromFile("dependencies/sprites/split.png");
}

void GameUI::createUI(sf::RenderWindow* window)
{
    // Projectiles left text
    mProjectilesText.setFont(mFont);
    mProjectilesText.setCharacterSize(24);
    mProjectilesText.setFillColor(sf::Color::White);
    mProjectilesText.setPosition(window->getSize().x - 200, 10);

    // Enemies left text
    mEnemiesText.setFont(mFont);
    mEnemiesText.setCharacterSize(24);
    mEnemiesText.setFillColor(sf::Color::White);
    mEnemiesText.setPosition(window->getSize().x - 200, 40);

    // Level text
    mLevelText.setFont(mFont);
    mLevelText.setCharacterSize(24);
    mLevelText.setFillColor(sf::Color::White);
    mLevelText.setPosition(window->getSize().x / 2 - 50, 10);
}

void GameUI::initializeProjectileSelection()
{
    const float SPRITE_SPACING = 70.f;
    const float SPRITE_Y = 10.f;
    const float DESCRIPTION_Y = 60.f;

    for (int i = 0; i < 5; ++i)
    {
        mProjectileSprites[i].setTexture(mProjectileTextures[i]);
        mProjectileSprites[i].setPosition(10 + i * SPRITE_SPACING, SPRITE_Y);
        mProjectileSprites[i].setScale(0.5f, 0.5f);

        mProjectileDescriptions[i].setFont(mFont);
        mProjectileDescriptions[i].setCharacterSize(18);
        mProjectileDescriptions[i].setFillColor(sf::Color::White);
        mProjectileDescriptions[i].setPosition(10 + i * SPRITE_SPACING, DESCRIPTION_Y);
    }

    mProjectileDescriptions[0].setString("Standard: Basic projectile");
    mProjectileDescriptions[1].setString("Bouncy: Bounces off surfaces");
    mProjectileDescriptions[2].setString("Explosive: Explodes on impact");
    mProjectileDescriptions[3].setString("Heavy: Deals more damage");
    mProjectileDescriptions[4].setString("Split: Splits into multiple projectiles");

    mSelectedProjectileType = Projectile::Type::Standard;
    mHoveredProjectile = -1;

    // Position current projectile type text to the right of the last sprite
    mCurrentProjectileText.setFont(mFont);
    mCurrentProjectileText.setCharacterSize(24);
    mCurrentProjectileText.setFillColor(sf::Color::White);
    mCurrentProjectileText.setPosition(10 + 5 * SPRITE_SPACING, SPRITE_Y);
    updateCurrentProjectileText();
}

void GameUI::updateCurrentProjectileText()
{
    std::string typeStr;
    switch (mSelectedProjectileType)
    {
    case Projectile::Type::Standard: typeStr = "Standard"; break;
    case Projectile::Type::Bouncy: typeStr = "Bouncy"; break;
    case Projectile::Type::Explosive: typeStr = "Explosive"; break;
    case Projectile::Type::Heavy: typeStr = "Heavy"; break;
    case Projectile::Type::Split: typeStr = "Split"; break;
    }
    mCurrentProjectileText.setString("Current Type: " + typeStr);
}

void GameUI::update(int projectilesLeft, int enemiesLeft, int currentLevel)
{
    mProjectilesText.setString("Projectiles: " + std::to_string(projectilesLeft));
    mEnemiesText.setString("Enemies: " + std::to_string(enemiesLeft));
    mLevelText.setString("Level: " + std::to_string(currentLevel));
    updateCurrentProjectileText();
}

void GameUI::render(sf::RenderWindow& window)
{
    window.draw(mProjectilesText);
    window.draw(mEnemiesText);
    window.draw(mLevelText);
    renderProjectileSelection(window);
}

void GameUI::updateProjectileAvailability(Projectile::Type type, bool available)
{
    if (type == Projectile::Type::Standard)
        return;

    int index = static_cast<int>(type);
    mProjectileAvailability[index] = available;

    if (!available && mSelectedProjectileType == type)
    {
        mSelectedProjectileType = Projectile::Type::Standard;
    }

    // Update UI visuals
    mProjectileSprites[index].setColor(available ? sf::Color::White : sf::Color(128, 128, 128));
}

void GameUI::handleProjectileSelection(const sf::Vector2i& mousePosition)
{
    for (int i = 0; i < 5; ++i)
    {
        if (mProjectileSprites[i].getGlobalBounds().contains(mousePosition.x, mousePosition.y))
        {
            if (mProjectileAvailability[i])
            {
                mSelectedProjectileType = static_cast<Projectile::Type>(i);
            }
            break;
        }
    }
}

void GameUI::renderProjectileSelection(sf::RenderWindow& window)
{
    for (int i = 0; i < 5; ++i) 
    {
        window.draw(mProjectileSprites[i]);
        if (static_cast<Projectile::Type>(i) == mSelectedProjectileType) 
        {
            // Highlight the selected projectile type
            sf::RectangleShape highlight(sf::Vector2f(mProjectileSprites[i].getGlobalBounds().width, mProjectileSprites[i].getGlobalBounds().height));
            highlight.setPosition(mProjectileSprites[i].getPosition());
            highlight.setFillColor(sf::Color::Transparent);
            highlight.setOutlineColor(sf::Color::Yellow);
            highlight.setOutlineThickness(2);
            window.draw(highlight);
        }
    }

    if (mHoveredProjectile != -1)
    {
        window.draw(mProjectileDescriptions[mHoveredProjectile]);
    }

    window.draw(mCurrentProjectileText);
}

Projectile::Type GameUI::getSelectedProjectileType() const
{
    return mSelectedProjectileType;
}

bool GameUI::isProjectileAvailable(Projectile::Type type) const
{
    // Ensure standard projectile is always available
    if (type == Projectile::Type::Standard)
        return true;

    return mProjectileAvailability[static_cast<int>(type)];
}

void GameUI::setSelectedProjectileType(Projectile::Type type)
{
    if (isProjectileAvailable(type))
    {
        mSelectedProjectileType = type;
        updateCurrentProjectileText();
    }
}

void GameUI::initializeProjectileAvailability()
{
    // Set all projectiles to available initially
    mProjectileAvailability.fill(true);
}

void GameUI::handleMouseHover(const sf::Vector2i& mousePosition)
{
    mHoveredProjectile = -1;
    for (int i = 0; i < 5; ++i)
    {
        if (mProjectileSprites[i].getGlobalBounds().contains(mousePosition.x, mousePosition.y))
        {
            mHoveredProjectile = i;
            break;
        }
    }
}

void GameUI::handleMouseClick(const sf::Vector2i& mousePosition)
{
    for (int i = 0; i < 5; ++i)
    {
        if (mProjectileSprites[i].getGlobalBounds().contains(mousePosition.x, mousePosition.y))
        {
            if (mProjectileAvailability[i])
            {
                mSelectedProjectileType = static_cast<Projectile::Type>(i);
                updateCurrentProjectileText();
            }
            break;
        }
    }
}
