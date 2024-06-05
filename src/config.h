#ifndef CONFIG_H
#define CONFIG_H
#include <string>

struct Config {
  std::string inputFile;
  std::string dotFile;
  size_t blockSize;
  size_t nbThreadsComputeDiagonalTask;
  size_t nbThreadsComputeColumnTask;
  size_t nbThreadsUpdateTask;
  bool print;
};

void parseCmdArgs(int argc, char **argv, Config &config);

#endif
