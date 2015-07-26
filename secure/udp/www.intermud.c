// MorgenGrauen MUDlib
//
// www.who.c
//
// $Id: www.intermud.c 8103 2012-08-28 19:16:42Z Zesstra $

#pragma strong_types
#pragma combine_strings

#include <udp.h>
#include <www.h>

private int filter_ldfied(string nm, string chars, mapping muds)
{
  if(nm[0] >= chars[0] && nm[0] <= chars[1])
    return muds[nm][0];
  return 0;
}

string Request(mapping cmds)
{
  string err;
  mapping muds;
  if(!sizeof(cmds) || !cmds[IMUD] || !cmds[TYPE]) 
  {
    if(!cmds[PAGE]) cmds[PAGE] = "af";
    return 
 "<H1 ALIGN=\"CENTER\">Intermud Request</H1>"
+"<H2 ALIGN=\"CENTER\">"
+"MUDs from <I>"+capitalize(cmds[PAGE][0..0])
+" to "+capitalize(cmds[PAGE][1..1])+"</I><BR>\n<B>"
+"<A HREF=\""+MUDWWW+"?"+REQ+"="+R_INTERMUD+"&"+PAGE+"=af\">[A-F]</A> "
+"<A HREF=\""+MUDWWW+"?"+REQ+"="+R_INTERMUD+"&"+PAGE+"=gk\">[G-K]</A> "
+"<A HREF=\""+MUDWWW+"?"+REQ+"="+R_INTERMUD+"&"+PAGE+"=ls\">[L-S]</A> "
+"<A HREF=\""+MUDWWW+"?"+REQ+"="+R_INTERMUD+"&"+PAGE+"=tz\">[T-Z]</A></B></H2>"
+"<H6 ALIGN=\"CENTER\">"
+"Diese Seite ist Englisch, da es ein Gateway fuer Intermud sein soll, "
+"d.h. auch englischsprachige Nutzer sollen etwas davon haben :)</H6>\n"
+"<HR><FORM METHOD=GET ACTION=\""+MUDWWW+"\">"
+"<INPUT TYPE=hidden NAME=\""+REQ+"\" VALUE=\""+R_INTERMUD+"\">\n"
+"<INPUT TYPE=hidden NAME=\""+BACK+"\" "
+"VALUE=\""+MUDWWW+"?"+REQ+"="+R_INTERMUD+"\">\n"
+"<H3>To get information via InterMUD you need to give the following "
+"information:</H3>\n"
+"First of all you need to select the type of request "
+"<SELECT NAME=\""+TYPE+"\">"
+"<OPTION>"+implode(INETD->query("commands") - ({"channel",
                                                 "mail",
                                                 "htmlwho",
                                                 "tell",
                                                 "locate",
                                                 "www"}), "<OPTION>\n")
+"</SELECT><BR>and the MUD you want to question "
+"<SELECT NAME=\""+IMUD+"\">"
+"<OPTION>"+implode(
              map(sort_array(
                          m_indices(muds = INETD->query("hosts")-(["www"])),
                          #'>), 
                        #'filter_ldfied, cmds[PAGE], muds) - ({0}), "<OPTION>\n")
+"</SELECT>.<P>"
+"Eventually some requests require more data like a nickname: "
+"<INPUT NAME=\""+IDATA+"\" SIZE=\"12\"><P>\n"
+"When you're finished with editing and selecting you may "
+"<INPUT TYPE=submit VALUE=\"Send the Request\"><BR>"
+"or in case something went wrong <INPUT TYPE=reset> the form."
+"</FORM>";
  }
  if(err = INETD->_send_udp(cmds[IMUD],([
                                      REQUEST: cmds[TYPE],
                                      SENDER: (string)this_object(),
                                      DATA: cmds[IDATA],
                                    ]), 1))
    return ERROR(err);
  call_out("udp_reply", TIMEOUT, 
			([DATA:"<H1><P ALIGN=\"CENTER\">TIMEOUT!</P></H1>",
		          NAME: cmds[IMUD], REQUEST: cmds[REQ]]));
  return 0;
}

void udp_reply(mapping data)
{
  remove_call_out("udp_reply");
  WWW->Send(0, "<H1>Intermud Request: "+data[REQ]+" <I>"+data[NAME]+"</I>"
	     +"</H1><HR><PRE>"
              +data[DATA]+"</PRE>", MUDWWW+"?"+REQ+"="+R_INTERMUD);
}
