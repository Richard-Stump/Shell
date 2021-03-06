#include <cstdio>

#include "shell.hh"

int yyparse(void);

void Shell::prompt() {

  if(isatty()) {
    printf("myshell>");
  }
  fflush(stdout);
}

int main() {
  Shell::prompt();
  yyparse();
}

Command Shell::_currentCommand;
