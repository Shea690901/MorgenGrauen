inherit "std/virtual/v_compiler.c";

#pragma strong_types

#include <thing/properties.h>

#define NEED_PROTOTYPES
#include <v_compiler.h>
#undef NEED_PROTOTYPES

#define VCHOME(x)  ("/doc/beispiele/virtual/zesstra/"+x)

protected void create() {
    ::create();

    // jeder Spieler kriegt eine "Kopie" von std_arena als Raum.
    SetProp(P_STD_OBJECT, VCHOME("std_arena"));
    SetProp(P_COMPILER_PATH, VCHOME(""));
}

public string Validate(string file)
{
    string raum, spieler;
    //.c abschneiden
    file=::Validate(file);
    // wenn das gewuenscht file dem Schema "arena|spielername" folgt, ist es
    // zulaessig.
    if(sscanf(file,"%s|%s",raum,spieler)==2 && raum=="arena")
       return file;
    // nicht zulaessig.
    return 0;
}

