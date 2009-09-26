// MorgenGrauen MUDlib
//
// room/items.c -- creating extra items in room
//
// $Id: items.c 7098 2009-01-31 18:17:12Z Zesstra $

#pragma strong_types
#pragma save_types
#pragma pedantic
#pragma range_check
#pragma no_clone

inherit "std/container/items";

protected void create_super() {
  set_next_reset(-1);
}     

void reset() 
{    
  ::reset();

  object *inh = all_inventory(this_object());
  if ( !pointerp(inh) || sizeof(inh) < 10 )
    return;
  // nur wenn keine Spieler anwesend sind.
  if ( !sizeof(inh & users()) )
    remove_multiple(3);
}
