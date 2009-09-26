/*
 * This file contains standart funs for text input and conversion,
 * filehandling and communication
 */

static mixed    comm;		/* An alist for internal communication */
static object   creature;	/* A thing that looks like the thing we are
				 * creating */
static string  *edittext;	/* The text given in gettext() */
static string   Filename;
static string   oldfile;	/* contains the file with the same name as
				 * Filename if exist */
static int      written;	/* Is != 0 if Filename is already written */

static string   file_print();	/* Just a prototyp */
static void     abort();	/* */

/*
inherit "/obj/lib/string";
#include "/obj/lib/string.h"
#include <properties.h>
*/

mixed SetProp(string label, mixed arg);

/* Associates the label to the argument */

static void SetLabel(string label, mixed argument) {
  comm = insert_alist(label, argument, comm);
}

/* Returns the associated worth to label */

static mixed QueryLabel(string label) { return assoc(label, comm); }

/* Returns the filename of the aktual object */

string QueryFile() { return Filename; }

/* Returns the variable edittext */

string *EditText() { return edittext; }

/* Clears the edittext variable */

static void ClearText() { edittext = ({}); }

/* Sets edittext to text */

static void SetText(string * text) { edittext = text; }

/* Returns the object we create */

object Creature() { return creature; }

/*
 * A patch of the _even_smore() of /obj/lib/string.c. smore now returns to
 * the function labeled with "nextfun"


int _even_smore(string str)
{
  int     nline, display, line;
  string  foo;
  display = 0;
  if (str == "a")
   display = 1;
  else if (str == "" || str == "d")
    _smore_line += _smore_chunk, display = 1;
  else if (str == "q") {
    write("== Ok.\n");
    call_other(this_object(), (string) assoc("nextfun", comm));
    return 1;
  } else if (str == "u" || str == "b")
    _smore_line -= _smore_chunk, display = 1;
  else if (str == "f")
    _smore_line = 1, display = 1;
  else if (str == "l")
    _smore_line = _smore_size - _smore_chunk, display = 1;
  else if (str == "?")
    smore_help(_smore_chunk);
  else if (sscanf(str, "%d", nline) == 1)
    _smore_line = nline, display = 1;
  if (_smore_line < 1)
    _smore_line = 1;
  if (display)
    if (_smore_line <= _smore_size)
      write(implode(_smore_text[_smore_line - 1.._smore_line +
				_smore_chunk - 2]
		    ,"\n") + "\n");
    else {
      _smore_text = 0;
      write("== EOT\n");
      call_other(this_object(), (string) assoc("nextfun", comm));
      return 1;
    }
  line = _smore_line + _smore_chunk - 1;
  line = (line > _smore_size) ? _smore_size : line;
  smore_prompt(_smore_line, line, _smore_size);
  input_to("_even_smore");
  return 0;
}

 * A patch of the smore() of /obj/lib/string.c. smore now returns to the
 * function labeled with "nextfun"


varargs int smore(mixed text, int quiet, int chunk)
{
  int line;
  if (!text) {
    call_other(this_object(), (string) assoc("nextfun", comm));
    return 0;
  }
  _smore_chunk = chunk ? chunk : 16;
  if (!pointerp(text))
    _smore_text = old_explode((string) text, "\n");
  else
    _smore_text = (string *) text;
  if (!(_smore_size = sizeof(_smore_text))) {
    call_other(this_object(), (string) assoc("nextfun", comm));
    return 0;
  }
  if (_smore_size <= _smore_chunk) {
    write(implode(_smore_text, "\n") + "\n");
    call_other(this_object(), (string) assoc("nextfun", comm));
    return 1;
  }
  _smore_line = 1;
  write(implode(_smore_text[_smore_line - 1.._smore_line + _smore_chunk - 2], "\n") + "\n");
  line = _smore_line + _smore_chunk - 1;
  line = (line > _smore_size) ? _smore_size : line;
  smore_prompt(_smore_line, line, _smore_size);
  input_to("_even_smore");
  return 1;
}

*/

/* Returns the help string */

static string help()
{
  return
    ("The maker accepts the following commands:\n" +
     "~h ...... shows this text            ~q ...... aborts the execution\n" +
     "~d ...... delete the last line and   ~r ...... show the last text\n" +
     "          redisplay                  ~f ...... shows the file\n" +
     ". / ** .. end the action and return  ~s ...... saves the object in the current\n" +
     "          to the menu                          form\n" +
     "~!<cmd>.. escape and command <cmd>   ~! ...... interrupt, restart with \"~m\"\n");
}
  
void remove()
{
  if (creature) creature->remove();
  if (creature) destruct(creature);
  return;
}

/* Aborts all */

static void abortx()
{
  write("Aborted...\n");
  if (creature) creature->remove();
  if (creature) destruct(creature);
  return say(this_player()->QueryProp(P_NAME)
	     + " stops to write a file and aborts.\n");
}

