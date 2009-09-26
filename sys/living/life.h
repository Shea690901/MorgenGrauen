// MorgenGrauen MUDlib
//
// living/life.h -- living life header
//
// $Id: life.h 6276 2007-04-29 12:40:59Z Zesstra $

#ifndef __LIVING_LIFE_H__
#define __LIVING_LIFE_H__

// properties

#define P_AGE                     "age"
#define P_ALIGN                   "align"

#define P_DEADS                   "deads"
#define P_GHOST                   "ghost"
#define P_FROG                    "frog"

#define P_FOOD                    "food"
#define P_MAX_FOOD                "max_food"
#define P_DRINK                   "drink"
#define P_MAX_DRINK               "max_drink"
#define P_ALCOHOL                 "alcohol"
#define P_MAX_ALCOHOL             "max_alcohol"

#define P_HP                      "hp"
#define P_MAX_HP                  "max_hp"
#define P_SP                      "sp"
#define P_MAX_SP                  "max_sp"
#define P_XP                      "xp"
#define P_NO_XP                   "no_xp"
#define P_LAST_XP                 "last_xp"

#define P_FOOD_DELAY              "food_delay"
#define P_DRINK_DELAY             "drink_delay"
#define P_ALCOHOL_DELAY           "alcohol_delay"
#define P_HP_DELAY                "hp_delay"
#define P_SP_DELAY                "sp_delay"
#define P_POISON_DELAY            "poison_delay"

#define P_POISON                  "poison"
#define P_MAX_POISON              "max_poison"

#define P_NO_REGENERATION         "no_regeneration"

#define P_ENEMY_DAMAGE            "enemy_damage"
#define P_TIMING_MAP              "timing_map"
#define P_LAST_DEATH_TIME         "last_death_time"
#define P_LAST_DEATH_PROPS        "last_death_props"
#define P_CORPSE_DECAY_TIME       "corpse_decay"

#endif // __LIVING_LIFE_H__

#ifdef NEED_PROTOTYPES

#ifndef __LIVING_LIFE_H_PROTO__
#define __LIVING_LIFE_H_PROTO__         

// prototypes

public int do_damage(int dam, mixed enemy);
public int reduce_hit_points(int dam);
public int restore_hit_points(int heal);

public int reduce_hit_point(int dam);  //Nur als Uebergansloesung (Rikus)

public void restore_spell_points(int h);
public void reduce_spell_points(int h);

public void heal_self(int h);

public varargs void die( int poisondeath, int extern );

public varargs void transfer_all_to( mixed dest, int check );

public varargs int drink_alcohol(int strength, int testonly, string mytext);
public varargs int drink_soft(int strength, int testonly, string mytext);
public varargs int eat_food(int strength, int testonly, string mytext);

public int buffer_hp(int val,int rate);
public int buffer_sp(int val,int rate);

protected void heart_beat();

public void show_age();

public int AddExp(int e);

// Set- und Query-Methoden
static int     _set_align(int a);
static int     _set_hp(int hp);
static int     _set_sp(int sp);
static int     _set_alcohol(int n);
static int     _set_drink(int n);
static int     _set_food(int n);
static int     _set_poison(int n);
static int     _query_age();
static int     _set_xp(int xp);
static mapping _query_enemy_damage();
static mixed  *_set_last_xp( mixed last );
static mixed   _set_die_hook(mixed hook);

// internal
private void _transfer( object *obs, mixed dest, int flag );
protected varargs void create_kill_log_entry(string killer, object enemy);
protected object GiveKillScore(object pl, int npcnum);
protected void update_buffers();

#endif // __LIVING_LIFE_H_PROTO__

#endif // NEED_PROTOYPES
