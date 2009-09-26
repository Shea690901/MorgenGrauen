#include "/p/service/padreic/kraeuter/plant.h"
#include "../files.h"

inherit "/p/service/padreic/kraeuter/plant";

void create()
{
   ::create(WASSER_HAHNENFUSS_WEISS);
}

varargs int move(mixed dest, int method)
{
   // die Einwohner des Zauberwalds sehen es nicht gerne, wenn jemand ihre
   // magischen Pflanzen pflueckt...
   int res;
   res=(int)::move(dest, method);
   if (res!=1) return res;
   if (environment()) environment()->SetProp(ZAUBERWALD, time()+AGGRESSIVE_TIME);
   return 1;
}
