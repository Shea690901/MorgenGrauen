/* RoomMaker- Headerfile
 *
 * Part of RoomMaker V0.94 (c) Sique 9/1992
 *
 * Author: Sique 9/92
 *
 * Last Update: Sep 29 1992
 */

/* Defines the sizes of a normal terminal screen */

#include "/obj/tools/roommaker/sizes.h"

/* Define some variables */
#define ROOMSTDS ({"room","hotelroom","port","shop","pub"})

/* Prototypes and global variables */

static void gotogetstring();
static void gotogettext();
static void abort();
static string help();
static string file_print();
static void SetLabel(string label,mixed argument);
static mixed QueryLabel(string label);
static void ClearText();
static void SetText(string *text);
static void gotomenu();
string QueryFile();
string *EditText();
string arrayprint(string *arr);
object Creature();
