#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

#include <iostream>
#include <memory>

#include "simpleCommand.hh"

SimpleCommand::SimpleCommand() : _running(false) {
  _arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
  printf("Freeing Simple command");

  if(_isChild) {
    printf(" which is a child process\n");
  }
  else {
    printf("\n");
  }

  // iterate over all the arguments and delete them
  for (auto & arg : _arguments) {
    if(arg != nullptr) {
      delete arg;
      arg = nullptr;
    }
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
  const char** argv = new const char*[argvCount + 1];

  for(size_t i = 0; i < argvCount; i++) {
    argv[i] = _arguments[i]->c_str();
  }

  argv[argvCount] = nullptr;

  return argv;
}

void SimpleCommand::freeArgv(const char**& argv)
{
  delete[] argv;
  argv = nullptr;
}

//execute the simple command
void SimpleCommand::execute() {
  int pid = fork();

  //if we are currently the child process
  if (pid == 0) {
    _isChild = true;

    //const char** args = getArgv();

    execvp("ls", (char* const*)"\0");  
  }
  else {
    _pid = pid; 
    _running = true;
  }
}

void SimpleCommand::wait() {
  if (_running) {
    waitpid(_pid, NULL, 0);
  }
}