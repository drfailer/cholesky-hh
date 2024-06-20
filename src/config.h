#ifndef CONFIG_H
#define CONFIG_H
#include <string>

struct ThreadsConfig {
  ThreadsConfig() {}
  ThreadsConfig(size_t nbThreadsComputeDiagonalTask,
                size_t nbThreadsComputeColumnTask,
                size_t nbThreadsUpdateTask,
                size_t nbThreadsSolveDiagonal,
                size_t nbThreadsUpdateVector)
      : nbThreadsComputeDiagonalTask(nbThreadsComputeDiagonalTask),
        nbThreadsComputeColumnTask(nbThreadsComputeColumnTask),
        nbThreadsUpdateTask(nbThreadsUpdateTask),
        nbThreadsSolveDiagonal(nbThreadsSolveDiagonal),
        nbThreadsUpdateVector(nbThreadsUpdateVector) {}

  size_t nbThreadsComputeDiagonalTask = 1;
  size_t nbThreadsComputeColumnTask = 8;
  size_t nbThreadsUpdateTask = 35;
  size_t nbThreadsSolveDiagonal = 8;
  size_t nbThreadsUpdateVector = 30;
};

struct Config {
  std::string inputFile;
  std::string dotFile;
  size_t blockSize;
  bool print;
  bool loop;
  ThreadsConfig threadsConfig;
};

void parseCmdArgs(int argc, char **argv, Config &config);

#endif
