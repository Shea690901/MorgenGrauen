// MorgenGrauen MUDlib
//
// player/restrictions.c -- container aspect of players
//
// $Id: restrictions.c 6813 2008-03-31 21:13:00Z Zesstra $

// This is a simple container to put objects in. It defines all functions
// which are necessary to describe an object which can be filled with
// other things.
//
// It will support restrictions for volume, weight etc.
//
// The following properties are defined:
// P_MAX_WEIGHT - maximum weight which container can carry
// P_WEIGHT_CONTENTS - current contents
// P_WEIGHT - builtin property: read->total weight, write->own weight
//
// Functions for manipulation of weight
// MayAddWeight(weight) - Can <weight> be inserted?
// AddWeight(weight) - Add an amount of <weight>
//
// IMPORTANT: unit should be interpreted as grams (g).
#pragma strong_types
#pragma save_types
#pragma range_check
#pragma no_clone
#pragma pedantic

inherit "/std/container/restrictions";

#define NEED_PROTOTYPES

#include <thing/properties.h>
#include <living/skills.h>
#include <properties.h>
#include <wizlevels.h>
#include <container.h>
#include <attributes.h>
#include <defines.h>
#include <new_skills.h>

// local properties prototypes
static int _query_max_weight();
static mixed _set_frog(mixed arg);

void create()
{
  ::create();
  
  Set(P_MAX_WEIGHT, NOSETMETHOD, F_SET_METHOD);
  Set(P_MAX_WEIGHT, SECURED, F_MODE);
}

// **** local property methods
static int _query_max_weight() {
  int str,val;
  mixed ski;
  
  if (QueryProp(P_GHOST) && !IS_WIZARD(ME))
    return 0;
  str=QueryAttribute(A_STR);
  ski = UseSkill(SK_CARRY, ([SI_SKILLARG : str ]));

  if (!intp(ski))
    ski = 0;
  
  if (str<0) {
    val=9200+str*160+(int)ski;
    if (val<3000) val=3000;
    return val;
  }
  val = 9200+str*800+(int)ski;
  if (val<3000)
    val = 3000;
  return val;
}

static object *InsertHooks=({});

varargs int PreventInsert(object ob)
{
  int i;
  
  i=sizeof(InsertHooks)-1;

  while (i>=0) {
    if (!objectp(InsertHooks[i]) || environment(InsertHooks[i])!=ME)
      InsertHooks[i]=0; // loeschen, s.u.
    else
      InsertHooks[i]->InsertNotify(ob);
    i--;
  }
  InsertHooks-=({0}); // genullte Elemente loeschen
  return 0;
}

void AddInsertHook(object ob)
{
  if (member(InsertHooks,ob)!=-1 || environment(ob)!=this_object())
    return;
  InsertHooks+=({ob});
}

void RemoveInsertHook(object ob)
{
  InsertHooks-=({ob});
}

object *QueryInsertHooks()
{
  return InsertHooks;
}

static mixed _set_frog(mixed arg) {
  mixed res;
  
  res=Set(P_FROG,arg);
  if (res)
    SetProp(P_ATTRIBUTES_MODIFIER,({"#frosch",([A_STR:-30])}));
  else
    SetProp(P_ATTRIBUTES_MODIFIER,({"#frosch",0 }));
  return res;
}
