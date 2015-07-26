// MorgenGrauen MUDlib
//
// who.c
//
// $Id: who.c 7391 2010-01-25 22:52:51Z Zesstra $

#include <config.h>
#include <udp.h>

int last;
int maxtoday, maxever;

void create()
{
  string tmp1, tmp2, dummy;

  if (time()-last<1800) return;
  last=time();
  tmp1=read_file("/etc/maxusers.ever",0,1);
  tmp2=read_file("/etc/maxusers",0,1);
  if (stringp(tmp1)&&strlen(tmp1)) sscanf(tmp1,"%d %s",maxever,dummy);
  if (stringp(tmp2)&&strlen(tmp2)) sscanf(tmp2,"%d %s",maxtoday,dummy);
}

string adjust(string str,int wid)
{
  int w2;

  w2=wid/2;
  wid=wid-w2;
  return extract("                                                          ",0,wid-1)+str+
    extract("                                                         ",0,w2-1);
}

void udp_who(mapping data)
{
  int i;
  string *lines;
  string wholiste,header;

  create();
  lines="/obj/werliste"->QueryWhoListe(0, 1);
  wholiste=implode(lines,"\n");
  lines=({
    "*------------------------------------------------------------------------*",
    "","","","",
    "*------------------------------------------------------------------------*"});
  header=MUDNAME", LDMud "+__VERSION__;
  lines[1]="|"+adjust(header,strlen(lines[0])-strlen(header)-2)+"|";
  header="Adresse: MG.Mud.DE (87.79.24.60) 23 (alternativ 4711)";
  lines[2]="|"+adjust(header,strlen(lines[0])-strlen(header)-2)+"|";
  header="Uptime: "+uptime();
  lines[3]="|"+adjust(header,strlen(lines[0])-strlen(header)-2)+"|";
  header=_MUDLIB_NAME_"-Mudlib "_MUDLIB_VERSION_;
  lines[4]="|"+adjust(header,strlen(lines[0])-strlen(header)-2)+"|";
  header=implode(lines,"\n");
  wholiste=header+"\n"+wholiste+sprintf("\n*** Anwesende im "MUDNAME": Max. heute %d, Rekord %d\n",maxtoday,maxever);
  INETD->_send_udp(data[NAME], ([
				REQUEST: REPLY,
				RECIPIENT: data[SENDER],
				ID: data[ID],
				DATA: wholiste
				]) );
}
