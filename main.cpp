#include <raylib.h>
#include <math.h>
#include <iostream>

#include "Grid.h"

void RenderChunk(Vector2 rootPosition) {
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            DrawCube(Vector3{rootPosition.x + x * 1 + 0.5f, 1, rootPosition.y + y * 1 + 0.5f}, 1, 1, 1, GREEN);
        }
    }
}

class Player {
private:
    float speed = 50.f;
    float cameraPitch = 0.0f;
    float cameraYaw = -90.0f;
    Vector3 offset = {0, -2, 0};
public:
    Vector3 position = {10, 32, 10};
    float height;

    Player(Vector3 pos, float h) {
        position = pos;
        height = h;
    }

    void UpdatePlayer(Camera3D& camera) {
        // Handle mouse look (rotation)
        Vector2 mouseDelta = GetMouseDelta();
        float mouseSensitivity = 0.2f;

        // Update yaw (left/right) and pitch (up/down) based on mouse movement
        cameraYaw += mouseDelta.x * mouseSensitivity;
        cameraPitch -= mouseDelta.y * mouseSensitivity;

        // Clamp pitch to avoid flipping
        if (cameraPitch > 89.0f) cameraPitch = 89.0f;
        if (cameraPitch < -89.0f) cameraPitch = -89.0f;

        // Calculate forward direction using yaw and pitch
        Vector3 direction;
        direction.x = cos(DEG2RAD * cameraYaw) * cos(DEG2RAD * cameraPitch);
        direction.y = sin(DEG2RAD * cameraPitch);
        direction.z = sin(DEG2RAD * cameraYaw) * cos(DEG2RAD * cameraPitch);

        // Calculate forward and right vectors for movement
        Vector3 forward = { direction.x, 0, direction.z }; // Ignore Y for level movement

        // Normalize forward vector
        float forwardLength = sqrtf(forward.x * forward.x + forward.z * forward.z);
        if (forwardLength > 0.0f) {
            forward.x /= forwardLength;
            forward.z /= forwardLength;
        }

        // Calculate right vector (perpendicular to forward)
        Vector3 right = { forward.z, 0, -forward.x };

        // Calculate movement based on key input
        Vector3 moveVec = { 0, 0, 0 };

        if (IsKeyDown(KEY_W)) {
            moveVec.x += forward.x;
            moveVec.z += forward.z;
        }
        if (IsKeyDown(KEY_S)) {
            moveVec.x -= forward.x;
            moveVec.z -= forward.z;
        }
        if (IsKeyDown(KEY_A)) {
            moveVec.x += right.x;
            moveVec.z += right.z;
        }
        if (IsKeyDown(KEY_D)) {
            moveVec.x -= right.x;
            moveVec.z -= right.z;
        }

        if (IsKeyDown(KEY_SPACE)) {
            moveVec.y += 1;
        }
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            moveVec.y -= 1;
        }
        // Normalize movement vector
        float moveLength = sqrtf(moveVec.x * moveVec.x + moveVec.z * moveVec.z);
        if (moveLength > 0.0f) {
            moveVec.x /= moveLength;
            moveVec.z /= moveLength;
        }

        // Apply movement
        float frameSpeed = speed * GetFrameTime();
        position.x += moveVec.x * frameSpeed;
        position.y += moveVec.y * frameSpeed;
        position.z += moveVec.z * frameSpeed;

        // Update camera position and target
        camera.position = (Vector3){ position.x + offset.x, position.y + height + offset.y, position.z + offset.z };
        camera.target = (Vector3){
            camera.position.x + direction.x,
            camera.position.y + direction.y,
            camera.position.z + direction.z
        };
    }
};

int main() {
    constexpr int screenWidth = 1080;
    constexpr int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "MinecraftPlusPlus");
    SetTargetFPS(60);

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
    camera.up = (Vector3){ 0.f, 1.0f, 0.0f };
    camera.fovy = 120.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    constexpr int renderDistance = 4;

    Player player = Player{ (Vector3){ 10.0f, 35.0f, 0.0f }, 2.0f };
    DisableCursor();

    while (!WindowShouldClose()) {
        player.UpdatePlayer(camera);
        BeginDrawing();
        ClearBackground(SKYBLUE);



        BeginMode3D(camera);

        Grid grid(Vector2{10, 10}, 16, BLACK);
        Grid miniGrid({160, 160}, 1, GRAY);
        miniGrid.DrawGrid();
        grid.DrawGrid();
        Vector2 cellPosition = grid.GetCellPosition(grid.GetGridPosition(Vector2{player.position.x, player.position.z}));

        DrawCircle3D(Vector3{player.position.x, 1, player.position.z}, renderDistance, Vector3{90, 0, 0}, 90, BLACK);
        for (int x = -renderDistance; x <= renderDistance; ++x) {
            for (int y = -renderDistance; y <= renderDistance; ++y) {
                Vector2 chunkPosition = { grid.GetGridPosition(cellPosition).x + x, grid.GetGridPosition(cellPosition).y + y };
                float dist = sqrtf(x * x + y * y);
                if (dist <= renderDistance) {
                    Vector2 cellPosition = grid.GetCellPosition(chunkPosition);
                    RenderChunk(cellPosition);
                }
            }
        }

        EndMode3D();

        std::cout << GetFPS() << std::endl;
        EndDrawing();
    }

    CloseWindow();
    return 0;
}