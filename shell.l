
/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include "shell.hh"
#include "y.tab.hh"

static  void yyunput (int c,char *buf_ptr);

void myunputc(int c) {
  unput(c);
}

%}

%option noyywrap

%%

$\(.*\) {
  std::string str(yytext);
  str = str.substr(2, str.length() - 2 - 1);

  std::string output("");

  Shell::executeSubshell(&str, &output);

  printf("Subshell output:\n%s", output.c_str());
}

`.*` {
  std::string str(yytext);
  str = str.substr(1, str.length() - 1 - 1);

  std::string output("");

  Shell::executeSubshell(&str, &output);
  
  printf("Subshell output:\n%s", output.c_str());
}

"exit"|"quit" {
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

[^ \t\n]*\\.[^ \t\n]* {
  std::string arg(yytext);

  std::string* finalStr = new std::string(""); 

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

"setenv"|"unsetenv"|"source"|"cd" {
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

  tmp = Shell::expandTilde(&tmp);

  yylval.cpp_string = new std::string(tmp);
  return WORD;
}
