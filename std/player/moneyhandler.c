// MorgenGrauen MUDlib
//
// player/moneyhandler.c -- money handler for players
// Nur noch aus Kompatibilitaetsgruenden vorhanden
//
// $Id: moneyhandler.c 6747 2008-03-11 23:26:03Z Zesstra $
#pragma strong_types
#pragma save_types
#pragma range_check
#pragma no_clone
#pragma pedantic

#include <living/moneyhandler.h>
inherit MONEYHANDLER;

// Funktionen sollen nur das Programm ersetzen, natuerlich nur in der
// Blueprint _dieses_ Objektes, nicht in anderen. ;-) BTW: NOrmalerweise
// sollte niemand hierdrin create() rufen, der das Ding hier erbt.
protected void create_super() {
  if (object_name(this_object()) == __FILE__[..<3])
    replace_program(MONEYHANDLER);
}

protected void create() {
  create_super();
}

