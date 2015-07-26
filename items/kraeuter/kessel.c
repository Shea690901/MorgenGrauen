// (c) 2003 by Thomas Winheller (padreic@mg.mud.de)
// Bei diesem File handelt es sich um einen universellen
// Kessel zum brauen von Traenken. Damit er sich in den
// Raum entsprechend einpassen kann, ist er invis.

// Ein Raum indem Traenke und Gift gebraut werden koennen
// soll, braucht nichts weiter machen als:
// -  eine Moeglichkeit zu implementieren Wasser in den Kessel
//    zu fuellen. Hierzu muss dann mittels AddWater(3000)
//    insgesamt 3l Wasser eingefuellt werden. AddWater gibt die
//    Menge des tatsaechlich noch eingefuelltn Wassers zurueck.
// Anmerkung: bisher enthaelt /obj/flasche noch keine Moeglichkeit
//            Fluessigkeiten von einem Fluessigkeitencontainer in
//            einen anderen zu schuetten, aber vielleicht aendert
//            das ja irgendwann mal wer - der Kessel kann dann
//            durch alle diese Flaschen befuellt werden!
#pragma strong_types,rtt_checks

inherit "/std/container";

#include <defines.h>
#include <properties.h>
#include <moving.h>
#include <fishing.h>
#include <items/kraeuter/kraeuter.h>
#include <items/flasche.h>

#ifndef BS
#  define BS(x)             break_string(x, 78)
#endif

int wassermenge;

void create()
{
   ::create();
   SetProp(P_SHORT, "Ein schwerer Kessel");
   SetProp(P_LONG,
     "Ein kleiner aber sehr schwerer Kessel, indem die verschiedensten Traenke "
    +"und Gifte gebraut werden koennen."); // \n werden in long() eingefuegt
   wassermenge=0;
   SetProp(P_NAME, "Kessel");
   SetProp(P_NAME_ADJ, "klein");
   SetProp(P_MATERIAL, MAT_BRONCE);
   SetProp(P_NOGET, "Der Kessel ist zu schwer, um ihn einfach mitnehmen "
      "zu koennen.\n");
   SetProp(P_MAX_WEIGHT, 100000); // ein _wirklich_ grosser Kessel ;o)
   SetProp(P_WEIGHT, 50000);
   SetProp(P_VALUE, 25000);
   SetProp(P_MAX_OBJECTS, 9); // max. 8 Kraeuter + Wasser
   SetProp(P_GENDER, MALE);
   SetProp(P_LIQUID, 3000); // in den Kessel passen 3l :o)
   AddId(({"kessel", KESSELID}));
   AddAdjective(({"klein", "kleiner"}));
   AddCmd(({"leer", "leere"}), "cmd_leeren");
   AddCmd(({"brau", "braue", "koch", "koche"}), "cmd_brauen");
   AddCmd("fuell|fuelle&trank|heiltrank|gifttrank|ergebnis|kesselinhalt|"
    "inhalt&in&@PRESENT", "cmd_fuellen",
	  "Was willst Du fuellen?|Willst Du etwas in etwas fuellen?|"
    "Worein willst Du den Kesselinhalt fuellen?");
   AddCmd(({"tauch", "tauche"}), "cmd_tauchen");
}

#define TRANKFERTIG "_lib_p_krauttrankfertig"

private void clear_kessel()
{
  int si;
  object *inv=all_inventory();
  for (si=sizeof(inv)-1; si>=0; si--) {
    inv[si]->remove();
    if (objectp(inv[si])) destruct(inv[si]);
  }
  SetProp(P_WATER, 0);
  wassermenge=0; // Wasser entleeren...
  SetProp(TRANKFERTIG, 0);
}

int AddWater(int menge)
{
   int old;
   old=wassermenge;
   wassermenge=wassermenge+menge;
   if (wassermenge>QueryProp(P_LIQUID))
     wassermenge=QueryProp(P_LIQUID);
   if (wassermenge<=0)
   {
      wassermenge=0; // wasser entnahme
      SetProp(P_WATER, 0);
   }
   else
      SetProp(P_WATER, W_DEAD);
   return wassermenge-old;
}

static int cmd_leeren(string str)
{
  notify_fail("WAS moechtest Du (aus-)leeren?\n");
  if (!str) return 0;
  if (str[<4..]==" aus") str=str[0..<5];
  if (!id(str)) return 0;
  if (!QueryProp(P_WATER)) {
    write("Im Kessel ist bisher noch keine Fluessigkeit enthalten, die "
      "Du ausleeren\nkoenntest.\n");
    return 1;
  }
  write("Vorsichtig nimmst Du den Kessel und schuettest seinen Inhalt in den Abfluss.\n");
  say(BS(PL->Name(WER)+" nimmt den Kessel und schuettet den Inhalt in den Abfluss."));
  clear_kessel();
  return 1;
}

