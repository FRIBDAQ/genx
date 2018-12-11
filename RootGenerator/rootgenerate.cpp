/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  rootgenerate.cpp
 *  @brief: Code generator for the CERN Root.
 */

/**
 * This program generates code to support environment neutral unpacking
 * of event data.  The intermediate representation of type and instance
 * definitions generated by the parser for the data definition language
 * is taken as input on stdin (so we can be pipelined).  A .h and
 * .cpp file are generated.
 *
 * Usage:
 *      rootgenerate basename
 *
 * Which generates basename.h, basename.cpp, and basename-linkdef.h
 * basename.h, basename.cpp are sufficient for the unpacking code
 * basename-linkdef.h provides a file that can be used to generate a
 * root dictionary for the classes/structs we generate.
 */

#include <iostream>
#include <instance.h>
#include <definedtypes.h>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <math.h>

const char* programVersionString("rootgenerate version 1.0 (c) NSCL/FRIB");

/**
 * usage
 *    Outputs an error message and program usage text to the desired
 *    stream.
 *
 * @param f - the stream to which output is directed.
 * @param msg - the message that precedes the usage text.
 */
static void
usage(std::ostream& f, const char * msg)
{
    f << msg << std::endl;
    f << "Usage\n";
    f << "   rootgenerate basename\n";
    f << "Where:\n";
    f << "   basename is the base name for the generated files.  The files\n";
    f << "            created are basename.h, basename.cpp and basename-linkdef.h\n";
    f << "The program expects the intermediate representation to be on stdin\n";
    
    exit(EXIT_FAILURE);
}
/**
 * commentHeader
 *    Generate a comment header for a file.
 * @param f - the file into which the header is generated.
 * @param filename -name of the file.
 * @param descrip - brief description
 */
static void commentHeader(std::ostream& f, const std::string& filename,  const char* descrip)
{
    f << "/**\n";
    f << "*  @file  " << filename << std::endl;
    f << "*  @brief " << descrip  << std::endl;
    f << "*\n";
    f << "*   This file was generated by " << programVersionString << std::endl;
    f << "*   Do NOT edit by hand\n";
    f << "*/\n";
}

/**
 * writeClassHeader
 *    Write the invariant part of a class definition. This is the class,
 *    and canonical method definitions.
 * @param f -- stream to which the definition is written.
 * @param name - name of the class
 */
static void
writeClassHeader(std::ostream& f, const std::string& name)
{
    f << "class " << name << " : public TObject {\n";
    f << "public:\n";
    f << "   " << name << "();\n";            // Default constructor.
    f << "    ~" << name << "();\n";          // default destructor.
    f << "   " << name  << "(const " << name << "&);\n";  // Copy constructor
    f << "   " << name  << "& operator=(const " << name << "& rhs);\n";  // assignment
    f << "   void Reset(); \n";               // Reset elements to NaN
    
    f << std::endl;
}
/**
 * writeClassMembers
 *    Writes the class members from the field list.
 *    There are four types of members:
 *    *  value - these are just Double_t.
 *    *  array - these are arrays of Double_t.
 *    *  structure - these are just items of that type (structures map to classes).
 *    *  structarray - these are arrays of the above.
 *
 *  @param f - stream to which code is emitted.
 *  @param flist - list of field definitions.
 *  @note in root we ignore the ValueOptions as they have no meaning.
 */
static void writeClassMembers(std::ostream& f, const FieldList& flist)
{
    for (FieldList::const_iterator p = flist.begin(); p != flist.end(); p++) {
        //  Let's try to be generic:
        
        std::string fieldName = p->s_name;
        std::string fieldType = "Double_t";           // Default to primitive type.
        unsigned    n         = 1;                    // Default to scalar:
        
        if ((p->s_type == structure) || (p->s_type == structarray)) {
            fieldType = p->s_typename;
        }
        if ((p->s_type == array) || (p->s_type == structarray)) {
            n = p->s_elementCount;
        }
        
        f << "   " << fieldType << " " << fieldName;
        if (n > 1) {
            f << "[" << n << "]";
        }
        
        f << ";\n";
    }
    f << "\n";
}

