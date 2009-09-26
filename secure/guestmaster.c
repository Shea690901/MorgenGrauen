// MorgenGrauen MUDlib
//
// guestmaster.c -- Verwalter von Gast-Logins
//
// $Id: guestmaster.c 6142 2007-01-31 20:34:39Z Zesstra $

static int max_guests; /* Max no. of guests. -1 is 'unlimited' */
static object *guests;
static int    *ltime;

nomask void create() {
  if (sizeof(old_explode(object_name(this_object()),"#")) != 1) {
    destruct(this_object());
    return;
  }
  guests = ltime = ({});
  max_guests = 10;
}

nomask int new_guest () {
  int ix;

  if (!max_guests) return 0;
  if (old_explode(object_name(previous_object()),"#")[0] != "/secure/login")
    return 0;
  for (ix = 0; ix < sizeof(guests); ix++)
  {
    if (guests[ix] && !interactive(guests[ix]))
    {
       guests[ix] = 0;
    }
  }

  if ((ix = member_array(0, guests)) == -1) {
    ix = sizeof(guests);
    if (max_guests < 0 || ix < max_guests) 
    {
      guests += ({ 0 }), ltime += ({ 0 });
    }
    else {
      int mintime, minix;
      mintime = time();
      for (ix = 0; ix < sizeof(guests); ix++) {
        if (guests[ix] && ltime[ix] < mintime) mintime=ltime[ix], minix=ix;
      }
      ix = minix;
    }
  } 
  return ix+1;
}

nomask void set_guest (int ix, object pl) {
  if (old_explode(object_name(previous_object()),"#")[0] != "/secure/login") return;
  guests[ix-1] = pl;
  ltime[ix-1] = time();
}

nomask int query_max_guests() { return max_guests; }
nomask int set_max_guests(int v) {
  if (this_player() && query_wiz_level(this_player()) >= 40)
    max_guests = v;
  return max_guests;
}

