#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <memory>

#include "simpleCommand.hh"

SimpleCommand::SimpleCommand() {
  _arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
  // iterate over all the arguments and delete them
  for (auto & arg : _arguments) {
    delete arg;
  }
}

void SimpleCommand::insertArgument( std::string * argument ) {
  // simply add the argument to the vector
  _arguments.push_back(argument);
}

// Print out the simple command
void SimpleCommand::print() {
  for (auto & arg : _arguments) {
    std::cout << "\"" << *arg << "\" \t";
  }
  // effectively the same as printf("\n\n");
  std::cout << std::endl;
}

std::unique_ptr<const char*[]> SimpleCommand::getArgv() {
  size_t argvCount = _arguments.size() + 1;
  char*[] argv = new char*[argvCount];

  argv[argvCount - 1] = '\0';

  const char*[] const_argv = (const char*[])argv;

  return std::unique_ptr<const char*[]>(const_argv);
}

//execute the simple command
void SimpleCommand::execute() {
  std::unique_ptr<const char*[]> args = getArgv();

  for(size_t i = 0; args[i] != nullptr; i++) {
    std::cout << args[i] << std::endl;
  }
}
