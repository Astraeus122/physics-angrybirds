#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <memory>
#include "scene.h"
#include "menu.h"
#include "main_menu.h"
#include "pause_menu.h"
#include "win_screen.h"
#include "lose_screen.h"
#include "instructions_screen.h"
#include "level_scene.h"

class Game
{
public:
    enum class GameState 
    {
        MainMenu,
        Playing,
        Paused,
        GameOver,
        GameWon,
        HowToPlay
    };

    Game();
    ~Game();
    void run();
    void setWindow(sf::RenderWindow* window);
    void setScene(std::unique_ptr<Scene> scene);
    void setState(GameState newState);
    void handleEvent(const sf::Event& event);

    void nextLevel();
    void restartLevel();
    void quitGame();
    void resetGameProgress();

private:
    void processEvents();
    void update(sf::Time deltaTime);
    void render();

    sf::RenderWindow* mWindow;
    std::unique_ptr<Menu> mCurrentMenu;
    GameState mGameState;
    static const sf::Time TimePerFrame;

    void initializeMenus();
    void handleMainMenuCallback(int option);
    void handlePauseMenuCallback(int option);
    void handleHowToPlayCallback();

    Scene* mCurrentScene;
    std::unique_ptr<LevelScene> mLevelScene;
    int mCurrentLevelNumber;

    void loadLevel(int levelNumber);
    void resetGameState();
    bool mIsQuitting;
};

#endif