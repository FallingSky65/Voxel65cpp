#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <map>
#include <raylib-cpp.hpp>
#include <utility>
#include "Settings.hpp"
#include "Block.hpp"
#include "Terrain.hpp"

class Chunk;

typedef std::map<std::pair<int, int>, Chunk*> WorldMap;

class Chunk {
private:
  int worldX, worldY, worldZ;
  Block chunkData[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_LENGTH];
  Mesh chunkMesh;
  Material material;
  Terrain terrain;
  WorldMap* chunks;
  bool isAir(int x, int y, int z);
  void genMesh();
public:
  bool loaded;
  bool updated;
  Chunk();
  Chunk(int x, int y, int z, Material _material, Terrain _terrain, WorldMap* _chunks);
  ~Chunk();
  void setChunkPos(int x, int y, int z);
  void genDataUniform(Block block);
  void genDataRandom();
  void genDataTerrain();
  void loadChunk();
  void unloadChunk();
  void reloadChunk();
  bool breakBlock(int i, int j, int k);
  void draw();
};

#endif // !CHUNK_HPP
#define CHUNK_HPP

