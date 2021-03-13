#include <cstdio>
#include <stdlib.h>
#include <time.h>
#include <vector>

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <pwd.h>

#include "shell.hh"

int yyparse(void);

std::vector<const char*> exitMessages = {
  "Goodbye!",
  "Goodbye!",
  "Goodbye!",
  "Goodbye!",
  "Goodbye!",
  "Goodbye!",
  "Goodbye!",
  "Toodle-loo!",
  "See Ya!",
  "Goodbyte!",
  "Hasta la vista, Baby!",
  "Au revoir!",
  "Bye-bye!",
  "Bye-byte!",
  "Don't leave yet -- There's a daemon around that corner! ",
  "Go ahead and leave. See if I care.",
  "Rage quitting a shell!?"
};

void Shell::prompt() {
  if( isatty(0) ) {
    char workingDir[PATH_MAX];

    getcwd(workingDir, PATH_MAX);

    printf("\033[32m%s>\033[0m", workingDir);
    fflush(stdout);
  }
}

void Shell::sigInt(int ) {
  Shell::_currentCommand.clear();
  Shell::prompt();
}

void Shell::sigChild(int , siginfo_t* info, void* ) {
  int pid = info->si_pid;
  
  while(waitpid(-1, nullptr, WNOHANG) > 0);

  for(size_t i = 0; i < _backgroundProcesses.size(); i++) {
    if(_backgroundProcesses[i]._pid == pid) {
      if(_backgroundProcesses[i]._isLast) {
        printf("%d exited\n", pid);
        Shell::prompt();
      }

      _backgroundProcesses.erase(_backgroundProcesses.begin() + i);
    }
  }
}

//display a random error message
void Shell::printExitMessage() {
  srand(time(NULL));

  int messIndex = rand() % exitMessages.size();

  printf("%s\n", exitMessages[messIndex]);
}

void Shell::exit() {
  Shell::printExitMessage();
  Shell::_currentCommand.clear();
  ::exit(1);
}

void Shell::changeDir(std::string* path)
{
  std::string finalPath;

  finalPath = Shell::expandTilde(path);

  if(chdir(finalPath.c_str()) != 0) {
    perror("cd");
  }
}

std::string Shell::getHome()
{
  struct passwd* pw = getpwuid(getuid());

  std::string home(pw->pw_dir);

  //printf("%s\n", home.c_str());

  return home;
}

std::string Shell::expandTilde(std::string* string)
{
  //printf("Expanding \"%s\"\n", string->c_str());

  std::string finalStr;

  for(size_t i = 0; i < string->length(); i++) {
    if(string->at(i) == '~') {
      finalStr += Shell::getHome();
    }
    else {
      finalStr += string->at(i);
    }
  }

  //printf("Final string: \"%s\"\n", finalStr.c_str());

  return finalStr;
}

void Shell::addBackgroundProcess(int pid, bool last) {
  _backgroundProcesses.push_back( {pid, last} );
}

int main() {
  //set up the interrupt signal handler
  struct sigaction intAction;
  intAction.sa_handler = Shell::sigInt;
  sigemptyset(&intAction.sa_mask);
  intAction.sa_flags = SA_RESTART;

  if(sigaction(SIGINT, &intAction, NULL)) {
    perror("Could not set SIGINT handler");
    exit(2);
  }

  //set up the child signal handler
  struct sigaction childAction;
  childAction.sa_sigaction = Shell::sigChild;
  sigemptyset(&childAction.sa_mask);
  childAction.sa_flags = SA_RESTART | SA_SIGINFO;

  if(sigaction(SIGCHLD, &childAction, NULL)) {
    perror("Could not set SIGCHILD handler");
    exit(2);
  }

  Shell::prompt();
  yyparse();
}

Command Shell::_currentCommand;
std::vector<BackgroundProcess> Shell::_backgroundProcesses;
