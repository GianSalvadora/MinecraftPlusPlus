#include <raylib.h>
#include "Grid.h"
#include "ChunkManager.h"
#include "Player.h"
#include "rlgl.h"
int main() {

    constexpr int screenWidth = 1080;
    constexpr int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "MinecraftPlusPlus");
    Camera3D camera = { 0 };
    camera.position = (Vector3){0.0f, 2.0f, 0.0f};
    camera.target = (Vector3){0.0f, 2.0f, 0.0f};
    camera.up = (Vector3){0.f, 1.0f, 0.0f};
    camera.fovy = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    int renderDistance = 8;
    Player player = Player{(Vector3){10.0f, 35.0f, 0.0f}, 2.0f};
    DisableCursor();
    Grid grid(Vector2{10, 10}, 16, BLACK);
    ChunkManager chunkManager(&grid);


    while (!WindowShouldClose()) {
        player.UpdatePlayer(camera);
        BeginDrawing();
        ClearBackground(SKYBLUE);
        BeginMode3D(camera);
        grid.DrawGrid();

        if (chunkManager.IsInNewChunk(player)) {
            chunkManager.FormActiveChunks(renderDistance);
        }
        chunkManager.GenerateChunk();

        EndMode3D();
        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

