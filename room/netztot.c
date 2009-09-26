// MorgenGrauen MUDlib
//
// $Id: netztot.c 7155 2009-02-25 21:20:50Z Zesstra $

#include <wizlevels.h>
#include <moving.h>

create()
{
  call_out("invcheck",120);
  "/obj/sperrer"->NixGibts();
}

weg(ob)
{
  if (!objectp(ob))
    return;
  ob->remove();
  if (ob)
    destruct(ob);
}

wegraeumen(ob)
{
  object *x;
  
  if (!objectp(ob))
    return;
  for (x=deep_inventory(ob);sizeof(x);x=x[1..])
    weg(x[0]);
  ob->move("/room/void",M_NOCHECK|M_SILENT);
  weg(ob);
}

invcheck()
{
  object *inv;
  int i;
  
  while(remove_call_out("invcheck")!=-1);
  inv=all_inventory(this_object());
  for (i=sizeof(inv)-1;i>0;i--)
    if (interactive(inv[i]))
      set_object_heart_beat(inv[i],1);
    else
      if (!query_once_interactive(inv[i]) && object_name(inv[i])!="/obj/sperrer")
	call_out("wegraeumen",1,inv[i]);
  call_out("invcheck",120);
  "/obj/sperrer"->upd();
}

init()
{
  if (!this_player())
    return;
  catch(this_player()->StopHuntingMode());
  if (interactive(this_player()))
    catch(this_player()->move("room/gilde",M_GO));
  if (!query_once_interactive(this_player()))
  {
    this_player()->remove();
    if(this_player())
      destruct(this_player());
  }
  set_object_heart_beat(this_player(),0);
}

int_long()
{
  return "Dies ist der Netztotenraum. Es ist dunkel. Du siehst nichts.\n";
}

int_short()
{
  return "Nichts zu sehen.\n";
}

QueryProp(string str)
{
  switch (str) {
    case "int_long": return int_long();
    case "int_short": return int_short();
  }
}

// Nicht jeder Magier darf den Netztotenraum entsorgen.
string NotifyDestruct(object caller) {
    if( (caller!=this_object() && !ARCH_SECURITY) || process_call() ) {
      return "Du darfst den Netztotenraum nicht zerstoeren!\n";
    }
}

