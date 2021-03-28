#include <cstdio>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <string.h>

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <pwd.h>
#include <regex.h>
#include <dirent.h>

#include "shell.hh"
#include "y.tab.hh"

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
  if( isatty(0) && promptEnabled) {
    std::string promptName = "PROMPT";
    std::string promptValue = getEnv(promptName);

    //if the environment variable doesn't exist, print the current working
    //directory
    if(promptValue.empty()) {
      char workingDir[PATH_MAX];
      getcwd(workingDir, PATH_MAX);
      workingDir[PATH_MAX - 1] = '\0';
      printf("\033[91m%s>\033[0m ", workingDir);
    }
    else {
      printf("\033[91m%s\033[0m ", promptValue.c_str());
    }

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
void Shell::sigChild(int, siginfo_t* info, void* ) {
  while(waitpid(-1, nullptr, WNOHANG) > 0);

  for(size_t i = 0; i < _finalCommands.size(); i++) {
    FinalCommand command = _finalCommands[i];
    
    //if the command is in the list, remove it and set the corresponding return
    //value. If its a background process, print a message about it exiting
    if(command._pid == info->si_pid){
      if(command._background) {
        _lastBackRet = info->si_status;
        printf("%d exited\n", command._pid);
        Shell::prompt();
      }
      else {
        _lastRet = info->si_status;
      }

      _finalCommands.erase(_finalCommands.begin() + i);
    }
  }

  std::string errorName = "ON_ERROR";
  std::string errorVal = getEnv(errorName);

  if(!errorVal.empty() && info->si_status != 0) {
    printf("%d exited with code %d\n", info->si_pid, info->si_status);
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
  if(chdir(path->c_str()) != 0) {
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

std::string Shell::trimWhitespace(std::string& str)
{
  std::string output("");

  //trim the leading whitespace
  for(int i = 0; i < (int)str.length(); i++) {
    if(str[i] != ' ' && str[i] != '\t') {
      output = str.substr(i, str.length() - i);
      break;
    }
  }

  //trim the trailing whitespace
  for(int i = output.length() - 1; i > 0; i--) {
    if(output[i] != ' ' && output[i] != '\t') {
      output = output.substr(0, i + 1);
      break;
    }
  }

  return output;
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

std::string Shell::getEnv(std::string& name)
{
  if(name == "$") {
    return std::to_string(getpid());
  }
  else if(name == "?") { /* return code of last command */ 
    return std::to_string(_lastRet);
  }
  else if(name == "!") { /* PID of command last run in the background */ 
    return std::to_string(_lastBackPid);
  }
  else if(name == "_") { /* last argument in the fully expanded previous command */
    return lastArg;
  }
  else if(name == "SHELL") { /* Path of the shell */
    char resolved[PATH_MAX];
    realpath(argv[0], resolved); 
    return std::string(resolved);
  }
  else { /* Get the variable from the environment */
    const char* cVal = getenv(name.c_str());

    if(cVal)
      return cVal;
    else
      return "";
  }
}

std::string Shell::expandEnvironmentVars(std::string& string)
{
  std::string result = string; //string for manipulating the input, and string
                               //that is returned

  const char* regexStr = "\\$\\{[^\\}]+\\}"; //regex for matching environment
                                             //variables
  regex_t regex;
  int code = regcomp(&regex, regexStr, REG_EXTENDED);
  
  // If there is an error compiling the regex
  if(code != 0) {
    //get the compilation error string and put it into a buffer
    char errbuff[128]; errbuff[127] = '\0';
    regerror(code, &regex, errbuff, 127);
    
    fprintf(stderr, "environment variable regex is invalid!\n%d: %s\n", code, errbuff);
    ::exit(-1);
  }

  regmatch_t match;

  //loop until there are no more matches.
  while(regexec(&regex, result.c_str(), 1, &match, 0) == 0) {
    //get the name of the environment variable from inside the match ${NAME}
    //then get the environment variables value
    std::string name = result.substr(match.rm_so + 2,
                                     match.rm_eo - match.rm_so - 3);
    std::string value = getEnv(name);

    //construct a new string that replaces ${NAME} with VALUE
    std::string temp = result.substr(0, match.rm_so);
    temp += value;
    temp += result.substr(match.rm_eo, result.length() - match.rm_eo);

    //set the result to our new string, and possibly loop again to find the
    //next environment variable
    result = temp;
  }

  regfree(&regex);

  return result;
}

std::string Shell::expandTilde(std::string& string)
{
  if(string[0] != '~')
    return string;
  else if (string.length() == 1) {
    std::string name = "HOME";
    return getEnv(name);
  }

  size_t slashIndex = string.find('/');
  
  std::string subPath = "", userName = "";

  //Get the subpath and username from the tilde expansion. If there is no sub
  //path, just set the string for it to an empty string
  if(slashIndex != std::string::npos) {
    subPath = string.substr(slashIndex, string.length() - slashIndex);
    userName = string.substr(1, slashIndex - 1);
  }
  else {
    userName = string.substr(1, string.length() - 1);
  }

  //get home directory for the given user
  struct passwd* password = getpwnam(userName.c_str());
  if(password == nullptr) {
    fprintf(stderr, "User does not exist\n");
    ::exit(-1);
  }

  std::string home(password->pw_dir);

  return home + subPath;
}

std::string Shell::extractNextComponent(std::string& prefix, std::string& suffix)
{
  size_t slashIndex = suffix.find('/');

  std::string component;

  if(slashIndex == std::string::npos) {
    component = suffix;
    suffix = "";
  }
  else {
    component = suffix.substr(0, slashIndex);
    suffix = suffix.substr(slashIndex + 1);
    if(slashIndex == 0)
      prefix += '/';
  }

  return component;
}

bool Shell::pathHasWildcard(std::string& path) {
  return (path.find('*') == std::string::npos || 
          path.find('?') == std::string::npos);
}

bool Shell::recursivelyExpandWildcards(std::string prefix, std::string suffix) 
{
  const char* extraChar = prefix.empty() ? "" : "/";

  //If the suffix is empty, we cannot expand the argument any more
  //so it can be inserted
  if(suffix.empty()) {
    std::string* arg = new std::string(prefix);
    Command::_currentSimpleCommand->insertArgument(arg);

    return true;
  }

  //check if the user is trying to look at the root directory
  if(prefix.empty() && suffix[0] == '/') {
    prefix = '/';
    suffix = suffix.substr(1);
  }

  std::string component = extractNextComponent(prefix, suffix);

  if(!Shell::pathHasWildcard(component)) {
    std::string newPrefix = prefix + extraChar + component;
    return recursivelyExpandWildcards(newPrefix, suffix);;
  }
 
  std::string regexStr = wildcardToRegex(component);

  regex_t regex;
  int code = regcomp(&regex, regexStr.c_str(), REG_EXTENDED | REG_NOSUB);
  
  //If there is an error compiling the regex
  if(code != 0) {
    //get the compilation error string and put it into a buffer
    char errbuff[128]; errbuff[127] = '\0';
    regerror(code, &regex, errbuff, 127);

    regfree(&regex);
    
    fprintf(stderr, "Bad wildcard regex\n%d: %s\n", code, errbuff);
    return false;
  }

  std::string dir;
  if(prefix.empty())
    dir = ".";
  else
    dir = prefix;

  struct dirent** nameList;
  int nameCount = scandir(dir.c_str(), &nameList, NULL, alphasort);
  if(nameCount == -1) {
    regfree(&regex);
    return false;
  }

  bool foundMatch = false;

  for(int i = 0; i < nameCount; i++) { 
    const char* name = nameList[i]->d_name;

    if(regexec(&regex, name, 0, nullptr, 0) == 0) {
      std::string newPrefix = prefix + extraChar +  name;
      if(name[0] == '.' && component[0] == '.') {
        foundMatch |= recursivelyExpandWildcards(newPrefix, suffix);
      }
      else if(name[0] != '.') {
        foundMatch |= recursivelyExpandWildcards(newPrefix, suffix);
      }
    }

    free(nameList[i]);
  }

  free(nameList);

  regfree(&regex);
  return foundMatch;
}

void Shell::expandWildcards(std::string& path)
{

  //return the path if there are no wildcards
  if(path.find('*') == std::string::npos && path.find('?') == std::string::npos)
  {
    Command::_currentSimpleCommand->insertArgument(&path);
    return;
  }

  if(!Shell::recursivelyExpandWildcards("", path)) {
    Command::_currentSimpleCommand->insertArgument(&path);
  }
  return;
}

std::string Shell::wildcardToRegex(std::string wildcard)
{
  std::string result = "^";

  for(char c : wildcard) {
    switch(c) 
    {
    case '*':
      result += ".*"; 
      break;
    case '?':
      result += "."; 
      break;
    case '.':
      result += "\\."; 
      break;
    default:
      result += c;
      break;  
    }
  }
  
  return result + "$";
}

void Shell::addBackgroundProcess(int pid, bool last) {
  _backgroundProcesses.push_back( {pid, last} );
}

void Shell::addFinalCommand(int pid, bool background) {
  _finalCommands.push_back( { pid, background });
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
    _exit(1);
  }
  else if (pid < 0) {
    perror("fork error");
    return;
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

void Shell::doSubstitution(std::string* command, std::string* output) {
  *output = "";

  char templateName[] = "tempXXXXXX";
  char* tempName = mkdtemp((char*)templateName);
  if(tempName == NULL) {
    perror("mkdtemp error");
    return;
  }

  char fifoPath[32];

  fprintf(stderr, "temp dir: \"%s\"\n", tempName);
  sprintf(fifoPath, "%s/fifo", tempName);
  fprintf(stderr, "path: \"%s\"\n", fifoPath);

  int ret = mkfifo(fifoPath, 0700);
  if(ret == -1) {
    perror("mkfifo error");
    return;
  }


  int fdFifo = open(fifoPath , 0700);
  if(fdFifo < 0) {
    perror("Could not open fifo");
    return;
  }


  if(write(fdFifo, "asdf\n", 5) != 5) {
    perror("could not write");
    return;
  } 

  close(fdFifo);
  *output = fifoPath;
}

void lex_main(void);

int main(int argc, const char** argv) {
  Shell::argc = argc;
  Shell::argv = argv;

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

  lex_main();
}

int Shell::argc = 0;
const char** Shell::argv;
std::string Shell::lastArg = "";
int Shell::_lastBackPid = -1;

Command Shell::_currentCommand;
std::vector<BackgroundProcess> Shell::_backgroundProcesses;
std::vector<FinalCommand> Shell::_finalCommands;

int Shell::_lastRet = -1;
int Shell::_lastBackRet = -1;

bool Shell::promptEnabled = false;