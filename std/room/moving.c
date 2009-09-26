// MorgenGrauen MUDlib
//
// room/moving.c -- Entfernen von Raeumen
//
// $Id: moving.c 6371 2007-07-17 22:46:50Z Zesstra $

#pragma strong_types
#pragma save_types
#pragma pedantic
#pragma range_check
#pragma no_clone

int remove()
{
  destruct( this_object() );
  return 1;
}