static int cmd_brauen(string str)
{
  notify_fail("WAS moechtest Du brauen?\n");
  if (!str) return 0;
  if (str=="zaubertrank") {
     write("Ohne passendes Rezept duerfte dies schwierig werden...\n");
     return 1;
  }
  if (member(({"trank", "gift", "heiltrank"}), str)<0) return 0;
  if (!QueryProp(P_WATER)) {
    write("Vielleicht solltest Du zunaechst noch Wasser in den Kessel "
      "fuellen...\n");
    return 1;
  }
  if (wassermenge<QueryProp(P_LIQUID)) {
    write("Vielleicht solltest Du zunaechst noch etwas mehr Wasser in "
      "den Kessel\nfuellen...\n");
    return 1;
  }
  if (sizeof(all_inventory())<3) {
    write("Derzeit ist Dein Trank noch ein wenig waessrig.\n"
         +"Mindestens drei Zutaten muessen in einen Trank schon hinein.\n");
    return 1;
  }
  write("Vorsichtig laesst Du den Kessel etwas naeher zur Feuerstelle "
    "runter und\nwartest unter gelegentlichem Ruehren, bis er kocht.\n"
    "Dein Trank sollte nun fertig sein und Du kannst ihn nun abfuellen.\n"
    "Was er wohl fuer eine Wirkung haben wird?\n");
  say(PL->Name()+" laesst den Kessel zur Feuerstelle herunter und ruehrt "
    "langsam darin herum. Nach einer Weile kocht die Fluessigkeit darin, "
    "und "+PL->Name(WER)+" stellt das Ruehren wieder ein.");
  SetProp(TRANKFERTIG, 1);
  return 1;
}

static int cmd_fuellen(string str,mixed* params)
{
  if (BLUE_NAME(params[2])==TRANKITEM)
  {
    params[2]->Fill(all_inventory());
    write("Du nimmst den Kessel und fuellst seinen konzentrierten Inhalt "
      "in Deine\nGlasflasche. Hoffentlich ist Dir hier ein toller Trank "
      "gelungen.\n");
    say(BS(PL->Name(WER)+" nimmt den Kessel und fuellt seinen konzentrierten "
      "Inhalt in eine kleine Glasflasche. Was "+PL->QueryPronoun(WER)+" da "
      "wohl gebraut hat?"));
    clear_kessel();
  }
  else {
    write("Darein kannst Du den Trank leider nicht fuellen.\n");
  }
  return 1;
}

string long()
{
  string inv_desc = make_invlist(PL, all_inventory(ME));
  if (inv_desc=="") {
    if (QueryProp(P_WATER))
      return BS(Query(P_LONG)+" Derzeit ist er lediglich mit Wasser "
        "gefuellt.");
    return BS(Query(P_LONG)+" Er ist im Moment leer.");
  }
  if (QueryProp(P_WATER))
    return BS(Query(P_LONG)+" Er ist mit Wasser gefuellt, und Du siehst "
              +"folgende Kraeuter in ihm schwimmen:")+inv_desc;
  return BS(Query(P_LONG))+"Er enthaelt:\n"+inv_desc;
}

static int _query_invis()
{
  if (member(({"nimm", "nehm", "nehme", "leg", "lege",
               "steck", "stecke"}), query_verb())!=-1) return 0;
  return Query(P_INVIS, F_VALUE);
}

varargs string name(int casus, int demon)
{
  SetProp(P_INVIS, 0);
  string ret=::name(casus, demon);
  SetProp(P_INVIS, 1);
  return ret;
}

varargs int PreventInsert(object ob)
{
  int plantid = ob->QueryPlantId(); 
  int *inv = all_inventory(ME)->QueryPlantId();
  
  // es koennen natuerlich nur echte Kraeuter in den Kessel gelegt werden
  if ( plantid<=0 || !is_plant(ob) )
    return 1;
  
  if (QueryProp(TRANKFERTIG))
  {
    tell_object(PL, break_string(
      "Im Kessel ist ein fertiger Trank. Wenn Du etwas neues machen "
      "willst, leere den Kessel oder fuelle den Trank ab."));
    return 1;
  }
  else if ( !SECURE("krautmaster")->CanUseIngredient(PL, plantid) ) 
  {
    // mit Kraeutern ueber die man nichts weiss, kann man nicht brauen
    tell_object(PL, BS("Ueber die Wirkungsweise von "+ob->name(WEM)+
      " weisst Du bisher leider wirklich ueberhaupt nichts."));
    return 1;
  }
  else if ( sizeof(inv) == 8 ) 
  {
    tell_object(PL, BS("Mehr als acht Zutaten sollte man nie zu einem "
      "Trank vereinigen, und es sind schon acht im Kessel."));
    return 1;
  }
  else if (member(inv, plantid)>-1) 
  {
    tell_object(PL, BS("Im Kessel befindet sich bereits "+ob->name(WER)+
      ". Du kannst kein Kraut mehr als einmal verwenden."));
    return 1;
  }
  return 0;
}

varargs int PreventLeave(object ob)
{
  if (QueryProp(P_WATER)) {
    tell_object(PL, BS("Es befindet sich bereits Wasser im Kessel, die "
      "einzelnen Zutaten kannst Du nun leider nicht mehr einzeln "
      "rausholen, ohne den ganzen Kessel auszuleeren."));
      return -1;
  }
  return ::PreventLeave(ob);
}
