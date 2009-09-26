inherit "std/room";

#include <properties.h>
#include <moving.h>
#include <rooms.h>
#include "../files.h"

create()
{
  ::create();
  SetProp(P_LIGHT, 0);
  SetProp(P_INDOORS, 1);
  SetProp(P_INT_SHORT, "Steinebene");
  SetProp(P_INT_LONG,
  "Du bist im Hoehlenlabyrinth der Schreckensspitze. Dunkle Gaenge fuehren\n"+
  "nach Norden, Sueden und Westen. Der suedliche Gang ist etwas merkwuerdig,\n"+
  "denn sein Eingang ist mit glaenzendem Metall seltsam beschlagen.\n");
  AddDetail(({"felsboden", "boden"}), "Der blanke Felsboden geht im Sueden in Metall ueber.\n");
  AddDetail(({"gang", "gaenge"}), "Es sind Gaenge der Orks.\n");
  AddDetail("eingang", "Im Sueden glaenzen die Waende, die mit Metall beschlagen sind.\n");
  AddDetail(({"wand", "waende"}), "Die Waende sind da ... wie immer.\n");
  AddDetail("decke", "Die Decke ist nicht interessant.\n");
  AddDetail("fels", "Grauer Fels.\n");
  AddDetail("metall", "Es scheint Eisen zu sein.\n");
  AddDetail("eisen", "Ja, zweifellos, es ist Eisen.\n");
  AddExit("westen",L1("m4x3"));
  GUARDED_EXIT("norden", L1("m5x2"), "ork",
	       "Der Ork laesst Dich nicht vorbei.\n");
  GUARDED_EXIT("sueden", L1("m5x4"), "ork",
	       "Der Ork laesst Dich nicht vorbei.\n");
  AddItem(MON("orkchef"), REFRESH_REMOVE);
}

