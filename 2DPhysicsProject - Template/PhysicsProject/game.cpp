#include "Game.h"
#include "main_menu.h"

const sf::Time Game::TimePerFrame = sf::seconds(1.f / 60.f);

Game::Game() : mWindow(nullptr), mGameState(GameState::MainMenu) {}

Game::~Game()
{
    if (!mIsQuitting)
    {
        quitGame();
    }
}

void Game::setWindow(sf::RenderWindow* window)
{
    mWindow = window;
    setState(GameState::MainMenu);
}

void Game::initializeMenus()
{
    if (mWindow)
    {
        mCurrentMenu = std::make_unique<MainMenu>(*mWindow);
    }
}

void Game::setState(GameState newState)
{
    mGameState = newState;
    switch (mGameState)
    {
    case GameState::MainMenu:
        resetGameState();
        resetGameProgress(); 
        mCurrentMenu = std::make_unique<MainMenu>(*mWindow, [this](int option) { handleMainMenuCallback(option); });
        break;
    case GameState::Playing:
        mCurrentMenu.reset();
        if (!mLevelScene) {
            mLevelScene = std::make_unique<LevelScene>();
            mLevelScene->setWindow(mWindow);
            mLevelScene->initialize();
        }
        mCurrentScene = mLevelScene.get();
        break;
    case GameState::Paused:
        mCurrentMenu = std::make_unique<PauseMenu>(*mWindow, [this](int option) { handlePauseMenuCallback(option); });
        break;
    case GameState::GameOver:
        mCurrentMenu = std::make_unique<LoseScreen>(*mWindow, *this);
        break;
    case GameState::GameWon:
        mCurrentMenu = std::make_unique<WinScreen>(*mWindow, *this);
        break;
    case GameState::HowToPlay:
        resetGameState();
        mCurrentMenu = std::make_unique<InstructionsScreen>(*mWindow, [this]() { handleHowToPlayCallback(); });
        break;
    }
}

void Game::handleMainMenuCallback(int option)
{
    switch (option)
    {
    case 0: // Start Game
        setState(GameState::Playing);
        break;
    case 1: // How to Play
        setState(GameState::HowToPlay);
        break;
    case 2: // Quit
        if (mWindow)
        {
            mWindow->close();
        }
        break;
    }
}

void Game::run()
{
    if (!mWindow)
    {
        return;
    }

    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;

    while (mWindow->isOpen() && !mIsQuitting)
    {
        processEvents();
        timeSinceLastUpdate += clock.restart();

        while (timeSinceLastUpdate > TimePerFrame && !mIsQuitting)
        {
            timeSinceLastUpdate -= TimePerFrame;
            update(TimePerFrame);
        }

        if (!mIsQuitting)
        {
            render();
        }
    }
}
void Game::processEvents()
{
    sf::Event event;
    while (mWindow->pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            mWindow->close();

        handleEvent(event);
    }
}

void Game::handlePauseMenuCallback(int option)
{
    switch (option)
    {
    case 0: // Resume
        setState(GameState::Playing);
        break;
    case 1: // Main Menu
        setState(GameState::MainMenu);
        break;
    case 2: // Quit
        quitGame();
        break;
    }
}

void Game::handleHowToPlayCallback()
{
    setState(GameState::MainMenu);
}

void Game::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
    {
        if (mGameState == GameState::Playing)
        {
            setState(GameState::Paused);
        }
        else if (mGameState == GameState::Paused)
        {
            setState(GameState::Playing);
        }
    }

    switch (mGameState)
    {
    case GameState::MainMenu:
    case GameState::Paused:
    case GameState::GameOver:
    case GameState::GameWon:
    case GameState::HowToPlay:
        if (mCurrentMenu)
            mCurrentMenu->handleEvent(event);
        break;
    case GameState::Playing:
        if (mCurrentScene)
            mCurrentScene->handleEvent(event);
        break;
    }
}

void Game::update(sf::Time deltaTime)
{
    switch (mGameState)
    {
    case GameState::Playing:
        if (mCurrentScene && mLevelScene)
        {
            mCurrentScene->update(deltaTime);
            if (mLevelScene->isLevelCompleted())
            {
                nextLevel();
            }
            else if (mLevelScene->isLevelFailed())
            {
                setState(GameState::GameOver);
            }
        }
        break;
    case GameState::MainMenu:
    case GameState::Paused:
    case GameState::GameOver:
    case GameState::GameWon:
    case GameState::HowToPlay:
        if (mCurrentMenu)
            mCurrentMenu->update(deltaTime);
        break;

    case GameState::LevelTransition:
        if (mTransitionClock.getElapsedTime().asSeconds() >= TRANSITION_DURATION)
        {
            if (mLevelScene)
            {
                mLevelScene->setLevel(mCurrentLevelNumber);
            }
            setState(GameState::Playing);
        }
        break;
    }
}

void Game::render()
{
    mWindow->clear(sf::Color::White);

    switch (mGameState)
    {
    case GameState::Playing:
        if (mCurrentScene)
            mCurrentScene->render(*mWindow);
        break;
    case GameState::Paused:
        if (mCurrentScene)
            mCurrentScene->render(*mWindow);
        // Draw pause menu on top
    case GameState::MainMenu:
    case GameState::GameOver:
    case GameState::GameWon:
    case GameState::HowToPlay:
        if (mCurrentMenu)
            mCurrentMenu->render();
        break;
    }

    mWindow->display();
}

void Game::setScene(std::unique_ptr<Scene> scene)
{
    delete mCurrentScene;  // Delete the old scene
    mCurrentScene = scene.release();
    if (mCurrentScene && mWindow)
    {
        mCurrentScene->setWindow(mWindow);
        mCurrentScene->initialize();
    }

    // Check if the new scene is a LevelScene
    LevelScene* levelScene = dynamic_cast<LevelScene*>(mCurrentScene);
    if (levelScene)
    {
        mLevelScene.reset(levelScene);
        mCurrentLevelNumber = mLevelScene->getCurrentLevel();
    }
}

void Game::nextLevel()
{
    mCurrentLevelNumber++;
    if (mCurrentLevelNumber > 3)
    {
        setState(GameState::GameWon);
        resetGameProgress();
    }
    else
    {
        transitionToNextLevel();
    }
}

void Game::restartLevel()
{
    loadLevel(mCurrentLevelNumber);
}

void Game::loadLevel(int levelNumber)
{
    if (!mLevelScene)
    {
        mLevelScene = std::make_unique<LevelScene>();
        mLevelScene->setWindow(mWindow);
        mLevelScene->initialize();
    }

    mLevelScene->setLevel(levelNumber);
    mCurrentScene = mLevelScene.get();
    setState(GameState::Playing);
}

void Game::resetGameState()
{
    mLevelScene.reset();
    mCurrentScene = nullptr;
}

void Game::quitGame()
{
    mIsQuitting = true;

    resetGameState();
    mCurrentMenu.reset();

    if (mWindow)
    {
        mWindow->close();
    }
}

void Game::resetGameProgress()
{
    mCurrentLevelNumber = 1;
    if (mLevelScene)
    {
        mLevelScene->setLevel(mCurrentLevelNumber);
    }
}

void Game::transitionToNextLevel()
{
    setState(GameState::LevelTransition);

    // Start a timer for the transition
    mTransitionClock.restart();

    // Clear current level
    if (mLevelScene)
    {
        mLevelScene->clearLevel();
    }
}