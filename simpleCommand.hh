#ifndef simplcommand_hh
#define simplecommand_hh

#include <string>
#include <vector>
#include <memory>

typedef int (*builtin_func_t)(std::vector<std::string*> args);

struct SimpleCommand {

  // Simple command is simply a vector of strings
  std::vector<std::string *> _arguments;

  SimpleCommand();
  ~SimpleCommand();
  void insertArgument( std::string * argument );
  void print();

  virtual int execute(int fdIn, int fdOut, int fdErr);

protected:
  char* const* getArgv();
};


struct BuiltinCommand : public SimpleCommand {
  BuiltinCommand();
  int execute(int fdIn, int fdOut, int fdErr) override;
};

#endif
