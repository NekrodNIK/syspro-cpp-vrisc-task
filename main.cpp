#include "emulator.hpp"
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <ostream>
#include <print>
#include <sstream>

std::optional<std::string> readStringFromFile(const std::string& filename) {
  std::ifstream file{filename};
  if (!file)
    return {};
  std::stringstream buf;
  buf << file.rdbuf();
  return buf.str();
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Invalid args" << std::endl;
    return EXIT_FAILURE;
  }

  auto program = readStringFromFile(argv[1]);
  if (!program) {
    std::cerr << "Can't open file" << std::endl;
    return EXIT_FAILURE;
  }

  Emulator emulator;
  Emulator::Word result;
  try {
    result = emulator(program.value());
  } catch (const std::exception&) {
    std::cerr << "Invalid program" << std::endl;
    return EXIT_FAILURE;
  }
  std::println("result: {}", result);

  return EXIT_SUCCESS;
}
