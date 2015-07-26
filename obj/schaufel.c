/* Kopie von /items/fackel. Kopie deshalb, weil die Objekte hier
   ggf. auch ne GOD-ID kriegen und in /items/ nicht nur Goetter
   Schreibrechte haben.
   Wenn alle Benutzer umgestellt sind: diese Kopie loeschen.
 */
#if __BOOT_TIME > 1421010466
#pragma no_clone
#endif
#pragma strong_types,rtt_checks

#include <properties.h>
#include <language.h>
inherit "/std/thing";

protected void create()
{
  ::create();
  SetProp(P_SHORT,"Eine Schaufel");
  SetProp(P_LONG,"Du kannst versuchen, mit dieser Schaufel zu graben.\n");
  AddId("schaufel");
  AddId("\nschaufel");
  SetProp(P_NAME, "Schaufel");
  SetProp(P_GENDER, FEMALE);
  SetProp(P_VALUE, 50);
  SetProp(P_WEIGHT, 1200);
	AddCmd( ({"grab","grabe","buddel","buddele"}), "grab" );
}

int grab(string str) 
{
  _notify_fail("Du kannst hier nicht graben.\n");
  return 0;
}
