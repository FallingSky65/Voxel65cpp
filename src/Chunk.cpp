#include "Block.hpp"
#include "Chunk.hpp"
#include "Utils.hpp"
#include "Settings.hpp"
#include "Terrain.hpp"
#include "raylib.h"
#include "raymath.h"
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <vector>

Vector2 toTexCoord(BlockType blockType, int face, int i) {
  int x, y;
  switch (blockType) {
    case GRASS:
      if (face == 3) {
        x = 0; y = 0;
      } else if (face == 4) {
        x = 2; y = 0;
      } else {
        x = 1; y = 0;
      }
      break;
    case DIRT:
      x = 2; y = 0; break;
    case AIR:
    default:
      break;
  }

  if (i == 0) return Vector2{(x + 1.0f)/TEXTURE_WIDTH, (y + 1.0f)/TEXTURE_HEIGHT};
  if (i == 1) return Vector2{(x + 1.0f)/TEXTURE_WIDTH, (y + 0.0f)/TEXTURE_HEIGHT};
  if (i == 2) return Vector2{(x + 0.0f)/TEXTURE_WIDTH, (y + 0.0f)/TEXTURE_HEIGHT};
  if (i == 3) return Vector2{(x + 0.0f)/TEXTURE_WIDTH, (y + 1.0f)/TEXTURE_HEIGHT};
  else return Vector2{0.0f, 0.0f}; 
}

Chunk::Chunk() { }

Chunk::Chunk(int x, int y, int z, Material _material, Terrain _terrain, WorldMap* _chunks) {
  setChunkPos(x, y, z);
  material = _material;
  terrain = _terrain;
  genDataTerrain();
  chunks = _chunks;
  loaded = false;
  updated = false;
}

Chunk::~Chunk() {
  unloadChunk();
}

void Chunk::setChunkPos(int x, int y, int z) {
  worldX = x; worldY = y; worldZ = z;
}

void Chunk::genDataUniform(Block block) {
  for (int i = 0; i < CHUNK_WIDTH; i++) {
    for (int j = 0; j < CHUNK_HEIGHT; j++) {
      for (int k = 0; k < CHUNK_LENGTH; k++) {
        chunkData[i][j][k] = block;
      }
    }
  }
}

void Chunk::genDataRandom() {
  for (int i = 0; i < CHUNK_WIDTH; i++) {
    for (int j = 0; j < CHUNK_HEIGHT; j++) {
      for (int k = 0; k < CHUNK_LENGTH; k++) {
        if ((rand()&1) == 1) {
          chunkData[i][j][k] = Block{DIRT};
        } else {
          chunkData[i][j][k] = Block{AIR};
        }
      }
    }
  }
}

void Chunk::genDataTerrain() {
  for (int i = 0; i < CHUNK_WIDTH; i++) {
    for (int j = 0; j < CHUNK_HEIGHT; j++) {
      for (int k = 0; k < CHUNK_LENGTH; k++) {
        chunkData[i][j][k] = Block{terrain.Evaluate(i + worldX*CHUNK_WIDTH, j + worldY*CHUNK_HEIGHT, k + worldZ*CHUNK_LENGTH)};
      }
    }
  }
}

bool Chunk::isAir(int x, int y, int z) {
  int chunkX, chunkZ;
  if (x < 0) {
    chunkX = -1;
  } else if (x >= CHUNK_WIDTH) {
    chunkX = 1;
  } else {
    chunkX = 0;
  }
  if (y < 0 || y >= CHUNK_HEIGHT) return true;
  if (z < 0) {
    chunkZ = -1;
  } else if (z >= CHUNK_LENGTH) {
    chunkZ = 1;
  } else {
    chunkZ = 0;
  }

  if (chunkX == 0 && chunkZ == 0) return (chunkData[x][y][z].blockType == AIR);
  else {
    if (chunks->count({worldX+chunkX, worldZ+chunkZ}) == 0) return true;
    else {
      return (chunks->at({worldX+chunkX, worldZ+chunkZ})->chunkData[x-chunkX*CHUNK_WIDTH][y][z-chunkZ*CHUNK_LENGTH].blockType == AIR);
    }
  }
}

