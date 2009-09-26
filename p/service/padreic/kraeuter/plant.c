// (c) September 2000 by Padreic (Padreic@mg.mud.de)

inherit "std/thing";

#include <properties.h>
#include "../files.h"

#define PLANT_LIFETIME (24*3600)
#define DRIED_PLANT    -1

int age=time();
private static int plantId;
private static string cloner;

int PlantQuality();       /* gibt die Qualitaet der Pflanze in % an  */
void DryPlant(int qual);  /* verhindert ein verschimmeln der Pflanze */
int TimeToLive();         /* Sekunden bis zum schimmeln... */
nomask int SetPlantId(int id);
nomask int QueryPlantId();

// optional kann man als Parameter an create() uebergeben, von welchem
// Kraut man die Eigenschaften erben will. Dies kann nuetzlich sein, wenn
// man das Objekt noch leicht modifizieren will.
varargs create(string file)
{
   // auch die Blueprint _muss_ initialisiert sein!
   thing::create();
   plantId=0;
   SetProp(P_QUALITY, 100);
   SetProp(P_WEIGHT, 120);
   SetProp(P_VALUE, 70);
   // _unbedingt_ umsetzen bei Zutaten die keine Pflanzen sind!
   SetProp(P_MATERIAL, MAT_MISC_PLANT);
   // hier darf _nicht_ previous_object()->CustomizeObject() verwandt werden,
   // da nur die Blueprint wirklich vom VC geclont wird. Fuer die Clones
   // ruft der Master den VC nicht jedesmal erneut auf.
   if (stringp(file)) file=explode(file, "/")[<1];
   call_other(KRAEUTERVC, "CustomizeObject", file);
}

string short()
{
   string str;
   if (!stringp(str=QueryProp(P_SHORT))) return 0;
   if (age==DRIED_PLANT)
      return str+" (getrocknet).\n";
   else if (age+PLANT_LIFETIME<time())
      return str+" (verschimmelt).\n";
   return str+".\n";
}

int PlantQuality()
// durch ueberschreiben von PlantQuality ist es moeglich, Besonderheiten einer
// Pflanze zu implementieren. z.B. volle Wirkung nur bei Vollmond oder wenn
// sie richtig gepflueckt wurde, ansonsten negative Wirkung.
// 0    - keine Wirkung mehr
// > 0  - Wirkung in Prozent
// < 0  - neg. Wirkung auf die Gesundheit in Prozent
{
   int qual;
   if (age+PLANT_LIFETIME < time()) {
      qual=negate((time()-age-PLANT_LIFETIME)/360);
      return (qual < -100 ? -100 : qual);
   }
   return QueryProp(P_QUALITY);
}

int TimeToLive()
{  return age-time()+PLANT_LIFETIME;  }

static int _query_value()
{
   int val;
   if (plantId<=0)
      return 0;
   val=Query(P_VALUE)*PlantQuality()/100;
   if (val<0) return 0;
   return val-val%10;
}

void DryPlant(int qual)
// mit DryPlant kann die Pflanze getrocknet werden, sie kann dann nicht mehr
// schimmeln. Hierbei sollte die Qualitaet der Pflanze jedoch leiden, <qual>
// gibt hierbei in Prozent an, wie stark die Pflanze dabei leidet.
// Diese Funktion kann natuerlich auch ueberschrieben werden, wenn bestimmte
// Kraeuter erst durch trocknen in der Qualitaet steigen.
{
   age=DRIED_PLANT;
   SetProp(P_QUALITY, (QueryProp(P_QUALITY)*qual)/100);
}

/* Funktionen zum initialisieren der Pflanze */

nomask int SetPlantId(int id)
{
   int res;
   if (plantId) return -1;
   cloner=0; age=time();
   if (catch(cloner=call_other(PLANTMASTER, "CheckPlant", id)) || !cloner) id=-1;
   return (plantId=id);
}

nomask int QueryPlantId()
{  return plantId;  }

nomask string QueryCloner()
{  return cloner;   }
