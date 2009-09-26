/* This file contents the functions necessary for to get doors and their paths
 *
 * Part of RoomMaker V0.96 (c) Sique 1/1993
 *
 * Author: Sique 1/93
 *
 * Last Update: Jan 20 1993
 */

static string *door;     /* contains the door commands */
static string *dpath;    /* contains the door paths */
static string *dshort;   /* contains the door short descriptions */ 
static mixed  *dlong;    /* contains the door long descriptions */
static string *dkey;     /* contains the key ids for the doors */
static int    *dstatus;  /* contains the door status */
static string *statlist; /* contains the possible door stats */

#include "/obj/tools/roommaker/roommaker.h"
static void getdshort();
static void getdlong();
static void getdkey();
static void getstatus(string str);
static void gotogetstatus();
mixed SetProp(string prop, mixed arg);

/* Configures the getstring() */

static void enter_door()
{
  SetLabel("prompt","Door #" + (sizeof(door)+1) + ": ");
  SetLabel("nostring","nodoor");
  SetLabel("ready","gotomenu");
  SetLabel("undo","gotomenu");
  SetLabel("redisplay","door_redisplay");
  SetLabel("delete","delete_door");
  SetLabel("save_file","normal_save_string");
  SetLabel("insert_string","insert_door");
  SetLabel("delete_return","gotogetstring");
  gotogetstring();
}

/* Shows all doors and asks which of then should be deleted */

static void delete_door()
{
  int i;
  if(!sizeof(door))
  {
    write("No doors are given.\n");
    return (void)call_other(this_object(),QueryLabel("delete_return"));
  }
  if(sizeof(door) == 1) write("This door is");
  else write("These doors are");
  write(" given:\n");
  for(i=0;i<sizeof(door);i++)
    write((i+1) + ". " + door[i] + " ... " + dpath[i] + ", \"" +
          dshort[i] + "\"\n");
  write("Which door should be deleted?\n");
  input_to("delete_door2");
}

/* This fun REALLY deletes the door #number */

static void delete_door_really(int number)
{
  write("Deleting " + door[number] + " ...\n");
  SetLabel("deleted_door",({door[number],dpath[number]}));
  door = door[0..number-1] + door[number+1..];
  dpath = dpath[0..number-1] + dpath[number+1..];
  dshort = dshort[0..number-1] + dshort[number+1..];
  dlong = dlong[0..number-1] + dlong[number+1..];
  dkey = dkey[0..number-1] + dkey[number+1..];
  dstatus = dstatus[0..number-1] + dstatus[number+1..];
  if(QueryLabel("delete_return") == "gotogetstring")
    SetLabel("prompt","Door #" + (sizeof(door)+1) + ": ");
}

/* Tries to delete the given door */

static void delete_door2(string str)
{
  int number;                   /* number of the given door */
  if(sscanf(str,"%d",number) && number <= sizeof(door))
  {
    delete_door_really(number-1);
    return (void)call_other(this_object(),QueryLabel("delete_return"));
  }
  number = (member_array(str,door));
  if(number == -1)
  {
    write("There is no door named " + str + "!\n");
    return (void)call_other(this_object(),QueryLabel("delete_return"));
  }
  delete_door_really(number);
  return (void)call_other(this_object(),QueryLabel("delete_return"));
}

/* Shows all door commands and his paths */

static void door_redisplay()
{
  int i;
  if(!sizeof(door))
  {
    write("No doors are given.\ndoor #1: ");
    input_to("getstring");
    return;
  }
  if(sizeof(door) == 1) write("This door is");
  else write("These doors are");
  write(" given:\n");
  for(i=0;i<sizeof(door);i++)
    write(door[i] + " ... " + dpath[i] + ", \"" + dshort[i] + "\"\n");
  gotogetstring();
}

/* If the player doesn't enter anything to getstring() (str == "") this fun
 * prints out an error message and returns to getstring()
 */

static void nodoor()
{
  write("Every door need a name. Please enter them!\n");
  gotogetstring();
}

/* Configures getstring() for to take the path */