void Chunk::genMesh() {
  // Part 1 : Generate Mesh Data
  int triangleCount = 0;
  int vertexCount = 0;
  std::vector<Vector3> vertices;
  std::vector<Vector3> normals;
  std::vector<Vector2> texcoords;
  std::vector<Color> colors;
  std::vector<ushort> indices;
  Chunk* adjChunk;
  // TODO: Use indices for optimization, reuse vertices between quads.

  for (int i = 0; i < CHUNK_WIDTH; i++) {
    for (int j = 0; j < CHUNK_HEIGHT; j++) {
      for (int k = 0; k < CHUNK_LENGTH; k++) {
        BlockType blockType = chunkData[i][j][k].blockType;
        if (blockType == AIR) continue;
        if (isAir(i+1, j, k)) {
          int indexOffset = vertices.size();
          // vertices
          vertices.push_back(Vector3{1.0f + i, 0.0f + j, 0.0f + k});
          vertices.push_back(Vector3{1.0f + i, 1.0f + j, 0.0f + k});
          vertices.push_back(Vector3{1.0f + i, 1.0f + j, 1.0f + k});
          vertices.push_back(Vector3{1.0f + i, 0.0f + j, 1.0f + k});
          vertexCount += 4;
          // normals
          REPEAT(4) normals.push_back(Vector3{1.0f, 0.0f, 0.0f});
          // texcoords TODO
          REPEAT(4) texcoords.push_back(toTexCoord(blockType, 1, i));
          // ambient occlusion
          Color ao = {0, 0, 0, 0};
          ao.r = !isAir(i+1, j-1, k) + !isAir(i+1, j-1, k-1) + !isAir(i+1, j, k-1);
          ao.g = !isAir(i+1, j, k-1) + !isAir(i+1, j+1, k-1) + !isAir(i+1, j+1, k);
          ao.b = !isAir(i+1, j+1, k) + !isAir(i+1, j+1, k+1) + !isAir(i+1, j, k+1);
          ao.a = !isAir(i+1, j, k+1) + !isAir(i+1, j-1, k+1) + !isAir(i+1, j-1, k);
          ao.r = 255 - AO_STRENGTH*ao.r;
          ao.g = 255 - AO_STRENGTH*ao.g;
          ao.b = 255 - AO_STRENGTH*ao.b;
          ao.a = 255 - AO_STRENGTH*ao.a;
          colors.push_back(Color{ao.r, ao.r, ao.r, 255});
          colors.push_back(Color{ao.g, ao.g, ao.g, 255});
          colors.push_back(Color{ao.b, ao.b, ao.b, 255});
          colors.push_back(Color{ao.a, ao.a, ao.a, 255});
          // indices
          // tri 1
          indices.push_back(0+indexOffset); indices.push_back(1+indexOffset); indices.push_back(2+indexOffset);
          // tri 2 
          indices.push_back(2+indexOffset); indices.push_back(3+indexOffset); indices.push_back(0+indexOffset);
          triangleCount += 2;
        }
        if (isAir(i-1, j, k)) {
          int indexOffset = vertices.size();
          // vertices
          vertices.push_back(Vector3{0.0f + i, 0.0f + j, 1.0f + k});
          vertices.push_back(Vector3{0.0f + i, 1.0f + j, 1.0f + k});
          vertices.push_back(Vector3{0.0f + i, 1.0f + j, 0.0f + k});
          vertices.push_back(Vector3{0.0f + i, 0.0f + j, 0.0f + k});
          vertexCount += 4;
          // normals
          REPEAT(4) normals.push_back(Vector3{-1.0f, 0.0f, 0.0f});
          // texcoords TODO
          REPEAT(4) texcoords.push_back(toTexCoord(blockType, 2, i));
          Color ao = {0, 0, 0, 0};
          ao.r = !isAir(i-1, j-1, k) + !isAir(i-1, j-1, k+1) + !isAir(i-1, j, k+1);
          ao.g = !isAir(i-1, j, k+1) + !isAir(i-1, j+1, k+1) + !isAir(i-1, j+1, k);
          ao.b = !isAir(i-1, j+1, k) + !isAir(i-1, j+1, k-1) + !isAir(i-1, j, k-1);
          ao.a = !isAir(i-1, j, k-1) + !isAir(i-1, j-1, k-1) + !isAir(i-1, j-1, k);
          ao.r = 255 - AO_STRENGTH*ao.r;
          ao.g = 255 - AO_STRENGTH*ao.g;
          ao.b = 255 - AO_STRENGTH*ao.b;
          ao.a = 255 - AO_STRENGTH*ao.a;
          colors.push_back(Color{ao.r, ao.r, ao.r, 255});
          colors.push_back(Color{ao.g, ao.g, ao.g, 255});
          colors.push_back(Color{ao.b, ao.b, ao.b, 255});
          colors.push_back(Color{ao.a, ao.a, ao.a, 255});
          // indices
          // tri 1
          indices.push_back(0+indexOffset); indices.push_back(1+indexOffset); indices.push_back(2+indexOffset);
          // tri 2
          indices.push_back(2+indexOffset); indices.push_back(3+indexOffset); indices.push_back(0+indexOffset);
          triangleCount += 2;
        }
        if (isAir(i, j+1, k)) {
          int indexOffset = vertices.size();
          // vertices
          vertices.push_back(Vector3{0.0f + i, 1.0f + j, 0.0f + k});
          vertices.push_back(Vector3{0.0f + i, 1.0f + j, 1.0f + k});
          vertices.push_back(Vector3{1.0f + i, 1.0f + j, 1.0f + k});
          vertices.push_back(Vector3{1.0f + i, 1.0f + j, 0.0f + k});
          vertexCount += 4;
          // normals
          REPEAT(4) normals.push_back(Vector3{0.0f, 1.0f, 0.0f});
          // texcoords TODO
          REPEAT(4) texcoords.push_back(toTexCoord(blockType, 3, i));
          Color ao = {0, 0, 0, 0};
          ao.r = !isAir(i, j+1, k-1) + !isAir(i-1, j+1, k-1) + !isAir(i-1, j+1, k);
          ao.g = !isAir(i-1, j+1, k) + !isAir(i-1, j+1, k+1) + !isAir(i, j+1, k+1);
          ao.b = !isAir(i, j+1, k+1) + !isAir(i+1, j+1, k+1) + !isAir(i+1, j+1, k);
          ao.a = !isAir(i+1, j+1, k) + !isAir(i+1, j+1, k-1) + !isAir(i, j+1, k-1);
          ao.r = 255 - AO_STRENGTH*ao.r;
          ao.g = 255 - AO_STRENGTH*ao.g;
          ao.b = 255 - AO_STRENGTH*ao.b;
          ao.a = 255 - AO_STRENGTH*ao.a;
          colors.push_back(Color{ao.r, ao.r, ao.r, 255});
          colors.push_back(Color{ao.g, ao.g, ao.g, 255});
          colors.push_back(Color{ao.b, ao.b, ao.b, 255});
          colors.push_back(Color{ao.a, ao.a, ao.a, 255});
          // indices
          // tri 1
          indices.push_back(0+indexOffset); indices.push_back(1+indexOffset); indices.push_back(2+indexOffset);
          // tri 2 
          indices.push_back(2+indexOffset); indices.push_back(3+indexOffset); indices.push_back(0+indexOffset);
          triangleCount += 2;
        }
        if (isAir(i, j-1, k)) {
          int indexOffset = vertices.size();
          // vertices
          vertices.push_back(Vector3{0.0f + i, 0.0f + j, 0.0f + k});
          vertices.push_back(Vector3{1.0f + i, 0.0f + j, 0.0f + k});
          vertices.push_back(Vector3{1.0f + i, 0.0f + j, 1.0f + k});
          vertices.push_back(Vector3{0.0f + i, 0.0f + j, 1.0f + k});
          vertexCount += 4;
          // normals
          REPEAT(4) normals.push_back(Vector3{0.0f, -1.0f, 0.0f});
          // texcoords TODO
          REPEAT(4) texcoords.push_back(toTexCoord(blockType, 4, i));
          Color ao = {0, 0, 0, 0};
          ao.r = !isAir(i-1, j-1, k) + !isAir(i-1, j-1, k-1) + !isAir(i, j-1, k-1);
          ao.g = !isAir(i, j-1, k-1) + !isAir(i+1, j-1, k-1) + !isAir(i+1, j-1, k);
          ao.b = !isAir(i+1, j-1, k) + !isAir(i+1, j-1, k+1) + !isAir(i, j-1, k+1);
          ao.a = !isAir(i, j-1, k+1) + !isAir(i-1, j-1, k+1) + !isAir(i-1, j-1, k);
          ao.r = 255 - AO_STRENGTH*ao.r;
          ao.g = 255 - AO_STRENGTH*ao.g;
          ao.b = 255 - AO_STRENGTH*ao.b;
          ao.a = 255 - AO_STRENGTH*ao.a;
          colors.push_back(Color{ao.r, ao.r, ao.r, 255});
          colors.push_back(Color{ao.g, ao.g, ao.g, 255});
          colors.push_back(Color{ao.b, ao.b, ao.b, 255});
          colors.push_back(Color{ao.a, ao.a, ao.a, 255});
          // indices
          // tri 1
          indices.push_back(0+indexOffset); indices.push_back(1+indexOffset); indices.push_back(2+indexOffset);
          // tri 2 
          indices.push_back(2+indexOffset); indices.push_back(3+indexOffset); indices.push_back(0+indexOffset);
          triangleCount += 2;
        }
        if (isAir(i, j, k+1)) {
          int indexOffset = vertices.size();
          // vertices
          vertices.push_back(Vector3{1.0f + i, 0.0f + j, 1.0f + k});
          vertices.push_back(Vector3{1.0f + i, 1.0f + j, 1.0f + k});
          vertices.push_back(Vector3{0.0f + i, 1.0f + j, 1.0f + k});
          vertices.push_back(Vector3{0.0f + i, 0.0f + j, 1.0f + k});
          vertexCount += 4;
          // normals
          REPEAT(4) normals.push_back(Vector3{0.0f, 0.0f, 1.0f});
          // texcoords TODO
          REPEAT(4) texcoords.push_back(toTexCoord(blockType, 5, i));
          Color ao = {0, 0, 0, 0};
          ao.r = !isAir(i, j-1, k+1) + !isAir(i+1, j-1, k+1) + !isAir(i+1, j, k+1);
          ao.g = !isAir(i+1, j, k+1) + !isAir(i+1, j+1, k+1) + !isAir(i, j+1, k+1);
          ao.b = !isAir(i, j+1, k+1) + !isAir(i-1, j+1, k+1) + !isAir(i-1, j, k+1);
          ao.a = !isAir(i-1, j, k+1) + !isAir(i-1, j-1, k+1) + !isAir(i, j-1, k+1);
          ao.r = 255 - AO_STRENGTH*ao.r;
          ao.g = 255 - AO_STRENGTH*ao.g;
          ao.b = 255 - AO_STRENGTH*ao.b;
          ao.a = 255 - AO_STRENGTH*ao.a;
          colors.push_back(Color{ao.r, ao.r, ao.r, 255});
          colors.push_back(Color{ao.g, ao.g, ao.g, 255});
          colors.push_back(Color{ao.b, ao.b, ao.b, 255});
          colors.push_back(Color{ao.a, ao.a, ao.a, 255});
          // indices
          // tri 1
          indices.push_back(0+indexOffset); indices.push_back(1+indexOffset); indices.push_back(2+indexOffset);
          // tri 2 
          indices.push_back(2+indexOffset); indices.push_back(3+indexOffset); indices.push_back(0+indexOffset);
          triangleCount += 2;
        }
        if (isAir(i, j, k-1)) {
          int indexOffset = vertices.size();
          // vertices
          vertices.push_back(Vector3{0.0f + i, 0.0f + j, 0.0f + k});
          vertices.push_back(Vector3{0.0f + i, 1.0f + j, 0.0f + k});
          vertices.push_back(Vector3{1.0f + i, 1.0f + j, 0.0f + k});
          vertices.push_back(Vector3{1.0f + i, 0.0f + j, 0.0f + k});
          vertexCount += 4;
          // normals
          REPEAT(4) normals.push_back(Vector3{0.0f, 0.0f, -1.0f});
          // texcoords TODO
          REPEAT(4) texcoords.push_back(toTexCoord(blockType, 6, i));
          Color ao = {0, 0, 0, 0};
          ao.r = !isAir(i, j-1, k-1) + !isAir(i-1, j-1, k-1) + !isAir(i-1, j, k-1);
          ao.g = !isAir(i-1, j, k-1) + !isAir(i-1, j+1, k-1) + !isAir(i, j+1, k-1);
          ao.b = !isAir(i, j+1, k-1) + !isAir(i+1, j+1, k-1) + !isAir(i+1, j, k-1);
          ao.a = !isAir(i+1, j, k-1) + !isAir(i+1, j-1, k-1) + !isAir(i, j-1, k-1);
          ao.r = 255 - AO_STRENGTH*ao.r;
          ao.g = 255 - AO_STRENGTH*ao.g;
          ao.b = 255 - AO_STRENGTH*ao.b;
          ao.a = 255 - AO_STRENGTH*ao.a;
          colors.push_back(Color{ao.r, ao.r, ao.r, 255});
          colors.push_back(Color{ao.g, ao.g, ao.g, 255});
          colors.push_back(Color{ao.b, ao.b, ao.b, 255});
          colors.push_back(Color{ao.a, ao.a, ao.a, 255});
          // indices
          // tri 1
          indices.push_back(0+indexOffset); indices.push_back(1+indexOffset); indices.push_back(2+indexOffset);
          // tri 2 
          indices.push_back(2+indexOffset); indices.push_back(3+indexOffset); indices.push_back(0+indexOffset);
          triangleCount += 2;
        }
      }
    }
  }

  // Part 2: copy data into mesh, upload
  chunkMesh = {};
  chunkMesh.triangleCount = triangleCount;
  chunkMesh.vertexCount = vertexCount;
  chunkMesh.vertices = new float[vertices.size()*3];      //(float*)MemAlloc(vertices.size()*sizeof(Vector3));
  chunkMesh.normals = new float[normals.size()*3];        //(float*)MemAlloc(normals.size()*sizeof(Vector3));
  chunkMesh.texcoords = new float[texcoords.size()*2];    //(float*)MemAlloc(texcoords.size()*sizeof(Vector2));
  chunkMesh.colors = new unsigned char[colors.size()*4];  //(unsigned char*)MemAlloc(colors.size()*sizeof(Color));
  chunkMesh.indices = new ushort[indices.size()];         //(ushort*)MemAlloc(indices.size()*sizeof(ushort));

  if (chunkMesh.vertices == nullptr) TraceLog(LOG_ERROR, "Failed to allocate memory for vertices\n");
  if (chunkMesh.normals == nullptr) TraceLog(LOG_ERROR, "Failed to allocate memory for normals\n");
  if (chunkMesh.texcoords == nullptr) TraceLog(LOG_ERROR, "Failed to allocate memory for texcoords\n");
  if (chunkMesh.colors == nullptr) TraceLog(LOG_ERROR, "Failed to allocate memory for colors\n");
  if (chunkMesh.indices == nullptr) TraceLog(LOG_ERROR, "Failed to allocate memory for indices\n");

  memcpy(chunkMesh.vertices, vertices.data(), vertices.size()*sizeof(Vector3));
  memcpy(chunkMesh.normals, normals.data(), normals.size()*sizeof(Vector3));
  memcpy(chunkMesh.texcoords, texcoords.data(), texcoords.size()*sizeof(Vector2));
  memcpy(chunkMesh.colors, colors.data(), colors.size()*sizeof(Color));
  memcpy(chunkMesh.indices, indices.data(), indices.size()*sizeof(ushort));

  UploadMesh(&chunkMesh, false);
}

