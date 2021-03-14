#include <cstdio>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <string.h>

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <pwd.h>

#include "shell.hh"

int yyparse(void);

//list of random messages that can be printed when the user exits
std::vector<const char*> exitMessages = {
  "Goodbye!", "Goodbye!", "Goodbye!", "Goodbye!", "Goodbye!", "Goodbye!",
  "Goodbye!", "Toodle-loo!", "See Ya!", "Goodbyte!", "Hasta la vista, Baby!",
  "Au revoir!", "Bye-bye!", "Bye-byte!", "Rage quitting a shell!?",
  "Don't leave yet -- There's a daemon around that corner! ",
  "Go ahead and leave. See if I care.", "Good riddence!",
  "Don't leave, I\'ll miss you!", "Is it that time already?"
};

//display a small prompt of the directory the user is in
void Shell::prompt() {
  if( isatty(0) ) {
    //get the current working directory
    char workingDir[PATH_MAX];
    getcwd(workingDir, PATH_MAX);

    //display it in green, because why not?
    printf("\033[32m%s>\033[0m", workingDir);
    fflush(stdout);
  }
}

//handle a control-c interrupt
void Shell::sigInt(int ) {
  Shell::_currentCommand.clear();
  Shell::prompt();
}

//Handles the signal from a child exiting. This is so that child processes are
//proporly closed. 
void Shell::sigChild(int , siginfo_t* info, void* ) {
  int pid = info->si_pid;
  
  while(waitpid(-1, nullptr, WNOHANG) > 0);

  //remove the file from the list of background processes, and then
  //print a message if it was the last on in a commmand sequence.
  for(size_t i = 0; i < _backgroundProcesses.size(); i++) {
    if(_backgroundProcesses[i]._pid == pid) {
      if(_backgroundProcesses[i]._isLast) {
        printf("%d exited\n", pid);
        Shell::prompt();
      }

      _backgroundProcesses.erase(_backgroundProcesses.begin() + i);
    }
  }
}

//display a random error message
void Shell::printExitMessage() {
  srand(time(NULL));

  int messIndex = rand() % exitMessages.size();

  printf("%s\n", exitMessages[messIndex]);
}

//closes the shell and prints an exit message if the input is coming from a TTY
void Shell::exit() {

  if(isatty(0))
    Shell::printExitMessage();
  Shell::_currentCommand.clear();
  ::exit(0);
}

void Shell::changeDir(std::string* path)
{
  std::string finalPath;

  finalPath = Shell::expandTilde(path);

  if(chdir(finalPath.c_str()) != 0) {
    fprintf(stderr, "cd: can't cd to %s", path->c_str());
  }
}

//get the home folder of the user.
std::string Shell::getHome()
{
  //TODO: replace this code with code that uses the HOME environment variable
  struct passwd* pw = getpwuid(getuid());

  std::string home(pw->pw_dir);

  return home;
}

std::string Shell::expandTilde(std::string* string)
{
  std::string finalStr;

  //search for all the tildas and replace them with the home directory
  for(size_t i = 0; i < string->length(); i++) {
    if(string->at(i) == '~') {
      finalStr += Shell::getHome();
    }
    else {
      finalStr += string->at(i);
    }
  }

  return finalStr;
}

void Shell::setEnv(std::string* name, std::string* value)
{
  setenv(name->c_str(), value->c_str(), 1);
}

void Shell::unsetEnv(std::string* name)
{
  unsetenv(name->c_str());
}

void Shell::printEnv() {
  for(size_t i = 0; environ[i] != NULL; i++) {
    printf("%s\n", environ[i]);
  }
}

void Shell::addBackgroundProcess(int pid, bool last) {
  _backgroundProcesses.push_back( {pid, last} );
}

//execute a subshell command, given by *command, and place its output in *output
void Shell::executeSubshell(std::string* command, std::string* output,
                            bool replaceNewlines)
{
  //create the pipes nessesary to perform the subshell.
  //   One that goes from the parent to the child, and another that goes from
  //   the child to the parent
  int pipeIn[2], pipeOut[2];
  if(pipe(pipeIn) == -1) {
    perror("Error creating input pipe");
  }
  if(pipe(pipeOut) == -1) {
    perror("error creating output pipe");
  }

  int pid = fork();

  if(pid == 0) {
    //set stdin and stdout for the child to the propor pipe descriptors
    dup2(pipeIn[0], 0);
    dup2(pipeOut[1], 1);
    //close all the descriptors because we don't need them anymore
    close(pipeIn[0]);
    close(pipeIn[1]);
    close(pipeOut[0]);
    close(pipeOut[1]);

    //execute the subshell
    execlp("/proc/self/exe", "/proc/self/exe", (char*)NULL);

    perror("execlp error");
  }
  else if (pid < 0) {
    perror("fork error");
    _exit(1);
  }
  else {
    //write the given command and an exit statement to the child process
    write(pipeIn[1], command->c_str(), command->size());
    write(pipeIn[1], "\nquit\n", strlen("\nquit\n"));

    //close all the pipes except the output from the child. These need to be
    //closed so that the child knows when they can stop reading
    close(pipeIn[0]);
    close(pipeIn[1]);
    close(pipeOut[1]);
    
    char buff;

    while(read(pipeOut[0], &buff, 1) != 0) {
      //replace all newlines in the output with spaces
      if(replaceNewlines && buff == '\n') 
        buff = ' '; 

      *output += buff;
    }
    
    close(pipeOut[0]); //close the output from the child
  }
}

int main() {
  //set up the interrupt signal handler
  struct sigaction intAction;
  intAction.sa_handler = Shell::sigInt;
  sigemptyset(&intAction.sa_mask);
  intAction.sa_flags = SA_RESTART;

  if(sigaction(SIGINT, &intAction, NULL)) {
    perror("Could not set SIGINT handler");
    exit(2);
  }

  //set up the child signal handler
  struct sigaction childAction;
  childAction.sa_sigaction = Shell::sigChild;
  sigemptyset(&childAction.sa_mask);
  childAction.sa_flags = SA_RESTART | SA_SIGINFO;

  if(sigaction(SIGCHLD, &childAction, NULL)) {
    perror("Could not set SIGCHILD handler");
    exit(2);
  }

  Shell::prompt();
  yyparse();
}

Command Shell::_currentCommand;
std::vector<BackgroundProcess> Shell::_backgroundProcesses;
