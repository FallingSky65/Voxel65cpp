#ifndef WORLD_HPP
#define WORLD_HPP

#include <raylib-cpp.hpp>
#include "Camera3D.hpp"
#include "Settings.hpp"
#include "Chunk.hpp"
#include "Terrain.hpp"
#include "Utils.hpp"

class World {
private:
  //Chunk chunks[WORLD_WIDTH][WORLD_LENGTH];
  WorldMap chunks;
  Terrain terrain;
  Material material;
  ivec3 camPos;
  ivec3 toWorldCoords(Vector3 v);
public:
  World(Material _material);
  void breakBlock(raylib::Camera3D camera);
  void placeBlock(raylib::Camera3D camera);
  void update(raylib::Camera3D camera);
  void reload();
  void draw();
  int size();
};

#endif // !WORLD_HPP
#define WORLD_HPP

