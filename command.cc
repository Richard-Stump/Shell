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

/**
 * Execute the command line
 */
void Command::execute() {
    // Don't do anything if there are no simple commands
    if ( _simpleCommands.size() == 0 ) {
      clear();
      Shell::prompt();
      return;
    }

    //Print contents of Command data structure
    //print();

    //save the standard IO descriptors so that we can restore them later
    int tmpIn = dup(0);
    int tmpOut = dup(1);
    int tmpErr = dup(2);

    //file desciptors for redirection. Start with the same descriptors that were
    //used to save stdIO. These will either be duplicated or overwritten when
    //opening the redirection
    int fdIn       = tmpIn;
    int fdFinalOut = tmpOut;
    int fdErr      = tmpErr;

    //open the redirect files, and if any of them produce an error prompt the
    //user again
    if(!getRedirectDescriptors(fdIn, fdFinalOut, fdErr)) {
      close(tmpIn);
      close(tmpOut);
      close(tmpErr);
      clear();
      Shell::prompt();
      return;
    }

    int pid, fdOut;
    for(SimpleCommand* sc : _simpleCommands) {
      int nextFdIn; //the fdIn for the next command

      //if the current command is the last command, redirect it's output the
      //output file, otherwise, set up the pipe between two commands
      if(sc == _simpleCommands.back()) {
        fdOut = fdFinalOut;
      }
      else {
        int fdPipe[2];  //get a pipe from the os
        pipe(fdPipe);

        nextFdIn = fdPipe[0];
        fdOut = fdPipe[1];
      }

      pid = sc->execute(fdIn, fdOut, fdErr);

      if(_background) {
        Shell::addBackgroundProcess(pid, false);
      }
 
      //close the filed we've opened for in and out
      close(fdIn);
      close(fdOut);

      fdIn = nextFdIn; //set fdIn for the next command to execute
    }

    close(fdErr);

    //If the command is supposed to run in the background, add the last
    //command to the background list, else, wait on it.
    if(_background) {
      Shell::addBackgroundProcess(pid, true);
    }
    else {
      waitpid(pid, nullptr, 0);
    }

    //restore stdIO
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

/**
 * Gets the file descriptor to be used as the first input file, this is either
 * the current input descriptor, or a new one if name is not NULL
 * @param cur  The current input file handler
 * @param name The name of the file to open, or null to use the current one
 * @return The file descriptor to a read-only file
 */
int Command::getInRedirect(int cur, std::string* name) {
  if(name) {
    int flags = O_RDONLY;
    return open(name->c_str(), flags);
  }
  else {
    return dup(cur);
  }
}

/**
 * Gets a file descriptor that can be used as output. This is either a copy of
 * the current descriptor, or a new one if name is not NULL.
 * @param cur The current output file descriptor
 * @param name The name of the file to open, or null to use the current one
 * @param append If true, open the file as appendable.
 * @return File descriptor to the file to be used as output
 */
int Command::getOutRedirect(int cur, std::string* name, bool append) {
  if(name) {
    int flags = O_RDWR | O_CREAT | (append ? O_APPEND : O_TRUNC);
    int fd = open(name->c_str(), flags, 0777);//forgetting the permissions flag
                                              //here caused me hours of pain...

    return fd;
  }
  else {
    return dup(cur);
  }
}

/**
 * Gets the file descriptors to be used for redirecting input and output
 * @param fdIn File descriptor of the current input file
 * @param fdOut File desciprtor for the current output file
 * @param fdErr File descriptor for the current error file
 * @return True if successful, false if there was an error
 */
bool Command::getRedirectDescriptors(int& fdIn, int& fdOut, int& fdErr) {
  int success = true;
  
  //Open the input file and if its invalid, set success to false to indicate
  //an error
  fdIn = getInRedirect(fdIn, _inFile);
  if (fdIn < 0) {
    perror("Could not open input file");
    success = false;
  }
  
  fdErr = getOutRedirect(fdErr, _errFile, _appendErr);
  if (fdErr < 0) {
    perror("Could not open error file");
    success = false;
  }

  //If the output and error files are the same file, copy the descriptor for the
  //error file, otherwise open the output file.
  if (_outFile && _errFile && *_errFile == *_outFile)
    fdOut = dup(fdErr);
  else
    fdOut = getOutRedirect(fdOut, _outFile, _appendOut);

  if (fdOut < 0) {
    perror("Could not open output file");
    success = false;
  }

  return success;
}

SimpleCommand * Command::_currentSimpleCommand;
