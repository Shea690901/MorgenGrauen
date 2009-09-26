// MorgenGrauen MUDlib
//
// www.walk.c -- WWW Guest Walker Client
//
// $Id: www.walk.c 7076 2009-01-19 20:57:37Z Zesstra $

#pragma strong_types
#pragma combine_strings

#include <www.h>

inherit UDPPATH+"/www.crypt";

#include <properties.h>

#define TAB \
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789(){}[]-_+:;.,/"
#define PASSWD "7d.4A37d;Hs[."
#define CRYPT(str, tab, pwd)	_crypt(str, tab, pwd)
#define DECRYPT(str, tab, pwd)	_decrypt(str, tab, pwd)

#define NUMBER(i)	(([1:"einen",2:"zwei",3:"drei",4:"vier",5:"f&uuml;nf",\
			   6:"sechs",7:"sieben",8:"acht",9:"neun"])[i])

// global variables
// YOU NEED TO DEFINE, WHERE SOMEONE MAY GO OR NOT GO!


// DEFINE STARTINGPOINTS
private mapping StartPoints = ([ "Menschen" : "/gilden/abenteurer",
                                 "Zwerge"   : "/d/gebirge/room/zkapelle",
                                 "Elfen"    : "/d/wald/room/es_mitte",
                              ]);

// HERE DEFINE AREAS WHERE A WWW GUEST MAY RUN AROUND
private string *Paths = ({ "/gilden/",
			   "/d/ebene/room/PortVain/",
                           "/d/wald/room/",
	       	           "/d/wald/quasimodo/room/",
			   "/d/gebirge/room/he",
			   "/d/gebirge/room/z",
			   "/d/gebirge/room/schl",
			   "/d/gebirge/room/kueste",
			   "/d/gebirge/room/anhoehe",
			   "/d/gebirge/room/beisshoehle1",
			   "/d/gebirge/amynthor/maerchen/schloss",
                        });

// EXCLUDE ALL ROOMS WITH PARTIAL PATHS LIKE BELOW
private string *PathsExclude = ({"/d/wald/room/hoehle/",});


// prototypes
private string CreateLink(object item, string url, string type);
private void StraightExits(string dir, string file, mapping exits);
private int Validate(string file);
private string CRYPT(string str, string tab, string passwd);
private string DECRYPT(string str, string tab, string passwd);

