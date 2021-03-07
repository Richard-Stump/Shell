
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
    printf("   Yacc: Execute command\n");
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
    printf("   Yacc: insert argument \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand->insertArgument( $1 );\
  }
  ;

command_word:
  WORD {
    printf("   Yacc: insert command \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
  }
  ;

pipe_list:
  pipe_list PIPE command_and_args {
    printf("   Yacc: insert recursive pipelist\n");
  }
  | command_and_args {
    printf("   Yacc: insert pipelist with 1 command\n");
  }
  ;
/*
iomodifier_opt:
  GREAT WORD {
    printf("   Yacc: insert output \"%s\"\n", $2->c_str());
    Shell::_currentCommand._outFile = $2;
  }
  | 
  ;
*/
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

out_modifier:
  GREAT WORD {
    printf("   Yacc: insert output \"%s\"\n", $2->c_str());
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._appendOut = false;
  }
  | GREATGREAT WORD {
    printf("   Yacc: insert output with \"%s\" with append\n", $2->c_str());
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._appendOut = true;
  }
  | /* can be empty */
  ;

in_modifier:
  LESS WORD {
    printf("   Yacc: insert input \"%s\"\n", $2->c_str());
    Shell::_currentCommand._inFile = $2;
  }
  | /* can be empty */
  ;

err_modifer:
  TWOGREAT WORD {
    printf("   Yacc: insert error \"%s\"\n", $2->c_str());
    Shell::_currentCommand._errFile = $2;
    Shell::_currentCommand._appendErr = false;
  }
  | TWOGREATGREAT WORD {
    printf("   Yacc: insert error \"%s\" with append\n", $2->c_str());
    Shell::_currentCommand._errFile = $2;
    Shell::_currentCommand._appendErr = true;
  }
  | /* Can be empty */
  ;

out_and_err_modifier:
  AMPGREAT WORD {
    printf("   Yacc: insert out and error \"%s\"\n", $2->c_str());
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._appendOut = false;
    Shell::_currentCommand._errFile = $2;
    Shell::_currentCommand._appendErr = false;
  }
  | AMPGREATGREAT WORD {
    printf("   Yacc: insert out and error \"%s\" with append\n", $2->c_str());
    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._appendOut = true;
    Shell::_currentCommand._errFile = &$2;
    Shell::_currentCommand._appendErr = true;
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
