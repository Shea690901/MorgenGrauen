/* Kopie von /items/fackel. Kopie deshalb, weil die Objekte hier
   ggf. auch ne GOD-ID kriegen und in /items/ nicht nur Goetter
   Schreibrechte haben.
   Wenn alle Benutzer umgestellt sind: diese Kopie loeschen.
 */

#pragma strong_types,rtt_checks
#if __BOOT_TIME > 1421010466
#pragma no_clone
#endif

inherit "/std/lightsource";

#include <properties.h>
#include <language.h>

protected void create() {
  lightsource::create();
  AddId(({"fackel","\nfackel"}));
  SetProp(P_NAME,"Fackel");
  SetProp(P_VALUE,5);
  SetProp(P_WEIGHT,1000);
  SetProp(P_GENDER,FEMALE);
  SetProp(P_SHORT,"Eine Fackel");
  SetProp(P_LONG,"Eine ganz normale Fackel. Man kann sie anzuenden und wieder loeschen.\n");
  SetProp(P_ARTICLE,1);
  SetProp(P_FUEL,2000);
  SetProp(P_LIGHTDESC,
	  ({"glimmend","flackernd","leicht flackernd","brennend",
            "hell lodernd","frisch angezuendet"}));
  SetProp(P_LIGHT_TYPE, LT_TORCH);
  SetProp(P_MATERIAL,([MAT_MISC_WOOD:100]));
}
