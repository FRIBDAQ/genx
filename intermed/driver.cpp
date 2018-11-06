#include <string>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <list>
#include "instance.h"
#include "datadecl.tab.h"



extern FILE* yyin;

static void dumpInstances()
{
    std::cout << "Instancelist dump: \n";
    for (std::list<Instance>::iterator p = instanceList.begin(); p != instanceList.end(); p++) {
        std::cout << "-----------------------------\n";
        std::cout << p->toString();
    }
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage:\n" << std::endl;;
        std::cerr << "   driver declaration-file\n" << std::endl;;
        exit(EXIT_FAILURE);
    }
    
    FILE* declarations = fopen(argv[1], "r");
    if (!declarations) {
        std::cerr << "Failed to open data declaration file: " << argv[0] << std::endl;
        exit(EXIT_FAILURE);
    }
    
    yyin = declarations;              // Set the FLEX input stream:
    int result = yyparse();
    int exitCode = result ? EXIT_FAILURE : EXIT_SUCCESS;
    if (!result) {
        dumpInstances();
    }
    exit(exitCode);
}


/**
 *  Bison error reporting hook:
 **/
void yyerror(const char *s)
{
    std::cerr << "*** error: " << s << std::endl;
    exit(EXIT_FAILURE);
}