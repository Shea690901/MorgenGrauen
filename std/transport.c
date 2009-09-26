// MorgenGrauen MUDlib
//
// transport.c -- Basisklasse fuer Schiffe und aehnliche Transporter
//
// $Id: transport.c 7223 2009-06-11 14:02:18Z Zesstra $
#pragma strong_types
//#pragma save_types
#pragma range_check
#pragma no_clone
#pragma pedantic

inherit "std/thing/moving";
inherit "std/room";

#include <properties.h>
#include <moving.h>
#include <defines.h>
#include <language.h>
#include <transport.h>

/* transport.c
 * 
 * Ueberarbeitete und 
 * erweiterte Version : Tilly@MorgenGrauen, 10.01.02
 * Basierend auf      : transport.c@SilberLand (Woody@SilberLand), 05.12.99
 * Basierend auf      : Hates und Rumatas generisches Transport Objekt
 *                      MorgenGrauen 15.02.93
 *
 * Dokumentation in /doc/std/transport
 *                  /doc/beispiele/transporter/
 *                  /doc/wiz/transporter
 */
     
/*
 ********************* Variablen *********************
 */

// TODO: langfristig waer ja private schoen...
nosave mixed *route;    /* Liste der Haltepunkte. */
nosave int rpos;        /* Momentane Position in obiger Liste. */
nosave string roomCode; /* Code des aktuellen Raumes (oder 0). */

/*
 ********** Management der builtin-properties **********
 */

string _query_short()
{ 
  if (roomCode) return Query(P_SHORT); 
  return 0; 
}

string _query_transparent()
{ 
  if (roomCode) return Query(P_TRANSPARENT);
  return 0; 
}

mixed *_set_route(mixed *r) { return route = r; }
mixed *_query_route()       { return route; }

/*
 **************** Zugriffsfunktionen ***************
 */

void Halt()
{
  while (remove_call_out( "changeHp" )>-1);
  while (remove_call_out( "disconnect" )>-1);
}

varargs void Start(int pos)
{
  Halt();
  rpos = (pos >= sizeof(route))?-1:pos-1;
  call_out("changeHp",0);
}

void SetTravelCmds()
{
  if (pointerp(QueryProp(P_LEAVECMDS)))
     AddCmd(QueryProp(P_LEAVECMDS),"GoOutside");
  if (pointerp(QueryProp(P_ENTERCMDS)))
    AddCmd(QueryProp(P_ENTERCMDS),"GoInside");
  if (pointerp(QueryProp(P_TRAVEL_CMDS)))
    AddCmd(QueryProp(P_TRAVEL_CMDS),"GoInAndOutside");
  return;
}

mixed HasRoute(mixed dest)
{
  int i,s,z;
  string str;
  object ob;
  mixed harb;
  
  s = sizeof(route);

  for (i = rpos;i <= rpos+s-1;i++)
  {
    if (route[i%s][0] == HP_ROOM)
    {
      if (member(route[i%s][5],dest) != -1 &&
          objectp(ob=load_object(route[i%s][1])) &&
          pointerp(harb=ob->QueryProp(P_HARBOUR)) &&
	  sizeof(harb))
      {
        return ({ route[i%s][1], harb[0] });
      }
    }
  }
  return 0;
}

