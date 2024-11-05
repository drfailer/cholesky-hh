// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the
// software in any medium, provided that you keep intact this entire notice. You may improve, modify and create
// derivative works of the software or any portion of the software, and you may copy and distribute such modifications
// or works. Modified works should carry a notice stating that you changed the software and should note the date and
// nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the
// source of the software. NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND,
// EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR
// WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE
// CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS
// THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE. You
// are solely responsible for determining the appropriateness of using and distributing the software and you assume
// all risks associated with its use, including but not limited to the risks and costs of program errors, compliance
// with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of
// operation. This software is not intended to be used in any situation where a failure could cause risk of injury or
// damage to property. The software developed by NIST employees is not subject to copyright protection within the
// United States.

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

std::string dotFileName(size_t size, size_t blockSize, ThreadsConfig config) {
  std::ostringstream oss;
  oss << size << "-" << blockSize << "-" << config << ".dot";
  return oss.str();
}
