// MorgenGrauen MUDlib
// 
// zeitungsautomat.c -- Zeitungsautomat in der Abenteurergilde
// 
// $Id:	zeitungsautomat.c 6437 2007-08-18 22:58:58Z Zesstra $
// 

inherit "/std/thing";

#include <properties.h>
#include <language.h>
#include <moving.h>
#include <defines.h>
#include <www.h>

#define ZEITUNG "/obj/mpa"

void create()
{
  if (!clonep(this_object())) return;
  ::create();
  SetProp(P_SHORT,"Eine Zeitung");
  SetProp(P_LONG, 
    "Hier stehen immer die neuesten Nachrichten und Geruechte drin.\n");
  SetProp(P_NAME,"Zeitung");
  AddId(({"zeitung","mpa","servicepaket"}));
  SetProp(P_GENDER,FEMALE);
  SetProp(P_NOGET,1);
  SetProp(P_PICK_MSG, NO_PNG_MSG);
  SetProp(P_ARTICLE,1);
  SetProp(P_WWWINFO, "Man kann die MPA auch <A HREF=\"http://"
      +SERVER+MUDWWW + "?" +REQ + "=" + R_NEWS + "\">lesen</A>");
}

//#define FBMASTER "/p/daemon/fbmaster"

/*
void init()
{
    // call_out("putfb", 0, this_player());
    //add_action("nimm","nimm");
}
*/

/*
void putfb(object pl)
{
   string q;
   if(strstr(object_name(pl), "#") != -1) return;
  
   if((q = FBMASTER->online()) && interactive(pl))
    {
        if(!FBMASTER->done(pl, q))
            if(!present("fragebogen", pl))
            {
                object fb;
                (fb = clone_object("/p/service/hate/fragebogen"))->move(pl,M_GET);
                fb->ask(q);
            }
    }
}
*/

/*
int nimm(string str)
{
        if (!id(str)) return 0;
        ME->move(this_player(),M_GET);
        return 1;
}
*/

int get(object targ) {
  int res;

  if (!objectp(targ) || !interactive(targ))
    return ME_CANT_BE_TAKEN;
  
  if (present_clone(ZEITUNG, targ)) {
    tell_object(targ, "Du hast doch bereits eine Zeitung.\n");
    return ME_CANT_BE_TAKEN;
  }
  
  object zeitung=clone_object("/obj/mpa");
  if ((res=zeitung->move(targ,M_GET)) == MOVE_OK) {
    tell_object(targ, "Du nimmst die Zeitung. Sofort erscheint auf "
	"unerklaerliche Weise eine neue.\n");
    tell_room(environment(targ), break_string(
	targ->name(WER) + " nimmt die Zeitung. Sofort erscheint auf "
	"unerklaerliche Weise eine neue.\n",78),({targ}));
  }
  else {
    tell_object(targ, "Du kannst die Zeitung leider nicht mehr tragen.\n");
    zeitung->remove(1);
  }
  
  return res;
}

varargs int move(object target, mixed method)
{
  if (method&M_GET && living(target))
    return get(target);
  
  return ::move(target,method);
}

