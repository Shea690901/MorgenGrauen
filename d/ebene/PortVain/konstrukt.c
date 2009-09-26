#include <rooms.h>
#include <properties.h>

#define BS(x) break_string(x,78)

inherit "std/room";

create() {
	::create();
	SetProp(P_INT_LONG,BS("Du befindest Dich im \"Konstrukt\", einem weissen Nichts eines jungfraeulichen Universums. Dieser Ort wartet darauf, dass Du ihn mit Deiner Phantasie auffuellst. Von hier aus kann man wohl durch blossen Willen andere Orte erreichen, die es in diesem jungen Universum schon gibt.\n"));
	SetProp(P_INT_SHORT,"Im Konstrukt");
	SetProp(P_LIGHT,1);
	AddDetail(({"konstrukt","nichts"}),"Es ist weiss - und sonst Nichts.\n");
	AddDetail("universum","Es liegt dir zu Fuessen.\n");
	AddDetail(({"fuss","fuesse"}),"Deine Fuesse.\n");
	AddDetail("phantasie","Sie schlummert in dir. Es ist an dir, ihr Gestalt zu geben.\n");
	AddExit("bsp","/doc/beispiele/misc/bspraum1");
	AddExit("gilde","/gilden/abenteurer");
	AddExit("ssp","/doc/beispiele/ssp/l1/m3x1");
	AddExit("zauberwald","/doc/beispiele/zauberwald/room/eingang");
}
