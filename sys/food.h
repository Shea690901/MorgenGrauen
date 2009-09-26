/* -*- lpc -*- */
//
//  /\/\/\/\/\/\/\/\/\ --> Please read me carefully! <-- /\/\/\/\/\/\/\/\/\
//
//  Heilung -- Essen & Trinken (food.c)
//
//  "Projekt Essen" -- Idee und Konzept:     Anthea@mg.mud.de
//                     Technische Umsetzung: Anthea@mg.mud.de und
//                                           Gabylon@mg.mud.de
//
//   Weitergabe und Kopien aus diesem File oder Teilen davon sind erlaubt.
//
//       ---> Veraenderungen des Originalfiles sind NICHT erlaubt! <---
//                    (Ausgenommen hiervon: Bugfixes)
//
//   29.08.00, Anthea:  Objekt erstellt
//   07.08.03, Gabylon: Props umbenannt, Kosmetik
//
//  Kurzbeschreibung:
//  ----------------
//  Das ist das Headerfile fuer /std/food.c
//
//  Dank an Anthea, von der diese Idee stammt und mir freundlicherweise das
//  File zur Verfuegung gestellt hat!
//
//  /\/\/\/\/\/\/\/\/\/\/\/\/\ --> Thank You! <-- /\/\/\/\/\/\/\/\/\/\/\/\/\

#include <properties.h>

//#define   TP	   this_player()
//#define   TO       this_object()

//#define   ENV(x)   environment(x)

//#define   BS(x)    break_string(x, 78 , "", BS_LEAVE_MY_LFS)

//--------------------------------------------------------------------------
// FOOD-PROPERTIES - #defines
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//   F_IS_FOOD
//   Es ist was zum Essen. Aktion: "iss"
#define   F_IS_FOOD            "is_food"

//--------------------------------------------------------------------------
//   F_IS_DRINK
//   Es ist was zum Trinken. Aktion: "trink", "trinke"
#define   F_IS_DRINK           "is_drink"

// ACHTUNG: F_IS_FOOD UND F_IS_DRINK SCHLIESSEN SICH GEGENSEITIG AUS!!!
//          DAS HEISST, WENN F_IS_FOOD GESETZT IST, WIRD F_IS_DRINK GELOESCHT
//          UND UMGEKEHRT!

//--------------------------------------------------------------------------
//   F_IS_FULL
//   Testen, ob das Ding noch voll ist.
#define   F_IS_FULL            "is_full"

//--------------------------------------------------------------------------
//   F_MESSAGE
//   Die Meldung, die die Umstehenden erhalten, wenn gegessen/getrunken wird.
#define   F_MESSAGE            "mess"

//--------------------------------------------------------------------------
//   F_EATER
//   Die Meldung, die derjenige kriegt, der das Ding isst.
#define   F_EATER              "eater"

//--------------------------------------------------------------------------
//   F_EMPTY_CONTAINER
//   Wenn man das aufgegessen/ausgetrunken hat, wird die Short-Description
//   umgesetzt, so dass z.B. aus einer Flasche Cola eine Colaflasche wird.
#define   F_EMPTY_CONTAINER    "empty_con"

//--------------------------------------------------------------------------
//   F_EMPTY_GENDER
//   Das leere Gefaess muss ja nicht das selbe Geschlecht haben wie das volle.
#define   F_EMPTY_GENDER       "empty_gender"

//--------------------------------------------------------------------------
//   F_EMPTY_ID
//   Eine ID fuer das leere Gefaess, damit es auch irgendwie angesprochen
//   werden kann.
#define   F_EMPTY_ID           "empty_id"

//--------------------------------------------------------------------------
//   F_EMPTY_LONG
//   Eine Long-Description, wenn das Ding leer ist.
#define   F_EMPTY_LONG         "empty_long"

//--------------------------------------------------------------------------
//   F_ALC
//   Die Menge an Alkohol, die das Getraenk hat.
//   Wert muss zwischen 1 und 100 liegen (respektive 1% und 100%).
#define   F_ALC                "alc"

//--------------------------------------------------------------------------
//   F_LIQUID
//   Die Menge an Fluessigkeit, die das Getraenk hat.
#define   F_LIQUID             "water"

//--------------------------------------------------------------------------
//   F_FOOD_SIZE
//   Die Groesse der Speise.
#define   F_FOOD_SIZE          "food_size"

//--------------------------------------------------------------------------
//   F_POTION
//   Wie oft kann man abbeissen/trinken?
#define   F_POTION             "potion"

//--------------------------------------------------------------------------
//   F_HEAL
//   Soll die Mahlzeit eine Zusaetzliche Heilwirkung haben?
#define   F_HEAL_SP               "heal_sp"
#define   F_HEAL_HP               "heal_hp"

//--------------------------------------------------------------------------
//   F_NO_CONTAINER
//   Wenn diese Property auf 1 ist, wird das Ding nach dem Essen/Trinken
//   zerstoert, bei 0 erhaelt man das F_EMPTY_CONTAINER.
#define   F_NO_CONTAINER       "no_con"

//--------------------------------------------------------------------------
//   F_BAD
//   Falls das Essen verdorben ist, wird P_POISON im Spieler gesetzt in
//   Abhaengigkeit von F_FOOD_SIZE.
#define   F_BAD                "bad"

//--------------------------------------------------------------------------
//   F_POISON
//   Nun kann man endlich giftiges Essen bauen!
#define   F_POISON             "poison"
