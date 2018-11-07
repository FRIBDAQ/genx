

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

/** @file:  instance.cpp
 *  @brief: Implement instance types:
 *
 */

#include "instance.h"
#include <stdlib.h>
#include <sstream>
#include <set>

extern void yyerror(const char* msg);

/*
**
*   Define the instance table.
*   Instances consist of primitives (values and arrays) and
*   structured types (structs and structarrays).
*   These latter are instances or arrays of instances of types that must
*   have been previously defined as structs.
*/





std::list<Instance> instanceList;         // Must be created.
Instance currentInstance;                  // Instance beingcreated.


static std::set<std::string> instanceNames;      // Quicker check for dups:


/*-----------------------------------------------------------------------------
 * Static utilities:
 */

/**
 * findInstance
 *    Return a reference to the instance with the specified instance name.
 *    The caller must ensure this exists prior to calling.
 *
 *  @return const Instance&
 */
static const Instance&
findInstance(std::string name)
{
    for (std::list<Instance>::iterator p = instanceList.begin(); p!= instanceList.end(); p++) {
        if (p->s_name == name) {
            return *p;
        }
    }
    yyerror("BUGBUG - searched for nonexistent instance name");
}
/**
 * serializeString
 *    Serializes an std::string.  The string is stored as an unsigned character
 *    count followed by the string data itself.
 *
 *  @param f - the stream to which the string is being serialized.
 *  @param s - the string to serialize.
 *  @return std::ostream& f again.
 */
static std::ostream&
serializeString(std::ostream& f, const std::string& s)
{
    unsigned size = s.size();
    f.write(reinterpret_cast<char*>(&size), sizeof(unsigned));
    f.write(s.c_str(), size);
    return f;
}

/*----------------------------------------------------------------------------
 *  Method implementations.
 */

// ValueOptions methods:

/**
 * ValueOptions::toString
 *    Create a string representation of the value options struct.
 *
 *   @return std::string
 */
std::string
ValueOptions::toString() const
{
    std::stringstream sresult;
    sresult << "Low = " << s_low << " High = " << s_high << " bins= " << s_bins
        <<  " units: " << s_units;
    return sresult.str();
}

// Instance Methods:

/**
 * ValueOptions::serialize
 *    Produce a serialized version of a value options object.  This is just
 *    going to be a binary serialization of the members.   For strings
 *    we output a counted string using serializeString
 *
 *  @param f - References the output stream to which the serialization is done.
 *  @return f - References the output stream passed in.
 */
std::ostream&
ValueOptions::serialize(std::ostream& f) const
{
    f.write(reinterpret_cast<const char*>(&s_low), sizeof(double));
    f.write(reinterpret_cast<const char*>(&s_high), sizeof(double));
    f.write(reinterpret_cast<const char*>(&s_bins), sizeof(unsigned));
    return serializeString(f, s_units);
}

/**
 * Instance::toSTring
 *    Produce a string representation of an Instance struct.
 *
 * @return std::string
 */
std::string
Instance::toString() const
{
    std::stringstream sresult;
    sresult << "Type: ";
    switch (s_type) {
    case value:
        sresult << "value";
        break;
    case array:
        sresult << "array";
        break;
    case structure:
        sresult << "struct";
        break;
    case structarray:
        sresult << "array of struct";
        break;
    default:
        sresult << "** Not set*** ";
    }
    
    sresult << std::endl << "Name: " << s_name << std::endl;
    sresult << "Typename: " << s_typename << std::endl;
    sresult << "elements: " << s_elementCount << std::endl;
    sresult << s_options.toString() << std::endl;
    
    
    return sresult.str();
}
/**
 * Instance::serialize
 *
 *   Serializes the object to a stream.  The members are serialized independently
 *   and the s_options.serialize method is used for the value options.
 *
 * @param f - references the stream to which serialization is going to be done.
 * @return std::ostream& f again.
 */
std::ostream&
Instance::serialize(std::ostream& f) const
{
    // our primitives:
    
    f.write(reinterpret_cast<const char*>(&s_type), sizeof(InstanceType));
    serializeString(f, s_name);
    serializeString(f, s_typename);
    f.write(reinterpret_cast<const char*>(&s_elementCount), sizeof(unsigned));
    
    // The options object.
    
    return s_options.serialize(f);
}
/*-----------------------------------------------------------------------------
 *  API presented to the parser.
 */


/**
 *  addInstance
 *     Adds a new instance to the instance list; An error is thrown if
 *     the instance already exists.
 *
 * @param inst - instance to add.
 */
void
addInstance(const Instance& inst)
{
    // Check for duplicate name - that's an error:
    
    if (instanceNames.count(inst.s_name)) {
        const Instance& dup = findInstance(inst.s_name);
        
        std::stringstream errorMessage;
        errorMessage << " Duplicate instance name: " << inst.s_name
            << " already defined as: \n"
            << dup.toString() << std::endl;
        yyerror(errorMessage.str().c_str());
    } else {
        instanceList.push_back(inst);
        instanceNames.insert(inst.s_name);
    }
}
/**
 * serializeInstances
 *    Serializes all of the instances to an output stream.
 *    We output an unsigned that contains the number of instances and then
 *    just serialize each instance:
 *
 *    @param f - the output stream to which the serialization is done.
 *    @return std::ostream& f again.
 */
std::ostream&
serializeInstances(std::ostream& f)
{
    unsigned n = instanceList.size();
    f.write(reinterpret_cast<char*>(&n), sizeof(unsigned));
    for (std::list<Instance>::const_iterator p = instanceList.begin();
         p != instanceList.end(); p++) {
        
        p->serialize(f);
    }
}