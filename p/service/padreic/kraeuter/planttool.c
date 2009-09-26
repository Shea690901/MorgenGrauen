inherit "std/secure_thing";

#include <properties.h>
#define PLANTMASTER "/gilden/files.dunkelelfen/secure/plantmaster"

void create()
{
   if (!clonep(this_object())) return;
   ::create();
   SetProp(P_SHORT, "Das Kraeutertool");
   SetProp(P_LONG,
     "Das Kraeutertool hat folgende Befehle:\n"
    +"- showplant <kategorie> (Listet alle Pflanzen einer Kategorie auf).\n");
   SetProp(P_NAME, "Planttool");
   SetProp(P_GENDER, NEUTER);
   SetProp(P_AUTOLOADOBJ, 1);
   SetProp(P_NODROP, 1);
   SetProp(P_NEVERDROP, 1);
   AddId(({"tool", "kraeutertool", "planttool"}));
   AddCmd(({"showplant"}), "_plantmaster");
}

static int _plantmaster(string str)
{
   notify_fail("Unknown Function in plantmaster: _"+query_verb()+"()\n");
   return call_other(PLANTMASTER, "_"+query_verb(), str);
}
