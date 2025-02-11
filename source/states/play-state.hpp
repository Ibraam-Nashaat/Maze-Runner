#pragma once

#include <application.hpp>

#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/movement.hpp>
#include <systems/collision.hpp>
#include <asset-loader.hpp>
#include <systems/sound-system.hpp>
#include <systems/game-actions.hpp>
#include <GLFW/glfw3.h>

// This state shows how to use the ECS framework and deserialization.
class Playstate : public our::State
{

    our::World world;
    our::ForwardRenderer renderer;
    our::FreeCameraControllerSystem cameraController;
    our::MovementSystem movementSystem;
    our::CollisionSystem collisionSystem;

    void onInitialize() override
    {
        // First of all, we get the scene configuration from the app config
        auto &config = getApp()->getConfig()["scene"];
        // If we have assets in the scene config, we deserialize them
        if (config.contains("assets"))
        {
            our::deserializeAllAssets(config["assets"]);
        }
        // init assets audio unordered map
        our::SoundSystem::initMap();
        if (our::SoundSystem::global_music_state)
        {
            our::SoundSystem::play_custom_sound("Game", false, true);
        }
        //   reset app score
        our::GameActionsSystem::resetScore();
        // If we have a world in the scene config, we use it to populate our world
        if (config.contains("world"))
        {
            world.deserialize(config["world"]);
        }
        // We initialize the camera controller system since it needs a pointer to the app
        cameraController.enter(getApp());
        collisionSystem.enter(getApp());
        // Then we initialize the renderer
        auto size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);
    }

    void onDraw(double deltaTime) override
    {
        // Here, we just run a bunch of systems to control the world logic
        movementSystem.update(&world, (float)deltaTime);
        cameraController.update(&world, (float)deltaTime);
        collisionSystem.update(&world, (float)deltaTime);
        // And finally we use the renderer system to draw the scene
        renderer.render(&world);
        renderer.updateFogTime((float)glfwGetTime());

        // Get a reference to the keyboard object
        auto &keyboard = getApp()->getKeyboard();

        if (keyboard.justPressed(GLFW_KEY_ESCAPE))
        {
            // If the escape  key is pressed in this frame, go to the play state
            getApp()->changeState("score");
        }
        else if (keyboard.justPressed(GLFW_KEY_M))
        {
            // start or stop the background music
            if (our::SoundSystem::global_music_state)
            {
                our::SoundSystem::stop_custom_sound("Game");
                our::SoundSystem::global_music_state = false;
            }
            else
            {
                our::SoundSystem::play_custom_sound("Game", false, true);
                our::SoundSystem::global_music_state = true;
            }
        }
        else if (keyboard.justPressed(GLFW_KEY_F))
        {
            our::GameActionsSystem::toggleFlash();
            our::SoundSystem::play_custom_sound("FLASHLIGHT", false, false);
        }
    }

    void onDestroy() override
    {
        // Don't forget to destroy the renderer
        renderer.destroy();
        // On exit, we call exit for the camera controller system to make sure that the mouse is unlocked
        cameraController.exit();
        // Clear the world
        world.clear();
        // uminit all sounds
        // and we delete all the loaded assets to free memory on the RAM and the VRAM
        our::SoundSystem::destroy_sounds();
        our::clearAllAssets();
    }
};