// MorgenGrauen MUDlib
//
// room/exits.h -- exit specific defines
//
// $Id: exits.h 8797 2014-05-02 20:10:08Z Arathorn $

#ifndef __ROOM_EXITS_H__
#define __ROOM_EXITS_H__

// Properties

#define P_EXITS              "exits"
#define P_SPECIAL_EXITS      "special_exits"
#define P_HIDE_EXITS         "hide_exits"

#endif // __ROOM_EXITS_H__

// Prototypes
#ifdef NEED_PROTOTYPES

#ifndef __ROOM_EXITS_H_PROTO__
#define __ROOM_EXITS_H_PROTO__

static mapping _set_exits(mapping map_ldfied) ;
static mapping _query_exits();
static int _set_special_exits(mapping map_ldfied);
static mapping _query_special_exits();
static string _MakePath(string str);
void AddExit(mixed cmd, mixed room);
void RemoveExit(mixed cmd);
void AddSpecialExit(mixed cmd, mixed functionname);
void RemoveSpecialExit(mixed cmd);
varargs string GetExits( object viewer );
int _normalfunction();

#endif // __ROOM_EXITS_H_PROTO__

#endif // NEED_PROTOTYPES
