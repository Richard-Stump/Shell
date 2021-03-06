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

std::shared_ptr<const char**> SimpleCommand::createArgv() {
  size_t argvSize = _arguments.size() + 1;
  const char** argv = new char*[argvSize];

  for(size_t i = 0; i < argvSize; i++) {
    argv[i] = _arguments[i]->c_str();

  }

  argv[argvSize - 1][0] = '\0';

  return std::shared_ptr<const char**>(argv); //template errors suck...
}

//execute the simple command
void SimpleCommand::execute() {
  std::shared_ptr<char**> argv = createArgv();

  for(size_t i = 0; argv[i] != nullptr; i++)
  {
    printf("arg %d: \"%s\"", i, argv[i]);
  }
}
