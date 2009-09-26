inherit "std/room";

#pragma strong_types
#include <properties.h>

void create()
{
    if(!clonep(TO))
        return;
    ::create();

    SetProp(P_INT_LONG, break_string(
	  "Du stehst hier in einer voellig leeren Arena."
	  "Gegen wen solltest Du hier denn gleich kaempfen...?",78));
    SetProp(P_INT_SHORT, "In einer Arena");
    SetProp(P_LIGHT, 1);
    SetProp(P_INDOORS,0);
    
    AddExit("raus", "/gilden/abenteurer");
}
