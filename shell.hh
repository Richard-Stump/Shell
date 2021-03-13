#ifndef shell_hh
#define shell_hh

#include <vector>

#include <signal.h>

#include "command.hh"

struct BackgroundProcess
{
  int   _pid;
  bool  _isLast;
};

struct Shell {
  static void prompt();
  static void sigInt(int sig);
  static void sigChild(int sig, siginfo_t *info, void *ucontext);

  static void changeDir(std::string* path);

  static void printExitMessage();
  static void exit();

  static std::string getHome();
  static std::string expandTilde(std::string* string);

  static void setEnv(std::string* name, std::string* value);
  static void unsetEnv(std::string* name);
  static void printEnv();

  static void executeSubshell(std::string* command, std::string* output);

  static void addBackgroundProcess(int pid, bool last);

  static Command _currentCommand;

protected:
  static std::vector<BackgroundProcess> _backgroundProcesses;
};

#endif
