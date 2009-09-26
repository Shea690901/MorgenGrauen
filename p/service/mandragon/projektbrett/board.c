//
// Projectbrett
//

#include "projectmaster.h"
#include "/secure/wizlevels.h"
#include <moving.h>

inherit "/std/thing";

void create()
{
	if (!IS_CLONE(TO)) return;
	::create();
	if (!find_object(PROJECTMASTER)) call_other(PROJECTMASTER,"???");
	SetProp(P_NAME,"Schwarzes Brett");
	SetProp(P_NOGET,"Das ist viel zu schwer!!!");
	SetProp(P_GENDER,NEUTER);
	SetProp(P_SHORT,"Ein Schwarzes Brett");
	SetProp(P_INVIS,1);
AddId(({"projektbrett"}));
	RemoveId("Ding");
	AddCmd("projekthilfe","board_help");
	AddCmd("projektliste","board_list");
        AddCmd(({"lies","lese","les"}),"lesen_fun");
	AddCmd(({"projektdetail","projektdetails"}),"board_long");
	return;
}

int lesen_fun(string str)
{
// Bloedsinn eigentlich, ne std Funktion wie AddReadDetail zu ueberlagern.
// Zumindest "lies zettel" muss raus!
// Ark, 01.07.06.
  if (str!="projektbrett"&&str!="brett") return 0;
  tell_object(TP,
       BS("Als Du die Informationen auf den Zetteln zusammenfuegst, daemmert " +
          "es Dir: Dieses Brett wurde von Magiern aufgestellt, die Mitarbeiter " +
	  "fuer mehr oder weniger ehrgeizige Projekt suchen. Wenn Du eine dieser " +
	  "Projektbeschreibungen interessant finden solltest, so wende Dich doch " +
	  "einfach an den betreuenden Magier. Es ist nicht unbedingt notwendig, " +
	  "Magier zu werden, um an einem der Projekte mitzuarbeiten.\nUm Dir die " +
	  "Befehle zum bedienen des Brettes anzeigen zu lassen, gib einfach " +
	  "'projekthilfe' ein.\nViel Spass beim lesen!"));
  return 1;
}

int board_help(string arg)
{
	PROJECTMASTER->BoardHelp();
	return 1;
}

int board_list(string arg)
{
	arg=TP->_unparsed_args(1);
	if (!arg||!stringp(arg)||arg=="") (PROJECTMASTER)->ShowList();
	else PROJECTMASTER->ParseArgs(arg);
	return 1;
}

int board_long(string arg)
{
	int number;
	arg=TP->_unparsed_args(1);
	notify_fail("Syntax: projektdetail <PROJEKTNUMMER>\n");
	if (!arg||!stringp(arg)) return 0;
	if (!(number=to_int(arg))||(arg!=to_string(number))) return 0;
	notify_fail("Projektnummern koennen nur Zahlen groesser Null sein.\n");
	if (number<1) return 0;
	PROJECTMASTER->ShowLong(number);
	return 1;
}

int hilfe_fun(string arg)
{
	tell_object(TP,QueryProp(P_INFO));
	return 1;
}

