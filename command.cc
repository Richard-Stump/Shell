/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT 
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE 
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM  
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "command.hh"
#include "shell.hh"

Command::Command() {
    // Initialize a new vector of Simple Commands
    _simpleCommands = std::vector<SimpleCommand *>();

    _outFile = NULL;
    _inFile = NULL;
    _errFile = NULL;
    _background = false;
}

void Command::insertSimpleCommand( SimpleCommand * simpleCommand ) {
    // add the simple command to the vector
    _simpleCommands.push_back(simpleCommand);
}

void Command::clear() {
    // deallocate all the simple commands in the command vector
    for (auto simpleCommand : _simpleCommands) {
        delete simpleCommand;
    }

    // remove all references to the simple commands we've deallocated
    // (basically just sets the size to 0)
    _simpleCommands.clear();

    if ( _outFile ) {        
        //_outFile and _errFile could point to the same string. If they do,
        //set both to null
        if( _outFile == _errFile ) {
            _errFile = NULL;
        }
        
        delete _errFile;
    }
    _outFile = NULL;

    if ( _inFile ) {
        delete _inFile;
    }
    _inFile = NULL;

    if ( _errFile ) {
        delete _errFile;
    }
    _errFile = NULL;

    _background = false;
}

void Command::print() {
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    int i = 0;
    // iterate over the simple commands and print them nicely
    for ( auto & simpleCommand : _simpleCommands ) {
        printf("  %-3d ", i++ );
        simpleCommand->print();
    }

    printf( "\n\n" );
    printf( "  Output       Input        Error        Background\n" );
    printf( "  ------------ ------------ ------------ ------------\n" );
    printf( "  %-12s %-12s %-12s %-12s\n",
            _outFile?_outFile->c_str():"default",
            _inFile?_inFile->c_str():"default",
            _errFile?_errFile->c_str():"default",
            _background?"YES":"NO");
    printf( "\n\n" );
}

void Command::execute() {
    // Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
        Shell::prompt();
        return;
    }

    // Print contents of Command data structure
    print();

    // Add execution here
    // For every simple command fork a new process
    // Setup i/o redirection
    // and call exec
    int pid;

    for( SimpleCommand* sc : _simpleCommands ) {
      pid = fork();

      if(pid == 0) {
        char* const* argv = sc->getArgv();

        execvp(argv[0], argv);

        delete[] argv;

        perror("execvp error");
        _exit(1);
      }
      else if (pid < 0) {
        perror("Fork Error\n");
        return;
      }
      else {

      }
    }

    int fdStdin = dup(0);
    int fdStdout = dup(1);
    int fdStderr = dup(2);

    int fdRedirectIn = open(_inFile->c_str(), 0);

    int outFlags = _appendOut ? (O_CREAT | O_APPEND) : (O_CREAT);
    int fdRedirectOut = open(_outFile->c_str(), outFlags);

    int errFlags = _appendOut ? (O_CREAT | O_APPEND) : (O_CREAT);
    int fdRedirectErr = open(_errFile->c_str(), errFlags);

    dup2(0, fdRedirectIn);
    dup2(1, fdRedirectOut);
    dup2(2, fdRedirectErr);

    if(!_background) {
      waitpid(pid, nullptr, 0);
    }

    dup2(0, fdStdin);
    dup2(1, fdStdout);
    dup2(2, fdStderr);

    // Clear to prepare for next command
    clear();

    // Print new prompt
    Shell::prompt();
}

SimpleCommand * Command::_currentSimpleCommand;
