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
//   07.08.03, Gabylon: Code optimiert, F_POTION eingefuehrt, Kosmetik
//   09.10.06  Zesstra: Defines entfernt, die auch in /sys/food.h stehen.
//
//  Kurzbeschreibung:
//  ----------------
//  Das ist ein File fuer Essen und Trinken. Um es zu verwenden, einfach ein
//
//  -> inherit "/std/food";
//
//  an den Fileanfang setzten.
//
//  Ausserdem sollte noch ein
//
//  -> #include <food.h>
//
//  gesetzt werden.
//  vanion@chaosburg.de
//  Dank an Anthea, von der diese Idee stammt und mir freundlicherweise das
//  Originalfile zur Verfuegung gestellt hat!
//
//  /\/\/\/\/\/\/\/\/\/\/\/\/\ --> Thank You! <-- /\/\/\/\/\/\/\/\/\/\/\/\/\
// $Id: food.c 6988 2008-08-22 21:38:59Z Zesstra $
#pragma strong_types
#pragma save_types
#pragma no_clone
#pragma pedantic
#pragma range_check

inherit "std/thing";
#include <food.h>
#include <defines.h>
#include <properties.h>

// TODO: Variablen hier nach private aendern.

// alcohol, alc pro portion, essen, essen pro portion, heilung,
// gift, soft, soft pro portion
nosave int alc, alc_pp, food, food_pp, heal, p, soft, soft_pp;

// runden-counter
nosave int c=0;


void create()
{
  ::create();

  SetProp(P_SHORT,"Was zu essen");
  SetProp(P_LONG,"Was zu essen.\n");
  SetProp(P_NAME,"Was zu essen");
  SetProp(P_GENDER,NEUTER);
  SetProp(P_WEIGHT,50);
  SetProp(P_VALUE,10);
  SetProp(P_MATERIAL,([MAT_MISC_FOOD:100]));
//  Das F_MESSAGE,"isst etwas auf.");
  SetProp(F_IS_FOOD,1);
  SetProp(F_IS_DRINK,0);
  SetProp(F_IS_FULL,1);
  SetProp(F_ALC,0);
  SetProp(F_LIQUID,0);
  SetProp(F_FOOD_SIZE,5);
  SetProp(F_HEAL_SP,0);
  SetProp(F_HEAL_HP,0);
  SetProp(F_EMPTY_CONTAINER,"Leerer Teller");
  SetProp(F_EMPTY_GENDER,1);
  SetProp(F_NO_CONTAINER,1);
  SetProp(F_BAD,0);
  SetProp(F_POISON,0);
  SetProp(F_POTION,1);

  AddId("essen");

  AddCmd(({"iss"}),"eat_this");
}


