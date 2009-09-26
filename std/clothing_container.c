// MorgenGrauen MUDlib
//,
// clothing_container.c -- Kleidung, in die man was reinstecken kann
//
// $Id: clothing_container.c,v 1.3 2003/08/07 07:25:43 Rikus Exp $

#pragma strict_types
#pragma save_types
#pragma no_clone
#pragma pedantic
#pragma range_check

inherit "/std/thing/properties";
inherit "/std/thing/commands";
inherit "/std/thing/language";
inherit "/std/thing/envchk";
inherit "/std/container/restrictions";
inherit "/std/container/inventory";
inherit "/std/container/items";
inherit "/std/clothing/moving";
inherit "/std/clothing/wear";
inherit "/std/clothing/container_description";

//#define NEED_PROTOTYPES

#include <container.h>

protected void create() {
  properties::create();
  commands::create();
  container_description::create();
  restrictions::create();
  items::create();
  wear::create();
  envchk::create();
  SetProp(P_CONTAINER,1);
  AddId("Ding");
}

protected void create_super() {
  set_next_reset(-1);
}

void reset()  {
  items::reset();
}

int IsClothing() {return 1;}

