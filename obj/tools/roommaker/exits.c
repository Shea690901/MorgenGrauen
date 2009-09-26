/* This file contents the functions necessary for to get exits and their paths
 *
 * Part of RoomMaker V0.94 (c) Sique 9/1992
 *
 * Author: Sique 9/92
 *
 * Last Update: Sep 29 1992
 */

static string *exit;        /* contains the exit commands */
static string *path;        /* contains the exit paths */

#include "/obj/tools/roommaker/roommaker.h"

/* Configures the getstring() */

static void enter_exit()
{
  SetLabel("prompt","Exit #" + (sizeof(exit)+1) + ": ");
  SetLabel("nostring","noexit");
  SetLabel("ready","gotomenu");
  SetLabel("undo","gotomenu");
  SetLabel("redisplay","exit_redisplay");
  SetLabel("delete","delete_exit");
  SetLabel("save_file","normal_save_string");
  SetLabel("insert_string","insert_exit");
  SetLabel("delete_return","gotogetstring");
  gotogetstring();
}

/* Shows all exits and asks which of then should be deleted */

static void delete_exit()
{
  int i;
  if(!sizeof(exit))
  {
    write("No exits are given.\n");
    return (void)call_other(this_object(),QueryLabel("delete_return"));
  }
  if(sizeof(exit) == 1) write("This exit is");
  else write("These exits are");
  write(" given:\n");
  for(i=0;i<sizeof(exit);i++)
    write((i+1) + ". " + exit[i] + " ... " + path[i] + "\n");
  write("Which exit should be deleted?\n");
  input_to("delete_exit2");
}

/* This fun REALLY deletes the exit #number */

static void delete_exit_really(int number)
{
  write("Deleting " + exit[number] + " ...\n");
  SetLabel("deleted_exit",({exit[number],path[number]}));
  exit = exit[0..number-1] + exit[number+1..sizeof(exit)-1];
  path = path[0..number-1] + path[number+1..sizeof(path)-1];
  if(QueryLabel("delete_return") == "gotogetstring")
    SetLabel("prompt","Exit #" + (sizeof(exit)+1) + ": ");
}

/* Tries to delete the given exit */

static void delete_exit2(string str)
{
  int number;                   /* number of the given exit */
  if(sscanf(str,"%d",number) && number <= sizeof(exit))
  {
    delete_exit_really(number-1);
    return (void)call_other(this_object(),QueryLabel("delete_return"));
  }
  number = (member_array(str,exit));
  if(number == -1)
  {
    write("There is no exit named " + str + "!\n");
    return (void)call_other(this_object(),QueryLabel("delete_return"));
  }
  delete_exit_really(number);
  return (void)call_other(this_object(),QueryLabel("delete_return"));
}

/* Shows all exit commands and his paths */

static void exit_redisplay()
{
  int i;
  if(!sizeof(exit))
  {
    write("No exits are given.\nExit #1: ");
    input_to("getstring");
    return;
  }
  if(sizeof(exit) == 1) write("This exit is");
  else write("These exits are");
  write(" given:\n");
  for(i=0;i<sizeof(exit);i++)
    write(exit[i] + " ... " + path[i] + "\n");
  gotogetstring();
}

/* If the player doesn't enter anything to getstring() (str == "") this fun
 * prints out an error message and returns to getstring()
 */

static void noexit()
{
  write("Every exit need a name. Please enter them!\n");
  gotogetstring();
}

/* Configures getstring() for to take the path */

static void getpath()
{
  SetLabel("prompt","Path #" + sizeof(path) + ": ");
  SetLabel("nostring","nopath");
  SetLabel("ready","nopath");
  SetLabel("undo","nopath");
  SetLabel("redisplay","exit_redisplay2");
  SetLabel("delete","delete_exit");
  SetLabel("save_file","exit_save");
  SetLabel("insert_string","insert_exit2");
  gotogetstring();
}

/* If an exit command str already exists the fun aks for overwrite permission
 * If str doesn't exists yet, the fun stores string str in string *exit and
 * sets the QueryLabelciated path to "<no path>". Asks for the path.
 */

static void insert_exit(string str)
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
  i = member_array(str,exit);
  SetLabel("exit",str);
  if(i != -1)
  {
    write("We have already an exit named " + str + "!\n");
    write(exit[i] + " ... " + path[i] + "\n");
    write("Do you want to overwrite them? (y/n) ");
    input_to("double_exit");
    return;
  }
  exit += ({str});
  path += ({"<no path>"});
  return getpath();
}

/* If the player doesn't enter anything to getstring() (str == "") or he tried
 * to end the enter_exit()-loop, this fun prints out an error message and
 * returns to getstring()
 */

static void nopath()
{
  write("Every exit need a path. Please enter them!\n");
  gotogetstring();
}

/* If the last exit was already in the string *exit we asked the player for
 * overwrite access. Now we look at his answer...
 */

static void double_exit(string str)
{
  if(str == "y")
  {
    insert_alist(QueryLabel("exit"),exit);
    return getpath();
  }
  if(str == "n") return enter_exit();
  write("Do you want to overwrite them?? (y/n) ");
  input_to("double_exit");
}

/* This fun shows a list of all given exits and his paths and returns to
 * getstring()
 */

static void exit_redisplay2()
{
  int i;
  string pathdummy;
  pathdummy = path[member_array(QueryLabel("exit"),exit)];
  path[member_array(QueryLabel("exit"),exit)] = "<no path>";
  if(sizeof(exit) == 1) write("This exit is");
  else write("These exits are");
  write(" given:\n");
  for(i=0;i<sizeof(exit);i++)
    write(exit[i] + " ... " + path[i] + "\n");
  path[member_array(QueryLabel("exit"),exit)] = pathdummy;
  gotogetstring();
}

/* This fun prevents a writing of the actual file without a senseful path to
 * the at last given exit
 */

static void exit_save()
{
  write("You can't save. The exit need a path!\n");
  gotogetstring();
}

/* The str will be inserted as associated path to the last exit */

static void insert_exit2(string str)
{
  path[member_array(QueryLabel("exit"),exit)] = str;
  Creature()->AddExit(QueryLabel("exit"),str);
  return enter_exit();
}

static void goto_delete_exit()
{
  SetLabel("delete_return","gotomenu");
  delete_exit();
}