static void getdpath()
{
  SetLabel("prompt","Path #" + sizeof(dpath) + ": ");
  SetLabel("nostring","nodpath");
  SetLabel("ready","nodpath");
  SetLabel("undo","nodpath");
  SetLabel("redisplay","door_redisplay2");
  SetLabel("delete","delete_door");
  SetLabel("save_file","door_save");
  SetLabel("insert_string","insert_door2");
  gotogetstring();
}

/* If an door command str already exists the fun asks for overwrite permission
 * If str doesn't exists yet, the fun stores string str in string *door and
 * sets the associated path to "<no path>". Asks for the path.
 */

static void insert_door(string str)
{
  int i;

  switch(str) {
  case "n": str = "north"; break;
  case "s": str = "south"; break;
  case "e": str = "east"; break;
  case "w": str = "west"; break;
  case "u": str = "up"; break;
  case "d": str = "down"; break;
  }
  i = member_array(str,door);
  SetLabel("door",str);
  if(i != -1)
  {
    write("We have already an door named " + str + "!\n");
    write(door[i] + " ... " + dpath[i] + ", \"" + dshort[i] + "\"\n");
    write("Do you want to overwrite them? (y/n) ");
    input_to("double_door");
    return;
  }
  door += ({str});
  dpath += ({"<no path>"});
  dshort += ({"<no description>"});
  dlong += ({({"<It's a door without any description.>"})});
  dkey += ({"0"});
  dstatus += ({0});
  return getdpath();
}

/* If the player doesn't enter anything to getstring() (str == "") or he tried
 * to end the enter_door()-loop, this fun prints out an error message and
 * returns to getstring()
 */

static void nodpath()
{
  write("Every door needs a path. Please enter them!\n");
  gotogetstring();
}

/* If the last door was already in the string *door we asked the player for
 * overwrite access. Now we look at his answer...
 */

static void double_door(string str)
{
  if(str == "y")
  {
    insert_alist(QueryLabel("door"),door);
    return getdpath();
  }
  if(str == "n") return enter_door();
  write("Do you want to overwrite them?? (y/n) ");
  input_to("double_door");
}

/* This fun shows a list of all given doors and his paths and returns to
 * getstring()
 */

static void door_redisplay2()
{
  int i;
  string dummy;
  dummy = dpath[member_array(QueryLabel("door"),door)];
  dpath[member_array(QueryLabel("door"),door)] = "<no path>";
  if(sizeof(door) == 1) write("This door is");
  else write("These doors are");
  write(" given:\n");
  for(i=0;i<sizeof(door);i++)
    write(door[i] + " ... " + dpath[i] + ", \"" + dshort[i] + "\"\n");
  dpath[member_array(QueryLabel("door"),door)] = dummy;
  gotogetstring();
}

/* This fun prevents a writing of the actual file without a senseful path to
 * the at last given door
 */

static void door_save()
{
  write("You can't save. The door needs a path!\n");
  gotogetstring();
}

/* The str will be inserted as associated path to the last door */

static void insert_door2(string str)
{
  dpath[member_array(QueryLabel("door"),door)] = str;
  return getdshort();
}

static void goto_delete_door()
{
  SetLabel("delete_return","gotomenu");
  delete_door();
}

/* Configures the getstring for to receive the door short description */

static void getdshort()
{
  SetLabel("prompt","Short #" + sizeof(dshort) + ": ");
  SetLabel("nostring","nodshort");
  SetLabel("ready","nodshort");
  SetLabel("undo","nodshort");
  SetLabel("redisplay","door_redisplay3");
  SetLabel("insert_string","insert_door3");
  gotogetstring();
}

/* If the player doesn't enter anything to getstring() (str == "") or he tried
 * to end the enter_door()-loop, this fun prints out an error message and
 * returns to getstring()
 */

static void nodshort()
{
  write("Every door needs a short description. Please enter them!\n");
  gotogetstring();
}

/* This fun shows a list of all given doors and his paths and returns to
 * getstring()
 */

static void door_redisplay3()
{
  int i,j;
  string dummy;
  dummy = dshort[j = member_array(QueryLabel("door"),door)];
  dshort[j] = "<no description>";
  if(sizeof(door) == 1) write("This door is");
  else write("These doors are");
  write(" given:\n");
  for(i=0;i<sizeof(door);i++)
    write(door[i] + " ... " + dpath[i] + ", \"" + dshort[i] + "\"\n");
  dshort[j] = dummy;
  gotogetstring();
}

