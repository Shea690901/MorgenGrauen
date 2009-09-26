// MorgenGrauen MUDlib
//
// armour.h -- armour header
//
// $Id: armour.h 6306 2007-05-20 11:32:03Z Zesstra $
 
#ifndef __ARMOUR_H__
#define __ARMOUR_H__

#include <clothing.h>
#include <combat.h>

// properties
#ifndef P_NR_HANDS
#define P_NR_HANDS      "nr_hands"     // benoetigte Haende zum tragen (Schild)
#endif

#define P_AC            "ac"           // Ruestungsklasse (armour class)

#define P_ARMOUR_TYPE   "armour_type"  // Art der Ruestung (Helm, Ring usw.)

#define P_DEFEND_FUNC   "defend_func"  // Objekt das eine DefendFunc definiert

#endif // __ARMOUR_H__

#ifdef NEED_PROTOTYPES

#ifndef __ARMOUR_H_PROTO__
#define __ARMOUR_H_PROTO__

// prototypes
int QueryDefend (string *dam_type, mixed spell, object enemy);

// TO BE REMOVED

void SetDefendFunc(object ob);
object QueryDefendFunc();

#endif // __ARMOUR_H_PROTO__

#endif	// NEED_PROTOYPES
