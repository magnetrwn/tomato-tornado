#include <sys/types.h>

#include "box2d/box2d.h"
#include "raylib.h"

#include "world.hpp"
#include "storm.hpp"
#include "wind.hpp"
#include "warning.hpp"
#include "cursor.hpp"

int main(void) {
    constexpr static float SCR_WIDTH = 600.0f;
    constexpr static float SCR_HEIGHT = 960.0f;
    constexpr static float SCR_W_HALF = static_cast<float>(SCR_WIDTH) / 2.0f;
    constexpr static float SCR_H_HALF = static_cast<float>(SCR_HEIGHT) / 2.0f;
    constexpr static float TETROMINO_SIZE = 25.0f;

    #ifdef FPS
        SetTargetFPS(FPS);
    #else
        SetTargetFPS(60);
    #endif

    #ifdef VSYNC
        SetConfigFlags(FLAG_VSYNC_HINT);
    #endif

    InitWindow(SCR_WIDTH, SCR_HEIGHT, "Raylib + Box2D Testing");

    b2World world(b2Vec2{ 0.0f, 10.0f});

    WorldMgr worldMgr(world);

    worldMgr.add(
        WorldMgr::BodyInit(SCR_W_HALF, SCR_HEIGHT - 30.0f, 500.0f, 20.0f),
        WorldMgr::BodyType::STATIC
    );

    worldMgr.add(
        WorldMgr::BodyInit(SCR_W_HALF + 250.0f, SCR_HEIGHT - 50.0f, 20.0f, 60.0f),
        WorldMgr::BodyType::STATIC
    );

    worldMgr.add(
        WorldMgr::BodyInit(SCR_W_HALF - 250.0f, SCR_HEIGHT - 50.0f, 20.0f, 60.0f),
        WorldMgr::BodyType::STATIC
    );

    double lastTime = GetTime();
    double updateCloudsTime = GetTime();
    double changeWindTime = GetTime();
    float newTime = 0.0f;
    WindState wind;
    double addWarningTime = GetTime();
    WarningView warning(SCR_WIDTH, SCR_HEIGHT);
    PlayerCursor cursor(TETROMINO_SIZE);
    float cursorAngle = 0.0f;

    StormView storm(SCR_WIDTH, SCR_HEIGHT, 24, 80);

    cursor.set(6);

    while (!WindowShouldClose()) {
        BeginDrawing();

            float rndUnit = static_cast<float>(GetRandomValue(0, 10000) / 10000.0f);

            ClearBackground({ 53, 53, 53, 255 });

            worldMgr.prune(SCR_H_HALF + 275.0f, SCR_W_HALF, SCR_H_HALF);    

            if (GetTime() - lastTime >= 0.75f) {
                lastTime = GetTime();
                worldMgr.add(
                    WorldMgr::BodyInit(
                        SCR_W_HALF + static_cast<float>(GetRandomValue(-200, 200)), 
                        rndUnit * 110.0f, 
                        TETROMINO_SIZE, 
                        TETROMINO_SIZE, // NOTE: technically rectangular built tetrominoes are supported
                        rndUnit * 90.0f
                    ),
                    WorldMgr::BodyType::DYNAMIC,
                    GetRandomValue(-1, Tetromino::TETROMINO_COUNT - 1),
                    { 103, 164, 249, static_cast<unsigned char>(GetRandomValue(192, 255)) }
                );
            }

            worldMgr.update(GetFrameTime());
            worldMgr.draw();

            cursor.update(GetMousePosition(), cursorAngle);
            if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
                cursorAngle += 2.5f + (cursorAngle > 360.0f ? -360.0f : 0.0f);

            if (GetTime() - updateCloudsTime >= 0.1f) {
                updateCloudsTime = GetTime();
                storm.updateClouds();
            }

            if (GetTime() - changeWindTime >= 3.0f) {
                changeWindTime = GetTime();
                newTime = rndUnit * 16.0f - 8.0f;
                wind.setTargetWind(newTime);
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                warning.startWarning(static_cast<WarningView::WarningDir>(GetRandomValue(0, 2)));

            wind.step(GetFrameTime());
            warning.step(GetFrameTime());
            storm.updateDroplets(GetFrameTime(), wind.getWind());
            storm.draw();
            cursor.draw();
            warning.draw();
            
            DrawText(std::to_string(GetFPS()).c_str(), 10, 10, 30, RED);
            DrawText(std::to_string(worldMgr.count()).c_str(), 10, 52, 30, RED); 
            DrawText(std::to_string(wind.getWind()).c_str(), 10, 94, 30, RED);
            DrawText(std::to_string(newTime).c_str(), 10, 136, 30, LIME);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}