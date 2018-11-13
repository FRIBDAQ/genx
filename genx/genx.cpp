#include "genxparams.h"
#include <stdlib.h>
#include <iostream>
#include <string>

#ifndef PREFIX
#error "Must be compiled with -DPREFIX for installation directory"
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
/**
 * usage:
 *    Output error message, program help and exit
 * @param out - output stream.
 * @param msg - message to output.
 */
static void
usage(std::ostream& out, const char* msg)
{
    out << msg << std::endl;
    out.flush();
    cmdline_parser_print_help();
    exit(EXIT_FAILURE);
}

/**
 * main
 *    Entry point.
 *    -   Need an input file, a basename for the output file and
 *    -   a --target option value.
 */
int main(int argc, char** argv)
{
    gengetopt_args_info parsedArgs;
    cmdline_parser(argc, argv, &parsedArgs);
    
    // ensure we have the right number of 'inputs'
    
    if (parsedArgs.inputs_num != 2) {
        usage(
            std::cerr,
            "Incorrect number of non-option parameters.  Need an input file and output basename");
    }
    // Figure out where all the skeletons are buried:
    
    std::string bindir = TOSTRING(PREFIX);
    bindir += "/bin";
    
    std::cout << "Bin dir is " << bindir << std::endl;
    
    // construct the front end command:
    
    std::string parserCmd = bindir;
    parserCmd += "/";
    parserCmd += "parser";
    
    // Back end command depends on the target:
    
    std::string backend = bindir +"/";
    if (parsedArgs.target_arg == target_arg_spectcl) {
        backend += "specgenerate";
    } else {
        backend += "rootgenerate";
    }
    // Let's generate the command pipeline for the system(3) call:
    
    std::string command = parserCmd;
    command += " ";
    command += parsedArgs.inputs[0];      // First file is the .decl fil.
    command + " | " ;                     // Piped to:
    command += backend + " ";             // The selected backend.
    command += parsedArgs.inputs[1];      // second command is the basename.
    
    system(command.c_str());
}