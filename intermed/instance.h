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

/** @file:  instance.h
 *  @brief: Define types and functions/methods for managing instance definitions.
 *  
 */
#ifndef INSTANCE_H
#define INSTANCE_H
#include <list>
#include <ostream>
#include <istream>

#include <sstream>


// Enum defining the types of instances that can be created.

enum InstanceType {
    value,
    array,
    structure,                    // struct is areserved word :-)
    structarray
};

// Primitives have metadata associated with them.

struct ValueOptions {
    double s_low;
    double s_high;
    unsigned s_bins;
    std::string s_units;
    
    ValueOptions() : s_low(0), s_high(100), s_bins(100), s_units("")
    {
    }
    void Reinit() {
        s_low = 0;
        s_high = s_bins = 100;
        s_units = "";
    }
    std::string toString() const;
};

// Describes an instance.  note that not all fields are used for all types.

struct Instance {
    InstanceType   s_type;
    std::string    s_name;
    std::string    s_typename;
    unsigned       s_elementCount;
    ValueOptions   s_options;
    std::string toString() const;
};

void addInstance(const Instance& anInstance);

extern Instance currentInstance;
extern std::list<Instance> instanceList;
#endif