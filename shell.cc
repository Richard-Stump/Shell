#include <cstdio>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>
#include <signal.h>

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
  "Oh sure, run away from your responsibilities!",
  "Go ahead and leave. See if I care."
};

void Shell::prompt() {
  if( isatty(0) ) {
    printf("myshell>");
    fflush(stdout);
  }
}

void Shell::signal(int sig) {
  fprintf(stderr, "From signal handler\n");
  
  Shell::_currentCommand.clear();
  Shell::prompt();
}

void Shell::printExitMessage() {
  srand(time(NULL));

  int messIndex = rand() % exitMessages.size();

  printf("%s\n", exitMessages[messIndex]);
}

int main() {
  //set up the signal handler
  struct sigaction sigAction;
  sigAction.sa_handler = Shell::signal;
  sigemptyset(&sigAction.sa_mask);
  sigAction.sa_flags = SA_RESTART;

  if(sigaction(SIGINT, &sigAction, NULL)) {
    perror("Could not set SIGINT handler");
    exit(2);
  }

  Shell::prompt();
  yyparse();
}

Command Shell::_currentCommand;
