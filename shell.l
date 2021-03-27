
/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include <stdio.h>
#include <iostream>
#include <unistd.h>

#include "shell.hh"
#include "y.tab.hh"

static  void yyunput (int c,char *buf_ptr);

void myunputc(int c) {
  unput(c);
}
void yyerror(const char * s);

extern "C" char* read_line();

int mygetc(FILE* f) {
  static char *p;
  char ch;

  if(!isatty(0)) {
    return getc(f);
  }

  if(p == NULL || *p == 0) {
    char* s = read_line();
    p = s;
  }

  ch = *p;
  p++;

  return ch;
}

#undef getc
#define getc(f) mygetc(f)

%}

%option noyywrap

/* State for including source files */
%x incl
%%

source BEGIN(incl); /* string that signifies an include */

<incl>[ \t]*        /* consume whitespace */
<incl>[^ \t\n]+ {   /* Get the file name  */

  FILE* inFile = fopen(yytext, "r");

  /* throw an error if the file doesn't exist */
  if(!inFile) {
    yyerror("Could not open source file\n");
  }

  yypush_buffer_state(yy_create_buffer(inFile, YY_BUF_SIZE));

  BEGIN(INITIAL);
}
<<EOF>> {
  /* after the current source file is consumed, pop it from the stack */
  yypop_buffer_state();
  if(!YY_CURRENT_BUFFER) {
    yyterminate();
  }
}

$\(.*\) { /* Match the subshell */
  //obtain the actual command from the matched string
  std::string str(yytext);
  str = str.substr(2, str.length() - 2 - 1);

  std::string output("");

  Shell::executeSubshell(&str, &output);

  //put the characters back in the buffer in reverse order
  for(int i = output.length() - 1; i >= 0; --i) {
    myunputc(output[i]);
  }
}

`.*` { /* Match the subshell 2, electric boogaloo*/
  //obtain the actual command from the matched string
  std::string str(yytext);
  str = str.substr(1, str.length() - 1 - 1);

  std::string output("");

  Shell::executeSubshell(&str, &output);

  //put the characters back in the buffer in reverse order
  for(int i = output.length() - 1; i >= 0; --i) {
    myunputc(output[i]);
  }
}

"exit"|"quit" { /* commands to exit */
  Shell::exit();
}

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  return GREAT;
}

"<" {
  return LESS;
}

">&" {
  return AMPGREAT;
}

"2>" {
  return TWOGREAT;
}

"2>>" {
  return TWOGREATGREAT;
}

">>&" {
  return AMPGREATGREAT;
}

">>" {
  return GREATGREAT;
}

"|" {
  return PIPE;
}

"&" {
  return AMPERSAND;
}

"&>" {
  return AMPGREAT;
}

[^ \t\n]*\\.[^ \t\n]* { /* match any string that has an escape sequence in it */
  std::string arg(yytext);

  std::string* finalStr = new std::string(""); 

  //search through the string for all slashes, and then remove them
  for(size_t i = 0; i < arg.length(); i++) {
    if(arg[i] == '\\') {
      *finalStr += arg[++i];
    }
    else {
      *finalStr += arg[i];
    }
  }

  yylval.cpp_string = finalStr;
  return WORD;
}

\".*\" {
  std::string arg(yytext);
  yylval.cpp_string = new std::string(arg.substr(1, arg.size() - 2));
  return WORD;
}

"setenv"|"unsetenv"|"cd" {
  yylval.cpp_string = new std::string(yytext);
  return BUILTIN_PARENT;
}

"printenv" {
  yylval.cpp_string = new std::string(yytext);
  return BUILTIN_CHILD;
}

[^ \\<>|&\t\n][^ \\<>|&\t\n]* {
  /* Assume that file names have only alpha chars */
  std::string tmp(yytext);
  tmp = Shell::expandEnvironmentVars(tmp);

  yylval.cpp_string = new std::string(tmp);
  return WORD;
}
