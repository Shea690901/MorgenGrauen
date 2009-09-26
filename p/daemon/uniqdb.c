#pragma strong_types
#include "uniq.h"
#define ME this_object()
  
mapping uobjs;

void create() {
  if (clonep(ME))
    return destruct(ME);
  if (!restore_object(UNIQDB_SAVE))
    uobjs=m_allocate(0,2);
  seteuid(getuid(ME));
}

object CloneUniqObject(string fn) {
  object *obs,ob;
  int maxnum;

  maxnum=uobjs[fn,UNIQ_MAXNUM];
  if (maxnum<1)
    maxnum=1;
  obs=uobjs[fn,UNIQ_OBS];
  if (!pointerp(obs))
    obs=({});
  obs-=({0});
  if (sizeof(obs)>=maxnum)
    return 0;
  ob=clone_object(fn);
  obs+=({ob});
  maxnum=ob->QueryProp(P_MAX_CLONES);
  if (maxnum<1) {
    maxnum=1;
    ob->SetProp(P_MAX_CLONES,1);
  }
  uobjs[fn,UNIQ_MAXNUM]=maxnum;
  uobjs[fn,UNIQ_OBS]=obs;
  save_object(UNIQDB_SAVE);
  return ob;
}

object AltCloneUniqObject(string fn, string fn2) {
  object ob;
  
  if (objectp(ob=CloneUniqObject(fn)))
    return ob;
  return clone_object(fn2);
}

object *QueryUniqClones(string fn) {
  object *obs;
  
  obs=uobjs[fn,UNIQ_OBS];
  if (!pointerp(obs))
    obs=({});
  obs-=({0});
  return obs;
}

int remove() {
  return 0;
}
