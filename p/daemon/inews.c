// INEWS.C -- International News

#pragma strong_types
#pragma combine_strings

#include <daemon.h>

#define NEWSFILE "/p/daemon/inews.heute"

#define DEBUG(str) tell_object(find_player("hate"), sprintf("DEBUG: %O\n",str))

private static mixed newnews = ({0,1});
private static mixed times = ({2, 8, 14, 20});

void reset()
{
  int i, next;
  if(find_call_out("_sendnews") != -1) return;
  while(remove_call_out("NewsUpdate") != -1);

  i = 0;
  while(i < sizeof(times) &&
        (next = (times[i]-1) * 3600 - time() % (24 * 3600)) < 0)
    i++;
  if(i == sizeof(times))
    next = (23 * 3600) - (time() % (24 * 3600))
         + (times[0] * 3600);

  call_out("NewsUpdate", next-300);
}

void create()
{
  seteuid(getuid());
  reset();
}

int check(string ch, object pl, string cmd)
{
  return 1;
}

string name()
{
  return "DW";
}

// _sendnews() -- send news to channel
void _sendnews(mixed news, int i)
{
  mixed msg, tmp;
  while(remove_call_out("_sendnews") != -1);
  msg = implode(regexp(regexplode(news[i], "\n\n*"), "^[^\n][^\n]*"), "\n");
  tmp = msg[0..(strstr(msg, ". ") == -1 ? 46 : strstr(msg, ". "))];

  if(newnews[1])
  {
    mixed art;
    if(msg[0] == '\n') msg = msg[1..];
    if(msg[<1] == '\n') msg = msg[0..<2];
    art=({"dwnews","Deutsche Welle",0,0, tmp[0..46]+"\n  ",msg});
    "/secure/news"->WriteNote(art,1);
  }

  if(strlen(tmp) != 46) {
      tmp = implode(old_explode(tmp, "\n"), " ");
      CHMASTER->send("Nachrichten", this_object(),
                     implode(explode(tmp, " ") - ({"", " "}), " "),
                     MSG_SAY);
  }

  if(i < sizeof(news)-1) call_out("_sendnews", 30, news, i+1);
  else
  {
    string text;
    int next, k;
    k = 0;
    while(k < sizeof(times) &&
          (next = (times[k]-1) * 3600 - time() % (24 * 3600)) < 0)
      k++;
    if(k == sizeof(times))
    {
      next = (23 * 3600) - (time() % (24 * 3600))
           + (times[0] * 3600);
      k = 0;
    }
    text = "Ende der Meldungen. Die naechsten Nachrichten in "
   + (next / 3600) + " Stunde" + (next / 3600 == 1 ? "" :"n")
   + " und " + ((next % 3600) / 60)
   + " Minuten ("+(times[k]+1)+" Uhr).";
    CHMASTER->send("Nachrichten", this_object(), text, MSG_SAY);
    CHMASTER->send("Nachrichten", this_object(),
                   "Die vollstaendigen Artikel sind in der Rubrik \"dwnews\""
                   " in der MPA nachzulesen.", MSG_SAY);
    reset();
  }
}

void NewsUpdate()
{
  mixed *art;
  mixed data, tmp;
  string text, dt;

  while(remove_call_out("NewsUpdate") != -1);
  if( !stringp(tmp = read_file(NEWSFILE)) )
      return;

  data = " " + tmp;

  dt = regexp(old_explode(data, "\n   "), "\\(.* Uhr .*\\)")[0][1..<2];
  // tell_object(find_player("hate"), sprintf("%O", dt));

  data = old_explode(data, "\n   \n   ");
  if(sizeof(data) < 2) return;


  tmp = read_file(NEWSFILE, 1, 1);
  if(file_time(NEWSFILE) != newnews[0])
  {
    mixed artikel;
    "/secure/news"->RemoveNote("dwnews",-1);
    artikel=({"dwnews","Deutsche Welle",0,0, dt,"\n"+data[0]});
    "/secure/news"->WriteNote(art,1);
    newnews[0] = file_time(NEWSFILE);
    newnews[1] = 1;
  }
  else newnews[1] = 0;

  text = "In fuenf Minuten werden die "
       + (newnews[1] ? "neuesten ":"") + "Schlagzeilen, zusammengestellt "
       + "am "+dt+", auf Nachrichten gesendet.\n";
  CHMASTER->join("Allgemein", this_object());
  CHMASTER->send("Allgemein", this_object(), text, MSG_SAY);
  CHMASTER->leave("Allgemein", this_object());
  call_out("_sendnews", 300, data[1..], 0);
}

void clean_up(int refc)
{
  return 0;
}
