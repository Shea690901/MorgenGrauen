// MorgenGrauen MUDlib
//
// player/quests.h -- player quest handling
//
// $Id: quest.h 4454 2006-09-23 10:13:55Z root $
 
#ifndef __PLAYER_QUESTS_H__
#define __PLAYER_QUESTS_H__

// properties

#define P_QUESTS             "quests"
#define P_QP                 "questpoints"

#endif // __PLAYER_QUESTS_H__

#ifdef NEED_PROTOTYPES

#ifndef __PLAYER_QUESTS_H_PROTO__
#define __PLAYER_QUESTS_H_PROTO__ 

// prototypes

varargs int GiveQuest(string questname, string message);
int QueryQuest(string questname);

#endif // __PLAYER_QUESTS_H_PROTO__

#endif // NEED_PROTOYPES