void AddRoute(string room, int stay, int next, string code, mixed dest, 
                string deststr)
{
  object ob;
  int i;
  if (!pointerp(dest))
  {
    if (stringp(dest))
      dest = ({ dest });
    else
      dest = map(filter(explode(code," "),lambda( ({'x}),
                      ({ #'==, 'x, ({#'capitalize, 'x}) }) ) ),#'lower_case);
  }
  route += ({ ({ HP_ROOM, room, stay, next, code, dest, deststr }) });
  
  if(!QueryProp(P_NO_TRAVELING))
  {
    TRAVELD->AddHarbour(object_name(this_object()),room);
  }
}
    
void AddMsg(string msg, int next) { route += ({ ({ HP_MSG, msg, next }) }); }
void AddFun(string fun, int next) { route += ({ ({ HP_FUN, fun, next }) }); }

string QueryArrived() { return roomCode; }

mixed* QueryPosition()
{
  return ({ route[rpos][1],route[(rpos+1)<sizeof(route)?(rpos+1):0][1] });
}

object* QueryPassengers()
{
  return filter(all_inventory(),#'query_once_interactive); 
}

varargs string *QueryHarbours(int textflag)
{
  int i,j,k;
  string *ret, *s, h, *h2;

  ret = ({});

  if (textflag)
  {
    j=sizeof(route);
    for (i=0;i<j;i++)
      if (route[i][0] == HP_ROOM)
      {
        s = route[i][1]->QueryProp(P_HARBOUR);
        if (pointerp(s) && sizeof(s))
        {
          h2 = explode(s[1]," ");
          k=sizeof(h2);
          while(k--) h2[k]=capitalize(h2[k]);
          ret+=({ implode(h2," ") });
        }
        
      }
  }
  else
  {
     i=sizeof(route);
     while(i--) if (route[i][0] == HP_ROOM) ret += ({route[i][1]});
  }
  return ret;
}

void RemoveRoute()
{
  Halt();
  route = ({ });
  rpos  =   0;

  TRAVELD->RemoveShip(this_object());
}

varargs int Enter(object who)
{
  string *emsg;
  mixed efail;

  if (!objectp(who)) who = this_player();
  if (environment(who) == this_object())
  {
    tell_object(who,"Da bist Du doch bereits, schon vergessen?\n");
    return 1;
  }
  if (!QueryArrived()) return 0;
  if (QueryProp(P_MAX_PASSENGERS) && 
     (sizeof(QueryPassengers()) >= QueryProp(P_MAX_PASSENGERS)))
  {
    if (pointerp(efail=QueryProp(P_ENTERFAIL)))
    {
      if (sizeof(efail) == 2)
        tell_room(this_object(),who->Name(WER,2)+" "+process_string(efail[1])+
                                                   ".\n",({who}));      
      tell_object(who,process_string(efail[0])+".\n");
    }
    else if (stringp(efail))
       tell_object(who,process_string(efail)+".\n");
    else if (closurep(efail)) funcall(efail);
    return 1;
  }
  
  tell_object(who,"Du betrittst "+name(WEN,1)+".\n");
  if (pointerp(emsg=QueryProp(P_ENTERMSG)) && sizeof(emsg) == 2)
     return who->move(this_object(),M_GO,"",process_string(emsg[0]),
                                             process_string(emsg[1]));
  return who->move(this_object(),M_GO,
        name(WEN,1),"betritt","kommt herein");
}

varargs int Leave(object who)
{
  string *lmsg;
  mixed lfail;

  if (!objectp(who)) who = this_player();
  if (environment(who) != this_object())
  {
    if (QueryArrived())
    {
      tell_object(who,"Dafuer muesstest Du erstmal dort sein.\n");
      return 1;
    }
    return 0;
  }
  if (!QueryArrived())
  { 
    if (lfail=QueryProp(P_LEAVEFAIL))
    {
      if (pointerp(lfail) && sizeof(lfail))
      {
        if (sizeof(lfail) == 2)
           tell_room(this_object(),who->Name(WER,2)+" "+process_string(
               lfail[1])+".\n",({who}));
        tell_object(who,process_string(lfail[0])+".\n");
      }
      else if (stringp(lfail))
        tell_object(who,process_string(lfail)+".\n");
      else if (closurep(lfail)) funcall(lfail);
      return 1;
    }
    tell_object(who,"Fehler beim Verlassen des Transporters.\n"
                    "Bitte zustaendigen Magier verstaendigen.\n");
    return 1;
  }

  if (who->QueryProp(P_TRAVEL_INFO)) who->SetProp(P_TRAVEL_INFO,0);
  tell_object(who,"Du verlaesst "+name(WEN,1)+".\n");
  if (pointerp(lmsg=QueryProp(P_LEAVEMSG)) && sizeof(lmsg) == 2)
      return who->move(environment(),M_GO,"",process_string(lmsg[0]),
                                             process_string(lmsg[1]));
  return who->move(environment(),M_GO,
           name(WEN,1),"verlaesst","kommt herein");
}

/*
 ****************** Internal Functions ******************
 */

static int GoInside(string str)
{
  _notify_fail("Was moechtest Du denn genau?\n");
  if (stringp(str) && id(str)) {
      Enter();
      return 1;
  }
  return 0;
}

static int GoOutside(string str)
{
  _notify_fail("Was moechtest Du denn genau?\n");
  if (stringp(str) && id(str)) {
      Leave();
      return 1;
  }
  return 0;
}

static int GoInAndOutside(string str)
{
  string to;

  _notify_fail("Was moechtest Du denn genau?\n");
  if (!strlen(str)) return 0;  
  if ((sscanf(str,"auf %s",to) == 1 || sscanf(str,"in %s",to) == 1) && id(to))
    return Enter(),1;
  if ((sscanf(str,"von %s",to) == 1 || sscanf(str,"aus %s",to) == 1) && id(to))
    return Leave(),1;
}

void create()
{
  ::create();

  route = ({});

  SetProp(P_LEAVEFAIL,"Das ist momentan viel zu gefaehrlich");
  SetProp(P_ENTERFAIL,"Dort ist kein Platz mehr fuer Dich");
  SetProp(P_TRANSPARENT,1);

  AddId("Transporter");
  
  call_out("SetTravelCmds",1);
}

static varargs void disconnect(int change, int change_time)
{
  object room;
  mixed *departmsg;

  departmsg = QueryProp(P_DEPARTMSG);

  if ((room = environment()) && pointerp(departmsg))
  {
    tell_room(this_object(),process_string(departmsg[0]));
    tell_room(room,process_string(departmsg[1]));
  }

  roomCode = 0;

  if (change) call_out("changeHp",change_time);
}

static void connect(string room, string code)
{
  mixed *arrivemsg, *t;
  object *trav, ob;
  string *trs, *msgs;
  int i;

  if (roomCode) disconnect();
  
  roomCode = code?code:"";

  if (catch(move(room,M_SILENT|M_NOCHECK);publish))
  {
    roomCode = 0;
    return;
  }

  arrivemsg = QueryProp(P_ARRIVEMSG);

  if (pointerp(arrivemsg))
  {
    tell_room(this_object(),process_string(arrivemsg[0]));
    tell_room(room,process_string(arrivemsg[1]));
  }

  trav = filter(all_inventory(this_object()),#'living);

  i = sizeof(trav);
  while(i--)
  {
    if (pointerp(t = trav[i]->QueryProp(P_TRAVEL_INFO))&&
        t[0]==this_object()&&t[2]==room)
    { 
      if (trav[i]->InFight())
        tell_object(trav[i],break_string("Du solltest Deinen Kampf "
                "schnell beenden,denn eigentlich wolltest Du hier "
                "aussteigen.",78));
      else
        Leave(trav[i]);
      if (environment(trav[i])!=this_object())
        trav[i]->SetProp(P_TRAVEL_INFO,0);
    }
  }
  trav = filter(all_inventory(find_object(room))-trav,#'living);
  i=sizeof(trav);
  while(i--)
  {
    if (pointerp(t = trav[i]->QueryProp(P_TRAVEL_INFO))&&
        t[0] == environment(trav[i]) && t[1] == this_object())
    {
      if (trav[i]->InFight() && find_player(trav[i]))
        tell_object(find_player(trav[i]),
           break_string("Du solltest Deinen Kampf schnell beenden, denn "
                        "eigentlich wolltest Du mit "+name(WEM,1)+
                        " reisen.",78));
      else
        Enter(trav[i]);
      if (environment(trav[i]) == this_object()) 
      {
        t[0] = this_object();
        trav[i]->SetProp(P_TRAVEL_INFO,t);
      }
    }
  }
}

// this object never performs any clean-up, the driver should not call it
// again.
int clean_up(int arg) { return 0; }

void changeHp()
{
  if (++rpos == sizeof(route)) rpos = 0;
  if (route[rpos][0] == HP_MSG)
  {
    call_out("changeHp",route[rpos][2]);
    tell_room(this_object(),route[rpos][1]);
  }
  else if (route[rpos][0] == HP_FUN)
  {
    call_out("changeHp",route[rpos][2]);
    call_other(this_object(),route[rpos][1]);
  }
  else 
  {
    call_out("disconnect",route[rpos][2],1,route[rpos][3]);
    connect(route[rpos][1],route[rpos][4]);
  }
}

void __restart(string funname)
{
  if (!funname || funname == "" || (funname != "changeHp" && 
                                    funname != "disconnect"))
    return;
  while(remove_call_out(funname) != -1);
  call_out(funname,funname == "changeHp"?15:5);
}
