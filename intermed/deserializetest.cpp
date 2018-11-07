/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  deserializetest.cpp
 *  @brief: Test deserialization from stdin.
 */
#include "instance.h"
#include "definedtypes.h"
#include <iostream>
#include <stdlib.h>

/**
 *  We deserialize from stdin and dump back out in text form to stdout.
 */

int main(int argc, char** argv)
{
    std::list<TypeDefinition> types;
    deserializeTypes(std::cin, types);
    
    std::list<Instance> instances;
    deserializeInstances(std::cin, instances);
    
    // Now dump:
    
    std::cout << "----------------- types ---------------\n";
    for (std::list<TypeDefinition>::const_iterator p = types.begin(); p != types.end(); p++)
    {
        std::cout << "==\n";
        std::cout << p->toString() << std::endl;
    }
    std::cout << "---------------- instances ------------\n";
    for (std::list<Instance>::const_iterator p = instances.begin(); p != instances.end(); p++)
    {
        std::cout << "==\n";
        std::cout << p->toString() << std::endl;
    }
}
// Needed alas:

void yyerror(const char* m)
{
    std::cerr << "***ERROR** " <<  m << std::endl;
    exit(EXIT_FAILURE);
}