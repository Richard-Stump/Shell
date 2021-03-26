
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

/* these tokens have types associated with them, so they contain extra data */
%token <cpp_string> WORD
%token <cpp_string> BUILTIN_PARENT
%token <cpp_string> BUILTIN_CHILD

/* These tokens have no extra data associated with them */
%token NOTOKEN NEWLINE PIPE AMPERSAND
%token GREAT GREATGREAT LESS AMPGREAT AMPGREATGREAT TWOGREAT TWOGREATGREAT

%{
//#define yylex yylex
#include <cstdio>
#include "shell.hh"

void yyerror(const char * s);
int yylex();

%}

%%

/*Top of the parsing tree*/
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

/* A list of commands, their arguments, and the IO modifiers  */
/* The commands are separated by a '|' symbol, and they get   */
/* piped together                                             */
simple_command:	
  pipe_list iomodifier_opt NEWLINE {
    //printf("   Yacc: Execute command\n");
    Shell::_currentCommand.execute();
  }
  | pipe_list iomodifier_opt AMPERSAND NEWLINE {
    Shell::_currentCommand._background = true;
    Shell::_currentCommand.execute();
  }
  | NEWLINE { //prompt the user again if they only input a new line
    Shell::prompt();
  }
  | error NEWLINE { yyerrok; }
  ;

/* A list of commands to be piped together, or a single command */
pipe_list:
  pipe_list PIPE command_and_args
  | command_and_args
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
    Shell::expandWildcards( * $1 );
    //Command::_currentSimpleCommand->insertArgument( $1 );
  }
  ;

/* The name of a command to run. These commands are other programs or inernal */
/* commands depending on what the lexer returns as a token. Some internal     */
/* commands may be run as the parent or the child. External commands are      */
/* always run as a child process                                              */
command_word:
  WORD {
    //printf("   Yacc: insert command \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
    Command::_currentSimpleCommand->_isBuiltin = false;
    Command::_currentSimpleCommand->_runAsParent = false;
  }
  | BUILTIN_PARENT {
    //printf("   Yacc: insert bulitin parent \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
    Command::_currentSimpleCommand->_isBuiltin = true;
    Command::_currentSimpleCommand->_runAsParent = true;
  }
  | BUILTIN_CHILD {
    //printf("   Yacc: insert bulitin child \"%s\"\n", $1->c_str());
    Command::_currentSimpleCommand = new SimpleCommand();
    Command::_currentSimpleCommand->insertArgument( $1 );
    Command::_currentSimpleCommand->_isBuiltin = true;
    Command::_currentSimpleCommand->_runAsParent = false;
  }
  ;

iomodifier_opt:
  iomodifier_opt io_modifier
  | /* can be empty */
  ;

io_modifier:
  GREAT WORD {
    //Detect if the redirect has already been set. If so, throw an error
    if( Shell::_currentCommand._outFile ) {
      yyerror("Ambiguous output redirect.\n");
      yyerrok;
    }

    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._appendOut = false;
  }
  | GREATGREAT WORD {
    //Detect if the redirect has already been set. If so, throw an error
    if( Shell::_currentCommand._outFile ) {
      yyerror("Ambiguous output redirect.\n");
      yyerrok;
    }

    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._appendOut = true;
  }
  | TWOGREAT WORD {
    //Detect if the redirect has already been set. If so, throw an error
    if( Shell::_currentCommand._errFile ) {
      yyerror("Ambiguous output redirect.\n");
      yyerrok;
    }

    Shell::_currentCommand._errFile = $2;
    Shell::_currentCommand._appendErr = false;
  }
  | TWOGREATGREAT WORD {
    //Detect if the redirect has already been set. If so, throw an error
    if( Shell::_currentCommand._errFile ) {
      yyerror("Ambiguous output redirect.\n");
      yyerrok;
    }

    Shell::_currentCommand._errFile = $2;
    Shell::_currentCommand._appendErr = true;
  }
  | AMPGREAT WORD {
    //Detect if the redirect has already been set. If so, throw an error
    if( Shell::_currentCommand._outFile || Shell::_currentCommand._errFile ) {
      yyerror("Ambiguous output redirect.\n");
      yyerrok;
    }

    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._appendOut = false;
    Shell::_currentCommand._errFile = $2;
    Shell::_currentCommand._appendErr = false;
  }
  | AMPGREATGREAT WORD {
    //Detect if the redirect has already been set. If so, throw an error
    if( Shell::_currentCommand._outFile || Shell::_currentCommand._errFile ) {
      yyerror("Ambiguous output redirect\n");
      yyerrok;
    }

    Shell::_currentCommand._outFile = $2;
    Shell::_currentCommand._appendOut = true;
    Shell::_currentCommand._errFile = $2;
    Shell::_currentCommand._appendErr = true;

  }
  | LESS WORD {
    if( Shell::_currentCommand._inFile) {
      yyerror("Ambiguous input redirect\n");
      yyerrok;
    }

    Shell::_currentCommand._inFile = $2;
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
