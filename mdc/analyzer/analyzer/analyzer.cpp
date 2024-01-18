#include "manalyzer.h"

#include <iostream>
#include <boost/program_options.hpp>

#include "AnaFillHits.h"
#include "AnaPixelHistos.h"
#include "cntEvents.h"

using namespace std;

int main(int argc, char* argv[]) {
  std::cout << "======================================================================" << std::endl;
  std::cout << "proCalRec analyzer" << std::endl;
  std::cout << "======================================================================" << std::endl;

  // -- command line arguments
  std::vector<std::string> manalyzer_commands, analyzer_commands;
  std::vector<std::string> args(argv + 1, argv + argc);

  bool count(false);
  //  for (int i = 0; i < argc; i++){
  for (auto i = args.begin(); i != args.end();) {
    if (string::npos != i->find("-c")) {
      count = true;
      ++i; 
      continue;
    }
    manalyzer_commands.push_back(std::string(*i)); ++i;
  }
  
 
  printf("manalyzer cmds:\n");
  for (auto& x : manalyzer_commands) std::cout << x << std::endl;
  printf("analyzer cmds:\n");
  for (auto& x : analyzer_commands) std::cout << x << std::endl;

  if (count) {
    static TARegister tar1(new TAFactoryTemplate<AnaFillHits>);
    static TARegister tar0(new cntEventsFactory);
  } else {
    static TARegister tar1(new TAFactoryTemplate<AnaFillHits>);
    static TARegister tar2(new AnaPixelHistoFactory);
  }
  std::vector<char*> cstrings;
  cstrings.push_back(const_cast<char*>(" "));
  for(size_t i = 0; i < manalyzer_commands.size(); ++i) cstrings.push_back(const_cast<char*>(manalyzer_commands[i].c_str()));

  return manalyzer_main(cstrings.size(), cstrings.data());
}

