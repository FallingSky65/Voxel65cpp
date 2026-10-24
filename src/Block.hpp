#ifndef BLOCK_HPP
#define BLOCK_HPP

enum BlockType {
  AIR,
  GRASS,
  DIRT
};

typedef enum BlockType BlockType;

struct Block {
  BlockType blockType;
};

#endif // !BLOCK_HPP
#define BLOCK_HPP
