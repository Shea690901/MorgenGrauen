// Original von Jof, voellig ueberarbeitet
/*
Letzte Aenderung: Vanion, 15.04.02
                  - Die Langbeschreibung ist in der Regel so lang, dass sie
                    durch ein pl->More() ausgegeben werden sollte.
                  - Kleine Schoenheits-OP an der Funktion Partyliste. 
                    3 statt 2 fuehrende Leerzeichen
                  Vanion, 20.04.02
                  - Gaeste koennen sich nicht mehr zur Party anmelden
		  Humni, 03.06.2002
		  - Auf Wunsch von Deepblue geht jetzt auch 
		  ,,frage partybrett nach humni''.
 2005-01-11, Zook: Ein wenig umgestrickt. 	
*/


inherit "/std/thing";

#include <properties.h>
#include <language.h>
#include <mail.h>

#define __PARTY_NEED_ROOMS__
#include "party.h"
#include "brett.h"

#define ME this_object()
#define TP this_player()

static mapping newdata;
static string infotext, moreinfo;
static string *binfo;
int bsz;


create() {
  if (!clonep(ME)) return;
  ::create();
  newdata=([]);
  if (!stringp(infotext=read_file(BESTAETIGFILE)))
    infotext="";
  binfo=({"<alle>"})+regexplode(infotext,"\<[^<>]*\>");bsz=sizeof(binfo);
  if (!stringp(infotext=read_file(INFOFILE)))
    infotext="";
  if (!stringp(moreinfo=read_file(MOREINFOFILE)))
    moreinfo="";

  SetProp(P_NAME,"Partybrett");
  SetProp(P_SHORT,"Ein Partybrett");
  SetProp(P_ARTICLE,1);
  SetProp(P_NOGET,"Stehenlassen!\n");
  SetProp(P_WEIGHT,12345678);
  AddId( ({ "party\nbrett", "partybrett", "PartyBrett", "brett" }) );
  seteuid(getuid(ME));
  AddCmd("uebersicht","uebersicht");
  AddCmd("anmelden","anmelden");
  AddCmd("abmelden","abmelden");
  AddCmd(({"frage","frag"}),"fragen");
  AddCmd("partyliste","partyliste");
}

init() {
  ::init();
  if (getuid(TP)==ORGANISATOR) {
    add_action("bestaetigen","bestaetige");
    add_action("zeige_protokoll", "protokoll");
  }
}

static string *GetKeys() {
  mapping *k;
  string *keys;
  int i,j;

  k=Fragen();keys=({});
  for (i=sizeof(k)-1;i>=0;i--)
    keys+=({k[i][Q_KEY]});
  keys+=({KEY_BESTAETIGUNG});
  return keys;
}

static string CountUp(string *strs) {
  string res;
  int i;

  res="";
  for (i=sizeof(strs)-1;i>=0;i--) {
    res+=capitalize(strs[i]);
    if (i) res+=((i>1)?", ":" und ");
  }
  return res;
}

static string list_data(mapping dat) {
  mixed *qs,x;
  string text,str,key;
  int i,sz;

  qs=Fragen();sz=sizeof(qs);text="";
  for (i=0;i<sz;i++) {
    key=qs[i][Q_KEY];
    str=qs[i][Q_TEXT]+" "+capitalize(stringp(dat[key])?dat[key]:"(unbekannt)");
    if (mappingp(x=qs[i][Q_CHOICES]) && stringp(x[dat[key]]))
      str+=(" ("+x[dat[key]]+")");
    text+=break_string(str+"\n",78,"    ")[2..];
  }
  text+="\n";
  if (dat[KEY_DATUM])
    text+=sprintf("Angemeldet am %s\n",dtime(dat[KEY_DATUM]));
  if (dat[KEY_BESTAETIGUNG]=="ja")
    text+="*** Anmeldung bestaetigt ***\n";
  return text;
}

