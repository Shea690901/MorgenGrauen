// MorgenGrauen MUDlib
//
// npc.c -- a generic npc (non player character)
//
// Testversion mit support fuer AddItem
// soll spaeter in npc.c aufgehen
//
// $Id: npc.c 7124 2009-02-14 22:16:12Z Zesstra $
#pragma strong_types
#pragma save_types
#pragma range_check
#pragma no_clone
#pragma pedantic

inherit "/std/thing/properties";
inherit "/std/hook_provider";
inherit "/std/living/description";
inherit "/std/living/life";
inherit "/std/living/attributes";
inherit "/std/living/moving";
inherit "/std/living/skill_attributes";
inherit "/std/living/skills";
inherit "/std/living/clothing";
inherit "/std/npc/combat";
inherit "/std/npc/chat";
inherit "/std/npc/comm";
inherit "/std/container/restrictions";
inherit "/std/thing/commands";
inherit "/std/thing/language";
inherit "/std/npc/info";
inherit "/std/npc/put_and_get";
inherit "/std/npc/items";
inherit "/std/thing/envchk";

#include <config.h>
#include <properties.h>
#include <ansi.h>
#include <wizlevels.h>
#include <living.h>
#include <language.h>
#include <attributes.h>
#include <defines.h>
#include <health.h>
#include <npc.h>
#include <moving.h>
#include <guard.h>

static int _set_level(int arg);

protected void create()
{ 
  seteuid(getuid());
  properties::create();
  restrictions::create();
  commands::create();
  description::create();
  attributes::create();
  clothing::create();
  life::create();
  enable_commands();
  comm::create();
  combat::create();
  info::create();
  put_and_get::add_put_and_get_commands();
  add_team_commands();
  items::create();
  envchk::create();
  add_action("UseSpell","",1);

  SetProp(P_LIGHT_MODIFIER, 1);
  SetProp(P_WEIGHT_PERCENT, 100);
  SetProp(P_NAME, "Niemand");
  SetProp(P_MSGIN, "kommt an");
  SetProp(P_MSGOUT, "geht");
  SetProp(P_MMSGIN, "erscheint in einer Rauchwolke");
  SetProp(P_MMSGOUT, "verschwindet mit Knall und Schwefelduft");
  SetProp(P_GENDER, NEUTER );
  SetProp(P_WEIGHT, 75000);
  SetProp(P_MAX_WEIGHT, 50000);
  SetProp(P_RACE, "Npc");
  SetProp(P_MAX_HP,100);
  SetProp(P_MAX_SP,100);
  SetProp(P_HP,50);
  SetProp(P_SP,50);
  SetProp(P_MAX_ALCOHOL,100);
  SetProp(P_MAX_DRINK,100);
  SetProp(P_MAX_FOOD,100);
  SetProp(P_DRINK, 0);
  SetProp(P_FOOD, 0);
  SetProp(P_ALCOHOL, 0);
  SetProp(P_HANDS, ({ "", 30 }) );
  SetProp(P_MAX_HANDS, 2);
  SetProp(P_USED_HANDS, 0);
  SetProp(P_NPC, 1);
  SetProp(P_GUARD,100);
  SetProp(P_NO_TPORT,NO_TPORT_IN);

  set_heart_beat(1);
  heartbeat=1;
}

protected void create_super() {
  set_next_reset(-1);
}

void reset(){
  items::reset();
  combat::reset();
}

static int _set_level(int arg)
{
  Set(P_LEVEL, arg);
  SetAttr(A_CON, arg);
  SetAttr(A_DEX, arg);
  SetAttr(A_INT, arg);
  SetAttr(A_STR, arg);
  return(arg); //Rueckgabewert noetig!
}

varargs void create_default_npc(int level, int maxhp)
{ 
  if(level < 1) return;
  SetProp(P_LEVEL, level);
  if (maxhp<=0) maxhp = 42 + level*8;
  SetProp(P_MAX_HP, maxhp);
  SetProp(P_MAX_SP, maxhp);
  SetProp(P_HANDS, ({" mit blossen Haenden", level*10 }) );
  SetProp(P_BODY, (level*20)/3);
  SetProp(P_XP, level * maxhp * 50);
} 

protected void heart_beat()
{
  if( environment() ) life::heart_beat();
  if (!this_object()) return; // Evtl. an Gift gestorben...
  combat::heart_beat();
  chat::heart_beat();
}

void give_notify(object ob)
{
  put_and_get::give_notify(ob);
}

// Force the monster to do a command.
int command_me(string cmd) { return command(cmd); }

void init()
{
  combat::init();
  info::init();
  commands::init();
//  description::init();
}

void catch_tell(string str)
{}

// items initialisieren?
protected void NotifyMove(object dest, object oldenv, int method) {
    
  ::NotifyMove(dest, oldenv, method);
  // gestorben?
  if (!objectp(this_object())) return;

  if ( Query(NPC_NEEDS_ITEM_INIT, F_VALUE))
      _clone_items();
}

string _query_race()
{
   if (stringp(Query(P_RACE))) return capitalize(Query(P_RACE));
}