//--------------------------------------------------------------------------
//
//   int eat_this( string str )
//
//   Kommando zum Essen.
//--------------------------------------------------------------------------
int eat_this( string str )
{
  notify_fail("WAS moechtest Du essen?\n");
  if ( !str || !id(str) )
   return 0;

  if ( !QueryProp(F_IS_FOOD) )
   { write(("Das kann man nicht essen.\n")); return 1; }

  if ( !QueryProp(F_IS_FULL) || !QueryProp(F_POTION) )
   { write(("Da ist nichts mehr drin.\n")); return 1; }

  if ( QueryProp(F_POTION) > QueryProp(F_FOOD_SIZE) )
   SetProp(F_POTION,QueryProp(F_FOOD_SIZE));

// Auch Essen kann Alkohol enthalten, huhu Rochus :)
  alc=QueryProp(F_ALC);
  food=QueryProp(F_FOOD_SIZE);

  if ( alc < 0 )
   alc=0;

  if ( alc > 100 )
   alc=100;

  if ( food < 1 )
   food=1;

  alc_pp=alc/(QueryProp(F_POTION)+c);
  food_pp=food/(QueryProp(F_POTION)+c);

// Falls die Portionen nicht integer-glatt sind, wird in der letzten Runde
// die Differenz aufaddiert. *hick* !
  if ( QueryProp(F_POTION) == 1 )
   {
    if ( alc_pp*(c+1) != alc )
     alc_pp+=alc-alc_pp*(c+1);

    if ( food_pp*(c+1) != food )
     food_pp+=food-food_pp*(c+1);
   }

// write(BS("Alc_pp: "+alc_pp));
// write(BS("Food_pp: "+food_pp+"\n\n"));

  if ( food_pp < 1 )
   food_pp=1;

  if ( !this_player()->eat_food(food_pp) )
   return 1;

  if ( alc > 0 && alc_pp < 1 )
   alc_pp=1;

  if ( alc > 0 )
   if ( !this_player()->drink_alcohol(alc_pp) )
    return 1;

// --- Erfolg ---

  c++;

  p=QueryProp(F_POISON);

  if ( p < 0 )
   p=0;

  if ( p > 10 )
   p=10;

  if ( this_player()->SetProp(P_POISON,this_player()->QueryProp(P_POISON)+p) > 10 );

  if ( QueryProp(F_EATER) ) {
    tell_object(PL,break_string("Du "+QueryProp(F_EATER),78,0,
	  BS_LEAVE_MY_LFS));
  }
  else {
    tell_object(PL, break_string("Du isst "+QueryProp(P_SHORT)+".",78)); 
  }

  if ( QueryProp(F_MESSAGE) ) {
    tell_room(environment(PL),
	break_string(this_player()->name()+" "+QueryProp(F_MESSAGE),78,0,
	BS_LEAVE_MY_LFS),({PL}));
  }
  else {
    tell_room(environment(PL),
	break_string(this_player()->name()+" isst " 
	  + QueryProp(P_SHORT)+".",78),({PL}));
  }

  SetProp(F_POTION,QueryProp(F_POTION)-1);

  switch ( QueryProp(F_BAD) )
   {
   case 0:                           // Heilung
     if ( QueryProp(F_HEAL_SP) > 0 )
      {
       heal=QueryProp(F_HEAL_SP)/5;
       if ( heal < 1 ) heal=1;
       this_player()->buffer_sp(QueryProp(F_HEAL_SP),heal);
      }
     if ( QueryProp(F_HEAL_HP) > 0 )
      {
       heal=QueryProp(F_HEAL_HP)/5;
       if ( heal < 1 ) heal=1;
       this_player()->buffer_hp(QueryProp(F_HEAL_HP),heal);
      }
    break;

    default:                         // Verdorbenes Essen
     if ( QueryProp(F_BAD) > 11 ) SetProp(F_BAD,11);
     if ( this_player()->SetProp(P_POISON,this_player()->QueryProp(P_POISON)+QueryProp(F_BAD)) > 11 );
     write(break_string("Das war wohl nicht so gut. Dir wird ganz uebel!",78));
    break;
   }

  if ( !QueryProp(F_POTION) )
   {
    if ( QueryProp(F_NO_CONTAINER) )
     remove(1);
    else
     {
      SetProp(P_SHORT,QueryProp(F_EMPTY_CONTAINER));
      SetProp(P_GENDER,QueryProp(F_EMPTY_GENDER));
      SetProp(P_LONG,QueryProp(F_EMPTY_LONG));
      SetProp(P_NAME,QueryProp(F_EMPTY_CONTAINER));
      SetProp(F_IS_FULL,0);

      AddId(lower_case(QueryProp(F_EMPTY_CONTAINER)));
      AddId(lower_case(QueryProp(F_EMPTY_ID)));
     }
   }
  return 1;
}


