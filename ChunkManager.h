    //
    // Created by User on 4/3/2025.
    //

    #ifndef CHUNKMANAGER_H
    #define CHUNKMANAGER_H
    #include <raylib.h>
    #include <unordered_map>

    #include "Player.h"
    #include "Grid.h"

    struct VectorHash {
        size_t operator()(const Vector2& v) const
        {
            size_t xHash = std::hash<float>()(v.x);
            size_t yHash = std::hash<float>()(v.y) << 1;
            return xHash ^ yHash;
        }
    };

    struct VectorEqual {
        bool operator()(const Vector2& v1, const Vector2& v2) const {
            return (int)round(v1.x) == (int)round(v2.x) && (int)round(v1.y) == (int)round(v2.y);
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

        bool operator==(const Chunk& chunk) const {
            return (int)round(chunk.position.x) == (int)round(position.x) && (int)round(chunk.position.z) == (int)round(position.z);
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
        std::unordered_map<Vector2, Chunk, VectorHash, VectorEqual> activeChunks = {};
    public:
        Grid* grid;
        Chunk* currentPlayerChunk = nullptr;


        ChunkManager(Grid* _grid) {
            grid = _grid;
        }

        bool IsInNewChunk(Player &player) {
            const Vector2 playerGridCoordinates = grid->GetGridPosition(Vector2{player.position.x, player.position.z});
            const Vector2 gridCoordinateRoot = grid->GetCellPosition(playerGridCoordinates);
            if (activeChunks.find(playerGridCoordinates) != activeChunks.end()) {
                if (currentPlayerChunk != nullptr) {
                    if(*currentPlayerChunk == activeChunks[playerGridCoordinates]) {
                        return false;
                    }
                    activeChunks.erase(grid->GetGridPosition(Vector2{currentPlayerChunk->position.x, currentPlayerChunk->position.z}));
                    currentPlayerChunk = &activeChunks[playerGridCoordinates];
                    return true;
                }
                currentPlayerChunk = &activeChunks[playerGridCoordinates];
                return true;
            }
            activeChunks.insert({playerGridCoordinates, Chunk{Vector3{gridCoordinateRoot.x, 1, gridCoordinateRoot.y}}});
            return true;
        };


        void GenerateChunk() {;
            for (auto& valuePair : activeChunks) {
                valuePair.second.DrawChunk();
            }
        }
    };
    #endif //CHUNKMANAGER_H
