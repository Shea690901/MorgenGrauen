// MorgenGrauen MUDlib
//
// fishing.h -- Definitionen fuer alles, was sich ums Angeln dreht
//              Fragen, Vorschlaege etc. an Fraggle
//
// $Id: fishing.h 5292 2006-09-23 10:28:04Z root $

// Revision 1.2  19.4.95  Strings von P_WATER und P_FISH geaendert (Wargon)
// Revision 1.1  9.3.95  new: ASTD()  changed: STDANGEL STDFISH (Fraggle)
// Revision 1.0  created by Fraggle

#ifndef _FISHING_
#define _FISHING_

#define FISH(x) "/d/ebene/fraggle/aquarium/"+x

 // Der Ort des Aquariums, hier befinden sich die von Fraggle
 // programmierten Fische.

//standard objects (zum inheriten) :

#define STDANGEL "/d/ebene/fraggle/std/angel"
#define STDFISH  "/d/ebene/fraggle/std/fish"
#define ASTD(x)  "/d/ebene/fraggle/std/"+x

// Bsp: inherit ASTD("flasche");
//      inherit ASTD("haken");

/* *** PROPERTIES *** */

#define P_WATER  "water"

 //RAUM:
 //legt den Typ des Gewaesser fest, MUSS gesetzt sein, wenn Angeln moeglich
 //sein soll. Flags siehe unten.
 // z.B: SetProp(P_WATER,P_HARBOR) fuer einen Hafen!
 // NUR EIN FLAG DARF GESETZT SEIN!

 //ANGEL:
 // in der Angel koennen dagegen mehrere dieser Flags gesetzt sein:
 // SetProp(P_WATER,P_HARBOR|P_OCEAN) heisst, die Angel ist sowohl
 // im Hafen als auch auf offener See gut zu gebrauchen!

 //WASSERBEHAELTER
 // P_WATER gibt hier lediglich an, ob der Behaelter Wasser enthaelt
 // oder nicht. Der Wert sollte fuer spaetere Erweiterungen trotzdem
 // immer dem Wert entsprechen, woher das Wasser geholt wurde.

#define P_FISH   "fish"

 // RAUM:
 // Dichte der Fische im Gewaesser, OPTIONAL, 0=100%, 100=200%
 // 0 sollte normal sein (und ist default), bei starkem Fischbestand
 // kann positiver Wert <=100, bei geringem Bestand negativer Wert genommen
 // werden. aber Achtung! nicht zu klein, -600000 hiesse, der Angler
 // muesste 600.000 sekunden (plus ein paar zerdrueckte) warten!

 // KOEDER:
 // ein Koeder kann damit die Fischdichte (siehe RAUM)
 // um diesen Wert erhoehen oder veringern.
 // d.h.  Raum = -100 ,Koeder = +100 => Dichte=0

 // FISCH:
 // Das Property kann einige Zusaetzliche Eigenschaften festlegen.

#define P_LIQUID     "w_max_wasserfuellmenge"

 // Fassungsvermoegen des Behaelters in ml

#define P_LONG_EMPTY "w_longdesc_empty"

 // P_LONG fuer leeren Zustand der Stdflasche
 // Zusatz zu P_LONG fuer leeren Zustand des Stdhakens

#define P_LONG_FULL  "w_longdesc_full"

 // P_LONG fuer gefuellten Zustand der Stdflasche
 // Zur Erklaerung siehe am besten /d/ebene/fraggle/txt/flasche.txt

/* *** FLAGS *** */

// Gewaessertyp-Flags

#define W_BEACH     1    // Strand,Salzwasser, Fische: Scholle ...
#define W_HARBOR    2    // Hafen, Salzwasser, Fische: Dorsch ...
#define W_OCEAN     4    // Ozean,Meer,Salzwasser      Hai,Thunfisch...
#define W_RIVER     8    // Fluss,Suesswasser          Forelle...
#define W_POOL     16    // Teich,Suesswasser          Stichling...
#define W_LAKE     32    // See, Suesswasser           Karpfen...
#define W_ROCK     64    // Bergbach, nur Lachs+Forelle
#define W_STREAM   256   // Bach,Suesswasser
#define W_USER     128   // hier kann man ein eigenes Aquarium anhaengen!
#define W_DEAD     512   // Lebloses Wasser/enthaelt keine Fische/man kann
                         // aber die Stdflasche fuellen.

#define W_OTHER   1024   // Flasche enthaelt Fluessigkeit!=Wasser


 // in einem Raum mit SetProp(P_WATER,W_USER) MUSS auch eine Funktion
 // GetAquarium() { return fishlist; } enthalten sein,
 // fishlist ist dabei ein Stringarray mit den kompletten Filenamen
 // der Fische, Bsp.:
 // GetAquarium(){  return({"/d/ebene/fraggle/aquarium/fish"}); }

#define W_SHORT W_HARBOR|W_RIVER|W_POOL|W_LAKE|W_ROCK|W_USER|W_OCEAN|W_STREAM
#define W_LONG  W_BEACH|W_USER
#define W_SALT  W_HARBOR|W_OCEAN|W_BEACH              // Salzwasser
#define W_SWEET W_RIVER|W_POOL|W_LAKE|W_ROCK|W_STREAM  // Suesswasser

//Achtung! W_DEAD,W_USER ist nicht in den Kombi-definitionen enthalten!

// W_SHORT  diese Flags sollten bei kurzen Angeln gesetzt sein
// W_LONG   sollte man bei speziellen Strandruten setzen
// ACHTUNG: W_USER sollte immer gesetzt sein! Du weisst ja nicht,
//          was fuer ein Gewaesser hier vorliegt. In diesem Speziellen
//          Fall ist es das Problem des Raum-Programmierers,
//          Die Fishdichte zu bestimmen!

// Flags fuer Fische:
#define F_NOROTTEN    1   // Fisch fault nicht.
#define F_NOTHUNGRY   2   // isst Koeder nicht auf, falls zu schwer fuer Angel
#define F_REPLACE     4   // Soll sich beim Entfernen von Angel verwandeln.
#define F_NOHEAL      8   // Fisch heilt nicht bei Verzehr
// ID's fuer Angel,Haken,Wurm,Fisch

#define ANGEL_ID  "\nf_angel"
#define HAKEN_ID "\nf_koeder"
#define WURM_ID   "\nf_worm"
#define FISCH_ID   "\nf_fish"

#endif