/**
 * writeClassTrailer
 *    Writes the  ClassDef directive and closes the class definition:
 *
 *  @param f - stream to which code is emitted.
 *  @param name - name of the class.
 */
static void
writeClassTrailer(std::ostream& f, const std::string& name)
{
    f << "  ClassDef(" << name << ", 1)\n";    // No semicolon allowed !!!
    f << "};\n\n";
}

/**
 * writeStrutureDefs
 *    Write the struture definitions.
 * @param f  - stream to which the code is written.
 * @param types - List of type definitions to write.
 */
static void
writeStructureDefs(std::ostream& f, const std::list<TypeDefinition>& types)
{
    // For each struct we create a class that inherits from TOBject.
    // To be serializable the class has to have a default constructor,
    // destructor, copy constructor, assignment.  These will be implemented
    // in the CPP file not here.
    // we also need to close with a ClassDef directive.
    
    for (std::list<TypeDefinition>::const_iterator p = types.begin();
          p != types.end(); p++) {
        writeClassHeader(f, p->s_typename);
        writeClassMembers(f, p->s_fields);
        writeClassTrailer(f, p->s_typename);
    }
}
/**
 * writeInstanceDefs
 *    Write definitions of the instances declared by the user;
 *
 * @param f - stream to which the code is generated.
 * @param instances - list of instances to generate.
 * 
 */
static void
writeInstanceDefs(std::ostream& f, const std::list<Instance>& instances)
{
    // C++11 doesn't like a variable to be declared both  extern and defined
    // in the file so we conditionalize all of this on IMPLEMENTATION_MODULE
    // not being defined:
    
    f << "#ifndef IMPLEMENTATION_MODULE\n\n";
    
    //  Note to better support marshalling this struct in and out of
    //  MPI messages, we put all instances in a struct called
    //  instanceStruct and then make individual references to the
    //  struct elements.
    
    // Build the struct of instances:
    
    f << " extern struct { \n";
    for (auto p =instances.begin(); p != instances.end(); p++) {
        std::string fieldName = p->s_name;
        std::string fieldType = "   Double_t";           // Default to primitive type.
        unsigned    n         = 1;                    // Default to scalar:
        
        if ((p->s_type == structure) || (p->s_type == structarray)) {
            fieldType = p->s_typename;
        }
        if ((p->s_type == array) || (p->s_type == structarray)) {
            n = p->s_elementCount;
        }
        
        f << "   " << fieldType << " " << fieldName;
        if (n > 1) {
            f << "[" << n << "]";
        }
        
        f << ";\n"; 
    }
    f << "}  instanceStruct;\n";
    
    // Now generate external references to the instanceStruct elements.
    // note that a reference to an array is
    //    datatype (&instancName)[array_size];
    
    for (std::list<Instance>::const_iterator p = instances.begin();
         p != instances.end(); p++) {

        // We should be able to use the strategy used to generate fields here too:
        // TODO:  factor this out wrt writeClassMembers:
        f << "extern ";
        std::string fieldName = p->s_name;
        std::string fieldType = "Double_t";           // Default to primitive type.
        unsigned    n         = 1;                    // Default to scalar:
        
        if ((p->s_type == structure) || (p->s_type == structarray)) {
            fieldType = p->s_typename;
        }
        if ((p->s_type == array) || (p->s_type == structarray)) {
            n = p->s_elementCount;
        }
        
        f << "   " << fieldType << " (&" << fieldName << ")";
        if (n > 1) {
            f << "[" << n << "]";
        }
        
        f << ";\n";        
    }
    
    f << "\n#endif\n\n";
}
/**
 * writeApiPrototypes
 *    Writes the prototypes for the API functions.
 *
 *  @param f - stream to which the prototypes are written
 */
static
void writeApiPrototypes(std::ostream& f)
{
    f <<  "void Initialize();\n";
    f <<  "void SetupEvent();\n";
    f <<  "void CommitEvent();\n";
    
}
/**
 * generateHeader
 *    Generate the header file.
 *
 *  @param fname - name of the output file.
 *  @param nsname - name of the namespace all the decls go into.
 *  @param types  - list of data types.
 *  @param instances - list of top level instances.
 */