/* Writes the actual file and returns to getstring() */

static void normal_save_string()
{
  write(file_print()
	+ "Do you want to save this file " + Filename + "? (y/n) ");
  comm = insert_alist("nextfun", "gotogetstring", comm);
  input_to("filewrite");
}

/* Tries to write the (string)file_print() as new file */

static void filewrite(string str)
{
  if (str == "n") {
    write("No save done.\n");
    return (void) call_other(this_object(), assoc("nextfun", comm));
  }
  if (str == "y") {
    write("Trying to write " + Filename + "...\n");
    if (!written && !(file_size(Filename) == -1)) {
      write("File " + Filename + " already exists!\n");
      write("O.k to overwrite " + Filename + "? (y/n) ");
      input_to("correct_save");
      return;
    }
    if (written) {
      if (file_size(Filename[0..<3] + ".old") != -1
	  && !rm(Filename[0..<3] + ".old"))
	return write("Can't write file " + Filename
		     + "! Please control your access rights!\n");
      rename(Filename, Filename[0..<3] + ".old");
    }
    if (!write_file(Filename, file_print())) {
	    write("Can't write file " + Filename + "! Please control your access rights!\n");
	    return abortx();
	  }
    write("Done...\n");
	written = 1;
    return (void) call_other(this_object(), assoc("nextfun", comm));
  }
  write("Do you want to save this file?? (y/n) ");
  input_to("filewrite");
}

static void correct_save(string str)
{
  if (str == "y") {
    if (file_size(Filename + "~") != -1 && !rm(Filename + "~"))
      return write("Can't write file " + Filename
		   + "! Please control your access rights!\n");
    rename(Filename, Filename + "~");
    return filewrite("y");
  }
  if (str == "n")
    return (void) call_other(this_object(), assoc("nextfun", comm));
  write("O.k. to overwrite " + Filename + "?? (y/n) ");
  input_to("correct_save");
}

/* Asks befor aborting the player for to write the new file */

static void abort()
{
  write("Do you want to save the file " + Filename + " befor? (y/n) ");
  comm = insert_alist("nextfun", "abortx", comm);
  input_to("filewrite");
}

/* Gets a new filename or aborts */

static void getrightname(string str)
{
  if (str == "~q")
    return abortx();
  this_player()->command_me("make " + str);
  return;
}

/*
 * Convert a string array into a string, that print out a string like this:
 * "<array>[0]" + "<array>[1]" + "<array>[2]" ...
 */

static string arrayconvert(string * text)
{
  int    i;
  string outstring;
  outstring = "";
  for (i = 0; i < sizeof(text) - 1; i++)
    outstring += "\"" +
      implode(old_explode(implode(old_explode("*" + text[i] + "*", "\\"), "\\\\"),
		      "\""), "\\\"")[1..<2] + "\\n\"\ +\n";
  if (sizeof(text))
    outstring += "\"" +
      implode(old_explode(implode(old_explode("*" + text[sizeof(text) - 1] + "*","\\"),
			      "\\\\"),"\""), "\\\"")[1..<2] + "\\n\"\n";
  return outstring;
}

/* Converts a string array to a string */

static string arrayprint(string * text)
{
  int    i;
  string outstring;
  outstring = "";
  for (i = 0; i < sizeof(text); i++)
    outstring += text[i] + "\n";
  return outstring;
}

/*
 * Converts a string array into a "comment-format" like this: "/*
 * <array>[0]\n" + " * <array>[1]\n" + " * <array>[2]\n" ... " +/\n"
 */

static string comment(string * text)
{
  int    i;
  string outstring;
  if (!sizeof(text) || (sizeof(text) == 1 && text[0] == ""))
    return "";
  outstring = "/* " + text[0] + "\n";
  for (i = 1; i < sizeof(text); i++)
    outstring += " * " + text[i] + "\n";
  return outstring;
}

/* Prints the prompt and waits for the input of a string */

static void gotogetstring()
{
  write(assoc("prompt", comm));
  input_to("getstring");
}

/* Shows the actual file and returns to getstring() */

static void show_file_string()
{
  SetLabel("nextfun","gotogetstring");
  write(file_print());
/* smore(file_print(), 0, (int) environment()->QueryProp(P_PAGESIZE)); */
  return gotogetstring();
}

/*
 * Takes a string from the player and tries to interprete them as a maker
 * command. If failed, it calls the function that is assocciated to "insert_
 * string" with the string as argument
 */

