#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include "OpenSimplexNoise.hpp"
#include "Block.hpp"

class Terrain {
private:
  OpenSimplexNoise noise1, noise2, noise3, noise4;
public:
  Terrain() {}
  BlockType Evaluate(double _x, double _y, double _z) {
    double x = _x/32.0, y = _y/32.0, z = _z/32.0;
    double h = 60*(
      (0.5*noise1.Evaluate(x, z)+0.5)/2 +
      (0.5*noise2.Evaluate(x*2, z*2)+0.5)/4 +
      (0.5*noise3.Evaluate(x*4, z*4)+0.5)/8 +
      (0.5*noise4.Evaluate(x*8, z*8)+0.5)/16
    );
    if (_y <= h) {
      if (_y + 1 <= h) {
        return DIRT;
      } else {
        return GRASS;
      }
    } else {
      return AIR;
    }
  }
};

#endif // !TERRAIN_HPP
#define TERRAIN_HPP

#include "OpenSimplexNoise.hpp"

