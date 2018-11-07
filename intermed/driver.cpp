#include <string>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <list>
#include "instance.h"
#include "definedtypes.h"

#include "datadecl.tab.h"



extern FILE* yyin;
unsigned lineNum(1);


static void dumpTypes()
{
    std::cerr << "Defined data types: \n";
    for (std::list<TypeDefinition>::const_iterator p = typeList.begin();
         p != typeList.end(); p++) {
        
        std::cerr << "-------------------------\n";
        std::cerr << p->toString() << std::endl;
    }
}

static void dumpInstances()
{
    std::cerr << "Instancelist dump: \n";
    for (std::list<Instance>::iterator p = instanceList.begin(); p != instanceList.end(); p++) {
        std::cerr << "-----------------------------\n";
        std::cerr << p->toString();
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
        serializeTypes(std::cout);
        serializeInstances(std::cout);
    }
    exit(exitCode);
}


/**
 *  Bison error reporting hook:
 **/
void yyerror(const char *s)
{
    std::cerr << "*** error: " << lineNum << " : " << s << std::endl;
    exit(EXIT_FAILURE);
}