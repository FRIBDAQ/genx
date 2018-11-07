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

/** @file:  definedtypes.cpp
 *  @brief: Implement the defined type methods, functions and support utilties.
 */

#include "definedtypes.h"
#include <set>
#include <sstream>


// The global type definition list:

std::list<TypeDefinition> typeList;
Instance                  currentField;

static std::set<std::string> typeNames;
static std::set<std::string> fieldNames;

extern void yyerror(const char* msg);
/*-----------------------------------------------------------------------------
 *  private utilities
 */


/**
 * findDefinition
 *   @param defname
 *   @return const TypeDefinition&  definition with that name.
 *   @note the caller must have already ensured there is a def of that name.
 */
static const TypeDefinition&
findDefinition(const char* name)
{
    for(std::list<TypeDefinition>::const_iterator p = typeList.begin();
           p != typeList.end(); p++) {
       if (p->s_typename == name) return *p; 
    }
    yyerror("BUG - findDefinition - no such type");
}
/**
 * fieldExists
 *   @param  name - name of the field to lookup.
 *   @return bool - true if there's a field of that name in the struct being
 *                  accumulated (fieldnames set).
 */
static bool
fieldExists(const std::string& name)
{
    return fieldNames.count(name) > 0;
}
/**
 * findField
 *    @param t = TypeDefinition to search.
 *    @param name - name of a field.
 *    @return const Instance& - reference to the named field.
 *    @note the caller must have ensured there's a match.
 */
static const Instance&
findField(TypeDefinition& t, const std::string& name)
{
    for (FieldList::const_iterator p = t.s_fields.begin(); p != t.s_fields.end(); p++ ){
        if (p->s_name == name) return *p;
    }
    yyerror("BUG - findField - no such field");
}
/*------------------------------------------------------------------------------
 *   Public entries;
 */

/** Object methods for the TypeDefinition:

/**
 * TypeDefinition::toString
 *    Create a string representation of a struct so that it can be output
 *    e.g. as part of an error message or debugging tool.
 */
std::string
TypeDefinition::toString() const
{
    std::stringstream result;
    result << "Type: " << s_typename;
    result << " Fields:\n";
    for (FieldList::const_iterator p = s_fields.begin(); p != s_fields.end(); p++) {
        result << "  " << p->toString() << std::endl;
    }
    
    return result.str();
}

/**
 * TypeDefinition::serialize
 *     Produce a binary serialization of the type.
 *     This is a serializationof the typename, a count of the number of fields
 *     followed by asking each field to serialize itself.
 *
 *  @param f - references the stream to output the data to.
 *  @return ostream& -  f again.
 */
std::ostream&
TypeDefinition::serialize(std::ostream& f) const
{
    serializeString(f, s_typename);
    unsigned n = s_fields.size();
    f.write(reinterpret_cast<char*>(&n), sizeof(unsigned));
    for (FieldList::const_iterator p = s_fields.begin(); p != s_fields.end(); p++) {
        p->serialize(f);
    }
    return f;
}
/**
 * TypeDefinition::deserialize
 *   Deserialize from a stream into us;
 *   Note that any prior fields are removed.
 *
 * @param f - the stream from which we're deserializing.
 * @return istream& - f again.
 */
std::istream&
TypeDefinition::deserialize(std::istream& f)
{
    s_fields.clear();               // Empty set of fields.
    s_typename = deserializeString(f);
    unsigned nFields;
    f.read(reinterpret_cast<char*>(&nFields), sizeof(unsigned));
    for (int i = 0; i < nFields; i++) {
        Instance inst;
        inst.deserialize(f);
        s_fields.push_back(inst);
    }
    
    return f;
}
/**
 * newStruct:
 *   - Checks that the type is not a duplicate and yyerror's if it is.
 *   - Adds a instance to the typeList
 *   - Initializes the lastFieldOptions.
 *   - Inits the options that are in the currentField.
 *   - Empties the fieldNames set.
 *   - Adds the name of the type to the typeNames set.
 * 
 * @param structName - Name of the new struct being created.
 */
void
newStruct(const char* structName)
{
    if (structExists(structName)) { 
        std::stringstream errorMessage;
        errorMessage << " Struct " << structName << " is already defined as:\n";
        const TypeDefinition& p = findDefinition(structName); 
        errorMessage << p.toString() << std::endl;
        yyerror(errorMessage.str().c_str());
    } else {
        TypeDefinition newType;
        newType.s_typename = structName;
        typeList.push_back(newType);
        currentField.s_options.Reinit();
        currentInstance.s_options.Reinit();
        typeNames.insert(std::string(structName));
        fieldNames.clear();                // New field namespace for each struct.
    }
}
/**
 * addField
 *    - Ensure the field is uniquely named in the struct.
 *    - Add the field to the strut.
 *    - Initialize the field options (those'll get set later if needed).
 *    - Add the field name to the set of fieldnames for this struct.
 *
 *  @param fieldDef - const reference to the field to add.
 */
void
addField(const Instance& fieldDef)
{
    TypeDefinition& t(typeList.back());   // Building up this type.
    
    if (fieldExists(fieldDef.s_name)) {
        std::stringstream errorMessage;
        errorMessage  << "Struct " << t.s_typename << " already has a field named "
            << fieldDef.s_name << " defined as:\n";
        const Instance& p = findField(t, fieldDef.s_name);  //TODO: findField.
        errorMessage << p.toString() << std::endl;
        yyerror(errorMessage.str().c_str());
    } else {
        t.s_fields.push_back(fieldDef);
        t.s_fields.back().s_options.Reinit();
        fieldNames.insert(fieldDef.s_name);
    }
}

/**
 * setLastFieldOptions
 *   Because of the way productions pop in the bison grammer, we don't know
 *   the field options (if any) until after the field has been pushed to the
 *   last struct def.  This replaces the default options of the last field
 *   of the last struct with new options.
 *
 * @param opts - new options.
 */
void
setLastFieldOptions(const ValueOptions& opts)
{
    typeList.back().s_fields.back().s_options = opts;
}

/**
 * @param name - name of a struct.
 * @return bool - True if a structure by that name already exists.
 */

bool
structExists(const char* name)
{
    return typeNames.count(std::string(name)) > 0;    
}



/**
 * serializeTypes
 *    serializes the type list.  This is done by writing the number of types
 *    defined, then serializing each type in typeList.
 *
 *  @param f - references the stream to which the serialization is being done.
 *  @return std::ostream& - f again.
 */
std::ostream&
serializeTypes(std::ostream& f)
{
    unsigned n = typeList.size();
    f.write(reinterpret_cast<char*>(&n), sizeof(unsigned));
    for (std::list<TypeDefinition>::const_iterator p = typeList.begin();
         p != typeList.end(); p++) {
        p->serialize(f);
    }
    return f;
}
/**
 * deserializeTypes
 *    Deserializes the type list into a type list.  Note that if the user
 *    wants a typename set they have to then construct it from the
 *    list.
 *
 *  @param f - references the stream to be read from
 *  @param tlist - References the target type list.  If there are existing
 *                 list elements the deserialized list appends to them.
 *              
 */
std::istream&
deserializeTypes(std::istream& f, std::list<TypeDefinition>& tlist)
{
    // Get the number of types to deserialize:
    
    unsigned n;
    f.read(reinterpret_cast<char*>(&n), sizeof(n));
    for (int i =0; i < n; i++) {
        TypeDefinition t;
        t.deserialize(f);
        tlist.push_back(t);
    }
    return f;
}