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
    TCLAP::ValueArg<size_t> blockSizeArg("b", "blocksize", "Blocksize", false, 10, &sc);
    cmd.add(blockSizeArg);
    TCLAP::ValueArg<size_t> nbThreadsComputeColumnArg("c", "column", "Number of threads for the compute column task.", false, 4, &sc);
    cmd.add(nbThreadsComputeColumnArg);
    TCLAP::ValueArg<size_t> nbThreadsUpdateArg("u", "update", "Number of threads for the update task.", false, 4, &sc);
    cmd.add(nbThreadsUpdateArg);
    cmd.parse(argc, argv);

    config.inputFile = inputFileArg.getValue();
    config.blockSize = blockSizeArg.getValue();
    config.nbThreadsComputeColumnTask = nbThreadsComputeColumnArg.getValue();
    config.nbThreadsUpdateTask = nbThreadsUpdateArg.getValue();
  } catch (TCLAP::ArgException &e)  // catch any exceptions
  { std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }
}

