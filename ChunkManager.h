//
// Created by User on 4/3/2025.
//

#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H
#include <raylib.h>
#include <unordered_map>

#include "Player.h"
#include "Grid.h"
#include "Vector2Int.h"
#include <vector>

struct VectorHash {
    size_t operator()(const Vector2Int &v) const {
        size_t xHash = std::hash<float>()(v.x);
        size_t yHash = std::hash<float>()(v.y) << 1;
        return xHash ^ yHash;
    }
};

struct VectorEqual {
    bool operator()(const Vector2Int &v1, const Vector2Int &v2) const {
        return round(v1.x) == round(v2.x) && round(v1.y) == round(v2.y);
    }
};

class Chunk {
public:
    Vector3 position;

    Chunk() {
        position = Vector3{0, 0, 0};
    }

    Chunk(Vector3 _position) {
        position = _position;
    }

    bool operator==(const Chunk &chunk) const {
        return (int) round(chunk.position.x) == (int) round(position.x) && (int) round(chunk.position.z) == (int)
               round(position.z);
    }

    void DrawChunk() {
        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 16; y++) {
                for (int z = 0; z < 16; z++) {
                    DrawCube(Vector3{position.x + x * 1 + 0.5f, position.y + y * 1 + 0.5f, position.z + z * 1 + 0.5f},
                             1, 1, 1, GREEN);
                }
            }
        }
    }
};

class ChunkManager {
private:
    std::unordered_map<Vector2Int, Chunk, VectorHash, VectorEqual> activeChunks = {};

public:
    Grid *grid;
    Chunk *currentPlayerChunk = nullptr;


    ChunkManager(Grid *_grid) {
        grid = _grid;
    }

    bool IsInNewChunk(Player &player) {
        const Vector2Int playerGridCoordinates = grid->GetGridPosition(Vector2{player.position.x, player.position.z});
        const Vector2 gridCoordinateRoot = grid->GetCellPosition(playerGridCoordinates);
        if (activeChunks.find(playerGridCoordinates) != activeChunks.end()) {
            if (currentPlayerChunk != nullptr) {
                if (*currentPlayerChunk == activeChunks[playerGridCoordinates]) {
                    return false;
                }
                currentPlayerChunk = &activeChunks[playerGridCoordinates];
                return true;
            }
            currentPlayerChunk = &activeChunks[playerGridCoordinates];
            return true;
        }
        activeChunks.insert({playerGridCoordinates, Chunk{Vector3{gridCoordinateRoot.x, 1, gridCoordinateRoot.y}}});
        return true;
    };

    void FormActiveChunks(int renderDistance) {
        if (currentPlayerChunk != nullptr) {
            std::vector<Vector2Int> chunkPositions;

            Vector3 centerChunkPos = currentPlayerChunk->position;
            Vector2Int centerChunkGridPosition = grid->GetGridPosition(Vector2{centerChunkPos.x, centerChunkPos.z});

            int minX = centerChunkGridPosition.x - renderDistance;
            int maxX = centerChunkGridPosition.x + renderDistance;
            int minY = centerChunkGridPosition.y - renderDistance;
            int maxY = centerChunkGridPosition.y + renderDistance;

            std::cout << minX << std::endl;
            std::cout << minY << std::endl;
            std::cout << maxX << std::endl;
            std::cout << maxY << std::endl;

            for (int x = minX; x <= maxX; x++) {
                for (int y = minY; y <= maxY; y++) {
                    Vector2Int gridPos{x, y};
                    Vector3 newCellPos{grid->GetCellPosition(gridPos).x, 1, grid->GetCellPosition(gridPos).y};

                    if (activeChunks.find({x, y}) == activeChunks.end()) {
                        activeChunks.insert({{x, y}, Chunk{newCellPos}});
                    }

                    chunkPositions.push_back({x, y});
                }
            }

            std::vector<Vector2Int> chunksToRemove;
            for (const auto &p : activeChunks) {
                std::vector<Vector2Int>::iterator i = chunkPositions.begin();

                for (; i != chunkPositions.end(); i++) {
                    if (i->x == p.first.x && i->y == p.first.y) {
                        break;
                    }
                }

                if (i == chunkPositions.end()) {
                    chunksToRemove.push_back(p.first);
                }
            }

            for (const auto &p : chunksToRemove) {
                activeChunks.erase(p);
            }
        }
    }



    void GenerateChunk() {
        std::cout << "Generating " << activeChunks.size() << " Chunks" << std::endl;
        for (auto &valuePair: activeChunks) {
            valuePair.second.DrawChunk();
        }
    }
};
#endif //CHUNKMANAGER_H
