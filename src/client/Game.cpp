#include "Game.hpp"

Game::Game():
    window(&instanceSettings),
    validationLayer(&instanceSettings)
{
    instance.create(instanceSettings);
    window.create(instance,640, 720, "YAVoG");
    queue.create(window,deviceSettings);

    server.create(gf.directory/"saves"/"example");
}

Game::~Game()
{
}
