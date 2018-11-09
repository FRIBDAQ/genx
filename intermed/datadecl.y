%{
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include "instance.h"
#include "definedtypes.h"

extern int yylex();
extern int yyparse();
extern FILE* yyin;
void yyerror(const char* s);
void yywarning(const char* s);

static int checkIndex(double);
static void verifyTypeField(const char* type, const char* field);
static void verifyTypeInstance(const char* type, const char *instance);
static void warnIfTypeName(const std::string& inst);
%}
%union {
    double number;
    char* str;
}

/* Tokens: */

%token VALUE
%token ARRAY
%token STRUCT
%token STRUCTARRAY
%token STRUCTINSTANCE
%token STRUCTARRAYINSTANCE
%token <str> NAME
%token LOW
%token HIGH
%token BINS
%token UNITS
%token <number> NUMBER
%token LBRACK
%token RBRACK
%token LCURLY
%token RCURLY
%token EQUALS
%token DOUBLE

%%

/** Production rules */

input_file: instances | struct_decls instances
    ;
    
struct_decls: struct_decl | struct_decl struct_decls
    {
    }


struct_decl: struct_name_part struct_member_part
    ;
    
struct_name_part: STRUCT NAME
    {
        newStruct($2);
    }

struct_member_part: LCURLY struct_members RCURLY
    ;
            
struct_members: struct_member | struct_member struct_members
    {
    }
struct_member: value_field | array_field | substruct | substruct_array
    {
    }


value_field: value_fieldname | value_fieldname_with_options
    ;
value_fieldname: VALUE NAME
    {
        Instance newInstance;
        newInstance.s_type =  value;
        newInstance.s_name = $2;
        free($2);
        newInstance.s_typename = "";
        newInstance.s_elementCount = 1;
        addField(newInstance);         
    }

value_fieldname_with_options: value_fieldname valueoptions
    {
        // Current Instance has the value options to put in the most recent
        // field.
        
        setLastFieldOptions(currentInstance.s_options);
        currentInstance.s_options.Reinit();
    }

valueoptions:   valueoption | valueoption valueoptions
    ;
    
valueoption: low_option | high_option | bins_option | units_option
    ;
    
low_option: LOW EQUALS NUMBER
    {
        currentInstance.s_options.s_low = $3;
    }

high_option: HIGH EQUALS NUMBER
    {
        currentInstance.s_options.s_high  = $3;
    }
    
bins_option: BINS EQUALS NUMBER
    {
        currentInstance.s_options.s_bins = $3;
    }
    
units_option: UNITS EQUALS NAME
    {
        currentInstance.s_options.s_units = $3;
        free($3);                    // Malloced by strdup.
    }

array_field: simple_array_field | array_field_with_options
    ;

simple_array_field: ARRAY NAME LBRACK NUMBER RBRACK
    {
        int count = checkIndex($4);
        Instance newField;
        newField.s_type = array;
        newField.s_name = $2;
        free($2);
        newField.s_typename = "";
        newField.s_elementCount = count;
        addField(newField);
    }
    
array_field_with_options: simple_array_field valueoptions    
    {
        // Current Instance has the value options to put in the most recent
        // field.
        
        setLastFieldOptions(currentInstance.s_options);
        currentInstance.s_options.Reinit();
    }
    
substruct: STRUCT NAME NAME
    {
        // It's an error not to have the name of the substruct defined
        // yet:
        
        verifyTypeField($2, $3);
        Instance newField;
        newField.s_type = structure;
        newField.s_name = $3;
        newField.s_typename = $2;
        newField.s_elementCount = 1;
        addField(newField);
    }

substruct_array: STRUCTARRAY NAME NAME LBRACK NUMBER RBRACK
    {
        verifyTypeField($2, $3);
        int count = checkIndex($5);
        Instance newInstance;
        newInstance.s_type = structarray;
        newInstance.s_name = $3;
        newInstance.s_typename = $2;
        newInstance.s_elementCount = count;
        addField(newInstance);
    }

instances: instance | instance instances
    ;
    
    
