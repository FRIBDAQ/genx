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
#include <math.h>


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
    f << "\n/** Data Structure definitions **/\n\n";
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
    f << "\n/** Actual instances that your unpacker fills in **/\n\n";
    f << "#ifndef IMPLEMENTATION_MODULE\n";
    for (std::list<Instance>::const_iterator p = instances.begin();
         p != instances.end(); p++) {
        writeExternDecl(f, *p);
    }
    f << std::endl;
    f << "#endif";
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
    // There are three entry points that are provided.  Some targets
    // may use some others others.  These are:
    //   Initialize - one-time initialization (e.g. do it in CreateAnalysisPipeline)
    //   SetupEvent - Any setup required prior to event processing (do in unpack code).
    //   CommitEvent - Any actions required after unpacking (e.g. root tree fills) do
    //                  in the unpack code as well.
    //  Here we just provide prototypes for these:
    
    f << "\n/** API functions callable by the user **/\n\n";
    f << "void Initialize();\n";
    f << "void SetupEvent();\n";
    f << "void CommitEvent();\n";
    f << "\n";
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
    
    f << "\nnamespace " << nsname  << "  {\n\n";
    
    writeTypeDefs(f, types);
    writeExterns(f, instances);
    writeApi(f);
    
    f << "}\n";
   
   
    f << "#endif\n";                         // Close the include guard
    
    // Close the output file before returning.
    
    f.close();
}
/**
 * emitStructArrayInitialization
 *    Emit the code needed to initialize an array of structs.
 *
 *  @param f - output stream to which code is emitted.
 *  @param i - instance that must be a structarray.
 *
 */
static void
emitStructArrayInitialization(std::ostream& f, const Instance& i)
{
    // Figure out how many digits of 'index' we need:
    
    int digits = (log10(i.s_elementCount) + 1);
    
    // Initialize in a for loop.
    
    f << "   for (int i = 0; i < " <<  i.s_elementCount << "; i++) {\n";
    f << "      char index[" << digits + 1 << "];\n";
    f << "      sprintf(index, \"%0" << digits << "d\", i);\n";
    f << "      std::string elname = name + \".\" +  \"" << i.s_name << ".\" + index;\n";
    f << "      " << i.s_name << "[i].Initialize(elname.c_str());\n";
    f << "   }\n";
}
/**
 * emitFieldInitialization
 *    Emits the initialization calls required for each field.
 *
 *  @param f - output stream to which the code is emitted.
 *  @param i - references the field description being initialized.
 *  @param name -base name for initialization.
 *
 *  values - get initialized using CTreeParameter::Initialize(name, channels, low, high, units)
 *  arrays -get initialized using CTreeParameterArray::Initialize(name, low, high, units, elements, 0);
 *  structs - just call the struct's initializer.
 *  structarrays - call the struct's initializer once for each element.
 *
 *  @note the function we're generating is parameterized by 'basename' which
 *        is used to derive the names of things being initialized.
 */
static void
emitFieldInitialization(std::ostream& f, const Instance& i)
{
    switch (i.s_type)
    {
    case value:
        f << "   " << i.s_name << ".Initialize(name + '.' + \"" << i.s_name << "\""
          << ", " << i.s_options.s_bins
          << ", " << i.s_options.s_low
          << ", " << i.s_options.s_high
          << ", \"" << i.s_options.s_units << "\");\n";
        break;
    case array:
        f << "   " << i.s_name <<".Initialize(name + '.' + \"" << i.s_name << "\""
        << ", " << i.s_options.s_low
        << ", " << i.s_options.s_high
        << ", " << i.s_options.s_bins
        << ", \"" << i.s_options.s_units << "\""
        << ", "  << i.s_elementCount
        << ", 0);\n";
        break;
    case structure:
        f << "   " << i.s_name << ".Initialize((name + '.' + \"" << i.s_name << "\").c_str()"
        << ");\n";
        break;
    case structarray:
        emitStructArrayInitialization(f, i);
        break;
    default:
        std::cerr << "*BUG field initialization generation - unknown type: " << i.s_type;
        std::cerr << i.toString() << std::endl;
        exit(EXIT_FAILURE);
    }
}
/**
 * emitInitializeMethods
 *     Writes the Initialize method for each data type.
 *     This is done for each derived data type
 *
 * @param f - output stream to which the code is written.
 * @param ns - Namespace in which everything lives.
 * @param types - List of derived types
 */
static void
emitInitializeMethods(
   std::ostream& f, const std::string& ns, const std::list<TypeDefinition>& types
)
{
    for (std::list<TypeDefinition>::const_iterator p = types.begin();
         p != types.end(); p++) {
        
        f << "\n";
        f << "void " << ns << "::" << p->s_typename << "::Initialize(const char* basename)\n";
        f << "{\n";
        f << "   std::string name(basename);\n";
        for (FieldList::const_iterator pf = p->s_fields.begin();
             pf != p->s_fields.end(); pf++) {
            
            emitFieldInitialization(f, *pf);
        }
        
        
        f << "}\n";
    }
}
/*
 *  emitInstance
 *     Emit a single instance variable.
 *
 *  @param f - the stream to which the code is emitted.
 *  @param i - the instance to emit
 *  @param ns - namespace
 */
