#ifndef command_hh
#define command_hh

#include "simpleCommand.hh"

// Command Data Structure

struct Command {
  std::vector<SimpleCommand *> _simpleCommands;
  std::string * _outFile;
  std::string * _inFile;
  std::string * _errFile;
  bool _background;
  bool _appendOut;
  bool _appendErr;

  Command();
  void insertSimpleCommand( SimpleCommand * simpleCommand );

  void clear();
  void print();
  void execute(); 

  static SimpleCommand *_currentSimpleCommand;

private:
  int getInRedirect(int cur, std::string* name);
  int getOutRedirect(int cur, std::string* name, bool append);
  
  bool getRedirectDescriptors(int& fdIn, int& fdOut, int& fdErr);

  int _pid;
};

#endif
