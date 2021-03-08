
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%code requires 
{
#include <string>

#if __cplusplus > 199711L
#define register      // Deprecated in C++11 so remove the keyword
#endif
}

%union
{
  char        *string_val;
  // Example of using a c++ type in yacc
  std::string *cpp_string;
}

%token <cpp_string> WORD
%token NOTOKEN  NEWLINE PIPE AMPERSAND
%token GREAT GREATGREAT LESS AMPGREAT AMPGREATGREAT TWOGREAT TWOGREATGREAT

%{
//#define yylex yylex
#include <cstdio>
#include "shell.hh"

void yyerror(const char * s);
int yylex();

%}

%%

goal:
  command_list
  ;

command_list:
  command_line
  | command_list command_line
  ;

command_line: 
  simple_command
       ;

simple_command:	
  pipe_list iomodifier_opt NEWLINE {
    //printf("   Yacc: Execute command\n");
    Shell::_currentCommand.execute();
  }
  | pipe_list iomodifier_opt AMPERSAND NEWLINE {
    Shell::_currentCommand._background = true;
    Shell::_currentCommand.execute();
  }
  | NEWLINE 
  | error NEWLINE { yyerrok; }
  ;

command_and_args:
  command_word argument_list {
    Shell::_currentCommand.
    insertSimpleCommand( Command::_currentSimpleCommand );
  }
  ;

argument_list:
  argument_list argument
  | /* can be empty */
  ;

argument:
  WORD {
    //printf("   Yacc: insert argument \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand->insertArgument( $1 );\
  }
  ;

command_word:
  WORD {
    //printf("   Yacc: insert command \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
  }
  ;

pipe_list:
  pipe_list PIPE command_and_args {
    //printf("   Yacc: insert recursive pipelist\n");
  }
  | command_and_args {
    //printf("   Yacc: insert pipelist with 1 command\n");
  }
  ;
/* out_and_err_modifier should not apear with either out_modifier or 
 * err_modifer 
 */
iomodifier_opt:
  out_and_err_modifier in_modifier
  | in_modifier out_and_err_modifier
  | out_modifier err_modifer in_modifier
  | out_modifier in_modifier err_modifer
  | in_modifier out_modifier err_modifer
  | in_modifier err_modifer out_modifier
  | err_modifer in_modifier out_modifier
  | err_modifer out_modifier in_modifier
  ;

iomodifier_opt:
  iomodifier_opt io_modifier
  | /* can be empty */
  ;

io_modifier:
  GREAT WORD {
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._appendOut = false;

    if( _outFile ) {
      yyerror("Ambiguous output redirect");
      yyerrok;
    }
  }
  | GREATGREAT {
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._appendOut = true;

    if( _outFile ) {
      yyerror("Ambiguous output redirect");
      yyerrok;
    }
  }
  | TWOGREAT {
    Shell::_currentCommand._errFile = $2;
    Shell::_currentCommand._appendErr = false;

    if( _errFile ) {
      yyerror("Ambiguous output redirect");
      yyerrok;
    }
  }
  | TWOGREATGREAT {
    Shell::_currentCommand._errFile = $2;
    Shell::_currentCommand._appendErr = true;

    if( _errFile ) {
      yyerror("Ambiguous output redirect");
      yyerrok;
    }
  }
  | AMPGREAT {
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._appendOut = false;
    Shell::_currentCommand._errFile = $2;
    Shell::_currentCommand._appendErr = false;

    if( _outFile || _errFile ) {
      yyerror("Ambiguous output redirect");
      yyerrok;
    }
  }
  | AMPGREATGREAT {
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._appendOut = true;
    Shell::_currentCommand._errFile = $2;
    Shell::_currentCommand._appendErr = true;

    if( _outFile || _errFile ) {
      yyerror("Ambiguous output redirect");
      yyerrok;
    }
  }
  | LESS {
    Shell::_currentCommand._inFile = $2;

    if( _inFile) {
      yyerror("Ambiguous input redirect");
      yyerrok;
    }
  }
  ;

%%

void
yyerror(const char * s)
{
  fprintf(stderr,"%s", s);
}

#if 0
main()
{
  yyparse();
}
#endif
