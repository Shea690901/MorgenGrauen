// MorgenGrauen MUDlib
//
// player/viewcmd.c -- player view command handling
//
// $Id: viewcmd.c 6715 2008-02-03 22:06:56Z Zesstra $

#pragma strong_types
#pragma save_types
#pragma range_check
#pragma no_clone
#pragma pedantic

#define NEED_PROTOTYPES

#include "/sys/thing/properties.h"
#include "/sys/living/put_and_get.h"
#include "/sys/living/description.h"

#include <container.h>
#include <player.h>
#include <properties.h>
#include <rooms.h>
#include <wizlevels.h>
#include <defines.h>
#include <moving.h>
#include <new_skills.h>
#include <ansi.h>

#include <sys_debug.h>

varargs mixed More(string str, int fflag, string returnto);

void create()
{
  Set(P_BRIEF, SAVE, F_MODE);
  Set(P_BLIND, SAVE, F_MODE);
}

int _toggle_brief()
{
  int brief;

  if (query_verb()=="kurz")
    brief=1;
  else if (query_verb()=="ultrakurz")
    brief=2;
  else brief=0;
  SetProp(P_BRIEF, brief);
  write("Du bist nun im \""+
        (brief?(brief==1?"Kurz":"Ultrakurz"):"Lang")+"\"modus.\n");
  return 1;
}

private int sortinv(mixed a, mixed b) { return a[0] > b[0]; }

private string collectinv(mixed obj)
{
  if(obj[0]=="") return 0;
  return (obj[2] ? " " : "")
       + obj[0]
       + (obj[1] > 1 ? " ("+obj[1]+")" : "");
}

#define I_AUTOLOAD      1
#define I_KEEP          4
#define I_FORMATTED     16
#define I_ARMOUR        64
#define I_SORT          256
#define I_WEAPON        1024
#define I_FORCE_SORT    4096
#define I_NO_TABLE      16384

private string getflags(string arg, int flags)
{
  int no, i;
  if(strlen(arg) < 2) return 0;
  no = (arg[0] == '-');

  for(i = 1; i < strlen(arg); i++)
  {
    switch(arg[i])
    {
    case 'a': flags |= I_AUTOLOAD << no; break;
    case 'b': flags |= I_KEEP << no; break;
    case 'f': flags |= I_FORMATTED << no; break;
    case 'r': flags |= I_ARMOUR << no; break;
    case 's': flags |= I_SORT << no; break;
    case 'w': flags |= I_WEAPON << no; break;
    case 'v': flags |= (I_ARMOUR | I_WEAPON) << !no; break;
    case '1': flags |= I_NO_TABLE; break;
        // Die Option macht nur Aerger und kommentiert ist sie eh nicht.
        // Wer das dringend braucht, soll Wargons Schiebepuzzle benutzen.
        //
        // Tiamak, 15.10.2000
        //    case 'S': flags |= I_FORCE_SORT << no; break;
    default : return arg[i..i];
    }
  }
}

static int _check_keep(object ob)
{
  return (ob->QueryProp(P_KEEP_ON_SELL))==geteuid(ME);
}

static int _check_visible(object ob)
{
  return ( !(ob->QueryProp(P_INVIS)) && stringp(ob->QueryProp(P_SHORT)) );
}