static void
generateHeader(
    const std::string& fname, const std::string& nsname,
    const std::list<TypeDefinition>& types, const std::list<Instance>& instances
)
{
    std::string headerName = fname+".h";
    std::ofstream f(headerName.c_str());
    commentHeader(f, headerName, "Defines types, instances and API");
    char cstrName[fname.size() +1];
    strcpy(cstrName, fname.c_str());
    std::string baseFilename = basename(cstrName);
    
    f << "#ifndef " << baseFilename << "_h" <<  std::endl;
    f << "#define " << baseFilename << "_h" <<  std::endl;
    f << "#include <TObject.h>\n\n";
    
    // All of the file lives in the namespace:
    
    f << "namespace " << nsname << " {\n\n";
    
    writeStructureDefs(f, types);
    writeInstanceDefs(f, instances);
    writeApiPrototypes(f);
    
    f << "}\n";
    f << "#endif\n";
    f.close();
}

/**
 * generateLinkDef
 *    Generate a LinkDef file that specifies link C++ lines for each of our
 *    derived types (classes).
 *
 * @param fname - name of the file in which to do this.
 * @param ns    - Namespace name in which we've generated out classes.
 * @param types - Type list that has our class information.
 */
static void
generateLinkDef(
    const std::string& fname, const std::string& nsname,
    const std::list<TypeDefinition>& types
)
{
    std::ofstream f(fname.c_str());
    commentHeader(f, fname, "Linkdef file for dictionaries");
    f << "#ifdef __CINT__\n\n";
    f << "#pragma link off all globals;\n";
    f << "#pragma link off all classes;\n";
    f << "#pragma link off all functions;\n\n";
    
    for (std::list<TypeDefinition>::const_iterator p = types.begin();
         p != types.end(); p++) {
        
        f << "#pragma link C++ class " << nsname << "::" << p->s_typename << "+;\n";
    }
    
    f << "\n#endif\n";
    f.close();
}
/**
 * generateResetImplementation
 *     Generates the Reset method of a class.
 *     -  For scaler values just set to NaN
 *     -  for arrays set each element to NaN
 *     -  for structures  call Reset
 *     -  for structure arrays call Reset on each element of the array.
 *
 * @param f      - the stream into which code is generated.
 * @param nsname - name of the namespace in which the classes are defined.
 * @param type   - Type definition.
 */
static void
generateResetImplementation(
    std::ostream& f, const std::string& nsname, const TypeDefinition& type
)
{
    f << "void " << nsname << "::" << type.s_typename << "::Reset() {\n";
    
    for (FieldList::const_iterator p = type.s_fields.begin(); p != type.s_fields.end(); p++) {
        std::string rhs;
        switch (p->s_type) {
        case value:                
            f << "   " << p->s_name << "= NAN;\n";
            break;
        case structure:
            f << "   " << p->s_name <<".Reset();\n";
            break;
        // Arrays/structarrays need to generate loops.
        
        case array:
            rhs = " = NAN";
            break;
        case structarray:
            rhs = ".Reset()";
            break;
        default:
            std::cerr << "Unrecognized data type: " << p->s_type << std::endl;
            std::cerr << p->toString() <<std::endl;
            exit(EXIT_FAILURE);
        }
        // if this is an array we an generate loops in common code using rhs
        // after the common field name [i] crap:
        
        if ((p->s_type == array) || (p->s_type == structarray)) {
            
            f << "   for (int i = 0; i < " << p->s_elementCount << "; i++) {\n";
            f << "       " << p->s_name << "[i]" << rhs << ";\n";
            f << "   } \n";
        }
    }
    f << "}\n\n";
}
/**
 * implementClass
 *    Implements the method of a class.
 *
 *  @param f - The stream to which the implementation code is written.
 *  @param nsname - namespace the class is defined in
 *  @param type   - References the type definition of the class.
 */
