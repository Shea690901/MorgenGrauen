/*OBJ*/
/* Von Rumata nach /obj geschoben */

/* Version 1.3 by Fraggle (17.1.1995) */

 // PreventInsert: verhindert Fuellen wie std/container
/* Version 1.2 by Fraggle (17.1.1995) */

 // Flasche kann nun mittels environment(TP)->GetLiquid()
 // etwas anderes als Wasser enthalten, sofern
 // environment(TP)->QueryProp(P_WATER)==W_OTHER
 // BUG: auch 1l Methan wiegt 1 Kg, aendere ich spaeter

/* Version 1.1 by Fraggle (17.1.1995) */

inherit "std/thing";
#include <language.h>
#include <properties.h>
#include <fishing.h>


#define DEFAULT_LIQ "Wasser"
#define TP this_player()

private string once_used,co_filename,p_long_tmp;
private object cont_obj;

public liquid_name;

create()
{
 if (!clonep(this_object())) return;
 ::create();

 liquid_name=DEFAULT_LIQ;
 p_long_tmp=0;  // Wichtig, falls kein P_LONG_EMPTY vorhanden!
 once_used=0;        // bisher noch nicht benutzt (init())

 AddId(({"wasserbehaelter","behaelter","\nwater_source"}));
 SetProp(P_NAME, "Wasserbehaelter");
 SetProp(P_GENDER , MALE );
 SetProp(P_ARTICLE, 1 );
 SetProp(P_SHORT, "Ein Standard-Wasserbehaelter");
 SetProp(P_LONG,"Nichts zu sehen.\n");
 SetProp(P_LONG_EMPTY,""); //Beschreibung fuer leeren Zustand
 SetProp(P_LONG_FULL,"");  //Beschreibung fuer gefuellten Zusand
 SetProp(P_LIQUID,1000);   // Fuellmenge = 1 Liter
 SetProp(P_WATER,0);       // Flasche ist defaultmaessig leer!
 SetProp(P_VALUE,10);
 SetProp(P_WEIGHT,20);     // ACHTUNG! LEERGEWICHT der Flasche!
 AddCmd(({"fuell","fuelle"}),"fuelle");
 AddCmd(({"leere"}),"leere");
}
init()
 {
  ::init();
  if(!(once_used))setup();
 }
setup()
 {
  string tmp,pronomen;

  if(p_long_tmp==0)
   {
    p_long_tmp=this_object()->QueryProp(P_LONG);
    once_used=1; //erster (richtiger) Aufruf!
   }
  pronomen = capitalize(QueryPronoun(WER));

  if(this_object()->QueryProp(P_WATER))
   {
    if(once_used)SetProp(P_WEIGHT, this_object()->QueryProp(P_WEIGHT)
                                  +this_object()->QueryProp(P_LIQUID));
    if((tmp=this_object()->QueryProp(P_LONG_FULL))!="")
     {
       if(liquid_name!=DEFAULT_LIQ)tmp=ReplaceWasser(tmp,liquid_name);
       SetProp(P_LONG,tmp);
     }
    else
     {
       SetProp(P_LONG,p_long_tmp+pronomen+" enthaelt "+liquid_name+".\n");
     }
    AddId(lower_case(liquid_name));
   }
  else
   {
    if((tmp=this_object()->QueryProp(P_LONG_EMPTY))!="")
     {
       SetProp(P_LONG,tmp);
     }
    else
     {
       SetProp(P_LONG,p_long_tmp+pronomen+" ist leer.\n");
     }
    if(id(lower_case(liquid_name)))SetProp(P_IDS,QueryProp(P_IDS)-({lower_case(liquid_name)}));
    liquid_name=DEFAULT_LIQ; //reset liquid_name
   }
  return ;
 }

leere(str)
{
 notify_fail("Was willst Du denn leeren?\n");
 if(!id(str))return 0;
 if(!( this_object()->QueryProp(P_WATER)))
  {
   write("Da ist kein "+liquid_name+" drin!\n");
   return 1;
  }
 write("Du leerst "+this_object()->name(WEN,1)+".\n");
 say(this_player()->name()+" leert "+this_object()->name(WEN,0)+".\n",TP);
 empty();
 return 1;
}
fuelle(str)
{
 string strid,strobj;
 int contents,liquidtype;

 notify_fail("Was willst Du denn womit fuellen?\n");
 if(!str)return 0;
 if(!id(str))
  {
   if(sscanf(str,"%smit%s",strid,strobj)!=2)return 0;
   if(!id(implode(old_explode(strid, " "), "")))return 0;
  // if(implode(old_explode(strobj, " "), "")!="wasser")return 0;
  }
 if(liquidtype=(environment(TP)->QueryProp(P_WATER)))
  {
   if(this_object()->QueryProp(P_WATER))
    {
     write(capitalize(this_object()->name(WER,1))+" ist bereits voll!\n");
     return 1;
    }
   contents=this_object()->QueryProp(P_LIQUID);
   environment(TP)->AddWater(-contents);
     // Wasser von Umgebung abziehen!
     // Man kann als Magier die Funktion AddWater(int n) dazu benuetzten,
     // beispielsweise eine Pfuetze zu leeren, ...
   if(liquidtype&W_OTHER)
    {
     if(co_filename=(environment(TP)->GetLiquid())) //mittels Getliquid kann
       if(cont_obj=clone_object(co_filename))        //was anderes als Wasser
        liquid_name=cont_obj->name();                //in die Flasche getan werden
     if(PreventInsert(cont_obj))                    //Fuellen verhindern!
      {
        liquid_name=DEFAULT_LIQ;
        return 1;
      }
    }
   write("Du fuellst etwas "+liquid_name+" in "+this_object()->name(WEN,1)+".\n");
   say(this_player()->name()+" fuellt etwas "+liquid_name+" in "+this_object()->name(WEN,0)+".\n",TP);
   SetProp(P_WATER,liquidtype);
   setup(); // Wichtig! Props setzen!!!
   SetProp(P_WEIGHT,this_object()->QueryProp(P_WEIGHT)+contents);
    //wie praktisch, 1 ml == 1 g :)
  }
else
  {
    write("Du findest hier nichts, was Du in "+this_object()->name(WEN,1)+" fuellen koenntest!\n");
  }
 return 1;
}
int empty()
{
 int contents;
 if(! (this_object()->QueryProp(P_WATER)) )return 0;   // War schon leer!
 contents=this_object()->QueryProp(P_LIQUID);
 SetProp(P_WEIGHT,this_object()->QueryProp(P_WEIGHT)-contents); // Wasser wieder raus!!!
 if((this_object()->QueryProp(P_WATER))&W_OTHER)
  {
    environment(TP)->PutLiquid(co_filename);
  }
 else
  {
    environment(TP)->AddWater(contents);
  }
 SetProp(P_WATER,0);
 setup();                          // Wichtig! Props setzen!!!
 return contents; //gib die ml an Umgebung ab :)
}

string ReplaceWasser(string src,string replc)
 {
  string part1,part2;

  if(!src)return src;
  if(sscanf(src,"%sWasser%s",part1,part2))src=part1+replc+part2;
  return src;
 }
IsBottle()
{
   return 1;      // hier kann man das environment abtesten!
}
PreventInsert(object obj)
 {
   if(obj->id("gas")) //default: NICHT Gasdicht!
    {
      write("Das Gas entweicht sofort wieder!\n");
      return 1;
    }
   return 0;     // Zum Ueberschreiben! Present(){return 0;} z.B.
                 // macht Die Flasche Gasdicht!
                 // Oder man kann die Flasche verschliessbar machen.
 }

