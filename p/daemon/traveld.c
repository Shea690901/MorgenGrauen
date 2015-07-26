
/*
 * Ueberarbeitete und 
 * erweiterte Version: 10.01.02 Tilly@MorgenGrauen
 * Basierend auf     : Revision 1.1  11.05.99 Woody@SilberLand, 
 *                     SilberLand MUDlib
 */

#pragma strong_types,save_types
#pragma no_clone,no_shadow

#define NEED_PROTOTYPES

#include <config.h>
#include <properties.h>
#include <transport.h>
#include <wizlevels.h>
#include <daemon.h>

mapping ships;
mapping harbours;

void create()
{
  seteuid(getuid(this_object()));

  if (!restore_object((TRAVELD_SAVEFILE)))
  {
    ships    = ([]);
    harbours = ([]);
    return;
  }
}
                                                                                              
public void AddHarbour(string ship, string harbour)
{
  if (!stringp(ship)) return;
  if (!stringp(harbour)) return;
  
  if ((ship[0..2] == "/p/" || ship[0..8] == "/players/") &&
      !IS_ARCH(getuid(previous_object()))) 
  {
    return;
  }
   
  call_other(harbour,"???");

  if (find_object(ship) && find_object(ship)->QueryProp(P_NO_TRAVELING) == 0)
  {
    if (!sizeof(ships))
    {
      ships = ([ ship : ({ harbour }) ]);
    }
    else if (member(ships,ship) <= 0)
    {
      ships += ([ ship : ({ harbour }) ]);
    }
    else if (member_array(harbour,ships[ship]) == -1)
    {
      ships[ship] += ({ harbour });
    }
  }
  
  if (find_object(harbour) && 
      find_object(harbour)->QueryProp(P_NO_TRAVELING) == 0)
  {
    if (find_object(ship) && find_object(ship)->QueryProp(P_NO_TRAVELING) > 0)
    {
      return;
    }
    else if (!sizeof(harbours))
    {
      harbours = ([ harbour : ({ ship }) ]);
    }
    else if (member(harbours,harbour) <= 0)
    {
      harbours += ([ harbour : ({ ship }) ]);
    }
    else if (member_array(ship,harbours[harbour]) == -1)
    { 
      harbours[harbour] += ({ ship });
    }
  }
  save_object((TRAVELD_SAVEFILE));
}

void reset()
{
  if (extern_call() && previous_object()) return;

  save_object((TRAVELD_SAVEFILE));
}

int remove()
{
  save_object((TRAVELD_SAVEFILE));
  destruct(this_object());
  return 1;
}

public void RemoveShip(object ship)
{
  if (extern_call() && !IS_ARCH(getuid(previous_object()))) return;

  if(member(ships,object_name(ship)) >= 1)
  {
    ships -= ([ object_name(ship) ]);
  }

  save_object((TRAVELD_SAVEFILE));
}

public varargs mixed HasShip(object harbour, string ship)
{
  mixed obs;

  if (!objectp(harbour)) return 0;
  if (!obs=harbours[object_name(harbour)]) return 0;

  obs = map(obs,#'find_object) - ({0});

  if (stringp(ship))
  {
    obs = filter_objects(obs,"id",ship);
  }
  return sizeof(obs)?obs:0;
}

public mixed RouteExists(object ship, string dest)
{
  if (!objectp(ship) || !stringp(dest))
  {
    return 0;
  }
  return ship->HasRoute(dest);
}

public mapping QueryShips() 
{ 
  return deep_copy(ships); 
}

public mapping QueryAllHarbours() 
{ 
  return deep_copy(harbours); 
}

