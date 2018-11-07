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
#include <libgen.h>
#include <string.h>

const char* programVersionString("specgenerate version 1.0 (c) NSCL/FRIB");

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
 * commentHeader
 *    Create a comment header.
 * @param f - file into which to write the header.
 * @param name - filename .
 * @param desc - File description.
 * @note - programVersionString is an implicit input.
 * 
 */
static void
commentHeader(std::ostream& f, const std::string& name, const std::string& desc)
{
    f << "/**\n";
    f << "*  @file " << name << std::endl;
    f << "*  @brief " << desc << std::endl;
    f << "*\n";
    f << "* This file was created by " << programVersionString << std::endl;
    f << "* Do not edit by hand\n";
    f << "*/\n";
}
/**
 * writeFieldDefinition
 *    Write the definition of a single field of a struct.
 *
 *  @param f - stream to write to.
 *  @param field - the field to write.
 */
static void
writeFieldDefinition(std::ostream& f, const Instance& field)
{
    // what we do depends on the field type
    
    switch (field.s_type) {
    case value:
        f << "   CTreeParameter " << field.s_name << ";\n";
        break;
    case array:
        f << "   CTreeParameterArray " << field.s_name << ";\n";
        break;
    case structure:
        f << "   struct " << field.s_typename << " " << field.s_name << ";\n";
        break;
    case structarray:
        f << "   struct " << field.s_typename << " " << field.s_name << "[" << field.s_elementCount << "];\n";
        break;
    default:
        std::cerr << "**BUG: Invalid field type: " << field.s_type << std::endl;
        std::cerr << field.toString() << std::endl;
        exit(EXIT_FAILURE);
    }
}
/**
 * writeTypeDefinition
 *    Write a definition for a single type.  This is a struct whose fields
 *    are either CTreeParameter, CTreeParamterArrays, derived types or
 *    arrays of derived types:
 *
 *  @param f - stream to write to.
 *  @param t - reference to a type definition.
 */
static void
writeTypeDefinition(std::ostream& f, const TypeDefinition& t)
{
    f << "struct " << t.s_typename << " {\n";
    for (FieldList::const_iterator p = t.s_fields.begin(); p != t.s_fields.end(); p++) {
        writeFieldDefinition(f, *p);
    }
    f << "   void Initialize(const char* basename);\n";
    f << "};\n\n";
}

/** writeTypeDefs
 *   Write the type definitions.  Each type creates a struct definition
 *   The struct definition has a void Initialize(const char* basename);
 *   method as well.
 *
 * @param f - the file stream to write to.
 * @param types - list of type definitions to write.
 */
static void
writeTypeDefs(std::ostream& f, const std::list<TypeDefinition>& types)
{
    for (std::list<TypeDefinition>::const_iterator p = types.begin();
         p != types.end(); p++) {
        
        writeTypeDefinition(f, *p);
    }
}
/**
 * writeExternDecl
 *    Writes an external declaration.
 *
 *  @param f - stream on which to write the data.
 *  @param i - references an instance.
 */
static void
writeExternDecl(std::ostream& f, const Instance& i)
{
    f << "extern ";
    switch (i.s_type) {
    case value:
        f << "CTreeParameter";
        break;
    case array:
        f << "CTreeParameterArray";
        break;
    case structure:
        f << "struct " << i.s_typename;
        break;
    case structarray:
        f << "struct " << i.s_typename << " "
            << i.s_name << "[" << i.s_elementCount << "];\n";
        return;              // since we need to add the index stuff.
    default:
        std::cerr << "*BUG - invalid  instance type  " << i.s_type << std::endl;
        std::cerr << i.toString() << std::endl;
        exit(EXIT_FAILURE);
    }
    f << " " << i.s_name << ";\n";
}
/**
 * writeExterns
 *    Writes external instance declarations for each instance.
 *    values -> CTreeParameter
 *    array -> CTreeParameterArray
 *    structs -> struct
 *    structarray -> array of structs.
 *
 *  @param f - the file stream to write to.
 *  @param instances -list of instanes for which we need to create externs.
 */
static void
writeExterns(std::ostream& f, const std::list<Instance>& instances)
{
    for (std::list<Instance>::const_iterator p = instances.begin();
         p != instances.end(); p++) {
        writeExternDecl(f, *p);
    }
    f << std::endl;
}
/**
 * writeApi
 *    There's an API for this that's analyzer neutral.  We need to
 *    define the prototypes for it:
 *
 * @param f - stream to which to write the prototype definitions.
 */
static void
writeApi(std::ostream& f)
{
    
}
/**
 * generateHeader
 *    Create the header file.  This has struct definitions for
 *    each of the derived types inluding an Initialize method declaration
 *    that takes as a parameter the base name of the tree parameter. Additionally
 *    it declares externs for each instance, and the prototypes for the
 *    API functions.
 *
 * @param base  - the output basename (used to create the namespace.)
 * @param types - References the type list.
 * @param instances - References the instance list.
 */
static void generateHeader(
    const std::string& base, const std::list<TypeDefinition>& types,
    const std::list<Instance>& instances
)
{
    // Generate the filename for the header and the namespace name:
    
    std::string filename = base + ".h";
    char cstrFilename[base.size() + 1];
    strcpy(cstrFilename, base.c_str());
    std::string nsname   = basename(cstrFilename);
    
    // Open the output file:
    
    std::ofstream f(filename.c_str());
    
    //  Throw out the comment header:
    
    commentHeader(f, filename, "Define the types, instances and API");

    f << "#ifndef " << nsname << "_h\n";  // Include guard.
    f << "#define " << nsname << "_h\n";
    f << "#include <TreeParameter.h>\n";   // We're generating tree parameter types.
   
    // Everything we create is inside a namespace: nsname:
    
    f << "\nnamespace nsname {\n\n";
    
    writeTypeDefs(f, types);
    writeExterns(f, instances);
    writeApi(f);
    
    f << "}\n";
   
   
    f << "#endif\n";                         // Close the include guard
    
    // Close the output file before returning.
    
    f.close();
}

/**
 * generateCPP
 *    Generates the .cpp file.  This generates a file containing instances
 *    definitions as well as the implementations of the API functions.
 *
 *  @param base - basename of the output file (the namespace name is derived from this).
 *  @param types - type definitions.
 *  @param instances - instance list.
 */
static void
generateCPP(
    const std::string& base, const std::list<TypeDefinition>& types,
    const std::list<Instance>& instances
)
{}

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
    // Deserialize the type and instance lists from stdin.
    
    std::list<TypeDefinition> types;
    deserializeTypes(std::cin, types);
    
    std::list<Instance> instances;
    deserializeInstances(std::cin, instances);
    
    std::string basename = argv[1];
    
    
    
    
    generateHeader(basename, types, instances);
    generateCPP(basename, types, instances);
    
    
    exit(EXIT_SUCCESS);
}
// Refernced by the stuff we use.
void yyerror(const char* m) {
    usage(std::cerr, m);
}