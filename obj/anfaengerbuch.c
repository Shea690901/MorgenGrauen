inherit "std/thing";

#include <properties.h>

create()
{
  if(!clonep(this_object())) return;
  ::create();
  SetProp(P_SHORT, "Ein Buch");
  SetProp(P_LONG,
	"Du haelst ein leinengebundenes Buch in Deinen Haenden. Der Titel heisst:\n"+
	"'Erste Schritte im MorgenGrauen'. Du kannst es mit 'lies buch' lesen.\n");
  SetProp(P_NAME,"Handbuch");
  SetProp(P_GENDER, NEUTER);
  SetProp(P_WEIGHT,250);
  SetProp(P_NOBUY, 1);
  AddId(({"buch","erste schritte","handbuch"}));
  AddCmd(({"lies","lese"}), "lies");
}

lies(str)
{
  string was;
  notify_fail("Was willst Du lesen ? Syntax: LIES <ETWAS>.\n");
  if(!str) return 0;
  if(sscanf(str,"%s",was)!=1) return 0;
  if(member(QueryProp(P_IDS), was)<0) return 0;
  say(this_player()->name() + " liest ein Buch.\n",this_player());
  this_player()->more("/obj/anfaenger.text");
  return 1;
}

reset()
{
  if(!clonep(this_object())) return 1;
	if(!environment(this_object()))
		return remove();
  if(!interactive(environment(this_object())))
  {
    if(!clonep(environment(this_object())))
      tell_room(environment(this_object()),"Ein Buch verschwindet mit Knall und Schwefelduft.\n");
    return remove();
  }
  return 1;
}
