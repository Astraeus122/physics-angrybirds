#include "Game.h"
#include "main_menu.h"
#include "level_one_scene.h"

const sf::Time Game::TimePerFrame = sf::seconds(1.f / 60.f);

Game::Game() : mWindow(nullptr), mGameState(GameState::MainMenu) {}

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
        mCurrentScene.reset();
        mCurrentMenu = std::make_unique<MainMenu>(*mWindow, [this](int option) { handleMainMenuCallback(option); });
        break;
    case GameState::Paused:
        mCurrentMenu = std::make_unique<PauseMenu>(*mWindow, [this](int option) { handlePauseMenuCallback(option); });
        break;
    case GameState::GameOver:
        mCurrentMenu = std::make_unique<LoseScreen>(*mWindow);
        break;
    case GameState::GameWon:
        mCurrentMenu = std::make_unique<WinScreen>(*mWindow);
        break;
    case GameState::HowToPlay:
        mCurrentScene.reset();
        mCurrentMenu = std::make_unique<InstructionsScreen>(*mWindow, [this]() { handleHowToPlayCallback(); });
        break;
    case GameState::Playing:
        mCurrentMenu.reset();
        if (!mCurrentScene) {
            mCurrentScene = std::make_unique<LevelOneScene>();
            mCurrentScene->setWindow(mWindow);
            mCurrentScene->initialize();
        }
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

    while (mWindow->isOpen())
    {
        processEvents();
        timeSinceLastUpdate += clock.restart();

        while (timeSinceLastUpdate > TimePerFrame)
        {
            timeSinceLastUpdate -= TimePerFrame;
            update(TimePerFrame);
        }

        render();
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
        if (mWindow) {
            mWindow->close();
        }
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
    case GameState::MainMenu:
    case GameState::Paused:
    case GameState::GameOver:
    case GameState::GameWon:
    case GameState::HowToPlay:
        if (mCurrentMenu)
            mCurrentMenu->update(deltaTime);
        break;
    case GameState::Playing:
        if (mCurrentScene)
            mCurrentScene->update(deltaTime);
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
        // Fall through to draw pause menu on top
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
    mCurrentScene = std::move(scene);
    if (mCurrentScene && mWindow)
    {
        mCurrentScene->setWindow(mWindow);
        mCurrentScene->initialize();
    }
}