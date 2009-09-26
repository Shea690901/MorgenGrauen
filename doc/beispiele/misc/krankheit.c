/*  Paracelsus: Eine Krankheit
**
**      Beispiel fuer die Verwendung von P_X_ATTR_MOD und P_X_HEALTH_MOD
**
**      Diese Krankheit setzt alle Attribute herab und erniedrigt
**      zusaetzlich P_MAX_HP und P_MAX_SP um jew. 20 (zusaetzlich zu den
**      Auswirkungen der Attributsenkungen).
**      Gestartet wird die Krankheit, indem sie einfach in das Opfer
**      bewegt wird.
**      Man beachte, dass im remove() die P_X_*-Properties ein leeres
**      Mapping zugewiesen bekommen und keine 0. Nur so werden die
**      Krankheitsfolgen rueckgaengig gemacht.
*/

#include <properties.h>
#include <moving.h>
#include <language.h>
#include <class.h>

inherit "/std/thing";

create()
{
    if (!clonep(this_object()))
        return;
    ::create();

    SetProp(P_SHORT,0);
    SetProp(P_LONG,0);
    SetProp(P_NAME,"Krankheit");
    SetProp(P_GENDER,FEMALE);
    SetProp(P_WEIGHT,0);
    SetProp(P_VALUE,0);
    SetProp(P_MATERIAL,MAT_MISC_MAGIC);
    SetProp(P_NODROP,1);
    SetProp(P_NEVERDROP,1); // Wirkt nach Tod weiter

// ---->

    SetProp(P_X_HEALTH_MOD,
    ([
        P_HP : -20,   // Max. Lebenspunkte um 20 runter
        P_SP : -20    // Max. Konzentrationspunkte um 20 runter
    ]) );
    SetProp(P_X_ATTR_MOD,
    ([
        A_CON : -1,   // Ausdauer um 1 runter, reduziert auch max. LP!
        A_DEX : -1,   // Geschicklichkeit um 1 runter
        A_INT : -2,   // Intelligenz um 2 runter, reduziert auch max. KP!
        A_STR : -4,   // Staerke um 4 runter
    ]) );

// <----

    AddId( ({"PARA\nSICK\nEXAMPLE"}) );

    AddClass(CL_DISEASE); // Damit Kleriker helfen koennen.
    SetProp(P_LEVEL,15);  // Aber nicht ganz so einfach zu entfernen
}

// Diese Krankheit kann nicht bewegt werden, wenn sie erst einmal im
// Zielobjekt ist. Das hat den Vorteil *grins*, dass der Spieler sie
// auch ueber den Tod hinaus behaelt.
varargs int move(mixed dest,int method) 
{
    if (environment())
        return ME_CANT_LEAVE_ENV;
    return (int)::move(dest,method);
}

varargs int remove() 
{
    SetProp(P_X_ATTR_MOD,([]));   // Wichtig! Leeres Mapping setzen,
    SetProp(P_X_HEALTH_MOD,([])); // und nicht 0!
    return (int)::remove();
}
