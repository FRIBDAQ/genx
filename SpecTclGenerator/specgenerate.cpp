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

/** @file:  specgenerate.cpp
 *  @brief: Generate SpecTcl tree parameters etc. from intermediate representation.
 */

#include <instance.h>
#include <definedtypes.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>


/**
 * usage:
 *     Outputs an error message and program usage then exits in error.
 * @param f - stream to which stuff are output.
 * @param msg - message to output.
 */
static void usage(std::ostream& f , const char* msg)
{
    f << msg << std::endl;
    f << "Usage\n";
    f << "    specgenerate basname\n";
    f << "Where:\n";
    f << "  basename is the base name of the generated files.  Two files\n";
    f << "  are created a header (basename.h) and code file (basename.cpp)\n";
    exit(EXIT_FAILURE);
}

/**
 *  main
 *     Entry point
 *     Deserialize the intermediate representation of the parse
 *     generate the header and implementation files.
 *
 *   Usage:
 *       specgenerate outputbase
 *
 *   Files generated will be outputbase.h and outputbase.cpp
 */
int main (int argc, char** argv)
{
    if (argc != 2) {
        usage(std::cerr, "Incorrect number of command line parameters");
    }
    
}
// Refernced by the stuff we use.
void yyerror(const char* m) {
    usage(std::cerr, m);
}