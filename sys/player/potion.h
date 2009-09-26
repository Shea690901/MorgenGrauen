// MorgenGrauen MUDlib
//
// player/potion.h -- potion services
//
// $Id: potion.h 4457 2006-09-23 10:13:58Z root $

#ifndef __PLAYER_POTION_H__
#define __PLAYER_POTION_H__

// properties

#define P_POTIONROOMS         "potionrooms"
#define P_KNOWN_POTIONROOMS   "known_potionrooms"
#define P_HEALPOTIONS         "healpotions"
#define P_TRANK_FINDEN        "trank_finden"
#define P_VISITED_POTIONROOMS "visited_potionrooms"
#define P_BONUS_POTIONS       "bonus_potions"

#endif // __PLAYER_POTION_H__

#ifdef NEED_PROTOTYPES

#ifndef __PLAYER_POTION_H_PROTO__
#define __PLAYER_POTION_H_PROTO__ 

// prototypes

varargs int FindPotion(string s);

#endif // __PLAYER_POTION_H_PROTO__

#endif // NEED_PROTOYPES

