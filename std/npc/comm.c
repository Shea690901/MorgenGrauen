// MorgenGrauen MUDlib
//
// npc/comm.c -- Basiskommunikation fuer NPCs
//
// $Id: comm.c 6371 2007-07-17 22:46:50Z Zesstra $
#pragma strong_types
#pragma save_types
#pragma range_check
#pragma no_clone
#pragma pedantic

#include <language.h>
#define NEED_PROTOTYPES
#include <thing/description.h>

void create() {
  add_action( "sage", "sag", 1 );
  add_action( "echo", "echo" );
  add_action( "emote", "emote" );
}

int echo( string str ) {
  say( str + "\n" );
  return 1;
}

int sage( string str ) {
  say( break_string(str, 78, capitalize(name(WER,2))+" sagt: "));
  return 1;
}

int emote( string str ) {
  say( capitalize(name(WER,2))+" "+str+"\n" );
  return 1;
}