static void
implementClass(std::ostream& f, const std::string& nsname, const TypeDefinition& type)
{
    f << "// Implementation of methods for class: "
      << nsname << "::" << type.s_typename << std::endl << std::endl;
    
    f << "ClassImp(" << nsname << "::" << type.s_typename << ");\n\n";
    // Constructor invokes Reset:
    
    f << nsname << "::" << type.s_typename << "::" << type.s_typename << "() {\n";
    f << "   Reset();\n";
    f << "}\n\n";
    
    // Destructor is empty.. but required by root (I think).
    
    f << nsname << "::" << type.s_typename << "::~" << type.s_typename << "() {}\n\n";
    
    // Copy construction is implemented in terms of assignment.
    
    f << nsname << "::" << type.s_typename << "::" << type.s_typename << "(const "
      << nsname << "::" << type.s_typename << "& rhs) {\n";     
    f << "   *this = rhs;\n";
    f << "}\n\n";
    
    // Assignment... assigns all members from rhs.  returns *this
    // * No self-assignment protection needed.
    // * Relies on substructs to have implemented operator= too.
    
    f << nsname << "::" << type.s_typename << "& "
      << nsname << "::" << type.s_typename << "::operator=(const "
      << nsname << "::" << type.s_typename << "& rhs) {\n";
    
    for (FieldList::const_iterator p = type.s_fields.begin(); p != type.s_fields.end(); p++) {
        if ((p->s_type == value) || (p->s_type == structure)) {    // scalar:
            f << "   " << p->s_name << " = rhs." << p->s_name <<";\n";
        } else {                                                   // array gen forloop.
            f << "   for(int i = 0; i < " << p->s_elementCount << "; i++) { \n";
            f << "       " << p->s_name << "[i] = rhs." << p->s_name << "[i];\n";
            f << "   }\n";
        }
    }
    f << "   return *this;\n";  
    f << "}\n\n";
    
    // Reset - is a bit more complex and therefore spun off to another function:
    
    generateResetImplementation(f, nsname, type);
}

/**
 * generateClassImplementations
 *    Generates the implementations of each class defined by the user.
 *
 *  @param f       - stream into which the code is generated.
 *  @param nsname  - Name of the namespace the classes were generated in.
 *  @param types   - list of types defined by the user.
*/
static void
generateClassImplementations(
    std::ostream& f, const std::string& nsname,
    const std::list<TypeDefinition>& types
)
{
    f << "// Class method implementations: \n\n";
    
    for(std::list<TypeDefinition>::const_iterator p = types.begin();
        p != types.end(); p++) {
        implementClass(f, nsname, *p);
    }
}
/**
 *  GenerateInstances.
 *  - values/arrays are Double_t
 *  - structures and structarrays are just their type.
 *
 * @param f      - stream to which code is written.
 * @param nsname - namespace in which everything is defined.
 * @param instances- instance list.
 */
static void
generateInstances(
    std::ostream& f, const std::string& nsname, const std::list<Instance>& instances
)
{
    f << "//   Instance definitions\n\n";
    f << "namespace " << nsname << " {\n";
    
    // What we write here is a struct of instances named 'instanceStruct'.
    // then we write and initialize references to each element of that struct.
    //
    f << "struct { \n";
 for (auto p =instances.begin(); p != instances.end(); p++) {
        std::string fieldName = p->s_name;
        std::string fieldType = "   Double_t";           // Default to primitive type.
        unsigned    n         = 1;                    // Default to scalar:
        
        if ((p->s_type == structure) || (p->s_type == structarray)) {
            fieldType = p->s_typename;
        }
        if ((p->s_type == array) || (p->s_type == structarray)) {
            n = p->s_elementCount;
        }
        
        f << "   " << fieldType << " " << fieldName;
        if (n > 1) {
            f << "[" << n << "]";
        }
        
        f << ";\n"; 
    }
    f << "}  instanceStruct;\n";
       
    
    for (std::list<Instance>::const_iterator p = instances.begin();
         p != instances.end(); p++) {
        
        // Figure out the actual type to use:
        
        std::string typeName = "Double_t";        // Value and array:
        if ((p->s_type == structure) || (p->s_type == structarray)) {
            typeName =  p->s_typename;
        }
        // Do we need [size]?
        
        std::stringstream suffix;
        if ((p->s_type == array) || (p->s_type == structarray)) {
            suffix << "[" << p->s_elementCount << "]";
        }
        f << typeName << " (&"  << p->s_name <<") " << suffix.str()
            << "(instanceStruct." << p->s_name <<")"
            << ";\n";
        
    }
    f << "}\n";
}
/**
 * generateClearInstances
 *    Sets the entire tree to NAN:
 *    values - get set to NAN
 *    structure instances get Reset
 *    arrays - iterated over and set to NAN
 *    struct arrays iterated over and Reset.
 *
 * @param f   - Stream to which code is emitted.
 * @param nsname - namespace  in which all of these are defined.
 * @param instances - list of instances.
 */
