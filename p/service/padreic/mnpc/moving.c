/* Changelog:
   * 28.01.2007 Zesstra
     neues Flag MNPC_EXACT_AREA_MATCH auf Magierwunsch hin eingefuehrt.
   * 03.12.2006 Zesstra
     _set_mnpc_flag() meldet sich jetzt ggf. erst im walk_master ab, sonst
     wuerde der MNPC bei Aenderung der Flags erneut im walk_master angemeldet.
   * 11.09.2007, Zesstra
     save_types und strong_types aktiviert.
     command_me-Prototypen auskommentiert und command_me() nach command()
     geaendert.
   * 18.09.2007, Zesstra
     Wenn der NPC in einem Raum mit nur SEs steht und diese benutzen darf,
     teleportiert er sich nun nicht mehr mit 20% Wahrscheinlichkeit nach Haus.
*/
#pragma save_types
#pragma strong_types

inherit "std/living/moving";

#include <moving.h>
#include <defines.h>
#include <properties.h>
#include "/p/service/padreic/mnpc/mnpc.h"
#include <combat.h>

#define ENV    environment
#define PO     previous_object()

static int meet_last_player;
//static command_me(string str);

// noetige Prototypes
mixed InFight();

static void mnpc_create()
{
  if (PO && member(inherit_list(PO), "/std/room.c")!=-1)
    Set(MNPC_HOME, object_name(PO));
  else if (PL && ENV(PL))
    Set(MNPC_HOME, object_name(ENV(PL)));
  else Set(MNPC_HOME, MNPC_DFLT_HOME);
  Set(P_MNPC, 1);
  Set(MNPC_AREA, ({}));
  Set(MNPC_DELAY, MNPC_DFLT_DELAY);
  Set(MNPC_FUNC, 0);
  Set(MNPC_RANDOM, 0);
  Set(MNPC_WALK_TIME, MNPC_DFLT_WALK);
  SetProp(MNPC_FLAGS, 0);
  SetProp(P_ENABLE_IN_ATTACK_OUT, 1);
  meet_last_player=time();
}

