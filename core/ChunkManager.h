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
#include <cstdint>
#include "FastNoiseLite.h"
#include "Chunk.h"


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

class ChunkManager {
private:
    std::unordered_map<Vector2Int, Chunk, VectorHash, VectorEqual> activeChunks = {};

public:
    Grid *grid;
    Chunk *currentPlayerChunk = nullptr;


    ChunkManager(Grid *_grid) {
        grid = _grid;
    }

    //Called every frame, checks if player is in a different chunk from currentPlayerChunk
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

    //Creates new Chunks around the currentPlayerChunk (Currently called every frame that the player is in a new chunk)
    void FormActiveChunks(int renderDistance) {
        if (currentPlayerChunk != nullptr) {
            std::vector<Vector2Int> chunkPositions;

            //Gets the center
            Vector3 centerChunkPos = currentPlayerChunk->position;
            Vector2Int centerChunkGridPosition = grid->GetGridPosition(Vector2{centerChunkPos.x, centerChunkPos.z});

            //Gets the range
            int minX = centerChunkGridPosition.x - renderDistance;
            int maxX = centerChunkGridPosition.x + renderDistance;
            int minY = centerChunkGridPosition.y - renderDistance;
            int maxY = centerChunkGridPosition.y + renderDistance;

            //Creates the chunks around the center centerChunk
            for (int x = minX; x <= maxX; x++) {
                for (int y = minY; y <= maxY; y++) {
                    Vector2Int gridPos{x, y};
                    Vector3 newCellPos{grid->GetCellPosition(gridPos).x, 1, grid->GetCellPosition(gridPos).y};

                    //Checks if x, y is already in activeChunks
                    if (activeChunks.find({x, y}) == activeChunks.end()) {
                        activeChunks.insert({{x, y}, Chunk{newCellPos}});
                    }

                    //stores the positions
                    chunkPositions.push_back({x, y});
                }
            }

            //stores the positions that are in the activeChunks and not in the new chunkPositions
            std::vector<Vector2Int> chunksToRemove;
            for (const auto &p: activeChunks) {
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

            //remove every unnecessary chunks
            for (const auto &p: chunksToRemove) {
                activeChunks.erase(p);
            }
        }
    }

    //Called every frame to draw (OPTIMIZE THIS LINE)
    void GenerateChunk() {
        for (auto &valuePair: activeChunks) {
            valuePair.second.DrawChunk();
        }
    }
};

#endif //CHUNKMANAGER_H
