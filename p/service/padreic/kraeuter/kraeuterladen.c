// (c) 2003 by Padreic (padreic@mg.mud.de)
// Es kann bestimmte Laeden zum Handeln von Kraeutern geben.
// Zunaechst einmal gibt es einen in der Dunkelelfengilde.
// Hier koennen Kraeuter ge- und verkauft werden.
// Grundsaetzlich kann es beliebig viele Kraeuterkundige
// Haendler geben, eine kurze Absprache waere jedoch von Vorteil.

// Der Laden erweitert die room/shop Funktionen.
// Zur Verwendung muss dieser noch mit einem std/room kombiniert
// werden. Dies erlaubt die Verwendung von eigenen stdraeumen oder
// Kombination mit Kneipen.

inherit "std/room/shop";

#define NEED_PROTOTYPES

#include <properties.h>
#include "../files.h"
#include "./plant.h"

static  int need_save;
static  int basisvalue; // gibt den _Durchschnittswert_ eines Krauts an
static  int maxvalue;   // bestimmt den max. Wert eines Krauts
private int *count;
private int sum;

static string sell_obj_only_plants(object ob, int short);

void create()
{
   int si;
   ::create();
   SetStorageRoom("/p/service/padreic/kraeuter/kraeuterstore");
   need_save=0;
   basisvalue=400;
   maxvalue=10000;
   seteuid(getuid()); // wird zum speichern benötigt
   if (!restore_object(object_name())) {
      count=({});
      sum=0;
   }
   // fuer jede PlantID einmal anlegen...
   si=sizeof(count);
   if (si<PLANTCOUNT) count=count+allocate(PLANTCOUNT-si);

   // reset in der BP abschalten
   if (load_name() == __FILE__[0..<3])
       set_next_reset(-1);
}

static string sell_obj_only_plants(object ob, int short)
// ueberschreibt ein Laden die Funktion sell_obj
// und ruft dort die Funktion sell_obj_only_plants
// auf, so wird automatisch dafuer gesorgt, das dieser
// Laden hier nur Kraeuter kauft.
// Anwendungsbeispiel:
// string sell_obj(ob, short)
// {
//    return ::sell_obj_only_plants(ob, short);
// }
{
   if (!is_plant(ob))
      return "Tut mir leid, damit kann ich nichts anfangen.";
   // es werden nur Kraeuter angekauft, die legal geclont wurden,
   // d.h. im entsprechenden Master ordentlich eingetragen wurden!
   if (ob->QueryPlantId()<=0)
      return ob->Name(WER, 2)+" wurde illegal geclont. Ich darf "
            +ob->QueryPronoun()+" leider nicht annehmen.";
   return ::sell_obj(ob, short);
}

void reset()
{
   ::reset();
   // Daten sind nicht sooo wichtig, als das bei jeder Aenderung
   // gesavet werden muesste. Daher nun im reset speichern.
   if (need_save) {
      if (((sum+PLANTCOUNT)*basisvalue/PLANTCOUNT)>maxvalue) {
         int i, newsum;
         for (i=sizeof(count)-1; i>=0; i--) {
            count[i] = count[i]*9/10;
            newsum+=count[i];
         }
         sum=newsum;
      }
      save_object(object_name());
      need_save=0;
   }
}

static string sell_obj(object ob, int short)
{
   // es werden nur Kraeuter angekauft, die legal geclont wurden,
   // d.h. im entsprechenden Master ordentlich eingetragen wurden!
   if (is_plant(ob) && ob->QueryPlantId()<=0)
      return ob->Name(WER, 2)+" wurde illegal geclont. Ich darf "
            +ob->QueryPronoun()+" leider nicht annehmen.";
   return ::sell_obj(ob, short);   
}

void UpdateCounter(object ob, int num)
{
   int id;
   id=ob->QueryPlantId();
   if (id>0 && is_plant(ob)) {
      // Kauf oder Verkauf von Kraeutern, veraendert den Wert der
      // Kraeuter
      count[id]+=num;
      if (count[id]<0) count[id]=0; // darf aber ansich nich passieren
      sum+=num;
      need_save=1;
   }
   ::UpdateCounter(ob, num);
}

static int realValue(object ob, object player)
// Die Preise die hier im Labor für Kräuter gezahlt und verlangt
// werden, sind nicht fix sondern haengen von Angebot und Nachfrage ab.
// Hier weiss man ueber den wahren Wert der einzelnen Kraeuter bescheid.
{
   int id, val;
   // Preise fuer normale Gueter ganz normal...
   if (!is_plant(ob))
       return ob->QueryProp(P_VALUE);
   // jede Kraeuterkategorie bekommt den gleichen Wert zugewiesen.   
   val=(sum+PLANTCOUNT)*basisvalue/PLANTCOUNT;
   // aber dieser Wert verteilt sich auf unterschiedlich viele Kraeuter
   id=ob->QueryPlantId();
   if (id<=0) return 0; // illegal geclont -> wertlos
   if ((id-1)>sizeof(count)) count=count+allocate(id-1-sizeof(count));
   val=val/(count[id]+1);
   return val-val%10; // Wert runden...
}

static varargs int QueryValue(object ob, int value, object player)
// gibt den Preis zurück, der zum Ankauf des Objektes verwendet werden soll
{
   return ::QueryValue(ob, realValue(ob, player), player);
}

static varargs int QueryBuyValue(mixed ob, object player)
// gibt den Preis an, zu dem das Objekt verkauft werden soll.
{
   if (objectp(ob))
      return (realValue(ob, player)*QueryBuyFact(player) + 50)/100;
   // fixed Objekte...
   return ::QueryBuyValue(ob, player);
}