int _inventory(string str)
{
  mixed *args, output;
  int ansi, i, flags, minv;
  mixed s, weapons, armours, misc;
  string format;

  if(CannotSee()) return 1;

  if((str = _unparsed_args()) && str!="")
  {
    string error;
    error = "Benutzung: i[nventar] [-/+1abfrsvw]\n";
    args = regexp(regexplode(str, "[-+][1abfrswv][1abfrswv]*"),
                  "[-+][1abfrswv][1abfrswv]*");
    if(!sizeof(args)) return (_notify_fail(error), 0);
    if(sizeof(args = map(args, #'getflags/*'*/, &flags) - ({ 0 })))
    {
      printf("%s: Unbekanntes Argument.\n"+error, implode(args, ", "));
      return 1;
    }
  }
  // Fuer Spieler gehen nur sichtbare Objekte in den Algorithmus
  if (IS_LEARNING(ME))  
    s = all_inventory(ME);
  else
    s = filter( all_inventory(ME), "_check_visible");
  
  ansi = member(({"vt100", "ansi"}), QueryProp(P_TTY)) != -1;
  minv = 1 | (flags & (I_FORMATTED | (I_FORMATTED << 1)) ? 2 : 0);
  format = (flags & I_NO_TABLE) ? "=" : "#";

//  if(flags & (I_FORCE_SORT | I_FORCE_SORT << 1))
//  {
//    closure sf;
//    sf = flags & I_FORCE_SORT ? #'> : #'<;
//    s = sort_array(s, lambda(({'a, 'b}),
//                             ({#'funcall, sf,
//                                   ({#'||,({#'call_other,'a,"short"}),""}),
//                                   ({#'||,({#'call_other,'b,"short"}),""})})));
//    map_objects(s, "move", this_object());
//    s = all_inventory(ME);
//  }

  if (flags & I_AUTOLOAD)
    s = filter_objects(all_inventory(ME),"QueryProp",P_AUTOLOADOBJ);
  else if (flags & (I_AUTOLOAD << 1))
    s -= filter_objects(s,"QueryProp",P_AUTOLOADOBJ);

  if(flags & I_KEEP)
    s = filter(s,#'_check_keep);
  else if(flags & (I_KEEP << 1))
    s -= filter(s,#'_check_keep);

  armours = filter_objects(s, "QueryProp", P_ARMOUR_TYPE);
  // Kleidung dazu addieren, vorher die erkannten Ruestungen abziehen, die
  // muessen nicht nochmal durchiteriert werden.
  armours += filter_objects(s-armours, "IsClothing");
  // ja, die Zuweisung sieht doof aus.
  weapons = filter_objects((s = s-armours), "QueryProp", P_WEAPON_TYPE);
  misc = s - weapons; // rest ;-)

  if(flags & I_WEAPON)
  {
    s = weapons; misc = ({});
    if(!(flags & (I_ARMOUR))) armours = ({});
  }
  if(flags & I_ARMOUR)
  {
     s = armours; misc = ({});
     if(!(flags & I_WEAPON)) weapons = ({});
  }
  if(flags & (I_WEAPON << 1)) { weapons = ({}); s = armours + misc; }
  if(flags & (I_ARMOUR << 1)) { armours = ({}); s = weapons + misc; }

  output = "";
  if(flags & (I_FORMATTED | (I_FORMATTED << 1)))
  {
    s = make_invlist(this_player(), s, minv);
    if(flags & (I_SORT | (I_SORT << 1)))
    s = sort_array(s, #'sortinv/*'*/);
    output += sprintf("%"+format+"-78s\n",
                      implode(map(s,#'collectinv/*'*/),"\n"));
  }
  else
  {
    if(weapons && sizeof(weapons))
    {
      weapons = make_invlist(this_player(), weapons, minv);
      if(flags & (I_SORT | (I_SORT << 1)))
        weapons = sort_array(weapons, #'sortinv/*'*/);
      output += (ansi?ANSI_BOLD:"") + "Waffen:" + (ansi?ANSI_NORMAL:"")+"\n"
              + sprintf("%"+format+"-78s\n",
                        implode(map(weapons, #'collectinv/*'*/), "\n"));
    }
    if(armours && sizeof(armours))
    {
      armours = make_invlist(this_player(), armours, minv);
      if(flags & (I_SORT | (I_SORT << 1)))
        armours = sort_array(armours, #'sortinv/*'*/);
      output += (ansi?ANSI_BOLD:"") + "Kleidung & Ruestungen:" + (ansi?ANSI_NORMAL:"")+"\n"
              + sprintf("%"+format+"-78s\n",
                        implode(map(armours, #'collectinv/*'*/), "\n"));
    }
    if(misc && sizeof(misc))
    {
      misc = make_invlist(this_player(), misc, minv);
      if(flags & (I_SORT | (I_SORT << 1)))
        misc = sort_array(misc, #'sortinv/*'*/);
      output += (ansi?ANSI_BOLD:"") + "Verschiedenes:" + (ansi?ANSI_NORMAL:"")+"\n"
              + sprintf("%"+format+"-78s\n",
                        implode(map(misc, #'collectinv/*'*/), "\n"));
    }
  }

  if (output=="") 
    output += (ansi?ANSI_BOLD:"")+"Die Liste ist leer."+(ansi?ANSI_NORMAL:"");
  More(output);
  return 1;
}

private nosave int exa_cnt;
private nosave int exa_time;
private nosave string *exa;

varargs int _examine(string str, int mode)
{
  object base, *objs, env;
  string what, detail, parent, out, error;
  int i, size, done;

  if(CannotSee()) return 1;

  _notify_fail("Was willst Du denn untersuchen?\n");
  if (!str) return 0;

  if (member(({"boden","decke","wand","waende"}),old_explode(str," ")[0]) == -1) {
    exa_cnt -= (time() - exa_time)/2;
    exa_time = time();
    exa_cnt++;
    if (!exa)
      exa = ({ str });
    else
      exa += ({ str });
    if (exa_cnt > 10) {
      log_file("ARCH/LOOK", 
	  sprintf("%s: %s in %s\n%@O\n",dtime(time()),getuid(this_object()), 
	    environment() ? object_name(environment()) : "???",exa), 150000);
      exa_cnt = 0;
      exa = ({});
    }
    else if (exa_cnt < 0) {
      exa_cnt = 0;
      exa = ({});
    }
  }
  // do we look at an object in our environment ?
  if (sscanf(str,"%s in raum", what) || sscanf(str,"%s im raum", what))
      base = environment();
  // is the object inside of me (inventory)
  else if (sscanf(str,"%s in mir", what) || sscanf(str,"%s in dir", what))
      base = this_object();
  else {
      what = str;
      // get the last object we looked at
      base = QueryProp(P_REFERENCE_OBJECT);

      // if a reference object exists, test for its existance in the room
      // or in our inventory
      if (objectp(base)) {
        if(member(deep_inventory(environment()), base) != -1) {
          // check if base is inside of a living or non-transparent object
	  foreach(env : all_environment(base)) {
	    // Living oder intransparenter Container?
	    if (living(env) || !env->QueryProp(P_TRANSPARENT)) {
	      // in dem Fall ist ende, aber wenn das gefundene Env nicht
	      // dieses Living selber oder sein Env ist, wird das
	      // Referenzobjekt zusaetzlich genullt.
	      if (env != this_object() && env != environment())
		base = 0;
	      break;
	    }
	  }
	}
	else base = 0; // nicht im Raum oder Inventory
      }
  }

  // scan input if we want a specific object to look at
  if(sscanf(what, "%s an %s", detail, parent) == 2 ||
     sscanf(what, "%s am %s", detail, parent) == 2 ||
     sscanf(what, "%s in %s", detail, parent) == 2 ||
     sscanf(what, "%s im %s", detail, parent) == 2)
  {
    // if an ref object exists get its inventory, only if it is not a living
    if(base && !living(base))
      objs = base->locate_objects(parent, 1);
    else {
      // else get our inv and env
      objs = environment()->locate_objects(parent, 1)
           + locate_objects(parent, 1);
    }  
    if(sizeof(objs) > 1)
      return (notify_fail("Es gibt mehr als ein "+capitalize(parent)+".\n"), 0);
    else
      if(sizeof(objs)) base = objs[0];
      else return (notify_fail("Hier ist kein "+capitalize(parent)+".\n"), 0);
    objs = 0;
  }
  else detail = what;

  do {
    // if a base exists get its inventory, else get our inv and env
    if (base) {
      if (base == this_object() || base == environment() ||
         (base->QueryProp(P_TRANSPARENT) && !living(base)))
        objs = base->locate_objects(detail, 1);
    }
    else {
      objs = environment()->locate_objects(detail, 1)
           + locate_objects(detail, 1);
      base = environment();
    }

    if(!pointerp(objs) || !sizeof(filter_objects(objs,"short"))) {
      if((out = base->GetDetail(detail, QueryProp(P_REAL_RACE),SENSE_VIEW)) ||
         (out = base->GetDoorDesc(detail)))
      {
        SetProp(P_REFERENCE_OBJECT, base);
        return (write(out), 1);
      }
      else {
        if (base == environment()) {
          return (_notify_fail((error?error:"")+"Sowas siehst Du "+(error?
                                                                    "auch ":"")
                              +"da nicht!\n"), 0);
	}
        else {
          error = "Du findest an "+base->name(WEM)+" kein \""
                + capitalize(detail)+"\".\n"
                + "Dein Blick wendet sich der Umgebung zu.\n";
          base = 0;
        }
      }
    }
    else done = 1;
  } while(!done);

  // Examine all found objects
  _notify_fail("Sowas siehst Du nicht.\n");
  foreach(object ob: objs) {
    if (ob->short()) {
      SetProp(P_REFERENCE_OBJECT, ob);
      tell_object(ME, ob->long(mode));
      return 1;
    }
  }

  return 0;
}

varargs int _sense_exa(string str, int mode)
{
  object base, *objs, env;
  string what, detail, parent, out, error;
  int i, size, done, sense;

  if(member(({"lausche","lausch","hoer","hoere"}),query_verb())==-1){
    _notify_fail("Du kannst nichts Besonderes riechen.\n");
    sense = SENSE_SMELL;
  }else{
      if(QueryProp(P_DEAF))
          return notify_fail("Du bist taub!\n"), 0;

    _notify_fail("Du kannst nichts Besonderes hoeren.\n");
    sense = SENSE_SOUND;
  }

  if (!str){
    if(!detail =
        environment()->GetDetail(SENSE_DEFAULT,QueryProp(P_REAL_RACE),sense))
      return 0;
    write(detail);
    return 1;
  }
  else if(sscanf(str,"an %s",what)==1)str=what;

  // do we look at an object in our environment ?
  if (sscanf(str,"%s in raum", what) || sscanf(str,"%s im raum", what))
    base = environment();
  else
   // is the object inside of me (inventory)
   if (sscanf(str,"%s in mir", what) || sscanf(str,"%s in dir", what))
     base = this_object();
   else
   {
     what = str;
     // get the last object we looked at
     base = QueryProp(P_REFERENCE_OBJECT);

     // if a reference object exists, test for its existance in the room
     // or in our inventory
     if(base)
       if(member(deep_inventory(environment()), base) != -1)
       {
         env = base;
         // check if base is inside of a living or non-transparent object
         while((env = environment(env)) &&
               !living(env) && env->QueryProp(P_TRANSPARENT))
           /* do nothing */;
         if(env && env != this_object() && env != environment()) base = 0;
       }
       else base = 0;
   }

  // scan input if we want a specific object to look at
  if(sscanf(what, "%s an %s", detail, parent) == 2 ||
     sscanf(what, "%s am %s", detail, parent) == 2 ||
     sscanf(what, "%s in %s", detail, parent) == 2 ||
     sscanf(what, "%s im %s", detail, parent) == 2)
  {
    // if an ref object exists get its inventory, only if it is not a living
    if(base && !living(base))
      objs = base->locate_objects(parent, 1);
    else
      // else get our inv and env
      objs = environment()->locate_objects(parent, 1)
           + locate_objects(parent, 1);
    if(sizeof(objs) > 1)
      return (notify_fail("Es gibt mehr als ein "+capitalize(parent)+".\n"), 0);
    else
      if(sizeof(objs)) base = objs[0];
      else return (notify_fail("Hier ist kein "+capitalize(parent)+".\n"), 0);
    objs = 0;
  }
  else detail = what;

  do {
    // if a base exists get its inventory, else get our inv and env
    if(base)
      if(base == this_object() || base == environment() ||
         (base->QueryProp(P_TRANSPARENT) && !living(base)))
        objs = base->locate_objects(detail, 1);
      else ;
    else
    {
      objs = environment()->locate_objects(detail, 1)
           + locate_objects(detail, 1);
      base = environment();
    }

    if(!objs || !sizeof(objs))
      if(out = base->GetDetail(detail, QueryProp(P_REAL_RACE),sense))
      {
        SetProp(P_REFERENCE_OBJECT, base);
        return (write(out), 1);
      }
      else
        if(base == environment())
          return 0;
        else
        {
          base = 0;
        }
    else done = 1;
  } while(!done);

  // Examine all found objects
  size = sizeof(objs);
  for (i = 0; i < size; i++)
    if(objs[i]->short() &&
       (out=objs[i]->GetDetail(SENSE_DEFAULT,QueryProp(P_REAL_RACE),sense)))
    {
      SetProp(P_REFERENCE_OBJECT, objs[i]);
      write(out);
      return 1;
    }

  return 0;
}

varargs int look_into(string str,int mode)
{
  object *found_obs;

  if( CannotSee() ) return 1;
  _notify_fail("Wo willst Du denn reinschauen ?\n");
  found_obs=find_obs(str,PUT_GET_NONE);
  if (!found_obs)
  {
    if (environment() &&
        (environment()->GetDetail(str,QueryProp(P_REAL_RACE))||
         environment()->GetDoorDesc(str)))
      _notify_fail("Da kannst Du so nicht reinsehen.\n");
    return 0;
  }
  return _examine(str, mode);
}

/* Gebe die Umgebung des aktiven Spielers zurueck, lasse dabei  */
/* rekursiv geschachtelte Raeume zu.                            */
/* Wenn allow_short 0 ist, so wird immer die long-descr benutzt */
varargs string env_descr(int allow_short,int flags, int force_short )
{
  object env;
  int brief;

  env = environment(ME);

  if(!env)
    return "Du schwebst im Nichts ... Du siehst nichts, rein gar nichts ...\n";

  if (!force_short && (!allow_short || !QueryProp(P_BRIEF))) return env->int_long(ME,ME,flags);
  if (!flags && ((brief=QueryProp(P_BRIEF))>=2)) return "";
  return env->int_short(ME,ME);
}

int _look(string str)
{
  string s;
  int flag;

  if(CannotSee()) return 1;

  if(!str)
  {
    SetProp(P_REFERENCE_OBJECT, 0);
    write( env_descr() );
    return 1;
  }
  if(str=="-f" || str=="genau")
  {
    SetProp(P_REFERENCE_OBJECT, 0);
    write( env_descr(0,2) );
    return 1;
  }
  if(str=="-k" || str=="kurz")
  {
    SetProp(P_REFERENCE_OBJECT, 0);
    write( env_descr(1,2,1) );
    return 1;
  }
  if(str[0..2]=="-f "){
    flag=2;
    str=str[3..];
  }
  else if(str[0..5]=="genau "){
    flag=2;
    str=str[6..];
  }
  else flag = 0;
  if (sscanf(str,"%s an",s)) str=s;
  if (sscanf(str,"%s in mir",s)||sscanf(str,"%s in dir",s)) return _examine(str,flag);
  if (sscanf(str,"in %s",s)) return look_into(s,flag);
  return _examine(str,flag);
}

int _equipment(string arg)
{
  if (CannotSee()) return 1;
  call_other("/std/player/invmaster/invmaster", "ShowInv", ME, arg);
  return 1;
}

static string *_query_localcmds()
{
  return
    ({({"ausruestung", "_equipment",0,0}),
      ({"i","_inventory",0,0}),
      ({"inv","_inventory",0,0}),
      ({"inventur","_inventory",0,0}),
      ({"schau","_look",0,0}),
      ({"schaue","_look",0,0}),
      ({"unt","_examine",0,0}),
      ({"untersuch","_examine",0,0}),
      ({"betracht","_examine",0,0}),
      ({"untersuche","_examine",0,0}),
      ({"betrachte","_examine",0,0}),
      ({"betr","_examine",0,0}),
      ({"lausche","_sense_exa",0,0}),
      ({"lausch","_sense_exa",0,0}),
      ({"hoer","_sense_exa",0,0}),
      ({"hoere","_sense_exa",0,0}),
      ({"schnupper","_sense_exa",0,0}),
      ({"schnuppere","_sense_exa",0,0}),
      ({"riech","_sense_exa",0,0}),
      ({"rieche","_sense_exa",0,0}),
      ({"kurz","_toggle_brief",0,0}),
      ({"lang","_toggle_brief",0,0}),
      ({"ultrakurz","_toggle_brief",0,0}) 
    });
}