/* The str will be inserted as associated short description to the last door */

static void insert_door3(string str)
{
  dshort[member_array(QueryLabel("door"),door)] = str;
  return getdlong();
}
static void getdlong()
{
  ClearText();
  SetLabel("lenmax",SCREEN_WIDTH);
  SetLabel("ready","store_dlong");
  SetLabel("show_file","show_door");
  SetLabel("menustring","But you need to describe the door!\n");
  SetLabel("menu","gettext");
  write("Please describe the door (help with \"~h\")!\n");
  gotogettext();
}

static void show_door()
{
  int i;
  string *dummy;

  dummy = dlong[i = member_array(QueryLabel("door"),door)];
  if(sizeof(EditText())) dlong[i] = EditText();
  else dlong[i] = ({"<A door without any description.>"});
  write(file_print() + "]");
  if(sizeof(EditText())) dlong[i] = dummy;
  input_to("gettext");
}

static void store_dlong()
{
  int i;

  if(!sizeof(EditText())) {
    write(QueryLabel("menustring"));
    return gotogettext();
  }
  dlong[i = member_array(QueryLabel("door"),door)] = EditText();
  return getdkey();
}

static void getdkey()
{
  SetLabel("prompt","Key #" + sizeof(dkey) + ": ");
  SetLabel("nostring","gotogetstatus");
  SetLabel("ready","gotogetstatus");
  SetLabel("undo","gotogetstatus");
  SetLabel("redisplay","door_redisplay4");
  SetLabel("delete","delete_door");
  SetLabel("save_file","door_save");
  SetLabel("insert_string","insert_key");
  gotogetstring();
}

static void insert_key(string str)
{
  if(!str) str = "0";
  dkey[member_array(QueryLabel("door"),door)] = str;
  return gotogetstatus();
}

static void gotogetstatus()
{
  if(!sizeof(door)) return enter_door();
  write("Status: ");
  input_to("getstatus");
}

static void getstatus(string str)
{
  int i;
  if(str[0..1] == "~!")
  {
    if(str[2..-1] == "")
    {
      SetLabel("nextfun","gotogetstatus");
      write("You interrupt to make " + QueryFile() +
         "\nRestart with \"~m\".\n");
      say(this_player()->QueryProp(P_NAME) + " interupts to write a file.\n");
      SetProp("making",1);
      return;
    }
    this_player()->command_me(str[2..-1]);
    write("Status: ");
    input_to("getstatus");
    return;
  }
  switch(str) {
  case "":
    write("Door status is now OPEN.\n");
    return enter_door();
  case "~q": return abort();
  case "~h":
    write(help() +
"1 ....... door is per default open   2 ....... door is per default closed\n");
    if(dkey[member_array(QueryLabel("door"),door)] != "0")
      write("3 ....... door is per default locked\n");
    return gotogetstatus();
  case "~f":
    write(file_print() + "Status: ");
    input_to("getstatus");
    return;
  case  ".":
  case "**":
    write("Door status is now OPEN.\n");
    return enter_door();
  case "~r":
    if(sizeof(door) == 1) write("This door is");
    else write("These doors are");
    write(" given:\n");
    for(i = 0; i < sizeof(door); i++)
      write(door[i] + " ... " + dpath[i] + ", \"" + dshort[i] + "\"\n");
    return gotogetstatus();
  case "~d":
    SetLabel("delete_return","gotogetstatus");
    return delete_door();
  case "~s":
    write(file_print()
          + "Do you want to save this file " + QueryFile() + "? (y/n) ");
    SetLabel("nextfun","gotogetstatus");
    input_to("filewrite");
    return;
  /* end switch */ }
  if(str == "open" || str == "0" || str == "1") return enter_door();
  if(str == "closed" || str == "2")
  {
    dstatus[member_array(QueryLabel("door"),door)] = 1;
    return enter_door();
  }
  if(str == "locked" || str == "3"
     && dkey[member_array(QueryLabel("door"),door)] != "0")
  {
    dstatus[member_array(QueryLabel("door"),door)] = 2;
    return enter_door();
  }
  write("ERROR: don't understand! Please enter again: (Help with \"~h\")\n" +
     "Status: ");
  input_to("getstatus");
}
