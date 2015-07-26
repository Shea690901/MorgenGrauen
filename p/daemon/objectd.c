// MorgenGrauen MUDlib
//
// OBJECTD.C -- object daemon
//
// $Date: 1995/04/03 14:47:02 $
// $Revision: 8099 $
/* $Log: objectd.c,v $
 * Revision 1.2  1995/04/03 14:47:02  Wargon
 * QueryObject() verwendet bei BluePrints jetzt auch AddItem.
 *
 * Revision 1.1  1995/03/31  13:30:33  Hate
 * Initial revision
 *
 */

#pragma strong_types,save_types
#pragma no_clone, no_shadow

#include <rooms.h>
#include <properties.h>
#include <defines.h>
#include <daemon.h>

mapping objects;
private nosave int do_save;

#define CLASS   0
#define DATA    1

void create()
{
  seteuid(getuid(ME));
  objects = ([]);
  restore_object(OBJECTD_SAVE);
}

protected void reset() {
  if (do_save) {
    save_object(OBJECTD_SAVE);
    do_save=0;
  }
}

string AddObject(object obj, string env)
{
  if(!obj || !env || !objectp(obj) || !stringp(env)) return 0;

  // save information
  if(!member(objects, env))
    objects[env] = ({ ({  object_name(obj), obj->QueryProp(P_AUTOLOAD) }) });
  else
    objects[env] += ({ ({ object_name(obj), obj->QueryProp(P_AUTOLOAD) }) });

  do_save=1;
  return env;
}

int RemoveObject(object obj, string env)
{
  if(!obj || !env || !objectp(obj) || !stringp(env)) return 0;

  // save information
  if(member(objects, env))
  { int i;
    i = sizeof(objects[env]);
                while(i && (objects[env][i-1][CLASS] != object_name(obj))) i--;
    if(i > 0) objects[env][i-1..i-1] = ({});
  }
  if(!sizeof(objects[env]))
    m_delete(objects, env);

  do_save=1;
  return 1;
}

varargs void QueryObject(mixed env)
{
  mixed obj, oenv; int i;

  // take the caller as the questioning object
  if(!env || !stringp(env)) env = previous_object();
  else env = find_object(env);

  // target must be a blueprint
  if(IS_CLONE(env)) return 0;

  if(i = pointerp(obj = objects[env = object_name(oenv = env)])?sizeof(obj):0)
  {
    while(i--)
    {
       object o;
       string *n;

       n = old_explode(objects[env][i][CLASS], "#");
       if (sizeof(n) > 1)
         o = oenv->AddItem(n[0], REFRESH_DESTRUCT);
       else 
         o = oenv->AddItem(n[0], REFRESH_DESTRUCT, 1);
       o->SetProp(P_AUTOLOAD, objects[env][i][DATA]);
       objects[env][i][CLASS] = object_name(o);
    }
  }
}

mixed QueryObjects(mixed env)
{
  mixed obj;

  // take the caller as the questioning object
  if(!env || !stringp(env)) env = previous_object();
  else env = find_object(env);

  // target must be a blueprint
  if(IS_CLONE(env)) return 0;

  if(obj = objects[object_name(env)])
    obj = map(obj, #'[, 0);
  return obj;
}

int remove(int silent)
{
  save_object(OBJECTD_SAVE);
  destruct(ME);
  return 1;
}

