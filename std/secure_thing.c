/*
 * thing.c fuer Objekte, die sicherheitsrelevant sind.
 * Sollte von Dingen aus /secure oder Magiertools auf jeden
 * Fall inherited werden!
 *
 * 07.01.98 Rumata
 *
 *   Padreics Sicherheitsfeature eingebaut.
 *   int secure_level() gibt den niedrigsten level zurueck, der
 *   in der previous_object-Kette vorkommt. So koennen Objekte, die
 *   Kommandos von Highlevelmagiern abfangen wollen, erkannt und
 *   gestoppt werden.
 *   Syntax:
 *     secure_level() untersucht alle Objekte, die zur Auswertung dieser
 *       Funktion fuehren und gibt den niedigsten wizlevel zurueck, der
 *       bei diesen Objekten vorkommt.
 *
 *       Diese Funktion wird normalerweise ueber die in wizlevels.h
 *       definierten Makros *_SECURITY benutzt und dient als Ersatz fuer
 *       IS_ARCH() und Konsorten.
 *
 * 14.06.93 Rumata
 */
#pragma strong_types
#pragma save_types
#pragma no_clone
#pragma no_shadow
#pragma range_check
#pragma pedantic

inherit "std/thing";
#include <properties.h>

// int secure_level() // ist nun in simul_efun

int lies( string str )
{
  if( !str || !id(str) ) return 0;
  write( QueryProp( P_READ_MSG ) );
  return 1;
}

varargs string long(int mode)
{
  return funcall(QueryProp(P_LONG));
}

string short()
{
  string sh;
  if( sh=QueryProp(P_SHORT) )
    return funcall(sh)+".\n";
}

