// MorgenGrauen MUDlib
//
// room/description.c -- room description handling
//
// $Id: description.c 7221 2009-06-04 18:43:28Z Zesstra $

#pragma strong_types
#pragma save_types
#pragma pedantic
#pragma range_check
#pragma no_clone

inherit "/std/container/description";

#define NEED_PROTOTYPES

#include <properties.h>
#include <defines.h>
#include <wizlevels.h>
#include <language.h>
#include <doorroom.h>

void create()
{
  ::create();
  SetProp(P_NAME, "Raum");
  SetProp(P_INT_SHORT,"<namenloser Raum>");
  SetProp(P_INT_LONG,0);
  SetProp(P_ROOM_MSG, ({}) );
  SetProp(P_FUNC_MSG, 0);
  SetProp(P_MSG_PROB, 30);
  SetProp(P_LIGHT_ABSORPTION, 1);
  AddId(({"raum", "hier"}));
}

void init()
{
  mixed roommsg;

//  ::init();
  // Wenn P_ROOM_MSG gesetzt oder P_FUNC_MSG und kein Callout laeuft,
  // Callout starten.
  if( (((roommsg=QueryProp(P_ROOM_MSG)) && sizeof(roommsg)) ||
	QueryProp(P_FUNC_MSG) ) && 
      (find_call_out("WriteRoomMessage")==-1))
    call_out("WriteRoomMessage", random(QueryProp(P_MSG_PROB)));
}

varargs void AddRoomMessage(string *mesg, int prob, mixed *func)
{
  if (mesg && !pointerp(mesg))
    raise_error(sprintf(
      "AddRoomMessage(): wrong argument type, expected Array or 0, "
      "got %.20O",mesg));

   SetProp(P_ROOM_MSG, mesg);

  if (prob>0)
    SetProp(P_MSG_PROB, prob);

  if (func)
    SetProp(P_FUNC_MSG, func);
}

static void WriteRoomMessage()
{
  int i,tim;
  string *room_msg,func;
  mixed *func_msg;

  room_msg = (string *)QueryProp(P_ROOM_MSG);
  func_msg = QueryProp(P_FUNC_MSG);
  if ((!room_msg || !sizeof(room_msg)) && !func_msg)
    return;

  if (room_msg&&sizeof(room_msg))
  {
    i = random(sizeof(room_msg));
    tell_room(this_object(), room_msg[i]);
  }

  if (func_msg)
  {
    if (stringp(func_msg))
      func=(string)func_msg;
    else
      func=func_msg[random(sizeof(func_msg))];
    if (func && function_exists(func))
      call_other (this_object(), func, i);
  }

  while (remove_call_out("WriteRoomMessage")!=-1);
  tim=QueryProp(P_MSG_PROB);
  if(this_object() && sizeof(filter(
       deep_inventory(this_object()), #'interactive))) //')))
    call_out("WriteRoomMessage", (tim<15 ? 15 : tim));
}

varargs string int_long(mixed viewer,mixed viewpoint,int flags)
{
  string descr, inv_descr;

  flags &= 3;
  if( IS_LEARNER(viewer) && viewer->QueryProp( P_WANTS_TO_LEARN ) )
    descr = "[" + object_name(ME) + "]\n";
  else
    descr = "";

  descr += process_string(QueryProp(P_INT_LONG)||"");
  
  // ggf. Tueren hinzufuegen.
  if (QueryProp(P_DOOR_INFOS)) {
    string tmp=((string)call_other(DOOR_MASTER,"look_doors"));
    if (stringp(tmp) && strlen(tmp))
        descr += tmp;
  }
  
  // ggf. Ausgaenge hinzufuegen.
  if ( viewer->QueryProp(P_SHOW_EXITS) && (!QueryProp(P_HIDE_EXITS) 
	|| pointerp(QueryProp(P_HIDE_EXITS))) )
    descr += (string) (GetExits(viewer)||"");

  // Viewpoint (Objekt oder Objektarray) sind nicht sichtbar
  inv_descr = (string) make_invlist(viewer, all_inventory(ME) 
		  - (pointerp(viewpoint)?viewpoint:({viewpoint})) ,flags);

  if ( inv_descr != "" )
    descr += inv_descr;

  if(environment() && (inv_descr=QueryProp(P_TRANSPARENT)))
  {
    if(stringp(inv_descr)) descr += inv_descr;
    else                   descr += "Ausserhalb siehst Du:\n";
            
    descr += environment()->int_short(viewer,ME);
  }
                  
  return descr;
}

string int_short(mixed viewer,mixed viewpoint)
{
  string descr, inv_descr;

  descr = process_string( QueryProp(P_INT_SHORT)||"");
  if( IS_LEARNER(viewer) && viewer->QueryProp( P_WANTS_TO_LEARN ) )
    descr += " [" + object_name(ME) + "].\n";
  else
    descr += ".\n";

  if ( ( viewer->QueryProp(P_SHOW_EXITS)
         || ( environment(viewer) == ME && !viewer->QueryProp(P_BRIEF) ) )
       && (!QueryProp(P_HIDE_EXITS) || pointerp(QueryProp(P_HIDE_EXITS))) )
    descr += (string) (GetExits(viewer)||"");
  
  // Viewpoint (Objekt oder Objektarray) sind nicht sichtbar
  inv_descr = (string) make_invlist( viewer, all_inventory(ME) 
		  - (pointerp(viewpoint)?viewpoint:({viewpoint})) );

  if ( inv_descr != "" )
    descr += inv_descr;

  return descr;
}

/** Roommessages abschalten, wenn keine Interactives mehr da sind.
  */
// TODO: Irgendwann das varargs loswerden, wenn in der restlichen Mudlib
// TODO::exit() 'richtig' ueberschrieben wird.
varargs void exit(object liv, object dest) {
  // fall erbende Objekte das liv nicht uebergeben. Pruefung nur auf
  // previous_object(). Wenn Magier da noch irgendwelche Spielchen mit
  // call_other() & Co treiben, haben wir Pech gehabt, macht aber nicht viel,
  // weil die Raummeldungen dann im naechsten callout abgeschaltet werden.
  if (!living(liv=previous_object())) return;

  object *interactives = filter(all_inventory(), #'interactive);
  // liv wurde noch nicht bewegt, ggf. beruecksichtigen.
  if ( !sizeof(interactives) ||
      (interactive(liv) && sizeof(interactives) < 2) )
    while (remove_call_out("WriteRoomMessage")!=-1);
}

static int _set_int_light(int *light)
{
   int tmp;

   // zur Sicherheit
   if (!pointerp(light)) return -1;
   if (light[0]>QueryProp(P_LIGHT)) {
      // Licht verlaeuft sich in einem grossen Raum, daher Modifier abfragen...
      tmp=light[0]-QueryProp(P_LIGHT_ABSORPTION);
      // wenn sich das Vorzeichen geaendert hat, auf 0 setzen.
      light[0]=((tmp^light[0]) & 0x80000000 ? 0 : tmp);
   }
   if (light[1]<QueryProp(P_LIGHT) && light[1]<0) {
      // Licht verlaeuft sich in einem grossen Raum, daher Modifier abfragen...
      tmp=light[1]+QueryProp(P_LIGHT_ABSORPTION);
      // wenn sich das Vorzeichen geaendert hat, auf 0 setzen.
      light[1]=((tmp^light[1]) & 0x80000000 ? 0 : tmp);
   }
   light[2]=light[0]+light[1];
   return Set(P_INT_LIGHT, light);
}