static void getstring(string str)
{
  if (str[0..1] == "~!") {
    if (str[2..<1] == "") {
      comm = insert_alist("nextfun", "gotogetstring", comm);
      write("You interrupt to make " + Filename + "\nRestart with \"~m\".\n");
      say(this_player()->QueryProp(P_NAME) + " interupts to write a file.\n");
      SetProp("making", 1);
      return;
    }
    this_player()->command_me(str[2..<1]);
    return gotogetstring();
  }
  switch (str) {
  case "":
    return (void) call_other(this_object(), assoc("nostring", comm));
  case "~q":
    return abort();
  case "~h":
	write(help());
    return gotogetstring();
  case "~f":
	return show_file_string();
  case ".":
    return (void) call_other(this_object(), assoc("ready", comm));
  case "**":
    return (void) call_other(this_object(), assoc("ready", comm));
  case "~r":
    return (void) call_other(this_object(), assoc("redisplay", comm));
  case "~d":
	return (void) call_other(this_object(), assoc("delete", comm));
  case "~s":
    return (void) call_other(this_object(), assoc("save_file", comm));
    /* end switch */
  }
  return (void) call_other(this_object(), assoc("insert_string", comm), str);
}

/* Prints the message that is assocciated to "nextwrite" and starts gettext()
 */

static void gotogettext()
{
  write(assoc("nextwrite", comm) + "]");
  input_to("gettext");
}

/* Takes strings and stores them in string *edittext */

static void gettext(string str)
{
  string thefile, rest;
  string *include, *restarr;
  int    bytes;
  int    firstline,lastline;
  if (str[0..1] == "~!") {
    if (str[2..<1] == "") {
      SetLabel("nextfun", "gotogettext");
      write("You interrupt to make " + Filename + "\nRestart with \"~m\".\n");
      say(this_player()->QueryProp(P_NAME) + " interupts to write a file.\n");
      SetProp("making", 1);
      return;
    }
    this_player()->command_me(str[2..<1]);
    write("]");
    input_to("gettext");
    return;
  }
  if (str[0..1] == "~i") {
    if (str[2..] == "") {
      write("Usage: ~i <file> [<line>][-][<line>].\n]");
      input_to("gettext");
      return;
    }
    str = str[2..];
    if(str[0] == ' ') str = str[1..];
    if(!sscanf(str,"%s %s",thefile,rest))
      thefile = str;
    thefile = (string)this_player()->get_path(thefile);
    if((bytes = file_size(thefile)) == -2) {
      write(thefile + " is a directory.\n]");
      input_to("gettext");
      return;
    }
    if(bytes == -1) {
      write("No such file: " + thefile + ".\n]");
      input_to("gettext");
      return;
    }
    include = old_explode(read_file(thefile),"\n");
    if(!rest) {
      edittext += include;
      write("Including " + thefile + "...\n]");
      input_to("gettext");
      return;
    }
    restarr = old_explode("*-" + rest + "-*","-")[1..<2];
    if(restarr[0] == "") restarr[0] = "0";
    if(sizeof(restarr) == 1) restarr += restarr;
    if(restarr[1] == "") restarr [1] = "" + (sizeof(include)-1);
    rest = implode(restarr," - ");
    if(!sscanf(rest,"%d - %d",firstline,lastline)) {
      write("Wrong second argument. Usage: ~i <file> [<line>][-][<line>].\n]");
      input_to("gettext");
      return;
    }
    if(firstline > sizeof(include)-1) {
      write(thefile + " hasn't that many lines.\n]");
      input_to("gettext");
      return;
    }
    if(lastline > sizeof(include)-1)
      lastline = sizeof(include) -1;
    if(lastline < firstline) lastline = firstline;
    edittext += include[firstline..lastline];
    write("Including " + thefile + " line");
    if(lastline == firstline) write(" " + lastline + ".\n]");
    else write("s " + firstline + " - " + lastline + ".\n]");
    input_to("gettext");
    return;
  }
  switch (str) {
    case "~q":
      return abort();
    case "~r":
      write(arrayprint(edittext) + "]");
      input_to("gettext");
      return;
    case "~d":
      if (!sizeof(edittext)) {
	write("No lines in the buffer.\n]");
	input_to("gettext");
	return;
      }
      edittext = edittext[0..<2];
      comm = insert_alist("nextwrite", arrayprint(edittext), comm);
      return gotogettext();
    case ".":
      return (void) call_other(this_object(), assoc("ready", comm));
    case "**":
      return (void) call_other(this_object(), assoc("ready", comm));
    case "~f":
      return (void) call_other(this_object(), assoc("show_file", comm));
    case "~s":
      comm = insert_alist("nextfun", "gotogettext", comm);
      comm = insert_alist("nextwrite", "Continue...\n", comm);
      write(file_print() + "Do you want to save this file? (y/n) ");
      input_to("filewrite");
      return;
    case "~h":
      write(help() + "~i <file> includes the file <file>\n" +
                     "          (optional: begin - end)\n]");
      input_to("gettext");
      return;
      /* end switch */
    }
  if (strlen(str) > assoc("lenmax", comm)) {
    write("This line ist too long. Please enter again!\n]");
    input_to("gettext");
    return;
  }
  if (!str)
    edittext += ({""});
  else
    edittext += ({str});
  write("]");
  input_to("gettext");
}
