// MorgenGrauen MUDlib
//
// sys/player/util.h -- Headerfile fuer /std/player/util.c
//
// $Id: util.h 5764 2006-09-23 10:38:09Z root $

#ifndef __THING_UTIL_H__
#define __THING_UTIL_H__

#endif // __THING_UTIL_H__

#ifdef NEED_PROTOTYPES

#ifndef __THING_UTIL_H_PROTO__
#define __THING_UTIL_H_PROTO__

public void ShowPropList(string *props);
static void PrettyDump(mixed x);
static void DumpArray(mixed *x);
static void DumpMapping(mapping x);
static void DumpKeyValPair(mapping x, mixed key, int size);

#endif // __THING_UTIL_H_PROTO__

#endif // NEED_PROTOTYPES