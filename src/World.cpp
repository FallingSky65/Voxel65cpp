#include "World.hpp"
#include "raylib.h"
#include "raymath.h"
#include <cmath>

ivec3 World::toWorldCoords(Vector3 v) {
  return {(int)floor(v.x/CHUNK_WIDTH), (int)floor(v.y/CHUNK_HEIGHT), (int)floor(v.z/CHUNK_LENGTH)};
}

World::World(Material _material) {
  material = _material;
  for (int i = -RENDER_DIST*2; i <= RENDER_DIST*2; i++) {
    for (int j = -RENDER_DIST*2; j <= RENDER_DIST*2; j++) {
      chunks[{i, j}] = new Chunk(i, 0, j, material, terrain, &chunks);
    }
  }
  camPos.x = 0;
  camPos.y = 0;
  camPos.z = 0;
  /*
  for (int i = 0; i < WORLD_WIDTH; i++) {
    for (int j = 0; j < WORLD_LENGTH; j++) {
      chunks[i][j].setShader(_shader);
      chunks[i][j].setChunkPos(i, 0, j);
      chunks[i][j].genDataTerrain();
      chunks[i][j].genModel();
    }
  }
  */
}

void World::breakBlock(raylib::Camera3D camera) {
  Vector3 camDir = Vector3Normalize(Vector3Subtract(camera.GetTarget(), camera.GetPosition()));
  Vector3 rayPos = camera.GetPosition();
  while (Vector3Length(Vector3Subtract(rayPos, camera.GetPosition())) < REACH) {
    ivec3 chunkLoc = toWorldCoords(rayPos);
    if (chunkLoc.y == 0 && chunks.count({chunkLoc.x, chunkLoc.z}) != 0) {
      ivec3 blockLoc = {
        (int)floor(rayPos.x) - chunkLoc.x*CHUNK_WIDTH,
        (int)floor(rayPos.y) - chunkLoc.y*CHUNK_HEIGHT,
        (int)floor(rayPos.z) - chunkLoc.z*CHUNK_LENGTH
      };
      if (chunks.at({chunkLoc.x, chunkLoc.z})->breakBlock(blockLoc.x, blockLoc.y, blockLoc.z)) break;
    }
    rayPos = Vector3Add(rayPos, Vector3Scale(camDir, 0.1));
  }
}

void World::placeBlock(raylib::Camera3D camera) {

}

void World::update(raylib::Camera3D camera) {
  ivec3 camPosNew = toWorldCoords(camera.GetPosition());

  for (int i = -RENDER_DIST + camPos.x; i <= RENDER_DIST + camPos.x; i++) {
    for (int j = -RENDER_DIST + camPos.z; j <= RENDER_DIST + camPos.z; j++) {
      if (i < -RENDER_DIST + camPosNew.x || i > RENDER_DIST + camPosNew.x ||
          j < -RENDER_DIST + camPosNew.z || j > RENDER_DIST + camPosNew.z) {
        if (chunks.find({i, j}) != chunks.end() && chunks[{i, j}]->loaded) chunks[{i, j}]->unloadChunk();
      }
    }
  }

  camPos = camPosNew;
  for (int i = -RENDER_DIST + camPos.x; i <= RENDER_DIST + camPos.x; i++) {
    for (int j = -RENDER_DIST + camPos.z; j <= RENDER_DIST + camPos.z; j++) {
      if (chunks.count({i, j}) == 0) {
        chunks[{i, j}] = new Chunk(i, 0, j, material, terrain, &chunks);
        for (int ii = -1; ii <= 1; ii++) {
          for (int jj = -1; jj <= 1; jj++) {
            if (chunks.count({i+ii, j+jj}) != 0 && chunks[{i+ii, j+jj}]->loaded) chunks[{i+ii, j+jj}]->unloadChunk();
          }
        }
      }
    }
  }

  reload();
}

void World::reload() {
  for (int i = -RENDER_DIST + camPos.x; i <= RENDER_DIST + camPos.x; i++) {
    for (int j = -RENDER_DIST + camPos.z; j <= RENDER_DIST + camPos.z; j++) {
      if (chunks[{i, j}]->loaded) {
        chunks[{i, j}]->reloadChunk();
      } else {
        chunks[{i, j}]->loadChunk();
      }
    }
  }
}

void World::draw() {
  for (int i = -RENDER_DIST + camPos.x; i <= RENDER_DIST + camPos.x; i++) {
    for (int j = -RENDER_DIST + camPos.z; j <= RENDER_DIST + camPos.z; j++) {
      if (chunks[{i, j}]->loaded){
        chunks[{i, j}]->draw();
      }
    }
  }
}

int World::size() {
  return chunks.size();
}