//--------------------------------------------------------------------------
//
//   int drink_this( string str )
//
//   Kommando zum Trinken.
//--------------------------------------------------------------------------
int drink_this(string str)
{
  notify_fail("WAS moechtest Du trinken?\n");
  if ( !str || !id(str) )
   return 0;

  if ( !QueryProp(F_IS_DRINK) )
   { write(break_string("Das kann man nicht trinken.",78)); return 1; }

  if ( !QueryProp(F_IS_FULL) || !QueryProp(F_POTION) )
   { write(break_string("Da ist nichts mehr drin.",78)); return 1; }

  if ( QueryProp(F_POTION) > QueryProp(F_LIQUID)/50 )
   SetProp(F_POTION,QueryProp(F_LIQUID)/50);

  alc=QueryProp(F_ALC);
  soft=QueryProp(F_LIQUID);

  if ( alc < 0 )
   alc=0;

  if ( alc > 100 )
   alc=100;

  if ( soft < 1 )
   soft=1;

  alc_pp=alc/(QueryProp(F_POTION)+c);
// P_DRINK,100 == 1000 ml == 1 l
  soft_pp=soft/((QueryProp(F_POTION)+c)*10);

// Falls die Portionen nicht Integer-glatt sind, wird in der letzten Runde
// die Differenz aufaddiert. *hick* !
  if ( QueryProp(F_POTION) == 1 )
   {
    if ( alc_pp*(c+1) != alc )
     alc_pp+=alc-alc_pp*(c+1);

    if ( soft_pp*(c+1)*10 != soft )
     soft_pp+=soft-soft_pp*(c+1)*10;
   }

// write(BS("Alc_pp: "+alc_pp));
// write(BS("Soft_pp: "+soft_pp+"\n\n"));

  if ( soft_pp < 1 )
   soft_pp=1;

  if ( !this_player()->drink_soft(soft_pp) )
   return 1;

  if ( alc > 0 && alc_pp < 1 )
   alc_pp=1;

  if ( alc_pp > 0 )
   if ( !this_player()->drink_alcohol(alc_pp) )
    return 1;

// --- Erfolg ---

  c++;

  p=QueryProp(F_POISON);

  if ( p < 0 )
   p=0;

  if ( p > 10 )
   p=10;

  if ( this_player()->SetProp(P_POISON,this_player()->QueryProp(P_POISON)+p) > 10 );

  if ( QueryProp(F_EATER) ) {
    tell_object(PL,break_string("Du "+QueryProp(F_EATER),78,0,
	  BS_LEAVE_MY_LFS));
  }
  else {
    tell_object(PL, break_string("Du trinkst "+QueryProp(P_SHORT)+".",78)); 
  }

  if ( QueryProp(F_MESSAGE) ) {
    tell_room(environment(PL),
	break_string(PL->name()+" "+QueryProp(F_MESSAGE),78,0,
	BS_LEAVE_MY_LFS));
  }
  else {
    tell_room(environment(PL),
	break_string(PL->name()+" trinkt " + QueryProp(P_SHORT)+".",78));
  }

  SetProp(F_POTION,QueryProp(F_POTION)-1);

  switch ( QueryProp(F_BAD) )
   {
   case 0:                           // Heilung
     if ( QueryProp(F_HEAL_SP) > 0 )
      {
       heal=QueryProp(F_HEAL_SP)/5;
       if ( heal < 1 ) heal=1;
       this_player()->buffer_sp(QueryProp(F_HEAL_SP),heal);
      }
     if ( QueryProp(F_HEAL_HP) > 0 )
      {
       heal=QueryProp(F_HEAL_HP)/5;
       if ( heal < 1 ) heal=1;
       this_player()->buffer_hp(QueryProp(F_HEAL_HP),heal);
      }
    break;

    default:                         // Verdorbenes Trinken
     if ( QueryProp(F_BAD) > 11 ) SetProp(F_BAD,11);
     if ( this_player()->SetProp(P_POISON,this_player()->QueryProp(P_POISON)+QueryProp(F_BAD)) > 11 );
     write(break_string("Das war wohl nicht so gut. Dir wird ganz uebel!",78));
    break;
   }

  if ( !QueryProp(F_POTION) )
   {
    if ( QueryProp(F_NO_CONTAINER) )
     remove(1);
    else
     {
      SetProp(P_SHORT,QueryProp(F_EMPTY_CONTAINER));
      SetProp(P_GENDER,QueryProp(F_EMPTY_GENDER));
      SetProp(P_LONG,QueryProp(F_EMPTY_LONG));
      SetProp(P_NAME,QueryProp(F_EMPTY_CONTAINER));
      SetProp(F_IS_FULL,0);

      AddId(lower_case(QueryProp(F_EMPTY_CONTAINER)));
      AddId(lower_case(QueryProp(F_EMPTY_ID)));
     }
   }
  return 1;
}


void MakeDrink()
{
  SetProp(F_IS_DRINK,1);
  SetProp(F_FOOD_SIZE,0);
  AddCmd(({"trink","trinke"}),"drink_this");
  AddId("trinken");
  RemoveId("essen");
  RemoveCmd(({"iss"}));
}
