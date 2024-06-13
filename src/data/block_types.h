#ifndef BLOCK_TYPE_H
#define BLOCK_TYPE_H

enum BlockTypes {
  Block,    // generic block
  Diagonal, // diagonal block
  Column,   // column block
  Result,   // final result block
  Updated,  // block that has been updated by the UpdateSubMatrix task
  Vector,   // vector block used in the solver
};

#endif
