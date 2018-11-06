

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
// Create string representation of Value Options:

std::string
ValueOptions::toString() const
{
    std::stringstream sresult;
    sresult << "Low = " << s_low << " High = " << s_high << " bins= " << s_bins
        <<  " units: " << s_units;
    return sresult.str();
}

// Create string representation of an instance:

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

// Locate an instance with the specified name return a reference to that
// instance.   The caller must have checked this instance exists.

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

//  Add an instance first checking to see if it's a duplicate.

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