//XANDER?!?!?!?

#ifndef CHUNK_H
#define CHUNK_H

#include <raylib.h>
#include <cstdint>
#include <cstdint>


float GetBlock(float x, float y, float z) {
    // Base settings
    const int baseHeight = 32;             // Base terrain height
    const float mainTerrainScale = 0.005f; // Main terrain noise scale (lower = smoother)
    const float detailScale = 0.05f;       // Detail noise scale

    FastNoiseLite mainNoise;
    mainNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    mainNoise.SetFrequency(mainTerrainScale);
    mainNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    mainNoise.SetFractalOctaves(4);
    mainNoise.SetFractalLacunarity(2.0f);
    mainNoise.SetFractalGain(0.5f);

    // Get base terrain height
    float heightNoise = mainNoise.GetNoise(x, z);

    // Convert to [0,1] range from [-1,1]
    heightNoise = (heightNoise + 1.0f) * 0.5f;

    // Create region distinction - terrain type based on noise value
    float terrainFactor;

    if (heightNoise < 0.4f) {
        // Flatter plains - suppress height variation
        terrainFactor = heightNoise * 0.3f;
    } else if (heightNoise < 0.6f) {
        // Gradual transition zone - slight hills
        terrainFactor = 0.12f + (heightNoise - 0.4f) * 1.5f;
    } else {
        // Steep mountains - amplify height variation
        terrainFactor = 0.42f + powf((heightNoise - 0.6f) * 2.5f, 2.0f);
    }

    // Calculate terrain height with enhanced contrast
    int terrainHeight = baseHeight + static_cast<int>(terrainFactor * 60.0f);

    // Add surface variation only to transition zones and mountains
    if (y == terrainHeight - 1 && heightNoise > 0.4f) {
        FastNoiseLite detailNoise;
        detailNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        detailNoise.SetFrequency(detailScale);

        float detailValue = detailNoise.GetNoise(x, z);

        // Add small variation to surface layer only
        if (detailValue > 0.4f) {
            terrainHeight += 1;
        }
    }

    // Simple grass/dirt layering
    if (y < terrainHeight) {
        // Top layer is grass (value 2) - except very steep areas become stone
        if (y == terrainHeight - 1) {
            // Check if we're on a steep mountain (use steepness detection)
            FastNoiseLite gradientNoise = mainNoise;
            float steepness = fabsf(gradientNoise.GetNoise(x+1, z) - gradientNoise.GetNoise(x-1, z)) +
                              fabsf(gradientNoise.GetNoise(x, z+1) - gradientNoise.GetNoise(x, z-1));

            // Stone tops for very steep areas
            if (heightNoise > 0.8f && steepness > 0.15f) {
                return 3; // Stone
            }
            return 2;  // Grass
        }
        // Few layers below are dirt (value 1)
        else if (y > terrainHeight - 5) {
            return 1;  // Dirt
        }
        // Everything below is stone (value 3)
        else {
            return 3;  // Stone
        }
    }

    // Air above terrain
    return 0;
}
class Chunk {
private:
    uint8_t chunkData[16][256][16] = {0};
    Model chunkMesh;
    bool meshIsGenerated = false;

public:
    bool isDirty = true;
    Vector3 position;

    Chunk() {
        position = Vector3{0, 0, 0};
        InitializeChunkData();
    }

    Chunk(Vector3 _position) {
        position = _position;
        InitializeChunkData();
    }

    ~Chunk() {
        if (meshIsGenerated) {
            UnloadModel(chunkMesh);
        }
    }

