#ifndef BLOCK_TYPE_H
#define BLOCK_TYPE_H

enum BlockTypes {
  Block,    // generic block
  Diagonal, // diagonal block
  Result,   // final result block
  Updated,  // block that has been updated by the UpdateSubMatrix task
};

#endif
