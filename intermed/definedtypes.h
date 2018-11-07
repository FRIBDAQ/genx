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

/** @file:  definedtypes.h
 *  @brief: Defines types methods and functions to manage user defined types.
 *
 */


#ifndef DEFINEDTYPES_H
#define DEFINEDTYPES_H
#include "instance.h"           // There's overlap in the needs.
#include <list>
#include <string>
#include <ostream>
#include <istream>

// At this point only structures are allowed as defined types.
// Fundamentally a structure is a named entity with a list of fields.
// The information required to keep track of each field is the sam
// as the information required to keep track of an instance.  Therefore:


typedef std::list<Instance> FieldList;

struct TypeDefinition {
    std::string s_typename;
    FieldList   s_fields;
    std::string toString() const;
    std::ostream& serialize(std::ostream& f) const;
    std::istream& deserialize(std::istream& f);
};

// These are operations needed by struct definers:
// These are a bit whacky because of the order in which productions complete.

void newStruct(const char* structName);
void addField(const Instance& fieldDef);           // Add field to last structure.
void setLastFieldOptions(const ValueOptions& opts); // Add option to last field added.
bool structExists(const char* name);
std::ostream& serializeTypes(std::ostream& f) ;
std::istream& deserializeTypes(std::istream& f, std::list<TypeDefinition>& tlist);

extern std::list<TypeDefinition> typeList;
extern Instance currentField;

#endif