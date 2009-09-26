// MorgenGrauen MUDlib
//
// room/commands.c -- room commands handling
//
// $Id: commands.c 6371 2007-07-17 22:46:50Z Zesstra $

#pragma strong_types
#pragma save_types
#pragma pedantic
#pragma range_check
#pragma no_clone

inherit "std/thing/commands";

//#define NEED_PROTOTYPES

#include <thing/properties.h>
#include <config.h>
#include <properties.h>
#include <language.h>
#include <defines.h>

void init() 
{
  ::init();

  add_action("imposs", "lies");
  add_action("imposs", "such");
  add_action("imposs", "suche");
}

/* Fuer etwas bessere Fehlermeldungen als 'Wie bitte?' bei einigen */
/* allgemeinen Kommandos.					   */
int imposs()
{
  switch (query_verb())
  {
    case "lies":
    _notify_fail("Was moechtest Du lesen?\n"); break;	
    case "such": case "suche":
    _notify_fail("Du suchst, findest aber nichts.\n"); break;
  }
  return 0;
}