static void
generateClearInstances(
    std::ostream& f, const std::string& nsname,
    const std::list<Instance>& instances
)
{
    for (std::list<Instance>::const_iterator p = instances.begin();
         p != instances.end(); p++) {
        
        std::string suffix = "= NAN";           // For non structs.
        if ((p->s_type == structure) || (p->s_type == structarray)) {
            suffix = ".Reset()";
        }
        // If scaler just a single statement otherwise generate a loop:
        
        if ((p->s_type == value) || (p->s_type == structure)) {
            f << "   " << nsname << "::" << p->s_name << suffix << ";\n";
        } else {
            f << "   " << "for (int i = 0; i < " << p->s_elementCount << "; i++) {\n";
            f << "      " << nsname << "::" << p->s_name << "[i]" << suffix << ";\n";
            f << "   }\n";
        }
    }
}
/**
 * createBranchStructArray
 *     Creates the branches associated with an array of structs.
 *     We're going to make one branch per array element.  The namess of the
 *     branches will be instancename_nnn where nnn is the element number and
 *     has sufficient digits to maintain lexicographic sorting by indes number.
 *
 * @param f     - stream into which the code is emitted.
 * @param nsname - Name of the namespace containing objects and classes.
 * @param inst   - Instance for which we're making branches.
 */
static void
createBranchStructArray(
    std::ostream& f, const std::string& nsname, const Instance& inst
)
{
    int digits = log10(inst.s_elementCount) + 1;         // # digits in the index.
    f << "   for (int i = 0; i < " << inst.s_elementCount << "; i++) { \n";
    f << "       char index[" << digits+2 <<"];\n";
    f << "       sprintf(index, \"_%0" << digits << "d\", i);\n";  // Create the index part of the name.
    f << "       std::string branchName = std::string(\"" << inst.s_name << "\") +  index;\n";
    f << "       " << nsname << "::pTheTree->Branch(branchName.c_str(), \""
                 << nsname << "::" << inst.s_typename << "\", &"
                 << nsname << "::instanceStruct." << inst.s_name << "[i]);\n";
    f << "   }\n";
}
/**
 * createTree
 *    create the tree and its branches - one per instance
 *
 * @param f    - Stream into which the code is generated.
 * @param nsname - namespace in which everything was defined.
 * @param instances - list of instance descriptions.
 */
static void
createTree(
    std::ostream& f, const std::string& nsname,
    const std::list<Instance>& instances
)
{
    // Create the tree:
    
    f << "   " << nsname << "::pTheTree = new TTree(\""
        << nsname << "\", \"" << nsname << "\");\n";
    
    // A branch for each instance with the instance as the data pointer.
    
    for (std::list<Instance>::const_iterator p = instances.begin();
         p != instances.end(); p++) {
        switch (p->s_type) {
        case value:
            f << "   " << nsname << "::pTheTree->Branch(\"" << p->s_name << "\", &"
                << nsname << "::instanceStruct." << p->s_name << ", \""
                << p->s_name << "/D\");\n";
            break;
        case array:
            f << "   " << nsname << "::pTheTree->Branch(\"" << p->s_name << "\", "
                << nsname << "::instanceStruct." << p->s_name << ", \""
                << p->s_name << "[" << p->s_elementCount << "]/D\");\n";
            break;
        
        case structure:
            f << "   " << nsname << "::pTheTree->Branch(\"" << p->s_name << "\", \""
              << nsname << "::" << p->s_typename << "\", &"
              << nsname << "::instanceStruct." << p->s_name << ");\n";
            break;
        case structarray:          
            createBranchStructArray(f, nsname, *p);        // 'Array' of branches of structs.
            break;
        }
    }
}

