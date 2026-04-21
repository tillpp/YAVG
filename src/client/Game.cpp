#include "Game.hpp"

Game::Game():
    window(&instanceSettings),
    validationLayer(&instanceSettings)
{
    instance.create(instanceSettings);
    window.create(instance,640, 720, "YAVoG");

    queue.create(window,deviceSettings);
}

Game::~Game()
{
}
