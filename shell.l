
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

"printenv"|"setenv"|"unsetenv"|"source"|"cd" {
  yylval.cpp_string = new std::string(yytext);
  return BUILTIN;
}

[^ \\<>|&\t\n][^ \\<>|&\t\n]* {
  /* Assume that file names have only alpha chars */
  yylval.cpp_string = new std::string(yytext);
  return WORD;
}
