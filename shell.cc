#include <cstdio>
#include <stdlib.h>
#include <time.h>
#include <map>

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

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
    printf("myshell>");
    fflush(stdout);
  }
}

void Shell::sigInt(int sig) {
  fprintf(stderr, "From signal handler\n");

  Shell::_currentCommand.clear();
  Shell::prompt();
}

void Shell::sigChild(int sig, siginfo_t *info, void *ucontext) {
  int status;

  waitpid(info->si_pid, &status, 0);



  printf("%d exited\n", info->si_pid);
  prompt();
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

void Shell::changeDir()
{
  
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
  childAction.sa_flags = SA_RESTART;

  if(sigaction(SIGCHLD, &childAction, NULL)) {
    perror("Could not set SIGCHILD handler");
    exit(2);
  }

  Shell::prompt();
  yyparse();
}

void Shell::addBackgroundProcess(int pid, bool last) {
  _backgroundMap.insert({pid, last})
}

Command Shell::_currentCommand;
std::map<int, bool> _backgroundMap;