void Chunk::loadChunk() {
  genMesh();
  loaded = true;
  updated = false;
}

void Chunk::unloadChunk() {
  UnloadMesh(chunkMesh);
  loaded = false;
}

void Chunk::reloadChunk() {
  if (updated && loaded) {
    unloadChunk();
    loadChunk();
  }
}

bool Chunk::breakBlock(int i, int j, int k) {
  if (chunkData[i][j][k].blockType != AIR) {
    chunkData[i][j][k] = Block{AIR};
    updated = true;
    if (i == 0 && chunks->count({worldX-1, worldZ}) != 0)                                         chunks->at({worldX-1, worldZ})->updated = true;
    if (i == 0 && k == 0 && chunks->count({worldX-1, worldZ-1}) != 0)                             chunks->at({worldX-1, worldZ-1})->updated = true;
    if (k == 0 && chunks->count({worldX, worldZ-1}) != 0)                                         chunks->at({worldX, worldZ-1})->updated = true;
    if (k == 0 && i + 1 == CHUNK_WIDTH && chunks->count({worldX+1, worldZ-1}))                    chunks->at({worldX+1, worldZ-1})->updated = true;
    if (i + 1 == CHUNK_WIDTH && chunks->count({worldX+1, worldZ}) != 0)                           chunks->at({worldX+1, worldZ})->updated = true;
    if (i + 1 == CHUNK_WIDTH && k+1 == CHUNK_LENGTH && chunks->count({worldX+1, worldZ+1}) != 0)  chunks->at({worldX+1, worldZ+1}) ->updated = true;
    if (k + 1 == CHUNK_LENGTH && chunks->count({worldX, worldZ+1}) != 0)                          chunks->at({worldX, worldZ+1})->updated = true;
    if (k + 1 == CHUNK_LENGTH && i == 0 && chunks->count({worldX-1, worldZ+1}) != 0)              chunks->at({worldX-1, worldZ+1})->updated = true;
    return true;
  }
  return false;
}

void Chunk::draw() {
  if (loaded) DrawMesh(chunkMesh, material, MatrixTranslate((float)worldX*CHUNK_WIDTH, (float)worldY*CHUNK_HEIGHT, (float)worldZ*CHUNK_LENGTH));
}
