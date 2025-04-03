#ifndef GRID_H
#define GRID_H

#include "raylib.h"
#include "Vector2Int.h"


class Grid {
public:
    Vector2 gridSize;
    float cellSize;
    Color color;

    Grid(Vector2 _gridSize, float _cellSize, Color _color) {
        gridSize = _gridSize;
        cellSize = _cellSize;
        color = _color;
    }

    Vector2Int GetGridPosition(Vector2 worldPosition) {
        return Vector2Int{(int)floor(worldPosition.x/cellSize), (int)floor(worldPosition.y/cellSize)};
    }

    Vector2 GetCellPosition(Vector2Int cellPosition) {
        return Vector2{round(cellPosition.x * cellSize) , round(cellPosition.y * cellSize)};
    }

    void DrawGrid() {
        for (int x = 0; x < gridSize.x; x++) {
            for (int y = 0; y < gridSize.y; y++) {
                DrawLine3D(Vector3{x * cellSize, 1, y * cellSize}, Vector3{(x + 1) * cellSize, 1, y * cellSize}, color);
                DrawLine3D(Vector3{x * cellSize, 1, y * cellSize}, Vector3{x * cellSize, 1,  (y + 1) * cellSize}, color);
            }
        }

        DrawLine3D(Vector3{gridSize.x * cellSize, 1, 0}, Vector3{gridSize.x * cellSize, 1, gridSize.y * cellSize}, color);
        DrawLine3D(Vector3{0, 1, gridSize.y * cellSize}, Vector3{gridSize.x * cellSize, 1, gridSize.y * cellSize}, color);

    }
};

#endif