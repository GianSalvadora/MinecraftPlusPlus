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

float GetBlock(float x, float y, float z) {
    const int base = 10; // Base height of the terrain
    const float strength = 15; // Overall terrain height variation
    const int octaves = 4; // Number of noise layers
    const float lacunarity = 2.0f; // Frequency multiplier per octave
    const float persistence = 0.5f; // Amplitude multiplier per octave

    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    float noiseValue = 0.0f;
    float frequency = 0.3f;
    float amplitude = 1.0f;
    float totalAmplitude = 0.0f;

    for (int i = 0; i < octaves; i++) {
        noiseValue += noise.GetNoise(x * frequency, z * frequency) * amplitude;
        totalAmplitude += amplitude;
        frequency *= lacunarity;
        amplitude *= persistence;
    }

    noiseValue /= totalAmplitude; // Normalize the noise value
    int finalHeight = base + static_cast<int>(noiseValue * strength);

    return (y < finalHeight) ? 1 : 0;
}


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
private:
    uint8_t chunkData[16][64][16] = {0};

public:
    Vector3 position;

    Chunk() {
        position = Vector3{0, 0, 0};
        InitializeChunkData();
    }

    Chunk(Vector3 _position) {
        position = _position;
        InitializeChunkData();
    }

    bool operator==(const Chunk &chunk) const {
        return (int) round(chunk.position.x) == (int) round(position.x) && (int) round(chunk.position.z) == (int)
               round(position.z);
    }

    //Called every frame
    bool IsFaceVisible(int x, int y, int z, int face) {
        switch (face) {
            case 0: return (x == 0) || (chunkData[x - 1][y][z] == 0); // -x face
            case 1: return (x == 15) || (chunkData[x + 1][y][z] == 0); // +x face
            case 2: return (y == 63) || (chunkData[x][y + 1][z] == 0); // +y face
            case 3: return (y == 0) || (chunkData[x][y - 1][z] == 0); // -y face
            case 4: return (z == 15) || (chunkData[x][y][z + 1] == 0); // +z face
            case 5: return (z == 0) || (chunkData[x][y][z - 1] == 0); // -z face
            default: return false; // Invalid face
        }
    }

    //Called every frame
    void DrawChunk() {
        const int height = 1;
        const int width = 1;
        const int length = 1;

        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 64; y++) {
                for (int z = 0; z < 16; z++) {
                    if (chunkData[x][y][z] != 0 && IsVisible(x, y, z)) {
                        float halfWidth = width / 2.0f;
                        float halfHeight = height / 2.0f;
                        float halfLength = length / 2.0f;

                        float cx = position.x + x + halfWidth;
                        float cy = position.y + y + halfHeight;
                        float cz = position.z + z + halfLength;

                        Vector3 vertices[8] = {
                            Vector3{cx - halfWidth, cy - halfHeight, cz - halfLength}, //LLB 0
                            Vector3{cx + halfWidth, cy - halfHeight, cz - halfLength}, //RLB 1
                            Vector3{cx - halfWidth, cy + halfHeight, cz - halfLength}, //LTB 2
                            Vector3{cx + halfWidth, cy + halfHeight, cz - halfLength}, //RTB 3
                            Vector3{cx - halfWidth, cy - halfHeight, cz + halfLength}, //LLF 4
                            Vector3{cx + halfWidth, cy - halfHeight, cz + halfLength}, //RLF 5
                            Vector3{cx - halfWidth, cy + halfHeight, cz + halfLength}, //LTF 6
                            Vector3{cx + halfWidth, cy + halfHeight, cz + halfLength}, //RTF 7
                        };

                        if (IsFaceVisible(x, y, z, 5)) {
                            //          LEFT LOW        LEFT TOP        RIGHT LOW        RIGHT TOP
                            DrawFace(vertices[0], vertices[2], vertices[1], vertices[3], BROWN); // BACK
                        }

                        if (IsFaceVisible(x, y, z, 4)) {
                            //          LEFT LOW        RIGHT LOW       LEFT TOP        RIGHT TOP
                            DrawFace(vertices[4], vertices[5], vertices[6], vertices[7], BROWN); // FRONT
                        }
                        if (IsFaceVisible(x, y, z, 0)) {
                            //          LOW BACK        LOW FRONT       TOP BACK       TOP FRONT
                            DrawFace(vertices[0], vertices[4], vertices[2], vertices[6], BROWN); // LEFT
                        }
                        if (IsFaceVisible(x, y, z, 1)) {
                            //          LEFT LOW        RIGHT LOW       LEFT TOP        RIGHT TOP
                            DrawFace(vertices[1], vertices[3], vertices[5], vertices[7], BROWN); // RIGHT
                        }
                        if (IsFaceVisible(x, y, z, 3)) {
                            //          LOW BACK        RIGHT BACK      LEFT FRONT      RIGHT FRONT
                            DrawFace(vertices[0], vertices[1], vertices[4], vertices[5], DARKBROWN); // BOTTOM
                        }
                        if (IsFaceVisible(x, y, z, 2)) {
                            //          LEFT LOW        LEFT TOP        RIGHT LOW        RIGHT TOP
                            DrawFace(vertices[2], vertices[6], vertices[3], vertices[7], GREEN); // TOP
                        }
                    }
                }
            }
        }
    }

    //Called every frame
    void DrawFace(Vector3 v1, Vector3 v2, Vector3 v3, Vector3 v4, Color color) {
        DrawTriangle3D(v1, v2, v3, color);
        DrawTriangle3D(v4, v3, v2, color);
    }

    //
    void InitializeChunkData() {
        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 16; y++) {
                for (int z = 0; z < 16; z++) {
                    chunkData[x][y][z] = GetBlock(position.x + x, position.y + y, position.z + z);
                }
            }
        }
    }

    bool IsVisible(int x, int y, int z) {
        // If the block is air, it is not visible
        if (chunkData[x][y][z] == 0) return false;

        // Check neighboring blocks
        // If we're on the edge, assume visible for now
        if (x == 0 || x == 15 || y == 0 || y == 15 || z == 0 || z == 15) {
            return true;
        }

        // Check all 6 directions
        if (chunkData[x + 1][y][z] == 0) return true;
        if (chunkData[x - 1][y][z] == 0) return true;
        if (chunkData[x][y + 1][z] == 0) return true;
        if (chunkData[x][y - 1][z] == 0) return true;
        if (chunkData[x][y][z + 1] == 0) return true;
        if (chunkData[x][y][z - 1] == 0) return true;

        return false; // Fully surrounded
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

    //Called every frame to draw (OPTIMIZE THIS)
    void GenerateChunk() {
        for (auto &valuePair: activeChunks) {
            valuePair.second.DrawChunk();
        }
    }
};
#endif //CHUNKMANAGER_H
