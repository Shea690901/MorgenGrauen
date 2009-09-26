/* This file contains the functions for to get items
 *
 * Part of RoomMaker V0.94 (c) Sique 9/1992
 *
 * Author: Sique 9/92
 *
 * Last Update: Sep 29 1992
 */

static mixed *item;         /* contains the item settings */
static string *refresh;     /* contains the refresh modi */

mixed SetProp(string label,mixed arg);

#include "/obj/tools/roommaker/roommaker.h"

static void enter_item()
{
  SetLabel("prompt","Item #" + (sizeof(item[0])+1) + ": ");
  SetLabel("nostring","");
  SetLabel("show_file","show_file_string");
  SetLabel("ready","gotomenu");
  SetLabel("undo","gotomenu");
  SetLabel("redisplay","item_redisplay");
  SetLabel("delete","delete_item");
  SetLabel("save_file","normal_save_string");
  SetLabel("insert_string","insert_item");
  SetLabel("delete_return","gotogetstring");
  write("Please enter the path of the item (help with \"~h\")!\n");
  gotogetstring();
}

static void item_redisplay()
{
  int i;
  if(!sizeof(item[0]))
  {
    write("No items are given.\n");
    return gotogetstring();
  }
  if(sizeof(item[0]) == 1) write("This item is");
  else write("These items are");
  write(" given:\n");
  for(i=0;i<sizeof(item[0]);i++)
    write(item[0][i] + " ... " + refresh[item[1][i]] + "\n");
  gotogetstring();
}

static void gotogetmode()
{
  write("Refresh: ");
  input_to("getmode");
}

static void insert_item(string str)
{
  item[0] += ({str});
  item[1] += ({0});
  gotogetmode();
}

static void getmode(string str)
{
  int i;
  if(str[0..1] == "~!")
  {
    if(str[2..-1] == "")
    {
      SetLabel("nextfun","gotogetmode");
      write("You interrupt to make " + QueryFile() +
         "\nRestart with \"~m\".\n");
      say(this_player()->QueryProp(P_NAME) + " interupts to write a file.\n");
      SetProp("making",1);
      return;
    }
    this_player()->command_me(str[2..-1]);
    write("Refresh: ");
    input_to("getmode");
    return;
  }
  switch(str) {
  case "":
  {
    write("Refresh mode is now NONE.\n");
    return enter_item();
  }
  case "~q": return abort();
  case "~h":
  {
    write(help() +
"1 ....... no refresh                 2 ....... refresh on reset if item was\n"+
"3 ....... refresh on reset if item             destructed\n" +
"          was removed from room      4 ....... refresh every reset\nRefresh: ")
;
    input_to("getmode");
    return;
  }
  case "~f":
  {
    write(file_print() + "Refresh: ");
    input_to("getmode");
    return;
  }
  case "~u":
  {
    write("Remove " + item[0][sizeof(item[0])-1] + "\n");
    item[0] = item[0][0..sizeof(item[0])-2];
    item[1] = item[1][0..sizeof(item[1])-2];
    return enter_item();
  }
  case  ".":
  {
    write("Refresh mode is now NONE.\n");
    return enter_item();
  }
  case "**":
  {
    write("Refresh mode is now NONE.\n");
    return enter_item();
  }
  case "~r":
  {
    for(i=0;i<sizeof(item[0]);i++)
      write(item[0][i] + " ... " + refresh[item[0][i]] + "\n");
    write("Refresh: ");
    input_to("getmode");
    return;
  }
  case "~d":
  {
    write("Remove " + item[0][sizeof(item[0])-1] + "\n");
    item[0] = item[0][0..sizeof(item[0])-2];
    item[1] = item[1][0..sizeof(item[1])-2];
    return enter_item();
  }
  case "~s":
  {
    write(file_print()
          + "Do you want to save this file " + QueryFile() + "? (y/n) ");
    SetLabel("nextfun","gotogetmode");
    input_to("filewrite");
    return;
  }
  /* end switch */ }
  if(str == "none" || str == "0" || str == "1") return enter_item();
  if(str == "destruct" || str == "2")
  {
    item[1][sizeof(item[0])-1] = 1;
    return enter_item();
  }
  if(str == "remove" || str == "3")
  {
    item[1][sizeof(item[0])-1] = 2;
    return enter_item();
  }
  if(str == "always" || str == "4")
  {
    item[1][sizeof(item[0])-1] = 3;
    return enter_item();
  }
  write("ERROR: don't understand! Please enter again: (Help with \"~h\")\n" +
     "Refresh: ");
  input_to("getmode");
}

/* Shows all items and asks which of then should be deleted */
static void delete_item()
{
  int i;
  if(!sizeof(item[0]))
  {

    write("No items are given.\n");
    return (void)call_other(this_object(),QueryLabel("delete_return"));
  }
  if(sizeof(item[0]) == 1) write("This item is");
  else write("These items are");
  write(" given:\n");
  for(i=0;i<sizeof(item[0]);i++)
      write((i+1) + ". " + item[0][i] + " ... " + refresh[item[1][0]] + "\n");
  write("Which exit should be deleted?\n");
  input_to("delete_item2");
}

/* This fun REALLY deletes the item #number */

static void delete_item_really(int number)
{
  write("Deleting " + item[0][number] + " ...\n");
  SetLabel("deleted_item",({item[0][number],item[1][number]}));
  item[0] = item[0][0..number-1] + item[0][number+1..sizeof(item[0])-1];
  item[1] = item[1][0..number-1] + item[1][number+1..sizeof(item[1])-1];
  if(QueryLabel("delete_return") == "gotogetstring")
    SetLabel("prompt","Item #" + (sizeof(item[0])+1) + ": ");
}

/* Tries to delete the given item */

static void delete_item2(string str)
{
  int number;                   /* number of the given item */
  if(sscanf(str,"%d",number) && number <= sizeof(item[0]))
  {
    delete_item_really(number-1);
    return (void)call_other(this_object(),QueryLabel("delete_return"));
  }
  number = (member_array(str,item[0]));
  if(number == -1)
  {
    write("There is no item named " + str + "!\n");
    return (void)call_other(this_object(),QueryLabel("delete_return"));
  }
  delete_item_really(number);
  return (void)call_other(this_object(),QueryLabel("delete_return"));
}

static void goto_delete_item()
{
  SetLabel("delete_return","gotomenu");
  delete_item();
}
