/* This file contains the functions for comments, the internal long and short
 * description
 *
 * Part of RoomMaker V0.94 (c) Sique 9/1992
 *
 * Author: Sique 9/92
 *
 * Last Update: Sep 29 1992
 */

static string rshort;       /* contains the short description of the room */
static string *rlong;       /* contains the lines of the long description */
static string *kommentar;   /* contains the lines of the comment to this room */
static string menustring;   /* contains the roommaker menu */

#include "/obj/tools/roommaker/roommaker.h"

static void getshort(string str)
{
  if(str == "n")
  {
    write("Please enter the new filename: ");
    input_to("getrightname");
    return;
  }
  if(str == "y")
  {
    SetLabel("prompt","]");
    SetLabel("nostring","short_ready");
    SetLabel("abort","abortx");
    SetLabel("undo","short_ready");
    SetLabel("ready","short_ready");
    SetLabel("redisplay","short_redisplay");
    SetLabel("delete","delete_short");
    SetLabel("save_file","short_save");
    SetLabel("insert_string","store_short");
    write("Please enter a short description of the room (help with \"~h\")!\n");
    return gotogetstring();
  }
  write("Is this correct?? (y/n) ");
  input_to("getshort");
}

/* Configures getstring() , starts them for to get a short description and
 * forces getstring() to store the string with store_short2()
 */

static void getshort2()
{
  SetLabel("prompt","]");
  SetLabel("nostring","short_ready");
  SetLabel("abort","abortx");
  SetLabel("undo","gotomenu");
  SetLabel("ready","gotomenu");
  SetLabel("redisplay","short_redisplay");
  SetLabel("delete","delete_short2");
  SetLabel("save_file","normal_save_string");
  SetLabel("insert_string","store_short2");
  write(
    "----- Actual short description: -----------------------------------------------\n" +
    rshort + "\n" +
    "-------------------------------------------------------------------------------\n" +
    "Please edit the short description of the room (help with \"~h\")!\n");
  return gotogetstring();
}

/* Forces the player to give a short description or aborts */

static void short_ready()
{
  write("You need to enter a short description!\n]");
  input_to("getstring");
}

/* We can't redisplay anything */

static void short_redisplay()
{
  write("There is nothing to redisplay. Please enter the short description!\n]")
;
  input_to("getstring");
}

/* We can't write this file until now */

static void short_save()
{
  write("You can't save this file until now. You need first to configure something\nPlease enter the short description!\n]");
  input_to("getstring");
}

/* Writes any delete error and returns to getstring */

static void delete_short()
{
  write("No lines in buffer!\n");
  gotogetstring();
}

/* Writes any delete message and returns to getstring */

static void delete_short2()
{
  write("Short description deleted. Please enter a new short description!\n");
  gotogetstring();
}

/* configures gettext for to get the long description */

static void getlong()
{
  SetLabel("lenmax",SCREEN_WIDTH);
  SetLabel("ready","store_long");
  SetLabel("show_file","show_long");
  SetLabel("menustring",menustring);
  SetLabel("menu","menu");
  SetLabel("nextwrite","");
  if(sizeof(rlong) == 1 && rlong[0] == rshort + ".")
  {
    write("Please enter the long description of the room (help with \"~h\")!\n")
;
    ClearText();
  }
  else
  {
    SetText(rlong);
    write(
      "----- Actual long description: -----------------------------------------------\n" +
      arrayprint(rlong) +
      "------------------------------------------------------------------------------\n" +
      "Please edit the long description of the room (help with \"~h\")!\n");
  }
  gotogettext();
}

/* Inserts the string str as short description and asks for the long
 * description
 */

static void store_short(string str)
{
  rshort = str;
  Creature()->SetProp(P_INT_SHORT,rshort);
  rlong = ({str + "."});
  getlong();
}

/* Inserts the string str as short description and returns to the menu */

static void store_short2(string str)
{
  rshort = str;
  Creature()->SetProp(P_INT_SHORT,rshort);
  gotomenu();
}

/* Stores EditText() in rlong */

static void store_long()
{
  if(!sizeof(rlong)) rlong = ({rshort + "."});
  if(sizeof(EditText())) rlong = EditText();
  Creature()->SetProp(P_INT_LONG,arrayprint(EditText()));
  write(menustring);
  input_to("menu");
}

/* Prints the actual file with EditText() instead of rlong */

static void show_long()
{
  string *dummy;
  dummy = rlong;
  rlong = EditText();
  if(!sizeof(rlong)) rlong = dummy;
  write(file_print() + "]");
  rlong = dummy;
  input_to("gettext");
}

/* Returns to the menu */

static void normal_ready()
{
  write(menustring);
  input_to("menu");
}

/* Configures gettext() for to get a comment text */

static void insert_comment()
{
  SetLabel("lenmax",SCREEN_WIDTH - 3);
  SetLabel("ready","store_comment");
  SetLabel("show_file","show_comment");
  SetLabel("menustring",menustring);
  SetLabel("menu","menu");
  SetLabel("nextwrite","");
  if(!sizeof(kommentar))
  {
    write("Please enter a comment to the room (help with \"~h\")!\n")
;
    ClearText();
  }
  else
  {
    SetText(kommentar);
    write(
      "----- Actual comment: ------------------------------------------------------\n" +
      arrayprint(kommentar) +
      "----------------------------------------------------------------------------\n" +
      "Please edit the comment to the room (help with \"~h\")!\n");
  }
  gotogettext();
}

/* Stores EditText() in kommentar */

static void store_comment()
{
  if(sizeof(EditText())) kommentar = EditText();
  gotomenu();
}

/* Prints the actual file with EditText() instead of kommentar */

static void show_comment()
{
  string *dummy;
  dummy = kommentar;
  kommentar = EditText();
  if(!sizeof(kommentar)) kommentar = dummy;
  write(file_print());
  kommentar = dummy;
  gotogettext();
}

