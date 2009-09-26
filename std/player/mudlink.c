// MorgenGrauen MUDlib
//
// PLAYER/MUDLINK.C -- mudlink command handling
//
// $Date: 1994/01/13 16:59:35 $                      
// $Revision: 6371 $       
/* $Log: mudlink.c,v $
 * Revision 2.0  1994/01/13 16:59:35  mud
 * *** empty log message ***
 *
 * Revision 1.3  1994/01/08  13:12:20  mud
 * *** empty log message ***
 *
 * Revision 1.2  1994/01/05  16:17:07  mud
 * changed save variables partly to mapping entry
 *             
 */                            
#pragma strong_types
#pragma save_types
#pragma range_check
#pragma no_clone
#pragma pedantic

void create() {}

void AddLinkCommands()
{
  add_action("rpeers","rpeers");
  add_action("rwho","rwho");
  add_action("rtell","rtell");
}

int rpeers() {
  string u;
  u = geteuid(this_player());
  if(stringp(u) && find_player("mudlink"))
    tell_object(find_player("mudlink"),"rpeers "+u+"\n");
  return 1;
}

int rwho(string str) {
  string u;
  if (!str || str == "") {
    write("Usage: rwho <mud>\n");
    return 1;
  }
  if (stringp(u = geteuid(this_player())))
    tell_object(find_player("mudlink"),"rwho "+u+"="+str+"\n");
  return 1;
}

int rtell(string str) {
  string u;
  string a, b, c, d;

  if (!str || str == "") {
    write("Usage: rtell <player>@<mud> <message>\n");
    return 1;
  }
  if (sscanf(str, "%s@%s %s", a, b, c) != 3) {
    write("Usage: rtell <player>@<mud> <message>\n");
    return 1;
  }
  if (stringp(u = geteuid(this_player())))
    tell_object(find_player("mudlink"),"rpage "+u+"@"+a+"@"+b+"="+c+"\n");
  return 1;
}
