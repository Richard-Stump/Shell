#ifndef simplcommand_hh
#define simplecommand_hh

#include <string>
#include <vector>
#include <memory>

struct SimpleCommand {

  // Simple command is simply a vector of strings
  std::vector<std::string *> _arguments;

  SimpleCommand();
  ~SimpleCommand();
  void insertArgument( std::string * argument );
  void print();
  void execute();
  void wait();

  const char** getArgv();
  void freeArgv(const char** argv);

private:
  int   _pid;
  bool  _running;
  bool  _isChild = false;
};

#endif
