inherit "std/thing";
#include <properties.h>
#include <moving.h>

#pragma strong_types
#pragma strict_types

void create()
{
   if (!clonep(this_object()))
   {
     set_next_reset(-1);
     return;
   }
   ::create();

   SetProp(P_GENDER, 0);
   SetProp(P_NAME, "Gebuesch");
   SetProp(P_SHORT,"Ein Gebuesch");
   SetProp(P_LONG,"Du stehst vor einem gaanz normalen Gebuesch. Es traegt "
		  "Fruechte. Und eventuell\nkann man darin etwas finden.\n");
   SetProp(P_NOGET,"Es ist festgewachsen, eventuell kann man es ausgraben?\n");

   SetProp(P_WEIGHT,5000);
   SetProp(P_SIZE,100);

   AddId("busch");
   AddId("gebuesch");
   
   AddDetail(({"fruechte","frucht"}),
     "Es scheinen Beeren zu sein. Vielleicht kannst Du sie pfluecken?\n");
   AddDetail("beeren",
     "Pflueck sie doch, vielleicht bringt das ja was.\n");

   // Nur wenn die Syntax erfuellt ist, wird cmd_pfluecken() aufgerufen.
   // "pfluecke beeren von busch" etc.
   AddCmd("pfluecke|ernte&fruechte|beeren&@ID","cmd_pfluecken",
	   "Was willst Du denn @verben?|Wo willst Du denn die Beeren @verben?");

   // suchen gibt nur eine Meldung aus. Dafuer braucht man kuenftig keine
   // Funktionen mehr.
   AddCmd("such|suche|durchsuch|durchsuche&@ID&\nimpossible",0, 
          "Wo willst Du @VERBen?|Du durchsuchst das Gebuesch, findest aber nichts.^"
	  "@WER1 durchsucht ein Gebuesch, findet aber nichts.");
	  
   // Graben geht eh nicht. Daher nur Fehlermeldungen.
   // Das ^ sagt, dass hier ein return1 zurueckgegeben wird. Es gibt aber keine 
   // Raummeldung.
   AddCmd("grab|grabe&@ID&aus@\nimpossible",0,
	  "Was willst Du graben?|Du willst das Gebuesch ausgraben?|"
	  "Die Wurzeln scheinen tief zu rechen. Das wird nichts.^");
}

int cmd_pfluecken(string arg, mixed *param)
{

    object obj;
    
    obj=clone_object("/players/vanion/demo/obst");
    write("Verwundert pflueckst Du "+(obj->name())+" vom Busch. Komisch.\n");

    // Das hier ist ein Beispiel fuer AddCmd, daher mach ich mir nich 
    // die Muehe das richtig zu machen. Is eh nur fuer Magier!
    obj->move(this_player(),M_GET|M_NOCHECK);
    return 1;
}