string Request(mapping cmds)
{
  string file, text;
  object obj;
  
  if (!sizeof(cmds)) return ERROR("Anfrage ung&uuml;ltig!");
  if(!sizeof(cmds) || !cmds[ROOM]) 
    return "<H2>WebVoyeur!</H2>\n"
         +"Der WebVoyeur erlaubt Dir, Dich umzusehen ohne, da� Du direkt "
	 +"einloggen mu�t. Damit kann man sich einige Gegenden ansehen und "
	 +"das eine oder andere Ding oder auch Spieler betrachten.<P>\n"
	 +"Such Dir einen Anfangspunkt aus:<P>"
	 +"<UL><LI>"
	 +implode(map(m_indices(StartPoints), 
			  lambda(({'i}),
			  ({#'+, ({#'+, ({#'+, ({#'+, "<A HREF=\""+MUDWWW+"?"+REQ+"="+R_WALK+"&"+ROOM+"=", 'i}), "\">"}), 'i}), "</A>"}))),
                   "<LI>")+"</UL>" // '
         +"<P>"
	 +"Alternativ kannst Du auch �ber einen speziellen Service in "
	 +"einige Angebote anderer MUDs schauen. Hier gehts zum "
	 +"<A HREF=\"/cgi-bin/mudwww?REQ=intermud\">InterMUD</A>";
  else
    if(member(StartPoints, cmds[ROOM]))
      cmds[ROOM] = CRYPT(StartPoints[cmds[ROOM]], TAB, PASSWD);

  file = DECRYPT(cmds[ROOM], TAB, PASSWD);
  if(!Validate(file))
    return "<H1>"+MUDNAME+" -- MudWWW Zugang</H1><HR><TT>"
	 + "Hier geht es zwar noch weiter, aber aus gewissen Gr&uuml;nden "
	 + "k&ouml;nnen wir den Zugang weiter nicht erlauben!<P>"
	 + "Allerdings kannst Du auch richtig "
	 + "<A HREF=\"http://"+MUDHOST+"/online/\">"+MUDNAME+" spielen</A>. "
         + "Da geht es dann "
	 + "hier auch weiter."
	 + "Im Moment bleibt Dir nur die M&ouml;glichkeit "
	 + "zur&uuml;ck zu gehen."
	 + "</TT><HR>";
  text = "";

  // HERE COMES YOUR OWN SPECIAL ROOM/DETAILS ETC DESCRIPTION!!!!

  // load the room, if it doesn't already exists
  if(!(obj = find_object(file)))
  {
    file->__DoesThisRoomExist();
    obj = find_object(file);
  }
  if(obj)
  {
    int i, light;
    mapping exits;
    object *items;
    closure query;

    query = symbol_function("QueryProp", obj = find_object(file));
    if(light = funcall(query, P_LIGHT) > 0)
    {
      // it needs to be bright to see whats there!
      // put the short description in the first line and the long one as
      // body underneath the shadow line
      text = "<H1>"+funcall(query, P_INT_SHORT)+"</H1><HR>"
           + "<PRE>"+funcall(query, P_INT_LONG);

      // now lets have a look at the items (not details!)
      if(sizeof(items = all_inventory(obj)))
      {
	string *shorts;
         // if there is an aggressive monster there don't allow further
         // processing, just allow to go back (one could map_ldfied!)
	 if(sizeof(map_objects(items, "QueryProp", P_AGGRESSIVE)-({0})))
	   return "<H1>Gefahr!</H1><HR>"
                 +"<PRE>"
		 + "In diesem Raum ist es so gef&auml;hrlich, dass Du "
                 +"lieber sofort zur&uuml;ck gehst.</PRE>";

         // try to wrap the items with links
         if(sizeof(shorts = map(items, #'CreateLink/*'*/, 
                                             MUDWWW+"?"+ROOM+"="+
				         CRYPT(file, TAB, PASSWD)+
				         "&"+BACK+"="+MUDWWW+"?"+cmds[BACK],
                                             DETAIL) - ({0})))
           text+="<BR>Du siehst hier:<UL><LI>"+implode(shorts, "<LI>")+"</UL>";
         else text += "Hier steht und liegt niemand und nichts herum.";
      }
      text += "</PRE>";
    } 
    else // if it's too dark just display the following text
      text = "<H1>Irgendwo</H1><HR><PRE>"
           + "Es ist zu dunkel, um etwas zu sehen.</PRE>";

    // is there a details to be shown?
    if(cmds[DETAIL])
    {
      string det;
      object item;
      // don't show items, when it's dark or it is not found or
      // when the item is invisible
      cmds[DETAIL]=lower_case(cmds[DETAIL]);
      if(light <= 0 ||
         (!(det = funcall(query, P_DETAILS)[cmds[DETAIL]]) &&
          !((item = present(cmds[DETAIL], obj)) && !item->QueryProp(P_INVIS))))
        det = "\""+capitalize(cmds[DETAIL])+"\" kannst Du hier nicht sehen!<BR>"
            + "Es ist m&ouml;glicherweise entfernt worden.";
      if(objectp(item)) 
      {
	string wwwinfo;
	det = item->long();
	if(wwwinfo = item->QueryProp(P_WWWINFO)) det += wwwinfo;
	if(query_once_interactive(item))
	  det += "\nMehr Information zu <A HREF=\""+MUDWWW+"?"
	      + REQ+"="+R_FINGER+"&"
	      + "USER="+getuid(item)+"&"+BACK+"="+MUDWWW+"?"+cmds[BACK]+"\">"
	      + capitalize(getuid(item))+"</A>!";
      }
      if(!stringp(det)) det = (string)det[0];
      text += "<HR><H3>"+capitalize(cmds[DETAIL])+"</H3>"
	   + "<PRE>"+det+"</PRE><HR>";
    }

    // now display the exits
    text += "<H3>";
    exits = funcall(query, P_EXITS);
    walk_mapping(exits, #'StraightExits/*'*/, exits);
    if(!light || !(i = sizeof(exits))) 
      text += "Es gibt hier keine sichtbaren Ausg&auml;nge!";
    else 
    {
      string *ind;
      text += "Es gibt hier "+NUMBER(i)
            + " sichtbare"+(i==1?"n Ausgang":" Ausg&auml;nge")+": ";
      for(i = 0; i < sizeof(ind = m_indices(exits)); i++)
        text += "<A HREF=\""+MUDWWW+"?"+REQ+"="+R_WALK+"&"
	      + ROOM+"="+CRYPT(exits[ind[i]], TAB, PASSWD)
	      + "&"+BACK+"="+MUDWWW+"?"+cmds[BACK]+"\">"
	      + ind[i]+"</A> ";
    }
    text += "</H3>";

  }
  // this should never occur!
  else return "<H1>Der Raum ist momentan nicht verf&uuml;gbar!</H1>";
  return text
       + "<FORM ACTION=\""+MUDWWW+"\">"
       + "Was m&ouml;chtest Du n&auml;her betrachten? "
       + "<INPUT TYPE=\"hidden\" NAME=\""+REQ+"\" VALUE=\""+R_WALK+"\">"
       + "<INPUT TYPE=\"hidden\" NAME=\""+ROOM+"\" VALUE=\""
       + CRYPT(file, TAB, PASSWD)+"\">"
       + "<INPUT TYPE=text NAME=\""+DETAIL+"\">"
       + "<INPUT TYPE=submit VALUE=\"Ansehen!\"></FORM><BR>";
}

// extra functions

// CreateLink() -- create a link to with a special input type
private string CreateLink(object item, string url, string type)
{
  string s, *tmp; mixed nm; int i;
  if((s = item->short()) && (nm = item->QueryProp(P_NAME)))
  {
    if(pointerp(nm)) nm = nm[0];
    if(member(nm, ' ') != -1) nm = item->QueryProp(P_IDS)[<1];
    if((i = member(tmp = regexplode(s, nm), nm)) == -1) i = 0;
    tmp[i] = "<A HREF=\""+url+"&"+REQ+"="+R_WALK
           + "&"+type+"="+lower_case(nm)+"\">"+tmp[i]+"</A>";
    return implode(tmp, "");
  }
  return "";
}

private void StraightExits(string dir, string file, mapping exits)
{
  int i;
  if((i = member(file, '#')) != -1)
    file = file[i+1..];
  exits[dir] = file;
}

// Validate()
private int Validate(string file)
{
  int i;

	if(file[0]!='/')
	  file = "/"+file;

  i = sizeof(PathsExclude);
  while(i--)
    if(strstr(file, PathsExclude[i]) > 0) return 0;
  i = sizeof(Paths);
  while(i--)
    if(strstr(file, Paths[i]) != -1) return 1;
  return 0;
}
