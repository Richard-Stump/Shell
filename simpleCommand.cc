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

const char** SimpleCommand::getArgv() {
  size_t argvCount = _arguments.size();
  char** argv = new char*[argvCount + 1];

  argv[argvCount] = "\0";

  const char** const_argv = (const char**)argv;

  for(size_t i = 0; i < argvCount; i++) {
    const_argv[i] = _arguments[i];
  }

  return const_argv;
}

void SimpleCommand::freeArgv(const char** argv)
{
  delete[] argv;
}

//execute the simple command
void SimpleCommand::execute() {
  const char** args = getArgv();

  for(size_t i = 0; args[i] != nullptr; i++) {
    std::cout << args[i] << std::endl;
  }

  freeArgv(args);
}
