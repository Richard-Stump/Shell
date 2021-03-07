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

    int tmpIn = dup(0);
    int tmpOut = dup(1);
    int tmpErr = dup(2);

    int pid, fdIn, fdOut, fdErr;

    if(_inFile) {
      fdIn = open(_inFile->c_str(), O_RDONLY);
    }
    else {
      fdIn = dup(tmpIn);
    }

    if(_errFile) {
      int errFlags = O_WRONLY | O_CREAT | (_appendOut ? O_APPEND : O_TRUNC);
      fdErr = open(_errFile->c_str(), errFlags);
    }
    else {
      fdErr = dup(tmpErr)
    }

    dup2(fdErr, 2);

    for( SimpleCommand* sc : _simpleCommands ) {
      dup2(fdIn, 0);
      close(fdIn);
      
      //the last simple command
      if (sc == _simpleCommands.back()) 
      {
        //if the user has specified an output file, open it and use it. Else,
        //use the default stdout.
        if(_outFile) {
          int outFlags = O_WRONLY | O_CREAT | (_appendOut ? O_APPEND : O_TRUNC);
          fdOut = open(_outFile->c_str(), outFlags);
        }
        else {
          fdOut = dup(tmpOut);
        }
      }
      else {
        //not the last simple command
        //set the pipe up for this command
        int fdPipe[2];
        pipe(fdPipe);

        fdIn = fdPipe[0];
        fdOut = fdPipe[1];

      }

      dup2(fdOut, 1);
      close(fdOut);

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
    }

    if(!_background) {
      waitpid(pid, nullptr, 0);
    }

    close(fdErr);

    //restore the default stdin, stdout, and stderr
    dup2(tmpIn, 0);
    dup2(tmpOut, 1);
    dup2(tmpErr, 2);
    close(tmpIn);
    close(tmpOut);
    close(tmpErr);

    // Clear to prepare for next command
    clear();

    // Print new prompt
    Shell::prompt();
}

SimpleCommand * Command::_currentSimpleCommand;
