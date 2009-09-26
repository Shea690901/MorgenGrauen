// MorgenGrauen MUDlib
//
// armour.c -- armour standard object
//
// $Id: armour.c 6571 2007-10-21 14:41:10Z Zesstra $

#pragma strict_types
#pragma save_types
#pragma no_clone
#pragma range_check
#pragma pedantic

inherit "/std/thing/properties";
inherit "/std/thing/commands";
inherit "/std/thing/restrictions";
inherit "/std/armour/description";
inherit "/std/clothing/moving";
inherit "/std/armour/wear";
inherit "/std/armour/combat";
inherit "/std/thing/language";
inherit "/std/thing/envchk";

//#define NEED_PROTOTYPES

#include <config.h>
#include <properties.h>
#include <language.h>
#include <combat.h>
#include <wizlevels.h>
#include <defines.h>

protected void create() {
  properties::create();
  commands::create();
  restrictions::create();
  description::create();
  wear::create();
  combat::create();
  envchk::create();
  AddId("Ding");
}

protected void create_super() {
  set_next_reset(-1);
}

// Zum Ueberschreiben, damit es nicht buggt, wenn die Leute in ihren Objekten (wie
// gewuenscht) ::reset() rufen.
void reset() {
}
