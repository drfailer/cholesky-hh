#ifndef BLOCK_TYPE_H
#define BLOCK_TYPE_H

enum BlockTypes {
  MatrixBlock,       // generic matrix block
  VectorBlock,       // generic vector block
  VectorBlockPhase1, // result vector from the phase 1 of the solver
  Diagonal,          // diagonal block
  Column,            // column block
  Decomposed,        // decomposed block
  Result,            // final result block
  Updated,           // block that has been updated by the UpdateSubMatrix task
  Vector,            // vector block used in the solver
};

#endif