static void
emitInstance(std::ostream& f, const Instance& i, const std::string& ns)
{
    switch (i.s_type) {
    case value:
        f << "CTreeParameter ";
        break;
    case array:
        f << "CTreeParameterArray ";
        break;
    case structure:
        f << "struct " << i.s_typename << " ";
        break;
    case structarray:
        f << "struct " << i.s_typename << " "  << i.s_name << "[" << i.s_elementCount << "];\n";
        return;                              // can't fall throgh.
    default:
        std::cerr << "**BUG - unrecognized instance type: " << i.s_type <<std::endl;
        std::cerr << i.toString() <<std::endl;
        exit(EXIT_FAILURE);
    }
    f  << i.s_name << ";\n";
}
/**
 * emitInstances
 *   Emit the instance variables.  These are declared extern in the header.
 *
 * @param f - the stream to which the code is emitted.
 * @param instances - list of instances.
 * @param ns    - namespace.
 */
static void
emitInstances(std::ostream& f, const std::list<Instance>& instances, const std::string& ns)
{
    for (std::list<Instance>::const_iterator p = instances.begin();
         p != instances.end(); p++) {
       emitInstance(f, *p, ns); 
    }
}
/**
 * initStructArrayInstance
 *    Generate a loop to initialize a structure array.
 *    This needs to loop over all elements and initialize with stuff like
 *    name.nnn
 *
 *  @param f - stream to which the code is emitted.
 *  @param i - Instance reference.
 *  @param ns - namespace the instance lives in.
 */
static void
initStructArrayInstance(std::ostream& f, const Instance& i, const std::string& ns)
{
    int digits = log(i.s_elementCount) + 1;  // Number of digits in an index.
    
    f << "   for (int i = 0; i < " << i.s_elementCount << "; i++) \n";
    f << "   {\n";
    f << "        char index[" << digits+1 << "];\n";
    f << "        sprintf(index, \"%0" << digits << "d\", i);\n";
    f << "        std::string elname = " << i.s_name << " + index;\n";
    f << "        " << ns << "::" << i.s_name  << "[i].Initialize(elname.c_str());\n";
    f << "   }\n";
}
/**
 * initInstance
 *    Initializes an instance.  How this is done depends on the instance type.
 *
 * @param f - the stream to which code is written.
 * @param i - the instance being initialized.
 * @param ns - The namespace the instance lives in.
 */
static void
initInstance(std::ostream& f, const Instance& i, const std::string& ns)
{
    switch (i.s_type) {
    case value:
        f << "  " << ns << "::" << i.s_name << ".Initialize("
        << "\"" << i.s_name << "\", "
        << i.s_options.s_bins << ", "
        << i.s_options.s_low  << ", "
        << i.s_options.s_high << ", "
        << "\"" << i.s_options.s_units << "\""
        << ");\n";
        break;
    case array:
        f << "  " << ns << "::" << i.s_name << ".Initialize("
          << "\"" << i.s_name << "\", "
          << i.s_options.s_bins << ", "
          << i.s_options.s_low  << ", "
          << i.s_options.s_high << ", "
          << "\"" << i.s_options.s_units << "\", "
          << i.s_elementCount << ", 0);\n";
        break;
    case structure:
        f << "  " << ns << "::" << i.s_name << ".Initialize("
          << "\"" << i.s_name << "\");\n";
        break;
    case structarray:
        initStructArrayInstance(f, i, ns);
        break;
    default:
        std::cerr << "*BUG unrecognized instance type: " << i.s_type << std::endl;
        std::cerr << i.toString() << std::endl;
        exit(EXIT_FAILURE);
    }
}
/**
 * emitApi
 *    Emits the API functions.  For SpecTcl, the only one that matters
 *    is the Initialize function which needs to initialize all the instance
 *    variables.
 *
 * @param f - stream to which code is emitted.
 * @param instances - Instance list.
 * @param ns        - namespace our functions live in.
 */
static void
emitApi(std::ostream& f, const std::list<Instance>& instances, const std::string& ns)
{
    // First emit the ones that are empty:
    
    f << "void " << ns << "::SetupEvent() {}\n";
    f << "void " << ns << "::CommitEvent() {} \n";
    
    // Initialize has to init each instance
    
    f << "void " << ns << "::Initialize()\n{\n";
    for (std::list<Instance>::const_iterator p = instances.begin(); p != instances.end(); p++) {
        initInstance(f, *p, ns);
    }
    f << "}\n";
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
{
    // Generate the CPP filename and the namespace in which all the
    // functions will exist:
    
    std::string filename = base + ".cpp";
    std::string header  = base + ".h";
    char cstrFilename[base.size() + 1];
    strcpy(cstrFilename, base.c_str());
    std::string nsname   = basename(cstrFilename);
    
    // open the output file:
    
    std::ofstream f(filename.c_str());
    
    // Generate the file:
    
    commentHeader(f, filename, "Actual data declarations and exectuable code");
    f << "#define IMPLEMENTATION_MODULE\n";
    f << "#include \"" << header <<"\"\n";
    f << "#include <stdio.h>\n";
    
    // For each defined data type, we need to writes its Initialize
    // method.
    
    f << "\n/** Instance variables - unpack your stuff int these */ \n\n";
    
    f << "namespace " << nsname << " {\n";
    emitInstances(f, instances, nsname);
    f << "}\n";
    
    f << "\n/** Implementation of initialization methods */\n\n";
    emitInitializeMethods(f, nsname, types);
    
    f << "\n/** Implementation of the API functions */ \n\n";
    emitApi(f, instances, nsname);
    
    f.close();
        
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