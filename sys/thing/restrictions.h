// MorgenGrauen MUDlib
//
// thing/restrictions.h -- restrictions of the object (weight)
//
// $Id: restrictions.h 4709 2006-09-23 10:18:17Z root $
 
#ifndef __THING_RESTRICTIONS_H__
#define __THING_RESTRICTIONS_H__

// properties
#define P_WEIGHT             "weight"       // weight of an object
#define P_TOTAL_WEIGHT       "total_weight" // weight with all objects inside

#endif // __THING_RESTRICTIONS_H__

#ifdef NEED_PROTOTYPES

#ifndef __THING_RESTRICTIONS_H_PROTO__
#define __THING_RESTRICTIONS_H_PROTO__

static void _set_weight(int weight);
static mapping _set_extern_attributes_modifier(mapping xmod);
static mapping _set_extern_health_modifier(mapping xmod);

#endif // __THING_RESTRICTIONS_H_PROTO__

#endif	// NEED_PROTOYPES
