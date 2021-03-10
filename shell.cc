#include <cstdio>

#include <unistd.h>
#include <signal.h>

#include "shell.hh"

int yyparse(void);

std::vector<char*> exitMessages = {
    "Goodbye!",
    "Too-da-loo!",
    "See Ya!",
    "Goodbyte!",
    "Hasta la vista, Baby!"
}

void Shell::prompt() {
  if( isatty(0) ) {
    printf("myshell>");
    fflush(stdout);
  }
}

void Shell::signal(int sig) {
  fprintf(stderr, "From signal handler\n");
}

void ShellL::printExitMessage() {

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
