%{
%}
%union {
    int integer;
    char* str;
}

/* Tokens: */

%token VALUE
%token ARRAY
%token STRUCT
%token STRUCTARRAY
%token STRUCTINSTANCE
%token STRUCTARRAYINSTANCE
%token NAME
%token LOW
%token HIGH
%token BINS
%token UNITS
%token NUMBER
%token ENDLINE
%token LBRACK
%token RBRACK
%token LCURLY
%token RCURLY
%token EQUALS

%%


input_file: input_file1 | blankline input_file1
    ;

input_file1: instances | struct_decls instances
    ;
    
struct_decls: struct_decl | struct_decl struct_decls
    ;
    
struct_decl: STRUCT NAME LCURLY struct_members RCURLY
    ;
        
struct_members: struct_member | struct_member struct_members
    ;
struct_member: value | array | substruct | substruct_array
    ;

value: VALUE NAME valueoptions
    ;
valueoptions:   valueoption | valueoption valueoptions
    ;
valueoption: LOW EQUALS NUMBER | HIGH EQUALS NUMBER | BINS EQUALS NUMBER | UNITS EQUALS NAME
    ;
array: ARRAY NAME LBRACK NUMBER RBRACK valueoptions
    ;

substruct: STRUCT NAME NAME
    ;

substruct_array: STRUCTARRAY NAME NAME LBRACK NUMBER RBRACK

instances: instance | instance instances
    ;
    
    
instance: val_instance | array_instance | struct_instance | structarray_instance
    ;
    
val_instance: VALUE NAME valueoptions
    ;
array_instance: ARRAY NAME LBRACK NUMBER RBRACK valueoptions

struct_instance: STRUCTINSTANCE NAME NAME

structarray_instance: STRUCTARRAYINSTANCE NAME NAME LBRACK NUMBER RBRACK

blankline: ENDLINE | ENDLINE blankline
    ;

/** Production rules */



/** code */