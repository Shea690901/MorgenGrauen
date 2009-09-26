// (c) 2001 by Padreic (Padreic@mg.mud.de)

#include <properties.h>
#include "../files.h"
#include "/p/service/padreic/kraeuter/plant.h"

inherit PLANT("plant");
void create()
{ 
  ::create(EISZAHN);
  SetProp(P_NAME,     "Eiszahn");
  SetProp(P_NAME_ADJ, "kalt");
  SetProp(P_GENDER,   MALE);
  SetProp(P_LONG,
    "Der Eiszahn ist ein ausgesprochen merkwuerdiges Kraut.\n"
   +"Typisch sind die wie kleine Zaehne angeordneten Blaetter, die\n"
   +"am Hauptstengel entlang wachsen und sich stets zum Himmel recken.\n"
   +"Ausserdem strahlt das Kraut spuerbar Kaelte aus.\n");
  SetProp(PLANT_ROOMDETAIL,
     "Der Eiszahn fuehlt sich auf dem Eisboden sichtlich wohl.\n");
  SetProp(P_SHORT,    "Ein Eiszahn");
  AddId(({ "zahn", "eiszahn" }));
  AddDetail("kraut",
     "Der Eiszahn ist da sehr eigen: Er ist kein Pilz, er ist ein Kraut.\n");
  AddDetail("kaelte", "Huah, ganz schoen kalt.\n");
  AddDetail("blaetter",
    "Die Blaetter sehen aus wie kleine Zaehne. Sie sind ganz weiss und\n"
   +"ordentlich aufgereiht.\n");
  AddDetail("hauptstengel",
     "Eigentlich hat das Kraut nur einen Staengel, der leicht zur Seite haengt\n"
    +"weil die 'Zaehne' ausreichend Gewicht haben.\n");
  AddDetail("zaehne", "So nennt man die Blaetter des Eiszahns.\n");
}