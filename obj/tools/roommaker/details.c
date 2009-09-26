/* This file contains the functions for to get details and their descriptions
 *
 * Part of RoomMaker V0.94 (c) Sique 9/1992
 *
 * Author: Sique 9/92
 *
 * Last Update: Sep 29 1992
 */

static string *detail;      /* contains the descripted details */
static mixed *desc;         /* contains the lines of the detail descriptions */

#include "/obj/tools/roommaker/roommaker.h"

/* Configures the getstring() */

static void enter_detail()
{
  SetLabel("prompt","Detail #" + (sizeof(detail)+1) + ": ");
  SetLabel("nostring","nodetail");
  SetLabel("ready","gotomenu");
  SetLabel("undo","gotomenu");
  SetLabel("redisplay","detail_redisplay");
  SetLabel("delete","delete_detail");
  SetLabel("save_file","normal_save_string");
  SetLabel("insert_string","insert_detail");
  SetLabel("delete_return","gotogetstring");
  SetLabel("menustring","");
  SetLabel("nextfun","gotogetstring");
  SetLabel("nextwrite","");
  gotogetstring();
}

/* Gives an erro message if the player don't type anything (simply a "\n") */

static void nodetail()
{
  write("Please enter a detail! A player can't look at \"\"!\n");
  gotogetstring();
}

/* Shows the actual file with EditText() instead of desc[i] as description of
 * detail[i]
 */

static void show_detail()
{
  int    i;
  string *dummy;

  i = member_array(QueryLabel("detail"),detail);
  dummy = desc[i];
  if(sizeof(EditText())) desc[i] = EditText();
  else                   desc[i] = ({"<no description>"});
  write(file_print() + "]");
/*  smore(file_print(),0,this_player()->QueryPagesize()); */
  if(sizeof(EditText())) desc[i] = dummy;
  input_to("gettext");
}

/* shows all details and their descriptions */

static void show_all_details(int mode)
{
  int i;
  string description;
  if(!sizeof(detail))
  {
    write("No details are given.\n");
    return;
  }
  if(sizeof(detail) == 1) write("This detail is");
  else write("These details are");
  write(" given:\n");
  for(i=0;i<sizeof(detail);i++)
  {
    if(mode) write((i+1) + ". ");
    write(detail[i] + " ... ");
    description = implode(old_explode(arrayprint(desc[i]),"\n")," ");
    if(strlen(description) >
      SCREEN_WIDTH - 5 - (i>9?4:3)*mode - strlen(detail[i]))
      write(description[0..(SCREEN_WIDTH - 9 - (i>9?4:3)*mode
                            - strlen(detail[i]))] + "...\n");
    else write(description + "\n");
  }
}

/* shows all details that are given and returns to getstring */

static void detail_redisplay()
{
  show_all_details(0);       /* without numbering */
  gotogetstring();
}

static void getdesc()
{
  ClearText();
  SetLabel("lenmax",SCREEN_WIDTH);
  SetLabel("ready","store_desc");
  SetLabel("show_file","show_detail");
  SetLabel("menustring","But you need to describe the detail!\n");
  SetLabel("menu","gettext");
  write("Please describe the detail (help with \"~h\")!\n");
  gotogettext();
}

static void insert_detail(string str)
{
  int i;
  string description;
  i = member_array(str,detail);
  SetLabel("detail",str);
  if(i != -1)
  {
    write("We have already an detail named " + str + "!\n");
    write(detail[i] + " ... ");
    description = implode(old_explode(arrayprint(desc[i]),"\n")," ");
    if(strlen(description) > 74 - strlen(detail[i]))
      write(description[0..(70 - strlen(detail[i]))] + "...\n");
    else write(description + "\n");
    write("Do you want to overwrite them? (y/n) ");
    input_to("double_detail");
    return;
  }
  detail += ({str});
  desc +=({({"<no description>"})});
  return getdesc();
}

static void store_desc()
{
  int i;
  if(!sizeof(EditText()))
  {
    write(QueryLabel("menustring"));
    return gotogettext();
  }
  i = member_array(QueryLabel("detail"),detail);
  desc[i] = EditText();
  write("Do you want to add a detail with the same description? (y/n) ");
  input_to("same_desc");
}

static void same_desc(string str)
{
  write(QueryLabel("detail") + "..." +
	implode(desc[member_array(QueryLabel("detail"),detail)]," ") +
	"\n");
  if(str == "n") return enter_detail();
  if(str == "y")
  {
    write("Please enter the detail (help with \"~h\")!\n");
    SetLabel("prompt","Detail #" + (sizeof(detail)+1) + ": ");
    SetLabel("nostring","nodetail");
    SetLabel("show_file","show_detail");
    SetLabel("ready","gotomenu");
    SetLabel("undo","gotomenu");
    SetLabel("redisplay","detail_redisplay");
    SetLabel("delete","delete_detail");
    SetLabel("save_file","detail_save");
    SetLabel("insert_string","insert_detail2");
    SetLabel("delete_return","gotogetstring");
    return gotogetstring();
  }
  write("Do you want to add a detail with the same description?? (y/n) ");
  input_to("same_desc");
}

static void insert_detail2(string str)
{
  detail += ({str});
  if(sizeof(desc) == sizeof(detail)) desc = desc[0..<2];
  desc += ({desc[member_array(QueryLabel("detail"),detail)]});
  write(detail[<1] + "..." + implode(desc[<1]," ") + "\n");
  write("Do you want to add a detail with the same description? (y/n) ");
  input_to("same_desc");
}

static void double_detail(string str)
{
  if(str == "y") return getdesc();
  if(str == "n") return enter_detail();
  write("Do you want to overwrite them?? (y/n) ");
  input_to("double_detail");
}

/* Shows all details and asks which of then should be deleted */

static void delete_detail()
{
  show_all_details(1);             /* with numbering */
  if(!sizeof(detail))
    return (void)call_other(this_object(),QueryLabel("delete_return"));
  write("Which detail should be deleted?\n");
  input_to("delete_detail2");
}

/* This fun REALLY deletes the detail #number */

static void delete_detail_really(int number)
{
  write("Deleting " + detail[number] + " ...\n");
  SetLabel("deleted_detail",({detail[number],desc[number]}));
  detail = detail[0..number-1] + detail[number+1..sizeof(detail)-1];
  desc = desc[0..number-1] + desc[number+1..sizeof(desc)-1];
  if(QueryLabel("delete_return") == "gotogetstring")
    SetLabel("prompt","Detail #" + (sizeof(detail)+1) + ": ");
}

/* Tries to delete the given detail */

static void delete_detail2(string str)
{
  int number;                   /* number of the given detail */
  if(sscanf(str,"%d",number) && number <= sizeof(detail))
  {
    delete_detail_really(number-1);
    return (void)call_other(this_object(),QueryLabel("delete_return"));
  }
  number = (member_array(str,detail));
  if(number == -1)
  {
    write("There is no detail named " + str + "!\n");
    return (void)call_other(this_object(),QueryLabel("delete_return"));
  }
  delete_detail_really(number);
  return (void)call_other(this_object(),QueryLabel("delete_return"));
}

static void goto_delete_detail()
{
  SetLabel("delete_return","gotomenu");
  delete_detail();
}
