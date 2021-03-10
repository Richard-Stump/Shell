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

  int execute(int fdIn, int fdOut, int fdErr);

  char* const* getArgv();
};

#endif
