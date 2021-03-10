#ifndef shell_hh
#define shell_hh

#include "command.hh"

struct Shell {

  static void prompt();
  static void signal(int sig);

  static void printExitMessage();

  static Command _currentCommand;
};

#endif