/**
 * generateAPI
 *    Generates API  implementations for Initialize, SetupEvent and CommitEvent.
 *
 *  @param f  - file into which code is being generated.
 *  @param nsname - namespace all this stuff lives in.
 *  @param instances - instance list.
 */
static void
generateAPI(
    std::ostream& f, const std::string& nsname,
    const std::list<Instance>& instances
)
{
    f << "// Pointer to the tree:\n\n";
    
    f << "namespace " << nsname << " {\n";
    f << "TTree* " << "pTheTree(0);\n\n";
    f << "}\n";
    
    f << "// Setup event - resets the instances\n\n";
    f << "void " << nsname << "::SetupEvent() {\n";
    generateClearInstances(f, nsname, instances);
    f << "}\n\n";
    
    f << "// CommitEvent  Fills the tree\n\n";
    f << "void " << nsname << "::CommitEvent() {\n";
    f << "   pTheTree->Fill();\n";
    f << "}\n\n";
    
    f << "// Initialize - creates the trees and branches\n\n";
    f << "void " <<nsname << "::Initialize() {\n";
    createTree(f, nsname, instances);
    f << "}\n\n";
}
/**
 * generateCPP
 *    Generate the C++ file.
 * @param fname - name of the file to be generated.
 * @param headerName -name of the header file.
 * @param nsname - namespace all of the definitions live in.
 * @param types  - Derived type definitions.
 * @param instance - The instance definitions.
 */
static void
generateCPP(
    const std::string& fname, const std::string& headerName,
    const std::string & nsname,
    const std::list<TypeDefinition>& types, const std::list<Instance>& instances
)
{
    
    char cstrHeaderName[headerName.size()+1];
    strcpy(cstrHeaderName, headerName.c_str());
    std::string headerBaseName = basename(cstrHeaderName);
    
    std::ofstream f(fname.c_str());
    commentHeader(f, fname, "C++ Implementation file for root");
    f << "#define IMPLEMENTATION_MODULE\n";
    f << "#include \"" << headerBaseName << "\"\n\n";
    f << "#include <cmath>\n";
    f << "#include <TTree.h>\n";
    f << "#include <TBranch.h>\n";
    
    f << std::endl;
    
    
    generateClassImplementations(f, nsname, types);
    generateInstances(f, nsname, instances);
    generateAPI(f, nsname, instances);
    
    f.close();
}
/**
 * main
 *   entry point
 */
int main (int argc, char** argv)
{
    if (argc != 2) {
        usage(std::cerr, "Incorrect number of command line parameters");
    }
    // Deserialize the intermediate representation:
    
    std::list<TypeDefinition> types;
    deserializeTypes(std::cin, types);
    
    std::list<Instance> instances;
    deserializeInstances(std::cin, instances);
    
    // From the base name generate the names of the namespace, header, source
    // and linkdef file.  Note that for the namespace, we use basename to
    // remove any path information from basename as the user could do:
    //
    // rootgenerate ~/rootstuff/base
    //
    // which, by the time we're done gives us a namespace of base.
    
    
    std::string base   = argv[1];
    std::string headerName = base + ".h";
    std::string cppName    = base + ".cpp";
    std::string linkdefName = base + "-linkdef.h";
    
    
    if (nsName == "") {
        char cstrFilename[base.size() +1];  // all because basename(3) an
        strcpy(cstrFilename, base.c_str()); // modify its parameter.   
        nsName = basename(cstrFilename);        
    }
    
    std::string nsname  = nsName;
    
    
    //  Here we go:
    
    generateHeader(base, nsname, types, instances);
    generateLinkDef(linkdefName, nsname, types);
    generateCPP(cppName, headerName, nsname, types, instances);
    
}

void yyerror(const char* msg)
{
    usage(std::cerr, msg);
}