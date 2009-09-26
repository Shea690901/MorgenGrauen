// (c) by Padreic (Padreic@mg.mud.de)

// Dieses ist ein erstes Objekt das so oder aehnlich tatsaechlich im
// MorgenGrauen vorkommen koennte.

// Jedes Objekt im MorgenGrauen braucht bereits eine solche Vielzahl von
// Standardfunktionen, dass es ziemlich unmoeglich ist, diese alle in jedem
// Objekt zu definieren. Daher "erbt" man diese Funktionen von einem
// Standardobjekt.
// "Erben" bedeutet hierbei, das man auf alle Funktionen, die bereits im
// Standardobjekt definiert sind, zugreifen kann.
//
// Die folgende Zeile erklaert, dass man das Objekt an der Position
//  /std/thing.c "erben" moechte.
inherit "/std/thing";


// Die Funktion folgender Zeile kann man sich recht einfach wie folgt
// vorstellen: Wenn der Game-Driver dieses Objekt hier compiliert, dann
// tut er so, als wuerde die Datei /sys/properties.h genau an dieser Stelle
// hier im Code stehen.  (auch moeglich #include "/sys/properties.h")
//
// Die Datei properties.h beinhaltet einfach nur haufenweise Defines.
// Ein Define waere z.B.: #define WORLD write("Hello World.\n")
// Von nun an kann ich im Code immer WORLD; anstelle von
//  write("Hello World.\n"); schreiben.
// Auch dieser Befehl ist also eigentlich nichts weiter als ein einfaches
// Suchen und Ersetzen, wie man es aus jedem Textverarbeitungsprogramm
// kennt.
//
// Anmerkung: Alle Defines in dieser Datei starten uebrigens mit einem 'P_'.
// Am Besten solltest Du einfach mal kurz in /doc/properties.h reinsehn
// und ein bisschen rumlesen
//  (keine Angst, die muss man nicht alle kennen :).
#include <properties.h>


void create()
{
  // Aehnlich der efun::write() ist auch diese Funktion.
  // Die Funktion create() wurde naemlich bereits in /std/thing.c
  // beschrieben, kann aber nicht mehr ueber create() aufgerufen werden, da
  // dann ja wieder diese Funktion hier aufgerufen wuerde
  //  (wodurch wir dann eine Endlos-Rekursion haetten).
  // Kurzschreibweise waere ::create(); wenn man nur von einem Objekt erbt.
  thing::create(); // oder auch einfach ::create();

  // Auch diese Funktion wurde aus /std/thing geerbt. Da man diese Funktion
  // jedoch praktisch nie ueberschreibt, kann man auch direkt SetProp()
  // anstatt thing::SetProp(); schreiben (Wie Du bereits immer wieder
  //  merkst, kuerzt man ueberall dort, wo es der Uebersichtlichkeit dient,
  //  die Schreibweisen ab).
  // SetProp() ist eine der wichtigsten Funktionen ueberhaupt, wenn es darum
  // geht, ein Objekt zu beschreiben. Mit dieser Funktion kannst Du naemlich
  // zahlreiche Eigenschaften (engl. properties) des Objektes festlegen.
  SetProp(P_SHORT, "Einige wertvolle Juwelen");      // die Kurzbeschreibung
  SetProp(P_LONG, "Die Juwelen glitzern verlockend.\n"); // Langbeschreibung
  SetProp(P_NAME, "Juwelen"); // der eigentliche Name
  SetProp(P_NAME_ADJ, "wertvoll"); // das Adjektiv vor dem Namen
  SetProp(P_GENDER, FEMALE); // Geschlecht (kann auch MALE oder NEUTER sein)
  SetProp(P_WEIGHT, 5000); // dieses Objekt wiegt 5kg
  SetProp(P_VALUE, 10000); // das Objekt ist insgesamt 10000 Muenzen Wert
  
  // Mit dieser Funktion kann man eine id angeben, ueber die das Objekt
  // ansprechbar ist.
  // Anmerkung: Auch diese Funktion ist wieder von std/thing geerbt, im
  //   folgenden werde ich dies aber nicht mehr bei jeder Funktion extra
  //   angeben. Wenn nicht anders angegeben, handelt es sich ab jetzt
  //   naemlich _immer_ um ererbte Funktionen.
  AddId("juwelen");
  
  // In diesem Fall benoetigt man noch ein weiteres Adjektiv, da man das
  // Objekt ja auch mit wertvolle Juwelen ansprechen koennen moechte.
  // z.B.: unt wertvolle juwelen
  //       gib wertvolle juwelen an padreic
  AddAdjective("wertvolle");
}
