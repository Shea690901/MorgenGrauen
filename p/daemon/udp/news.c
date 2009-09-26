// NEWS.C -- rp-online.de news receiver

#pragma strong_types
#pragma combine_strings

#define HL1		"hl1"
#define HL2		"hl2"
#define URL		"url"
#define CATEGORY	"kategorie"
#define VALUE		"VALUE"

#include <daemon.h>
#include <language.h>

#define PORT    96668

#define DEBUGK(str) tell_object(find_player("kirk"), sprintf("DEBUG: %O\n",str))
#define DEBUGH(str) tell_object(find_player("hate"), sprintf("DEBUG: %O\n",str))

#define TEST "NEWS:<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> <ticker> <id></id> <kategorie>Special/Bundestagswahl 2002/</kategorie> <datum>20020730T090639+0200</datum> <hl2>Erhöhung der Erbschaftsteuer</hl2> <hl1>Grüne:</hl1> <url>http://www.rp-online.de/special/wahl2002/2002-0730/gruene_steuern.html</url> </ticker>"

int parse(mapping xml, string key, mixed array, int i) {
  mapping tmp;
  tmp = ([]);
  while(array[i] != "</"+key+">") {
    if(array[i][0] == '<') {
      i = parse(xml, array[i][1..<2], array, ++i) + 1;
    } else {
      tmp[VALUE] = array[i++];
    }
  }
  xml[key] = tmp;
  return i;
}

mapping parseXML(string msg) {
  mixed tmp, i;
  mapping xml;
  msg = implode(explode(msg,"NEWS:"),"");
  tmp = regexplode(msg, "[<][^>]*[>]") - ({ "", " " "\n" });
  xml = ([]);
  for(i = 1; i < sizeof(tmp); i++) {
    if(tmp[i][0] == '<' && tmp[i][1] != '/') {
      i = parse(xml, tmp[i][1..<2], tmp, ++i) + 1;
    }
  }
  return xml;
}



int check(string ch, object pl, string cmd)
{
  return 1;
}

string name()
{
  return "RPO";
}

// _sendnews() -- send news to channel
void _sendnews(mapping news)
{
  string tmp;

  news[CATEGORY][VALUE] = regreplace(news[CATEGORY][VALUE],"/$","",0);
  news[URL][VALUE] = regreplace(news[URL][VALUE],"^http://www\\.","",0);
  tmp = news[CATEGORY][VALUE]+": "+news[HL2][VALUE]+"; "+news[HL1][VALUE];
  //DEBUGK(tmp);
  //DEBUGH(tmp);
  CHMASTER->send("N", this_object(), tmp, MSG_SAY);
  CHMASTER->send("N", this_object(), news[URL][VALUE], MSG_SAY);
  //art=({"news","RPO",0,0, news[CATEGORY][VALUE]+"\n  ",({tmp,"http://www."+news[URL][VALUE]})});
  //"/secure/news"->WriteNote(art,1);
}

void clean_up(int refc)
{
  return 0;
}

void _receive_udp(mixed msg)
{
  mapping tmp;
  
  //DEBUGK(msg);
  tmp = parseXML(msg);
  //DEBUGK(tmp);
  //DEBUGH(tmp);
  _sendnews(tmp);
}

/*
void test()
{
  _receive_udp(TEST);
}
*/

void create()
{
  seteuid(getuid());
}

