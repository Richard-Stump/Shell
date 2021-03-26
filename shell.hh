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

struct FinalCommand {
  int _pid;
  bool _background;
};

struct Shell {
  static void prompt();
  static void sigInt(int sig);
  static void sigChild(int sig, siginfo_t *info, void *ucontext);

  static void changeDir(std::string* path);

  static void printExitMessage();
  static void exit();

  static std::string getHome();
  static std::string trimWhitespace(std::string& string);

  static void setEnv(std::string* name, std::string* value);
  static void unsetEnv(std::string* name);
  static void printEnv();
  static std::string getEnv(std::string& name);

  static std::string expandTilde(std::string& string);
  static std::string expandEnvironmentVars(std::string& string);
  static void expandWildcards(std::string& path);

  static void executeSubshell(std::string* command, std::string* output, 
                              bool replaceNewlines = true);

  static void addBackgroundProcess(int pid, bool last);
  static void addFinalCommand(int pid, bool background);

  static Command _currentCommand;

  static int argc;
  static const char** argv;
  static std::string lastArg;
  static int _lastBackPid;

protected:
  static std::string wildcardToRegex(std::string wildcard);
  static void recursivelyExpandWildcards(std::string prefix, 
                                         std::string suffix);
  static void nextPathComponent(std::string& prefix, std::string& suffix, 
                               std::string& component);
 
  static std::vector<BackgroundProcess> _backgroundProcesses;
  static std::vector<FinalCommand> _finalCommands;

  static int _lastRet;
  static int _lastBackRet;
};

#endif
