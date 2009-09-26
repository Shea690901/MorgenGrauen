#include <properties.h>
#include <moving.h>
#include "/secure/master.h"

object *obs;

private static int allowed()
{
  if (!process_call() && previous_object() && this_interactive() && ARCH_SECURITY)
    return 1;
  return 0;
}

create() {}

make(ob)
{
  object *list, env;
  string file,temp,dummy,err;
  int i,cloned;

  if (!allowed()) return -1;
  env=environment(ob);
  file=object_name(ob);
  write("Updating "+object_name(ob)+"...\n");
  if (sscanf(file,"%s#%s",temp,dummy)==2)
  {
    file=temp;
    cloned=1;
  }
  else
    cloned=0;
  list=all_inventory(ob);
  for (i=sizeof(list)-1;i>=0;i--)
    if (list[i] && query_once_interactive(list[i]))
    {
      list[i]->move("room/void",M_TPORT | M_SILENT | M_NO_SHOW | M_NOCHECK);
    } else
      list[i]=0;
  list-=({0});
  
  if (ob)
  {
    ob->remove();
    if( ob ) destruct(ob);
  }
  if (cloned)
  {
    if (ob=find_object(file))
    {
      ob->remove();
      if (ob)
	destruct(ob);
    }
    err=catch(ob=clone_object(file));
    if (!err)
      ob->move(env,M_TPORT | M_SILENT | M_NO_SHOW | M_NOCHECK);
  }
  else
  {
    err=catch(call_other(file,"???"));
    if (!err)
      ob=find_object(file);
    else
      ob=0;
  }

  if (!ob)
  {
    write("Error in loaded object. Staying in void ...\n");
    return;
  }
  for (i=sizeof(list)-1;i>=0;i--)
    if (list[i])
      list[i]->move(ob,M_TPORT | M_SILENT | M_NO_SHOW | M_NOCHECK);
}

updall(all)
{
  mixed *todo;
  int i;

  tell_object(find_player("jof"),sprintf("%d left\n",sizeof(all)));
  if (sizeof(all)>25)
  {
    todo=all[0..24];
    call_out("updall",1,all[24..]);
  } else todo=all; 
  for (i=sizeof(todo)-1;i>=0;i--)
    make(todo[i]);
}

doone(ob)
{
  if (member(inherit_list(ob),"/std/room.c")!=-1)
    obs+=({ob});
}

doit()
{
  object ob;

  obs=({});
  ob=debug_info(2,0);

  while (ob)
  {
    doone(ob);
    ob=debug_info(2,ob);
  }
  write("Done with "+sizeof(obs)+" obs ...\n");
  call_out("updall",1,obs);
}