static int _set_mnpc_flags(int flags)
{
  if (flags & MNPC_WALK) {
    if (!QueryProp(MNPC_HOME)) raise_error("unknown MNPC_HOME\n");
    //wenn die Flags neu gesetzt werden, wird der MNPC das zweite Mal im
    //Master angemeldet -> vorher abmelden (Zesstra)
    if (QueryProp(MNPC_FLAGS) & MNPC_WALK) {
	if (WALK_MASTER->Registered())
	    WALK_MASTER->RemoveWalker();
	else if (find_call_out("Walk")!=-1)
	    remove_call_out("Walk");
    }
    if ((Query(MNPC_DELAY)+Query(MNPC_RANDOM))<=MAX_MASTER_TIME)
      WALK_MASTER->RegisterWalker(Query(MNPC_DELAY), Query(MNPC_RANDOM));
    else call_out("Walk", Query(MNPC_DELAY)+random(Query(MNPC_RANDOM)));
  }
  // else nicht von Bedeutung, da in Walk() das flag getestet wird
  if (flags & MNPC_FOLLOW_PLAYER) {
    if (!Query(MNPC_PURSUER)) { // wurde dieses Flag neu eingeschaltet?
      if (environment()) { // Verfolgung aufnehmen...
        object *pursuer;
        pursuer=filter(all_inventory(ENV()), #'interactive);
        filter_objects(pursuer, "AddPursuer", ME);
        Set(MNPC_PURSUER, pursuer);
      }
      else Set(MNPC_PURSUER, ({}));
    }
  }
  else if (pointerp(Query(MNPC_PURSUER))) { // wird derzeit irgendwer verfolgt?
    // alle Verfolgungen abbrechen...
    filter_objects(Query(MNPC_PURSUER)-({ 0 }), "RemovePursuer", ME);
    Set(MNPC_PURSUER, 0); // Speicher freigeben...
  }
  else Set(MNPC_PURSUER, 0);
  // nur livings koennen command_me nutzen...
  if (!living(ME)) flags |= MNPC_DIRECT_MOVE;
  return Set(MNPC_FLAGS, flags);
}

static void mnpc_InsertEnemy(object enemy)
{
  if ( (Query(MNPC_FLAGS) & MNPC_FOLLOW_ENEMY) &&
         (member(Query(MNPC_PURSUER), PL)==-1)) {
     PL->AddPursuer(ME);
     Set(MNPC_PURSUER, Query(MNPC_PURSUER)+({ PL }));
  }
}

static void mnpc_reset()
{
  if (!Query(MNPC_HOME) || !(Query(MNPC_FLAGS) & MNPC_WALK)) return;
  if (meet_last_player>=0 && Query(MNPC_WALK_TIME)+meet_last_player < time()
      && environment() && !sizeof(filter(
      all_inventory(environment()), #'query_once_interactive))) {
    if (Query(MNPC_WALK_TIME) > MAX_MASTER_TIME) remove_call_out("Walk");
    meet_last_player=-1;
    move(Query(MNPC_HOME), M_TPORT|M_NOCHECK); // ab nach Hause und dort warten
  }
}

static int _query_mnpc_last_meet()
{   return meet_last_player;   }

static void mnpc_init()
{
  if (interactive(PL)) {
    if (meet_last_player<=0) {
      if ((Query(MNPC_DELAY)+Query(MNPC_RANDOM))<=MAX_MASTER_TIME)
        WALK_MASTER->RegisterWalker(Query(MNPC_DELAY), Query(MNPC_RANDOM));
      else call_out("Walk", Query(MNPC_DELAY)+random(Query(MNPC_RANDOM)));
    }
    if ( (Query(MNPC_FLAGS) & MNPC_FOLLOW_PLAYER) &&
         (member(Query(MNPC_PURSUER), PL)==-1) &&
        (!(Query(MNPC_FLAGS) & MNPC_FOLLOW_ENEMY) || IsEnemy(PL))) {
      PL->AddPursuer(ME);
      Set(MNPC_PURSUER, Query(MNPC_PURSUER)+({ PL }));
    }
    meet_last_player=time();
  }
  else {
    int lm;
    lm=PL->QueryProp(MNPC_LAST_MEET);
    if (meet_last_player<=0 && lm>0) {
       if ((Query(MNPC_DELAY)+Query(MNPC_RANDOM))<=MAX_MASTER_TIME)
          WALK_MASTER->RegisterWalker(Query(MNPC_DELAY), Query(MNPC_RANDOM));
       else call_out("Walk", Query(MNPC_DELAY)+random(Query(MNPC_RANDOM)));
       meet_last_player=lm;
    }
    else if (meet_last_player<lm) meet_last_player=lm;
  }
}

static void mnpc_move()
{
  if (environment() && (Query(MNPC_FLAGS) & MNPC_FOLLOW_PLAYER)) {
    object *liv;
    liv=Query(MNPC_PURSUER) & all_inventory(environment());
    filter_objects(Query(MNPC_PURSUER)-liv-({ 0 }), "RemovePursuer", ME);
    Set(MNPC_PURSUER, liv);
  }
  if (Query(MNPC_FUNC)) call_other(ME, Query(MNPC_FUNC));
}

static int PreventEnter(string file)
// darf der Raum betreten werden?
{
  string *area;

  if (!sizeof(area=Query(MNPC_AREA)))
    return 0; // Raum darf betreten werden
  else {
    int i;
    status exactmatch;
    exactmatch=Query(MNPC_FLAGS) & MNPC_EXACT_AREA_MATCH;
    if ((i=strstr(file, "#"))!=-1) file=file[i+1..<1];
    for (i=sizeof(area)-1; i>=0; i--) {
      if (exactmatch) {

	//exakter Vergleich, kein Substringvergleich gewuenscht
	if (file==area[i]) return 0;  //betreten
      }
      else
	if (strstr(file, area[i])==0) return 0; // Raum betreten
    }
    return 1; //  Raum darf nicht betreten werden
  }
}

static int mnpc_PreventFollow(object dest)
{
  if (dest && PreventEnter(object_name(dest)))
    return 2;
  return 0;
}

// Bewegungssimulation (Bewegungsmeldung) fuer bewegende non-livings
static int direct_move(mixed dest, int method, string direction)
{ 
   int res, para, tmp;
   string textout, textin, *mout, vc, fn;
   object oldenv, *inv;

   if (living(ME))
      return call_other(ME, "move", dest, method);
   else {
      oldenv = environment();
      para=QueryProp(P_PARA);
      if ((para>0) && stringp(dest)) {
         fn=dest+"^"+para;

         if (find_object(fn) || (file_size(fn+".c")>0))
            dest=fn;
         else if (file_size(vc=implode(explode(fn,"/")[0..<2],"/")+"/virtual_compiler.c")>0) {
            // wenn ein VC existiert, prüfen ob dieser ParaObjecte unterstuetzt
            // wenn ja, dann testen ob sich Raum laden laesst...
            if ((!catch(tmp=(int)call_other(vc,"NoParaObjects")) && (!tmp)) && 
                (!catch(call_other( fn, "???" ))))
                dest=fn;
         }
      }
      res = (int)call_other(ME, "move", dest, M_NOCHECK);
      if (oldenv==environment()) return res;

      // als erstes die Meldung fuer das Verlassen des Raumes...
      if ( method & M_TPORT )
         textout = (string) QueryProp(P_MMSGOUT) || (string) QueryProp(P_MSGOUT);
      else textout = (mout = explode( (string) QueryProp(P_MSGOUT) || "", "#" ))[0]
                     || (string) QueryProp(P_MMSGOUT);

      if (stringp(textout)) {
         if ( !strlen(direction) ) direction = 0;

         inv = all_inventory(oldenv) - ({ this_object() });
         inv = filter( inv, #'living);
         inv -= filter_objects( inv, "CannotSee", 1 );
 
         filter( inv, #'tell_object,
                       Name( WER, 2 ) + " " + textout +
                       (direction ? " " + direction : "") +
                       (sizeof(mout) > 1 ? mout[1] : "") + ".\n" );
      }

      // nun die Meldung für das "Betreten" des Raumes...

      if ( method & M_TPORT )
         textin = (string) QueryProp(P_MMSGIN);
      else textin = (string) QueryProp(P_MSGIN);

      if (stringp(textin)) {            
         inv = all_inventory(environment()) - ({ this_object() });
         inv = filter( inv, #'living);
         inv -= filter_objects( inv, "CannotSee", 1 );
         filter( inv, #'tell_object,
                       capitalize(name( WER, 0 )) + " " + textin + ".\n" );
      }
  }
}

int Walk()
{
  int     i, flags;
  mapping exits;
  string  *rooms, *dirs, *ex, tmp;

  flags=Query(MNPC_FLAGS);
  if (!(flags & MNPC_WALK)) return 0;

  if ((flags & MNPC_NO_WALK_IN_FIGHT) && InFight()) {
    meet_last_player=time();
    if ((Query(MNPC_DELAY)+Query(MNPC_RANDOM))>MAX_MASTER_TIME)
      call_out("Walk", Query(MNPC_DELAY)+random(Query(MNPC_RANDOM)));
    return 1;
  }

  if (!environment()) {
    meet_last_player=-1;
    if (!clonep(this_object())) return 0; // Blueprints nicht destructen
    catch(remove());
    if (this_object()) destruct(this_object());
    return 0;
  }

  if (Query(MNPC_WALK_TIME)+meet_last_player < time() && !sizeof(
      filter(all_inventory(environment()), #'query_once_interactive))) {
    if (flags & MNPC_GO_HOME_WHEN_STOPPED) {
      meet_last_player=-1;
      move(Query(MNPC_HOME), M_TPORT|M_NOCHECK);
    }
    else meet_last_player=0;
    return 0;
  }
  
  // Ausgaenge ermitteln.
  exits = (environment()->QueryProp(P_EXITS));
  rooms = m_values(exits); dirs  = m_indices(exits); ex = ({});
  for (i=sizeof(rooms)-1; i>=0; i--)
    if (!PreventEnter(rooms[i])) ex += ({ dirs[i] });

  /* Hier muessen wir auf die Zuverlaessigkeit unserer Magier bauen ... */
  if (flags & MNPC_DIRECT_MOVE) {
    // im direct mode keine SEs benutzbar...
    if (sizeof(ex)) {
       tmp=ex[random(sizeof(ex))];
       direct_move(explode(exits[tmp], "#")[<1], M_GO, "nach "+capitalize(tmp));
    }
    else {
       // Hngl. Nach Hause...
       direct_move(Query(MNPC_HOME), M_TPORT|M_NOCHECK, 0);
     } 
  }
  else if (flags & MNPC_ONLY_EXITS) {
    // logischerweise auch keine SEs benutzen...
    if (sizeof(ex)) {
      command(ex[random(sizeof(ex))]); /* Irgendwohin gehen */
    }
    else {
      // Hngl. Nach Hause...
      move(Query(MNPC_HOME), M_TPORT|M_NOCHECK);
   }
  }
  else {
    // Special Exits mitbenutzen.
    ex += m_indices(ENV()->QueryProp(P_SPECIAL_EXITS));
    if (sizeof(ex)) {
      command(ex[random(sizeof(ex))]); /* Irgendwohin gehen */
    }
    else {
      // Hngl. Gar keine Ausgaenge. Nach Hause...
      move(Query(MNPC_HOME), M_TPORT|M_NOCHECK);
    }
  }
  // Aufrufe in zerstoerten Objekten verhindern, die beim Bewegen sterben.
  if ( objectp(this_object()) && 
       (Query(MNPC_DELAY)+Query(MNPC_RANDOM))>MAX_MASTER_TIME )
    call_out("Walk", Query(MNPC_DELAY)+random(Query(MNPC_RANDOM)));
  return 1;
}