    //Called when chunk is created
    void InitializeChunkData() {
        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 256; y++) {
                for (int z = 0; z < 16; z++) {
                    chunkData[x][y][z] = GetBlock(position.x + x, position.y + y, position.z + z);
                }
            }
        }
    }

    //Called every frame
    void DrawChunk() {
        if (isDirty) {
            GenerateChunkMesh();
            isDirty = false;
        }

        DrawModel(chunkMesh, position, 1.0f, WHITE);
    }

    void GenerateChunkMesh() {
        if (meshIsGenerated) {
            UnloadModel(chunkMesh);
        }

        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> texCoords;
        std::vector<unsigned short> indices;
        std::vector<unsigned char> colors;

        unsigned short indexCounter = 0;

        float height = 1;
        float width = 1;
        float length = 1;

        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 256; y++) {
                for (int z = 0; z < 16; z++) {
                    int blockData = chunkData[x][y][z];
                    if (blockData != 0 && IsVisible(x, y, z)) {
                        //Gets the center of the chunk relative to the chunk position;
                        float cx = x + width / 2;
                        float cy = y + height / 2;
                        float cz = z + length / 2;

                        Vector3 blockVertices[8] = {
                            Vector3{cx - width / 2, cy - height / 2, cz - length / 2}, //LLB 0
                            Vector3{cx + width / 2, cy - height / 2, cz - length / 2}, //RLB 1
                            Vector3{cx - width / 2, cy + height / 2, cz - length / 2}, //LTB 2
                            Vector3{cx + width / 2, cy + height / 2, cz - length / 2}, //RTB 3
                            Vector3{cx - width / 2, cy - height / 2, cz + length / 2}, //LLF 4
                            Vector3{cx + width / 2, cy - height / 2, cz + length / 2}, //RLF 5
                            Vector3{cx - width / 2, cy + height / 2, cz + length / 2}, //LTF 6
                            Vector3{cx + width / 2, cy + height / 2, cz + length / 2}, //RTF 7
                        };

                        const Color sideColor = BROWN;
                        const Color topColor = GREEN;
                        const Color bottomColor = DARKBROWN;

                        if (IsFaceVisible(x, y, z, 5)) {
                            AddFaceToMesh(blockVertices[0], blockVertices[2], blockVertices[1], blockVertices[3],
                                          Vector3{0, 0, -1}, sideColor, vertices, normals, texCoords, colors, indices,
                                          indexCounter);
                        }

                        // Front face (+Z)
                        if (IsFaceVisible(x, y, z, 4)) {
                            AddFaceToMesh(blockVertices[4], blockVertices[5], blockVertices[6], blockVertices[7],
                                          Vector3{0, 0, 1}, sideColor, vertices, normals, texCoords, colors, indices,
                                          indexCounter);
                        }

                        // Left face (-X)
                        if (IsFaceVisible(x, y, z, 0)) {
                            AddFaceToMesh(blockVertices[0], blockVertices[4], blockVertices[2], blockVertices[6],
                                          Vector3{-1, 0, 0}, sideColor, vertices, normals, texCoords, colors, indices,
                                          indexCounter);
                        }

                        // Right face (+X)
                        if (IsFaceVisible(x, y, z, 1)) {
                            AddFaceToMesh(blockVertices[1], blockVertices[3], blockVertices[5], blockVertices[7],
                                          Vector3{1, 0, 0}, sideColor, vertices, normals, texCoords, colors, indices,
                                          indexCounter);
                        }

                        // Bottom face (-Y)
                        if (IsFaceVisible(x, y, z, 3)) {
                            AddFaceToMesh(blockVertices[0], blockVertices[1], blockVertices[4], blockVertices[5],
                                          Vector3{0, -1, 0}, bottomColor, vertices, normals, texCoords, colors, indices,
                                          indexCounter);
                        }

                        // Top face (+Y)
                        if (IsFaceVisible(x, y, z, 2)) {
                            AddFaceToMesh(blockVertices[2], blockVertices[6], blockVertices[3], blockVertices[7],
                                          Vector3{0, 1, 0}, topColor, vertices, normals, texCoords, colors, indices,
                                          indexCounter);
                        }
                    }
                }
            }
        }

        Mesh mesh = { 0 };
        mesh.vertexCount = vertices.size() / 3;
        mesh.triangleCount = indices.size() / 3;

        mesh.vertices = (float*)MemAlloc(vertices.size() * sizeof(float));
        memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(float));

        mesh.normals = (float*)MemAlloc(normals.size() * sizeof(float));
        memcpy(mesh.normals, normals.data(), normals.size() * sizeof(float));

        mesh.texcoords = (float*)MemAlloc(texCoords.size() * sizeof(float));
        memcpy(mesh.texcoords, texCoords.data(), texCoords.size() * sizeof(float));

        mesh.colors = (unsigned char*)MemAlloc(colors.size() * sizeof(unsigned char));
        memcpy(mesh.colors, colors.data(), colors.size() * sizeof(unsigned char));

        mesh.indices = (unsigned short*)MemAlloc(indices.size() * sizeof(unsigned short));
        memcpy(mesh.indices, indices.data(), indices.size() * sizeof(unsigned short));

        // Upload mesh data to GPU
        UploadMesh(&mesh, false);

        // Create material and model
        Material material = LoadMaterialDefault();
        chunkMesh = LoadModelFromMesh(mesh);

        meshIsGenerated = true;

    }

    void AddFaceToMesh(Vector3 v1, Vector3 v2, Vector3 v3, Vector3 v4,Vector3 normal, Color color,std::vector<float> &vertices, std::vector<float> &normals, std::vector<float> &texCoords, std::vector<unsigned char> &colors,std::vector<unsigned short> &indices, unsigned short &indexCounter) {
        // First triangle (v1, v2, v3)
        // Add vertices
        vertices.push_back(v1.x);
        vertices.push_back(v1.y);
        vertices.push_back(v1.z);
        vertices.push_back(v2.x);
        vertices.push_back(v2.y);
        vertices.push_back(v2.z);
        vertices.push_back(v3.x);
        vertices.push_back(v3.y);
        vertices.push_back(v3.z);

        // Add normals
        for (int i = 0; i < 3; i++) {
            normals.push_back(normal.x);
            normals.push_back(normal.y);
            normals.push_back(normal.z);
        }

        // Add basic texture coordinates
        texCoords.push_back(0.0f);
        texCoords.push_back(0.0f);
        texCoords.push_back(1.0f);
        texCoords.push_back(0.0f);
        texCoords.push_back(0.0f);
        texCoords.push_back(1.0f);

        // Add colors
        for (int i = 0; i < 3; i++) {
            colors.push_back(color.r);
            colors.push_back(color.g);
            colors.push_back(color.b);
            colors.push_back(color.a);
        }

        // Add indices
        indices.push_back(indexCounter);
        indices.push_back(indexCounter + 1);
        indices.push_back(indexCounter + 2);
        indexCounter += 3;

        // Second triangle (v4, v3, v2)
        // Add vertices
        vertices.push_back(v4.x);
        vertices.push_back(v4.y);
        vertices.push_back(v4.z);
        vertices.push_back(v3.x);
        vertices.push_back(v3.y);
        vertices.push_back(v3.z);
        vertices.push_back(v2.x);
        vertices.push_back(v2.y);
        vertices.push_back(v2.z);

        // Add normals
        for (int i = 0; i < 3; i++) {
            normals.push_back(normal.x);
            normals.push_back(normal.y);
            normals.push_back(normal.z);
        }

        // Add basic texture coordinates
        texCoords.push_back(1.0f);
        texCoords.push_back(1.0f);
        texCoords.push_back(0.0f);
        texCoords.push_back(1.0f);
        texCoords.push_back(1.0f);
        texCoords.push_back(0.0f);

        // Add colors
        for (int i = 0; i < 3; i++) {
            colors.push_back(color.r);
            colors.push_back(color.g);
            colors.push_back(color.b);
            colors.push_back(color.a);
        }

        // Add indices
        indices.push_back(indexCounter);
        indices.push_back(indexCounter + 1);
        indices.push_back(indexCounter + 2);
        indexCounter += 3;
    }

    //Checks if block is visible
    bool IsVisible(int x, int y, int z) {
        // If the block is air, it is not visible
        if (chunkData[x][y][z] == 0) return false;

        // Check neighboring blocks
        // If we're on the edge, assume visible for now
        if (x == 0 || x == 15 || y == 0 || y == 63 || z == 0 || z == 15){
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

    //Checks if face is Visible
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

    // == Comparator override, identifies if chunks are the same using position
    bool operator==(const Chunk &chunk) const {
        return (int) round(chunk.position.x) == (int) round(position.x) && (int) round(chunk.position.z) == (int)
               round(position.z);
    }

};


#endif //CHUNK_H
