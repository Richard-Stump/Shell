#ifndef shell_hh
#define shell_hh

#include <map>

#include "command.hh"

struct Shell {

  static void prompt();
  static void sigInt(int sig);
  static void sigChild(int sig, siginfo_t *info, void *ucontext);

  static void changeDir();

  static void printExitMessage();
  static void exit();

  static void addBackgroundProcess(int pid, bool last);

  static Command _currentCommand;

protected:
  static std::map<int, bool> _backgroundMap;
};

#endif
