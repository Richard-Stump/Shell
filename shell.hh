#ifndef shell_hh
#define shell_hh

#include "command.hh"

struct Shell {

  static void prompt();
  static void signal(int sig);

  static void changeDir();

  static void printExitMessage();
  static void exit();

  static Command _currentCommand;
};

#endif
