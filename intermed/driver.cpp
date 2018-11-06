#include "datadecl.tab.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
extern FILE* yyin;
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