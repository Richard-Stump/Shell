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
        
        delete _outFile;
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

    int pid, fdIn, fdOut, fdErr;

    //save the standard IO descriptors so that we can restore them later
    int tmpIn = dup(0);
    int tmpOut = dup(1);
    int tmpErr = dup(2);

    fdIn = getInRedirect(tmpIn, _inFile);
    fdErr = getOutRedirect(tmpErr, _errFile, _appendErr);

    for(SimpleCommand* sc : _simpleCommands) {
      int nextFdIn; //the fdIn for the next command

      //if the current command is the last command, redirect it's output to a
      //file, otherwise, set up the pipe between two commands
      if(sc == _simpleCommands.back()) {
        //if the output and error files are the same, just copy the descriptor
        //for the error file
        if(_outFile && _errFile && *_outFile == *_errFile)
          fdOut = dup(fdErr);
        else
          fdOut = getOutRedirect(tmpOut, _outFile, _appendOut);
      }
      else {
        int fdPipe[2];
        pipe(fdPipe);

        nextFdIn = fdPipe[0];
        fdOut = fdPipe[1];
      }

      pid = sc->execute(fdIn, fdOut, fdErr);

      //close the filed we've opened for in and out
      close(fdIn);
      close(fdOut);

      fdIn = nextFdIn; //set fdIn for the next command to execute
    }

    close(fdErr);

    //wait for the last process unless it is to run in the background
    if(!_background) {
      waitpid(pid, nullptr, 0);
    }

    //restore stdin and out
    dup2(tmpIn, 0);
    dup2(tmpOut, 1);
    dup2(tmpErr, 2);

    //close the temporary descriptors
    close(tmpIn);
    close(tmpOut);
    close(tmpErr);

    //clear the command table to prepare for the next command
    clear();

    //prompt the user for the next command
    Shell::prompt();
    return;
}

int Command::getInRedirect(int cur, std::string* name) {
  if(name) {
    int flags = O_RDONLY;
    return open(name->c_str(), flags);
  }
  else {
    return dup(cur);
  }
}

int Command::getOutRedirect(int cur, std::string* name, bool append) {
  if(name) {
    int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
    return open(name->c_str(), flags);
  }
  else {
    return dup(cur);
  }
}

SimpleCommand * Command::_currentSimpleCommand;
