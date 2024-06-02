#ifndef CONFIG_H
#define CONFIG_H
#include <string>

struct Config {
  std::string inputFile;
  size_t blockSize;
  size_t nbThreadsComputeColumnTask;
  size_t nbThreadsUpdateTask;
};

void parseCmdArgs(int argc, char **argv, Config &config);

#endif
