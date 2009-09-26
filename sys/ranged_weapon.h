// MorgenGrauen MUDlib
//
// ranged_weapon.h -- Headerfile fuer Fernkampfwaffen
//
// $Id: ranged_weapon.h 4718 2006-09-23 10:18:26Z root $

#ifndef __RANGED_WEAPON_H__
#define __RANGED_WEAPON_H__

// Properties

#endif // __RANGED_WEAPON_H__

#ifdef NEED_PROTOTYPES

#ifndef __RANGED_WEAPON_H_PROTO__
#define __RANGED_WEAPON_H_PROTO__

// Prototypes
static void SkillResTransfer(mapping from_M, mapping to_M);
static int shoot_dam(mapping shoot);
static string FindRangedTarget(string str, mapping shoot);
static int cmd_shoot(string str);

#endif // __RANGED_WEAPON_H_PROTO__

#endif // NEED_PROTOTYPES