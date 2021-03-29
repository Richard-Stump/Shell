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
#include "shell.hh"

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
  *argument = Shell::expandTilde(*argument);

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

  if(_isBuiltin && _runAsParent) {
    executeBuiltin();
    return -1;
  }

  int pid = fork();

  //if we are running as the child, load/execute the program specified in argv[1]
  //if there is an error while forking, exit
  //otherwise, return the PID of the child
  if(pid == 0) {
    if(_isBuiltin) {
      executeBuiltin();
      exit(0);
    }
    else {
      executeNormal();
    }
  }
  else if (pid < 0) {
    perror("fork error");
    _exit(1);
  }
  else {
    //fprintf(stderr, "c: child process: %d\n", pid);
    return pid;
  }
  
  return -1;
}

void SimpleCommand::executeNormal()
{
  char* const* argv = getArgv();

  execvp(argv[0], argv);

  perror("execvp error");
  _exit(1);
}

//execute the different builtin commands. This function doesn't have to worry
//about which functions are executed as children or parents because that is
//already handled by the lexer, parser, and Simplecommand::execute()
void SimpleCommand::executeBuiltin()
{
  //change the directory to the given argument. if there is no argument,
  //change to the home directory
  if(*_arguments[0] == "cd") {
    if(_arguments.size() == 1) {
      std::string name = "HOME";
      std::string home = Shell::getEnv(name);
      Shell::changeDir(&home);
    }
    else
      Shell::changeDir(_arguments[1]);
  }

  //set an environment variable
  if(*_arguments[0] == "setenv") {
    if(_arguments.size() == 3) {
      Shell::setEnv(_arguments[1], _arguments[2]);
    }
  }

  //unset an environment variable
  if(*_arguments[0] == "unsetenv") {
    if(_arguments.size() == 2) {
      Shell::unsetEnv(_arguments[1]);
    }
  }

  //print an environment variable. This one should run as a child so that it's
  //output can be piped
  if(*_arguments[0] == "printenv") {
    Shell::printEnv();
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
