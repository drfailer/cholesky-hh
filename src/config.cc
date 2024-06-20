#include "tclap/CmdLine.h"
#include "config.h"

class SizeConstraint : public TCLAP::Constraint<size_t> {
 public:
  [[nodiscard]] std::string description() const override {
    return "Positive non null";
  }
  [[nodiscard]] std::string shortID() const override {
    return "NonNullInteger";
  }
  [[nodiscard]] bool check(size_t const &value) const override {
    return value > 0;
  }
};

void parseCmdArgs(int argc, char **argv, Config &config) {
  // Parse the command line arguments
  try {
    TCLAP::CmdLine cmd("Cholesky Hedgehog", ' ', "0.1");
    SizeConstraint sc;
    TCLAP::ValueArg<std::string> inputFileArg("i", "input", "Input file name", true, "", "string");
    cmd.add(inputFileArg);
    TCLAP::ValueArg<std::string> dotFileArg("g", "graph", "dot file name", false, "", "string");
    cmd.add(dotFileArg);
    TCLAP::ValueArg<size_t> blockSizeArg("b", "blocksize", "Blocksize", false, 10, &sc);
    cmd.add(blockSizeArg);
    TCLAP::ValueArg<size_t> nbThreadsComputeColumnArg("c", "column", "Number of threads for the compute column task.", false, 4, &sc);
    cmd.add(nbThreadsComputeColumnArg);
    TCLAP::ValueArg<size_t> nbThreadsUpdateArg("u", "update", "Number of threads for the update task.", false, 4, &sc);
    cmd.add(nbThreadsUpdateArg);
    TCLAP::ValueArg<size_t> nbThreadsComputeDiagonalArg("d", "diagonal", "Number of threads for the compute diagonal task.", false, 1, &sc);
    cmd.add(nbThreadsComputeDiagonalArg);
    TCLAP::ValueArg<size_t> nbThreadsSolveDiagonalArg("s", "solDiag", "Number of threads for the solve diagonal task.", false, 1, &sc);
    cmd.add(nbThreadsSolveDiagonalArg);
    TCLAP::ValueArg<size_t> nbThreadsUpdateVectorArg("v", "upVec", "Number of threads for the update vector task.", false, 4, &sc);
    cmd.add(nbThreadsUpdateVectorArg);
    TCLAP::ValueArg<bool> loopArg("l", "loop", "Loop over thread config (require rebuild to change)", false, false, "bool");
    cmd.add(loopArg);
    TCLAP::ValueArg<bool> printArg("p", "print", "print", false, false, "bool");
    cmd.add(printArg);
    cmd.parse(argc, argv);

    config.inputFile = inputFileArg.getValue();
    config.dotFile = dotFileArg.getValue();
    config.blockSize = blockSizeArg.getValue();
    config.threadsConfig.nbThreadsComputeColumnTask = nbThreadsComputeColumnArg.getValue();
    config.threadsConfig.nbThreadsUpdateTask = nbThreadsUpdateArg.getValue();
    config.threadsConfig.nbThreadsComputeDiagonalTask = nbThreadsComputeDiagonalArg.getValue();
    config.threadsConfig.nbThreadsSolveDiagonal = nbThreadsSolveDiagonalArg.getValue();
    config.threadsConfig.nbThreadsUpdateVector = nbThreadsUpdateVectorArg.getValue();
    config.print = printArg.getValue();
    config.loop = loopArg.getValue();
  } catch (TCLAP::ArgException &e)  // catch any exceptions
  { std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }
}

std::ostream& operator<<(std::ostream& os, const ThreadsConfig& threadsConfig) {
  os << threadsConfig.nbThreadsComputeDiagonalTask << "-"
     << threadsConfig.nbThreadsComputeColumnTask << "-"
     << threadsConfig.nbThreadsUpdateTask << "-"
     << threadsConfig.nbThreadsSolveDiagonal << "-"
     << threadsConfig.nbThreadsUpdateVector;
  return os;
}

std::string dotFileName(size_t height, ThreadsConfig config) {
  std::ostringstream oss;
  oss << height << "-" << config << ".dot";
  return oss.str();
}
