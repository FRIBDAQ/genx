%{
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "instance.h"

extern int yylex();
extern int yyparse();
extern FILE* yyin;
void yyerror(const char* s);
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
    
struct_decl: STRUCT NAME LCURLY struct_members RCURLY
    {
    }
    
        
struct_members: struct_member | struct_member struct_members
    {
    }
struct_member: value | array | substruct | substruct_array
    {
    }

value: VALUE NAME | VALUE NAME valueoptions
    {
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
    
array: ARRAY NAME LBRACK NUMBER RBRACK | ARRAY NAME LBRACK NUMBER RBRACK valueoptions
    ;

substruct: STRUCT NAME NAME
    ;

substruct_array: STRUCTARRAY NAME NAME LBRACK NUMBER RBRACK
    ;

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
        int count= $4;
        if ((double)(count) != $4) {
            yyerror("Array indices must  be integers");
        }
        if (count <= 0) {
            yyerror("Array indices must be > 0");
        }
        currentInstance.s_elementCount = $4;   // TODO: Require positive integer.
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
    }

structarray_instance: STRUCTARRAYINSTANCE NAME NAME LBRACK NUMBER RBRACK
    {
    }

%%



/** code */
