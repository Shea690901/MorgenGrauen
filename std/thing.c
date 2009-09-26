// MorgenGrauen MUDlib
//
// thing.c -- standard object
//
// $Id: thing.c 6571 2007-10-21 14:41:10Z Zesstra $                      

#pragma strict_types
#pragma save_types
#pragma pedantic                                                                          
#pragma range_check                                                                       
//#pragma no_clone                                                                        

inherit "std/thing/properties";
inherit "std/thing/description";
inherit "std/thing/moving";
inherit "std/thing/language";
inherit "std/thing/commands";
inherit "std/thing/restrictions";
inherit "std/thing/envchk";

protected void create()
{
  seteuid(getuid());
  properties::create();
  commands::create();
  description::create();
  restrictions::create();
  envchk::create();
  AddId("Ding");

  return;
}

// wird gerufen, wenn implizit per inherit geladen. In diesem Fall wird kein
// Reset benoetigt, weil das Objekt nicht konfiguriert ist und nur das
// Programm wichtig ist.
protected void create_super() {
  set_next_reset(-1);
}

// Damit Man kann in ALLEN Standardobjekten ::reset aufrufen kann.
void reset() {}
