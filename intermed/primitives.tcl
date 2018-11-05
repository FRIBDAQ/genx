#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file primitives.tcl
# @brief Create symbol table elements for primitives (value array).
# @author Ron Fox <fox@nscl.msu.edu>
#

##
#  All of this can be done as snit::types since the specification
#  of a variable or array looks very much like object creation:
#  problems:: array is already a Tcl command...but I don't think we actually
#  will need its facilities so for now we'll just override it and, if necessary
#  we can rename our command later.
#
# The primitives need to register themselves with _something_, then an external
# entity iterates over that _something_ to dump the symbol table into a specific
# dict or part a dict (consider the difference between a value instance and a value
# as a struct field).
#
#  Therefore each primitive has a typevariable that can be set.  That typevariable
#  is where it will register instances of itself.  In that way the top level script
#  or a struct generator can dump the symbol tables of the appropriate bits and
#  pieces that make it up.
#


package provide DataPrimitives 1.0
package require snit

##
#  value
#    A scalar value.  In root this generates a double.  In
#    SpecTcl a CTreeParameter.
#
# OPTIONS:
#   *  - low (default 0) Axis hint for low end of range.
#   *  - high (default 100) Axis hint for high end of range.
#   *  - bins (default 100) Axis hint for number of axis bins.
#   *  - units (default "") Units of measure for the parameter.
#

snit::type value {
    option -low -default 0
    option -high -default 100
    option -bins -default 100
    option -units -default ""
    
    ##
    #  The stuff we need for object registration
    #
    
    typevariable  registrar
    typemethod setRegistrar name {
        set registrar $name 
    }
    typemethod register instance {
        lappend $registrar $instance
    }
    
    
    ##
    #  Constructor
    #
    constructor args {
        $self configurelist $args;   # process options.
        $type register $self;              # register self with the current registrar.
    }
    
    ##
    # dump
    #   Dumps the dict that is our symbol table entry:
    # @return dict.
    #
    method dump {} {
        return [dict create \
            type value low $options(-low) high $options(-high) \
            bins $options(-bins) units $options(-units)         \
        ]
    }
}
##
#  Array
#     Can't use array because it's a tcl command that's used either by
#     snit or tcltest.
#

snit::type Array {
    option -base -default 0
    option -low  -default 0
    option -high -default 100
    option -bins -default 100
    option -units -default ""
    variable elements 0
   ##
    #  The stuff we need for object registration
    #
    
    typevariable  registrar
    typemethod setRegistrar name {
        set registrar $name 
    }
    typemethod register instance {
        lappend $registrar $instance
    }
    
    # construct an instance:
    
    constructor {size args} {
        if {![string is integer -strict $size]} {
            error "Array size $size is not an integer"
        }
        if {$size <= 0 } {
            error "Array size $size is not > 0"
        }
        set elements $size
        
        $self configurelist $args
        $type register $self
    }
    method dump {} {
        return [dict create \
            type array low $options(-low) high $options(-high)  \
            bins $options(-bins) units $options(-units) elements $elements \
        ]
    }
    
}

##
#  set primitive Registrars:
#     Sets the registrars for all primitive data type:
#

proc setPrimitiveRegistrars reg {
    value setRegistrar $reg
    Array setRegistrar $reg
}