instance: val_instance | array_instance | struct_instance | structarray_instance
    {
    }
    
val_instance: simple_value | optioned_value
    ;
    
simple_value: VALUE NAME
    {
        currentInstance.s_options.Reinit();
        currentInstance.s_type = value;
        currentInstance.s_name = $2;
        free($2);
        
        // Warning if the instance name matches a type name:
        
        warnIfTypeName(currentInstance.s_name);
        addInstance(currentInstance);
      
    }
optioned_value: simple_value valueoptions
{
    // put the currentInstance options into the last instance options
    // and reinit:
    
    instanceList.back().s_options = currentInstance.s_options;
    currentInstance.s_options.Reinit();
}

array_instance: simple_array | optioned_array
    {

    }

simple_array: ARRAY NAME LBRACK NUMBER RBRACK
    {
        // Put an array element in the instance list:
        
        currentInstance.s_options.Reinit();
        currentInstance.s_type = array;
        currentInstance.s_name = $2;
        free($2);
        warnIfTypeName(currentInstance.s_name);
        
        int count= checkIndex($4);
        
        currentInstance.s_elementCount = count;
        addInstance(currentInstance);
    }
    
optioned_array: simple_array valueoptions
    {
        //Last element on the instance list is a simple_array
        // we set its option values from the current instance's options and
        // reinit:
        
        instanceList.back().s_options = currentInstance.s_options;
        currentInstance.s_options.Reinit();
    }

struct_instance: STRUCTINSTANCE NAME NAME
    {
        verifyTypeInstance($2, $3);
        currentInstance.s_options.Reinit();   // Don't actully have options but...
        currentInstance.s_type = structure;
        currentInstance.s_name = $3;
        free ($3);
        currentInstance.s_typename = $2;
        free($2);
        warnIfTypeName(currentInstance.s_name);
        currentInstance.s_elementCount =1;
        addInstance(currentInstance);
    }

structarray_instance: STRUCTARRAYINSTANCE NAME NAME LBRACK NUMBER RBRACK
    {
        verifyTypeInstance($2, $3);
        currentInstance.s_options.Reinit();   // Don't actully have options but...
        currentInstance.s_type = structarray;
        currentInstance.s_name = $3;
        free($3);
        currentInstance.s_typename = $2;
        free($2);
        warnIfTypeName(currentInstance.s_name);
        currentInstance.s_elementCount =$5;
        addInstance(currentInstance);
    }

%%


// Check array index - error if not valid otherwise returns
// the integer array index

static int checkIndex(double value)
{
    int count = value;
    if ((double)(count) != value) {
        yyerror("Array indices must  be integers");
    }
    if (count <= 0) {
        yyerror("Array indices must be > 0");
    }
    return count;
}

// Verify the existence of a struct type field if not yyerror.

static void verifyTypeField(const char* ty, const char* f)
{   
    if (!structExists(ty)) {
        std::stringstream errormsg;
        const TypeDefinition& t = typeList.back();
        errormsg << "Attempting to define a field of unknown type in \n";
        errormsg << "Struct " << t.s_typename << std::endl;
        errormsg << "Fieldname: " << f << " Type: " << ty << std::endl;
        yyerror(errormsg.str().c_str());
    }
}
// Verify type of a struct for an instance:

static void verifyTypeInstance(const char* ty, const char *i)
{   
    if (!structExists(ty)) {
        std::stringstream errormsg;
        errormsg << "Unknown type: " << ty
            << " for structinstance or structarrayinstance " << i << std::endl;
        yyerror(errormsg.str().c_str());
    }
}

// Root/C++11 will toss errors if the instance name is that same as a
// type.

static void warnIfTypeName(const std::string& instanceName)
{
    if (structExists(instanceName.c_str())) {
        std::string message = "Instance name ";
        message += instanceName;
        message += " is the same as a struct name.\n";
        message += "This will cause compilation errors for some targets (e.g. Root/g++)\n";
        message += "unless you compile with -fpermissive (not recommended)\n";
        yywarning(message.c_str());
    }
}