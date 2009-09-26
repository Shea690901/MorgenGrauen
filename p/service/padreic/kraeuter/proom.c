#define NEED_PROTOTYPES

#include <thing.h>
#include <moving.h>
#include <defines.h>
#include "../files.h"

#define STD_WACHSTUM  7200
#define BS(x)         break_string(x, 78)

mapping plantMap;

void AddDetail(mixed *keys, mixed descr);

// kann benutzt werden um schnell und einfach eine Pflanze in einem Raum
// hinzuzufuegen. Beispiel: AddPlant(BAERENKLAU);
// Diese Funktion erzeugt automatisch ein AddCmd() fuer das Pfluecken und
// (falls noch nicht vorhanden) Details fuer die Pflanze.
varargs int AddPlant(string filename, string npcId)
{
   int i,j,gender;
   object ob;
   string *ids, *adj, *det;
   mixed arr;

   if (!stringp(filename)) return -1;
   call_other(filename, "???");
   ob=find_object(filename);
   if (!objectp(ob)) raise_error("AddPlant(): Can't load "+filename+".\n");
   if (!mappingp(plantMap)) plantMap=([]);
   if (pointerp(arr=plantMap[filename]) && sizeof(arr)>2)
      raise_error("AddPlant(): "+filename+" already exists.\n");
   ids=ob->Query(P_IDS)-({ "Ding" });
   adj=ob->Query(P_ADJECTIVES);
   if (!pointerp(arr) || sizeof(arr)<2) arr=({0,0});
   plantMap[filename]=({arr[0], arr[1], ids, adj, ob->Name(WER, 1), npcId });
   det=({});
   adj=ob->QueryProp(P_NAME_ADJ);
   gender=Query(P_GENDER);
   Set(P_GENDER, ob->Query(P_GENDER));
   adj=map(adj, #'DeclAdj, WEN, 0);
   for (i=sizeof(ids)-1; i>=0; i--) {
      for (j=sizeof(adj)-1; j>=0; j--) det+=({ adj[j]+ids[i] });
   }
   det+=ids;
   det-=m_indices(Query(P_DETAILS)||([]));
   if (sizeof(det)) AddDetail(det, ob->Query(PLANT_ROOMDETAIL));
   AddCmd(({"pflueck", "pfluecke"}), "_pfluecken");
}

// Wenn jemand per Hand das Plantdetail hinzufuegen moechte...
// z.B. bei Verwendung von GetPlant() anstelle von AddPlant()
string AddPlantDetail(string filename)
{
   int i,j,gender;
   object ob;
   string *ids, *det, *adj;
   call_other(filename, "???");
   ob=find_object(filename);
   if (!objectp(ob)) raise_error("AddPlantDetail(): Can't load "+filename+".\n");
   det=({});
   ids=ob->Query(P_IDS)-({ "Ding" });
   adj=ob->QueryProp(P_NAME_ADJ);
   gender=Query(P_GENDER);
   Set(P_GENDER, ob->Query(P_GENDER));
   adj=map(adj, #'DeclAdj, WEN, 0);
   for (i=sizeof(ids)-1; i>=0; i--) {
      for (j=sizeof(adj)-1; j>=0; j--) det+=({ adj[j]+ids[i] });
   }
   AddDetail(det+ids, ob->Query(PLANT_ROOMDETAIL));
   Set(P_GENDER, gender);
}

int CheckPlant(string filename)
{
   mixed arr;
   if (!mappingp(plantMap)) plantMap=([]);
   if (!pointerp(arr=plantMap[filename]) || sizeof(arr)<2) {
      arr=plantMap[filename]=({ 0, 0 });
   }
   if (time()<arr[0]) return 0; // noch nicht nachgewachsen
   return 1;
}

// Moechte man AddPlant() nicht benutzen, weil man die Pflanze nicht einfach
// pfluecken, sondern vielleicht abschneiden, oder ausgraben soll, so kann
// man sich mittels GetPlant(filename) das Objekt erzeugen lassen. Gibt
// GetPlant() 0 zurueck, ist die Pflanze noch nicht wieder weit genug
// nachgewachsen.
object GetPlant(string filename)
{
   mixed arr;
   if (!mappingp(plantMap)) plantMap=([]);
   if (!pointerp(arr=plantMap[filename]) || sizeof(arr)<2) {
      arr=plantMap[filename]=({ 0, 0 });
   }
   if (time()<arr[0]) return 0; // noch nicht nachgewachsen
   arr[1]-=time();
   arr[1]=(arr[1]<=0 ? STD_WACHSTUM : (arr[1]<<1)+STD_WACHSTUM);
   arr[0]=time()+(arr[1]>>1); // Resetzeit
   arr[1]+=time();            // Zeit voelliger Regeneration
   return clone_object(filename);
}

static int _pfluecken(string str)
{
   int i, res;
   string *keys;
   mixed ids, adj;
   object ob;

   if (!mappingp(plantMap)) return 0;
   _notify_fail("WAS moechtest Du pfluecken?\n");
   ids=Query(P_IDS);
   adj=Query(P_ADJECTIVES);
   for (i=sizeof(keys=m_indices(plantMap))-1; i>=0; i--) {
      if (sizeof(plantMap[keys[i]])!=6) continue;
      Set(P_IDS, plantMap[keys[i]][2]);
      Set(P_ADJECTIVES, plantMap[keys[i]][3]);
      if (!catch(res=id(str)) && res) {
         if (stringp(str=plantMap[keys[i]][5]) &&
             objectp(ob=present(str, ME))) {
             write(BS(ob->Name(WER, 2)
                 +" laesst Dich leider nicht nah genug heran. Irgendwie"
                 +" musst Du Dich wohl zunaechst um "
                 +ob->QueryPronoun(WEN)+" kuemmern."));
         }
         else if (objectp(ob=GetPlant(keys[i]))) {
            if (ob->move(PL, M_GET)==1)
               write(BS("Vorsichtig pflueckst Du "+ob->name(WEN, 1)
                       +" und nimmst "+ob->QueryPronoun(WEN)+" an Dich."));
            else {
               write(BS("Vorsichtig pflueckst Du "+ob->name(WEN, 1)+" kannst "
                        +ob->QueryPronoun(WEN)+" aber nicht nehmen."));
               ob->move(environment(PL), M_SILENT|M_NOCHECK);
            }
         }
         else write(BS(plantMap[keys[i]][4]+" ist noch nicht reif genug "
                      +"und muss erst noch etwas wachsen."));
         break;
      }
   }
   Set(P_IDS, ids);
   Set(P_ADJECTIVES, adj);
   return (i>=0);
}
