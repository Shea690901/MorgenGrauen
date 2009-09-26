// (c) 2003 by Padreic (padreic@mg.mud.de)
// Stdstore fuer Kraeuterlaeden!

inherit "std/store";

#include <properties.h>
#include <bank.h>

void create()
{
   ::create();
   SetProp(P_MIN_STOCK, 50);
   // in jedem Reset verschwinden nur 2% aller Objekte
   SetProp(P_STORE_CONSUME, 2);
}

void reset()
{
   int i;
   object *obs;
   ::reset();
   obs=all_inventory();
   // damit die Kraeuter im Laden nicht schimmeln, ggf. trocknen...
   for (i=sizeof(obs)-1; i>=0; i--) {
      if (call_other(obs[i], "TimeToLive") < 3600) {
         call_other(obs[i], "DryPlant", 95);
      }
   }
}