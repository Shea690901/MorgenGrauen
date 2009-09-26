/* Paracelsus: Statkrallen
**
**     Beispiel fuer die Verwendung von P_M_ATTR_MOD
**
**     Zieht ein Spieler diese Krallen an, so erhoeht sich seine Staerke
**     um 2. Gleichzeitig wird das Erhoehen seiner Geschicklichkeit durch
**     andere Ruestungen/Waffen blockiert. 
**     Die Krallen koennen nur angezogen werden, wenn weder A_STR noch
**     A_DEX durch eine andere Ruestung/Waffe blockiert wird.
*/

#include <properties.h>
#include <language.h>
#include <combat.h>

inherit "/std/armour";

create()
{
    if (!clonep(this_object())) 
        return;
    ::create();

    SetProp(P_SHORT,"Statkrallen");
    SetProp(P_LONG,
        "Diese krallenbewehrten Handschuhe schimmern blaeulich.\n");
    SetProp(P_NAME,"Statkrallen");
    SetProp(P_INFO,"Die Krallen machen staerker.\n");
    SetProp(P_GENDER,FEMALE);
    SetProp(P_NOBUY,1);
    SetProp(P_WEIGHT,800);
    SetProp(P_VALUE,5000+random(2000));
    SetProp(P_ARMOUR_TYPE,AT_GLOVE);
    SetProp(P_AC,2);
    SetProp(P_EFFECTIVE_WC,15);
    SetProp(P_MATERIAL,
    ([
        MAT_LEATHER    : 60,
        MAT_MISC_METAL : 40
    ]) );

// ---->

    SetProp(P_M_ATTR_MOD,
    ([
        A_STR : 2,  // Staerke um 2 erhoehen
        A_DEX : 0   // Geschicklichkeit blockieren
    ]) );

// <----

    AddId( ({"handschuhe","krallen"}) );

    AddDetail( "schimmer",
        "Ein blaeulicher Schimmer liegt auf den Krallen.\n");
}