static uebersicht() {
  mapping alldat,dat,res;
  string *ind,*ind2,key,out;
  int i,j,sz2;
  mixed *qs;

  qs=Fragen();res=([KEY_BESTAETIGUNG:([])]);
  for (i=sizeof(qs)-1;i>=0;i--) {
    if (!mappingp(dat=qs[i])) continue;
    if (!stringp(key=dat[Q_KEY])) continue;
    if (dat[Q_CHOICES] || dat[Q_BOOLEAN])
      res[key]=([]);
  }
  ind2=m_indices(res);sz2=sizeof(ind2)-1;

  if (!mappingp(alldat=PM->QueryData())) alldat=([]);
  ind=m_indices(alldat);
  for (i=sizeof(ind)-1;i>=0;i--) {
    dat=alldat[ind[i]];
    for (j=sz2;j>=0;j--) {
      key=ind2[j];
      res[key][dat[key]]++;
    }
  }

  out="";
  ind2=sort_array(ind2,#'<);
  for (i=sz2;i>=0;i--) {
    if (!stringp(key=ind2[i])) continue;
    if (!mappingp(dat=res[key])) continue;
    ind=sort_array(m_indices(dat),#'<);
    out+=sprintf("%-15s ",capitalize(key+":"));
    for (j=sizeof(ind)-1;j>=0;j--)
      out+=sprintf("  %4s: %3d%s",ind[j],dat[ind[j]],(j?"  |":""));
    out+="\n";
  }

  write(out);
  return 1;
}

static partyliste(string str) {
  mapping *qs,q,alldat;
  string *ind,ix,x,pre,text;
  int i;

  if (!mappingp(alldat=PM->QueryData()))
    alldat=([]);
  ind=sort_array(m_indices(alldat),#'<);
  if (!str||str=="") {
    write("Bisher haben sich "+sizeof(ind)+" Mitspieler angemeldet.\n");
    write(break_string(CountUp(ind)+"\n",78,"   "));
    return 1;
  }
  if (member(GetKeys(),str)<0)
    return notify_fail("Ungueltiger Schluessel.\n"),0;

  q=([Q_BOOLEAN:1]);
  if (str!=KEY_BESTAETIGUNG) {
    qs=Fragen();
    for (i=sizeof(qs)-1;i>=0;i--) {
      q=qs[i];
      if (str!=q[Q_KEY]) continue;
      break;
    }
  }

  if (q[Q_BOOLEAN] || q[Q_CHOICES]) {
    q=(mappingp(q[Q_CHOICES])?(q[Q_CHOICES]):0);
    for (i=sizeof(ind)-1;i>=0;i--) {
      if (!stringp(ix=ind[i])) continue;
      if (!stringp(x=alldat[ix][str])) x="";
      x=("("+x+")        ")[0..9];
      ind[i]=x+ix;
    }
    ind=sort_array(ind,#'<);
  } else
    q=0;

  pre="";text="";
  for (i=sizeof(ind)-1;i>=0;i--) {
    if (!stringp(ix=ind[i])) continue;
    if (ix[0..0]=="(") {
      if (ix[0..9]!=pre) {
        text+="------------\n";
        pre=ix[0..9];
      }
      ix=ix[10..];
    }
    if (!mappingp(alldat[ix])) continue;
    x=alldat[ix][str];
    if (mappingp(q)) x=q[x];
    if (!stringp(x)) x="";
    x=capitalize(ix+":            ")[0..13]+capitalize(x)+"\n";
    text+=break_string(x,78,"  ")[2..];
  }
  TP->More(text);
  return 1;
}

static fragen(str) {
  mapping dat;
  string s,s2;

  if (!str) return 0;
  if (!sscanf(str,"%s nach %s",s2,s)||s==""||(!id(s2)&&s2!="brett")) return 0;
  str=lower_case(s);
  if (!mappingp(dat=PM->QueryUser(str))) {
    write(capitalize(str)+" hat sich bisher nicht angemeldet.\n");
    return 1;
  }
  s=list_data(dat);
  TP->More(s);
  return 1;

}

_query_long() {
  string s;

  s="Dies hier ist ein Brett mit Infos zur naechsten MorgenGrauen-Party.\n"
    +"Genauere Informationen findest du in der MPA in der Rubrik 'party'.\n"
    +infotext
    +"Dir stehen folgende Befehle zur Verfuegung:\n"
    "  anmelden, abmelden       - beides klar\n"
    "  frage brett nach <name>  - Infos ueber den Spieler mit dem Namen name\n"
    "  partyliste               - zeigt alle Angemeldeten an\n"
    "  partyliste <schluessel>  - zeigt Daten unter dem Eintrag schluessel an\n";
  if (objectp(TP) && getuid(TP)==ORGANISATOR) {
    s+="  bestaetige <name>        - Bestaetigt Spieler Ankunft des Geldes\n";
    s+="  protokoll                - Zeigt das Protokoll der Eintragungen\n";
  }

  s+="Es gibt folgende Schluessel:\n";
  s+=break_string(CountUp(sort_array(GetKeys(),#'<))+"\n",78);
/* Aenderung: Vanion, 15.04.02 
    Die Langbeschreibung ist in der Regel so lang, dass sie besser 
    durch ein More geschickt wird.
*/
  // Dabei sollte auch sichergestellt sein, dass this_player()!=0 ist.
  // Andernfalls scrollt's den ganzen Tag.
  if (this_player()) this_player()->More(s);
  return "";
/*  return s; */
}

static make_bmail(str) {
  mapping dat;
  string *parts,text,tag;
  int i;

  text="";
  if (!mappingp(dat=PM->QueryUser(str)))
    return text;
  for (i=0;i<bsz;i++) {
    tag=binfo[i++][1..<2];
    if (i>=bsz) continue;
    if (tag=="mudname")
      text+=capitalize(str)+binfo[i];
    else if (tag=="alle")
      text+=binfo[i];
    else if ((sizeof(parts=old_explode(tag,":"))>=2) && dat[parts[0]]==parts[1])
      text+=binfo[i];
  }
  return text;
}

static bestaetigen(str) {
  mixed *mail;
  string text;
  mapping dat;

  if (getuid(TP)!=ORGANISATOR)
    return notify_fail("Das kann nur der Organisator.\n"),0;
  if (!str)
    return "Fuer welchen Spieler soll die Anmelding bestaetigt werden?\n",0;
  if (!mappingp(dat=PM->QueryUser(str)))
    return notify_fail("Der Spieler ist nicht angemeldet.\n"),0;

  dat[KEY_BESTAETIGUNG]="ja";
  PM->SetUser(str,dat);

  text=make_bmail(str);
  if (text=="") {
    text="Hallo, "+capitalize(str)+"!\n\n";
    text+="Dein Geld ist angekommen und Deine Anmeldung somit bestaetigt.\n";
    text+=("\n  "+capitalize(ORGANISATOR)+", Partyorganisator\n");
  }

  mail = allocate(9);
  mail[MSG_FROM] = ORGANISATOR;
  mail[MSG_SENDER] = "MorgenGrauen";
  mail[MSG_RECIPIENT] = str;
  mail[MSG_CC]=0;
  mail[MSG_BCC]=0;
  mail[MSG_SUBJECT]="Bestaetigung der Partyanmeldung";
  mail[MSG_DATE]=dtime(time());
  mail[MSG_ID]="MorgenGrauen:"+time();
  mail[MSG_BODY]=text;
  
  "/secure/mailer"->DeliverMail(mail,1);

  printf("Anmeldung von %s bestaetigt.\n",capitalize(str));
  return 1;
}

static int zeige_protokoll(string str) {
  notify_fail("Diese Funktion ist noch nicht eingebaut!\n");

  return 0;
}


static abmelden() {
  if (!query_once_interactive(TP)) return 0;
  PM->DeleteUser(getuid(TP));
  write("OK, abgemeldet.\n");
  return 1;
}

static end_query() {
  mixed *mail;
  string text;
  mapping dat;

  if (!mappingp(dat=newdata[TP])) dat=([]);
  m_delete(newdata,TP);
  m_delete(dat,KEY_QUERY_NUMBER);
  PM->SetUser(getuid(TP),dat);
  write("OK, angemeldet.\n");

  text="Hallo, "+TP->Name(WER)+"!\n\n";
  text+=("Nachdem Du Dich zur Party angemeldet hast,\n"+
         "hier noch einmal die wichtigsten Daten:\n");
  text+=infotext;

  if (stringp(moreinfo) && moreinfo!="") {
    text+="\nWeitere Informationen:\n";
    text+=moreinfo;
  }

  text+="\nFolgende Eintragungen hast Du vorgenommen:\n";
  text+=list_data(dat);
  if (dat[KEY_BESTAETIGUNG]=="ja")
    text+="\nDein Geld ist bereits angekommen.\n";
  text+="\n\Diese Mail wurde automatisch generiert.\n";


  mail = allocate(9);
  mail[MSG_FROM] = ORGANISATOR;
  mail[MSG_SENDER] = "MorgenGrauen";
  mail[MSG_RECIPIENT] = getuid(TP);
  mail[MSG_CC]=0;
  mail[MSG_BCC]=0;
  mail[MSG_SUBJECT]="Partyanmeldung";
  mail[MSG_DATE]=dtime(time());
  mail[MSG_ID]="MorgenGrauen:"+time();
  mail[MSG_BODY]=text;
  
  return "/secure/mailer"->DeliverMail(mail,1);
}

static next_question() {
  mapping dat,q;
  int num,i;
  mixed qs;
  string str,key,x;

  if (!mappingp(dat=newdata[TP])) dat=([]);
  num=dat[KEY_QUERY_NUMBER];
  qs=Fragen();
  if (num>=sizeof(qs))
    return end_query();
  q=qs[num];key=q[Q_KEY];str=q[Q_TEXT];
  if (q[Q_BOOLEAN])
    str+=" (ja/nein)";
  if (stringp(x=dat[key]) || stringp(x=q[Q_DEFAULT]))
    str+=sprintf(" [%s]",x);
  write(str+"\n");

  qs=q[Q_CHOICES];q=0;
  if (mappingp(qs)) {
    q=qs;
    qs=sort_array(m_indices(qs),#'>);
  }
  if (pointerp(qs)) {
    write("Moeglichkeiten:\n");
    num=sizeof(qs);
    for (i=0;i<num;i++) {
      str="  "+capitalize(qs[i]);
      if (mappingp(q))
        str+=") "+q[qs[i]];
      write(str+"\n");
    }
  }

  input_to("get_answer");
}

static get_answer(string str) {
  mapping dat,q;
  int num;
  mixed qs;
  string key,x;

  if (!mappingp(dat=newdata[TP])) dat=([]);
  num=dat[KEY_QUERY_NUMBER];
  qs=Fragen();
  if (num>sizeof(qs))
    return end_query();
  q=qs[num];key=q[Q_KEY];

  // Default ermitteln
  if (!stringp(str) || str=="")
    if (!stringp(str=dat[key]) && !stringp(str=q[Q_DEFAULT])) {
      write("Du musst schon etwas eingeben...\n");
      input_to("get_answer");
      return;
    }

  // Bei eingeschraenkten Antworten anpassen soweit moeglich:
  qs=q[Q_CHOICES];
  if (q[Q_BOOLEAN] || qs)
    str=lower_case(str);
  if (q[Q_BOOLEAN]) {
    if (str=="j") str="ja";
    if (str=="n") str="nein";
  }

  if (mappingp(qs) && qs[str])
    write("--> "+capitalize(str)+" ("+qs[str]+")\n");
  else
    write("--> "+capitalize(str)+"\n");

  // Einschraenkungen beruecksichtigen
  if ((q[Q_BOOLEAN] && member(({"ja","nein"}),str)<0)
      || (mappingp(qs) && !qs[str])
      || (pointerp(qs) && member(qs,str)<0)) {
    write("Ungueltige Antwort.\n");
    input_to("get_answer");
    return;
  }

  dat[key]=str;
  dat[KEY_QUERY_NUMBER]++;
  newdata[TP]=dat;
  next_question();
}

static anmelden() {
  if (!query_once_interactive(TP)) return 0;
  
  // 20.04.02 by Vanion
  // Gaeste sollen sich nicht mehr anmelden koennen
  if(getuid(TP)[0..3]=="gast")
  {
    notify_fail("Als Gast kannst Du Dich nicht zur Party anmelden.\n");
    return 0;
  }
  if (!mappingp(newdata[TP]=PM->QueryUser(getuid(TP))))
    newdata[TP]=([]);
  newdata[TP][KEY_QUERY_NUMBER]=0;
  if (newdata[TP][KEY_BESTAETIGUNG]!="ja")
    newdata[TP][KEY_BESTAETIGUNG]="nein";
  if (!newdata[TP][KEY_DATUM])
    newdata[TP][KEY_DATUM]=time();
  next_question();
  return 1;
}
