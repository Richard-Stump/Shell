#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>


#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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

/**
 * Execute this simple command using the file descriptors is IO
 * @param fdIn  The file descriptor for input
 * @param fdOut The File descriptor for output
 * @param fdErr The file descriptor for error output
 * @return The PID of the new process
 */
int SimpleCommand::execute(int fdIn, int fdOut, int fdErr)
{
  //set the standard IO for the child process
  dup2(fdIn, 0);
  dup2(fdOut, 1);
  dup2(fdErr, 2);

  int pid = fork();

  //if we are running as the child, load/execute the program specified in argv[1]
  //if there is an error while forking, exit
  //otherwise, return the PID of the child
  if(pid == 0) {
    char* const* argv = getArgv();

    execvp(argv[0], argv);

    perror("execvp error\n");
    _exit(1);
  }
  else if (pid < 0) {
    perror("fork error\n");
    _exit(1);
  }
  else {
    return pid;
  }
}

/** 
 * Returns the arguments of the simple command as a char* const* array
 * @return An array of C char* const* pointers holding the arguments. This
 *         list is null terminated
 */
char* const* SimpleCommand::getArgv() {
  size_t argvCount = _arguments.size();
  const char** argv = new const char*[argvCount + 1];

  //loop through the argument list and add them to argv
  for(size_t i = 0; i < argvCount; i++) {
    argv[i] = _arguments[i]->c_str();
  }

  argv[argvCount] = nullptr;

  return (char* const*)argv;
}