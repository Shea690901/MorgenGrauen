// MorgenGrauen MUDlib
//
// questmaster.c -- Questmaster, verwaltet die normalen Quests und
//                  die MiniQuests
//
// $Id: questmaster.c 6897 2008-08-06 11:32:15Z Zesstra $
//
#pragma strict_types
#pragma no_clone
#pragma no_shadow
#pragma no_inherit
#pragma verbose_errors
#pragma combine_strings
//#pragma pedantic
//#pragma range_check
#pragma warn_deprecated

#include "/secure/wizlevels.h"
#include "/secure/questmaster.h"
#include "/secure/lepmaster.h"
#include <properties.h>
#include <daemon.h>
#include <defines.h>
#include <ansi.h>

private int max_QP;
private int opt_QP;
private mapping quests;

private int last_num;
private mapping miniquests;

private static mapping by_num;
private static mapping users_mq;


void save_info()
{
  save_object(QUESTS);
}

static void make_num(string name, mixed werte)
{
  by_num += ([werte[1] : ({werte[0], name})]);
}

void create()
{
  seteuid(getuid(this_object()));
  if (!restore_object(QUESTS))
  {
    quests = ([]);
    max_QP = 0;
    opt_QP = 0;

    miniquests = ([]);
    last_num = 0;
    
    save_info();
  }
  
  by_num = ([]);
  users_mq = ([]);
  walk_mapping(miniquests, #'make_num /*'*/ );
  
  set_next_reset(43200); // Reset alle 12 Stunden.
}

private static int allowed()
{
  if (previous_object() && geteuid(previous_object())==ROOTID)
    return 1;
  if (!process_call() && previous_object() && this_interactive() && ARCH_SECURITY)
    return 1;
  return 0;
}

void reset()
{
  by_num = ([]);
  walk_mapping(miniquests, #'make_num /*'*/ );
  set_next_reset(43200);
}
	     

/*
 * (1) ABSCHNITT "NORMALE QUESTS"
 */


/* Die Quests werden in einem Mapping gespeichert. Der Schluessel ist dabei der
   Quest-Name, die Eintraege sind Arrays der folgenden Form:

   1. Element ist die Zahl der durch diese Quest zu erwerbenden Questpunkte.
   2. Element ist die Zahl der Erfahrungspunkte, die der Spieler bekommt,
      wenn er diese Quest loest.
   3. Element ist ein Array mit den Filenamen der Objekte, denen es gestattet
      ist, diese Quest beim Player als geloest zu markieren (Erzmagier duerfen
      das aber sowieso immer).
   4. Element ist ein String, der die Quest kurz beschreibt. Dieser String wird
      dem Spieler vom Orakel als Hinweis gegeben.
   5. Element ist eine Zahl zwischen -1 und 100, die den Schwierigkeitsgrad der
      Quest angibt, nach Einschaetzung des EM fuer Quests. Das Orakel kann dann
      evtl. sinnige Saetze wie "Diese Quest erscheint mir aber noch recht
      schwer fuer Dich.", oder "Hm, die haettest Du ja schon viel eher loesen
      koennen." absondern. :)

      Ein Wert von -1 bedeutet eine Seherquest. Diese zaehlt nicht zu den
      Maximalen Questpunkten, sondern zaehlt als optionale Quest
   6. Element ist ein Integer von 0 bis 5 und gibt die "Klasse" an;
      ausgegeben werden dort Sternchen
   7. Element ist ein Integer, 0 oder 1.
      0: Quest voruebergehend deaktiviert (suspendiert)
      1: Quest aktiviert
   8. Element ist ein String und enthaelt den Namen des Magiers, der die
      Quest "verbrochen" hat.
   9. Element ist ein String, der den Namen eines Magiers enthaelt, der 
      evtl. fuer die Wartung der Quest zustaendig ist.
  10. Element ist eine Zahl von 0 bis 4, die der Quest ein Attribut
      gibt (0 fuer keines)
*/

// geaendert:
// 5  == diff geht nun von -1 bis 100
// 6  == klasse geht nun von 0 bis 5
// 10 == attribut geht nun von 0 bis 4


int RecalculateQP()
{
  int i;
  mixed q,n;
  
  if (!allowed())
    return -1;

  max_QP=0;
  opt_QP=0;
  
  n=m_indices(quests);
  q=m_values(quests);
    for (i=sizeof(q)-1;i>=0;i--) 
      if (q[i][Q_ACTIVE]) {
	if (q[i][Q_DIFF]>=0)
	  max_QP+=q[i][Q_QP]; 
	if (q[i][Q_DIFF]==-1)
	  opt_QP+=q[i][Q_QP];
      }
    
  return max_QP+opt_QP;
}

int AddQuest(string name, int questpoints, int experience,
      string *allowedobj, string hint, int difficulty, int questclass,
      int active, string wiz, string scndwiz, int questattribute)
{
  mixed *quest;
  int i;
  
  if (!allowed()) return 0;
  if (!stringp(name) || strlen(name)<5) return -1;
  if (questpoints<1) return -2;
  if (!intp(experience)) return -3;
  if (stringp(allowedobj)) allowedobj=({allowedobj});
  if (!pointerp(allowedobj)) return -4;
  for (i=sizeof(allowedobj)-1;i>=0;i--)
    {
      if (!stringp(allowedobj[i]) || allowedobj[i]=="") return -4;
      allowedobj[i]=(string)"/secure/master"->_get_path(allowedobj[i],0);
    }
  if (!stringp(hint) || hint=="") return -5;
  if (difficulty<-1 || difficulty>100) return -6;
  if (questclass<0 || questclass>5) return -11;
  if (active<0 || active>1) return -7;
  if (!stringp(wiz) || wiz=="" || 
      file_size("/players/"+(wiz=lower_case(wiz))) != -2) return -8;
  if (!stringp(scndwiz))
    scndwiz="";
  else if (file_size("/players/"+(scndwiz=lower_case(scndwiz))) != -2) 
    return -9;
  if (questattribute<0 || questattribute>4)
    return -10;
  if(quests[name]&&(quests[name][5]==0||quests[name][5]==1)&&quests[name][6])
    max_QP-=quests[name][0];
  quests+=([name:({questpoints,experience,allowedobj,hint,difficulty,questclass,active,wiz, scndwiz,questattribute})]);
  RecalculateQP();
  save_info();
  QMLOG(sprintf("add: %s %O (%s)",name,quests[name],
		getuid(this_interactive())));
  return 1;
}

int RemoveQuest(string name)
{
  mixed *quest;
  
  if (!allowed()) return 0;
  if (!quests[name]) return -1;
  QMLOG(sprintf("remove: %s %O (%s)",name,quests[name],
		getuid(this_interactive())));
  quests=m_delete(quests,name);
  RecalculateQP();
  save_info();
  return 1;
}


int QueryNeededQP()
{
  return REQ_QP;
}

int QueryMaxQP()
{
  return max_QP;
}

int QueryOptQP()
{
  return opt_QP;
}

int QueryTotalQP()
{
  return max_QP+opt_QP;
}

mixed *QueryGroupedKeys()
{
  string *qliste;
  mixed  *qgliste;
  int i, j;
  
  qgliste = allocate(sizeof(QGROUPS)+1); // letzte Gruppe sind die Seherquests
  qliste = m_indices(quests);
  
  for (i=sizeof(qgliste)-1;i>=0;i--)
    qgliste[i]=({});
  
  for (i=sizeof(qliste)-1;i>=0;i--)
    {
      // inaktive quest?
      if (!quests[qliste[i]][Q_ACTIVE]) 
	continue;
      // optionale quest? also Seherquest
	if (quests[qliste[i]][Q_DIFF]==-1) 
	  qgliste[sizeof(QGROUPS)] += ({qliste[i]});
	else {
	  // dann haben wir also eine normale Quest und daher Einordnung
	  // nach dem Schwierigkeitswert
	  for (j=sizeof(QGROUPS)-1; 
	       j>=0 && QGROUPS[j]>=quests[qliste[i]][Q_DIFF];j--)
	    ;
	  qgliste[j] += ({qliste[i]}); 
	}
    }
  
  return qgliste;
}


// folgende funk brauch ich glaube ich nicht mehr:
int QueryDontneed(object pl)
{
  raise_error("Ich glaub, die Func QueryDontneed braucht kein Mensch mehr. (Zook)");
}

/*
int *QueryDontneed(object pl)
{
  int i, j, qp_percent, nrgr;
  mixed *qgrouped;
  int *num, *pts, *med, *res;
  
  nrgr = sizeof(QGROUPS);
  
  if(!pl || !objectp(pl))
    pl=(this_player()?this_player():this_interactive());
  if(!pl)
    return allocate(nrgr);
  
  num=allocate(nrgr);
  pts=allocate(nrgr);
  med=allocate(nrgr);
  res=allocate(nrgr);
  
  qgrouped = QueryGroupedKeys()[0..<2];
  
  for (i=sizeof(qgrouped)-1;i>=0;i--) 
    for (j=sizeof(qgrouped[i])-1;j>=0;j--) {
      num[i]++;
      pts[i]+=quests[qgrouped[i][j]][0];
    } 
  
  qp_percent=pl->QueryProp(P_NEEDED_QP)*100/QueryMaxQP();
  if(!qp_percent)
    qp_percent=QP_PERCENT;
  
  for (i=nrgr-1;i>=0;i--)
    {
      med[i]=(pts[i]/(num[i]||1)||1);
      pts[i]-=pts[i]*qp_percent/100;
      res[i]=(pts[i]%med[i]>0?(pts[i]/med[i]+1):pts[i]/med[i]);
    }
  
  return res;
}
*/

// Die folgende Func braucht man nicht mehr
int QueryReadyForWiz(object player)
{
  raise_error("Die Func QueryReadyForWiz braucht keiner mehr. (Zook)");
}

/*
int QueryReadyForWiz(object player)
{
  int i, j;
  string *indices;
  mixed *notsolved, *qgrouped;
  int *groups,*dontneed;
  int bonus;
  
  if(!player || !objectp(player))
    player=(this_player()?this_player():this_interactive());
  
  if(!player)
    return -1;
  
  if (player->QueryProp(P_QP)<player->QueryProp(P_NEEDED_QP))
    return -1;
  
  // This is for security:
  if (player->QueryProp(P_NEEDED_QP)<QP_MIN&&player->QueryProp(P_QP)<QP_MIN)
    return -1;
  
  groups = allocate(sizeof(QGROUPS));
  dontneed = QueryDontneed(player);
  notsolved=({});
  
  qgrouped = QueryGroupedKeys()[0..<2];
  
  for (i=sizeof(qgrouped)-1;i>=0;i--) 
    for (j=sizeof(qgrouped[i])-1;j>=0;j--) {
      if (!player->QueryQuest(qgrouped[i][j]))
	groups[i]++;
      if (quests[qgrouped[i][j]][5]==1 &&
	  !player->QueryQuest(qgrouped[i][j]))
	notsolved+=({qgrouped[i][j]});
    }  
  
  for (i=sizeof(QGROUPS)-1; i>=0;i--)
    if(dontneed[i]-groups[i]>=0)
      bonus++;
  
  if (!sizeof(notsolved) && bonus == sizeof(QGROUPS))
    return 1;
  return 0;
}
*/

mixed *QueryQuest(string name) 
{
  if(!quests[name])
    return ({});
  if( extern_call() )
    return deep_copy( quests[name] );
  return quests[name];
}

int QueryQuestPoints(string name)
{
  if( !quests[name] )
    return -1;
  
  return quests[name][Q_QP];
}

mixed *QueryQuests()
{
  if( extern_call() )
    return ({m_indices(quests),map(m_values(quests),#'deep_copy /*'*/)}); 
  return ({ m_indices(quests), m_values(quests) });
}

string *QueryAllKeys()
{
  return m_indices(quests);
}


int SetActive(string name, int flag)
{
  mixed *quest;
  
  if (!allowed()) return 0;
  if (!(quest=quests[name])) return -1;
  switch(flag)
    {
    case 0:
      if (quest[Q_ACTIVE] == flag)
	return -2;
      quest[Q_ACTIVE] = flag;
      break;
    case 1:
      if (quest[Q_ACTIVE] == flag)
	return -2;
      quest[Q_ACTIVE] = flag;
      break;
    default:
      return -3;
    }
  quests[name]=quest;
  RecalculateQP();
  save_info();
  QMLOG(sprintf("%s: %s (%s)",(flag?"activate":"deactivate"),name,
		getuid(this_interactive())));
  return 1;
}

string name()
{
  return "<Quest>";
}
string Name() { return "<Quest>"; }

void Channel(string msg)
{
  if(!interactive(previous_object()))
    return;
  catch(CHMASTER->send("Abenteuer", this_object(), msg);publish);
}

 /* quoted from /sys/mail.h: */
#define MSG_FROM 0
#define MSG_SENDER 1
#define MSG_RECIPIENT 2
#define MSG_CC 3
#define MSG_BCC 4
#define MSG_SUBJECT 5
#define MSG_DATE 6
#define MSG_ID 7
#define MSG_BODY 8

void SendMail(string questname, mixed *quest, object player)
{
  mixed* mail;
  string text;
  
  mail = allocate(9);
  
  text =
    "Hallo "+capitalize(getuid(player))+",\n\n"+
    break_string("Nachdem Du gerade eben das Abenteuer '"+
		 questname +"' ("+quest[Q_QP]+" Punkte), das "+
		 capitalize(quest[Q_WIZ])+" fuer das MorgenGrauen entworfen hat, "
		 "mit Erfolg bestanden hast, sind "
		 "wir nun an Deiner Meinung dazu interessiert:", 78)+
    "\n  Hat Dir das Abenteuer gefallen und wieso bzw. wieso nicht?\n"
    "  Ist die Einstufung Deiner Meinung nach richtig? (AP und Stufe)\n"
    "  Gab es Probleme oder gar Fehler?\n"
    "  Hast Du Verbesserungsvorschlaege?\n\n";
  
  if (quest[8]!="") {
    text += break_string("Diese Nachricht wurde automatisch verschickt, "		 
	"wenn Du mit dem 'r' Kommando darauf antwortest, geht die Antwort "	 
	"direkt an "+capitalize(quest[Q_SCNDWIZ])
	+" als \"Warter\" des Abenteuers.",78);
    mail[MSG_FROM] = quest[Q_SCNDWIZ];
    mail[MSG_SENDER] = quest[Q_SCNDWIZ];
  } 
  else {
    text +=  
      break_string("Diese Nachricht wurde automatisch verschickt, wenn Du mit "
		   "dem 'r' Kommando darauf antwortest, geht die Antwort direkt an "+
		   capitalize(quest[Q_WIZ])+".", 78);
    mail[MSG_FROM] = quest[Q_WIZ];
    mail[MSG_SENDER] = quest[Q_WIZ];
  }
  
  
  mail[MSG_RECIPIENT] = getuid(player);
  mail[MSG_CC]=0;
  mail[MSG_BCC]=0;
  mail[MSG_SUBJECT]="Das Abenteuer: "+questname;
  mail[MSG_DATE]=dtime(time());
  mail[MSG_ID]="MorgenGrauen:"+time();
  mail[MSG_BODY]=text;
  
  "/secure/mailer"->DeliverMail(mail,0);
  return;
}   

static int compare (mixed *i, mixed *j)
{
  if (i[4] == j[4])
    return i[1] > j[1];
    else
    return i[4] > j[4];
}

varargs string liste(mixed pl)
{
  int qgroups, i, j, qrfw;
  mixed *qlists, *qgrouped, *qtmp;
  string str;
  string ja, nein, format, ueberschrift;

  if(!objectp(pl))
    if(stringp(pl))
      pl=find_player(pl) || find_netdead(pl);
  if(!objectp(pl))
    pl=PL;
  if(!objectp(pl))
    return "Ohne Spielernamen/Spielerobjekt gibt es auch keine Liste.\n";

  if ( ((string)pl->QueryProp(P_TTY)) == "ansi" 
      && !((int)pl->TestIgnore(({"farbe"}))) ) {
      ja = ANSI_GREEN + "Ja" + ANSI_NORMAL;
      nein = ANSI_RED + "Nein" + ANSI_NORMAL;
  }
  else {
      ja = "Ja";
      nein = "Nein";
  }

  str = "";
  // Festlegen des Ausgabeformates
  format = "%=-30s %3d %-6s %-9s %-5s %-12s %s\n";
  ueberschrift = sprintf("%-30s %3s %-6s %-9s %-5s %-12s %s\n",
		 "Abenteuer", "AP", "Klasse", "Attribut",
		 "Stufe", "Autor", "Geloest?");

  qgroups = sizeof(QGROUPS);
  qlists = allocate( qgroups+1 );
  for( i=qgroups; i>=0; i-- )
    qlists[i] = ({});

  qgrouped = QueryGroupedKeys();
  
  for (i=sizeof(qgrouped)-1;i>=0; i--)
    for (j=sizeof(qgrouped[i])-1;j>=0; j--) {
      qtmp = QueryQuest(qgrouped[i][j]);
      qlists[i] += ({ ({
	qgrouped[i][j],
        qtmp[Q_QP], QCLASS_STARS(qtmp[Q_CLASS]), 
	capitalize(QATTR_STRINGS[qtmp[Q_ATTR]]), 
	  qtmp[Q_DIFF], capitalize(qtmp[Q_WIZ]), 
	  pl->QueryQuest(qgrouped[i][j])?ja:nein
      }) });
    }

  for( i=0; i<qgroups; i++ )
  {
    if (sizeof(qlists[i])) {
      str += "\n" + ueberschrift; 
      str += sprintf("Stufen %d%s:\n", 
		     QGROUPS[i]+1, 
		     i==qgroups-1?"+":sprintf("-%d", QGROUPS[i+1]));
      qlists[i] = sort_array( qlists[i], "compare", ME );
      for( j=0; j<sizeof(qlists[i]); j++ ) {
	if(qlists[i][j][Q_DIFF]>=0)
	  str += sprintf( format,
			  qlists[i][j][0], qlists[i][j][1], qlists[i][j][2],
			  qlists[i][j][3], sprintf("%d",qlists[i][j][4]), 
			  qlists[i][j][5], qlists[i][j][6]);
      }
      str += "\n\n";
    }
  }
  
  qlists[qgroups] = sort_array(qlists[qgroups], "compare", ME);
  i = qgroups;
  if (sizeof(qlists[i])) {
    str += "\n" + ueberschrift;
    str += "Nur fuer Seher:\n";
    for( j=0; j<sizeof(qlists[qgroups]); j++ )  {
      if(qlists[i][j][Q_DIFF]==-1)
	str += sprintf( format,
			qlists[i][j][0], qlists[i][j][1], qlists[i][j][2],
			qlists[i][j][3], "S", qlists[i][j][5],
			qlists[i][j][6]);
    }
  }

  str += 
"\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"+
"AP       = Fuer die Quest vergebene Abenteuerpunkte\n"+
"Klasse   = Subjektive Einstufung innerhalb der Gruppe bzgl. Arbeitsaufwand,\n"+
"           Dauer oder Gefahren\n"+
"Attribut = Keines    -> Keine besonderen Charakteristika\n"+
"           Fleissig  -> Vornehmlich Sammeln, Auffinden von Items oder von\n"+
"                        Informationen\n"+
"           Heroisch  -> Heldentat, die Mut, Kampfbereitschaft und eine\n"+
"                        gewisse Bereitschaft zum Forschen braucht\n"+
"           Episch    -> Grosse Anforderungen an Ausdauer und Mut, grosser\n"+
"                        Aufwand fuer das Erforschen der Loesung, Kaempfe\n"+
"                        mittlerer Haerte\n"+
"           Legendaer -> Barden preisen die Einsatzbereitschaft der Helden,\n"+
"                        recht grosse Gefahr fuer Leib und Leben, mitunter\n"+
"                        lange Questzeit\n"+
"Stufe    = Empfohlene Stufe fuer die Quest, keine Voraussetzung, ausser bei\n"+
"           Seherquests, die teilweise den Status voraussetzen\n"+
"Autor    = Magier, der die Quest geschrieben hat\n"+
    "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";

  return str;
}


/* 
 * (2) ABSCHNITT "MINI" QUESTS
 */


int ClearUsersMQCache()
{
  if (!allowed())
    return 0;

  users_mq = ([]);

  return 1;
}

mixed QueryUsersMQCache()
{
  if (!allowed())
    return 0;

  return users_mq;
}


/* Beschreibung
 *
 * Die MiniQuests werden in einem Mapping gespeichert. 
 *
 * Der Key ist dabei der Name des Objektes, das die Quest im Spieler 
 * markieren darf.
 *
 * 1. Element ist die Zahl der durch diese Quest zu erwerbenden Stufenpunkte
 * 2. Element ist die Nummer unter der die MiniQuest gefuehrt wird.
 * 3. Element ist ein String, der die Quest kurz beschreibt. Dies ist optional.
 *    Wird keine Beschreibung gewuenscht, ist eine 0 einzutragen.
 * 4. Element ist ein Integer, 0 oder 1:
 *     0 : Quest ist fuer Spieler nicht sichtbar
 *     1 : Quest ist fuer Spieler z.B. bei einer Anschlagtafel sichtbar
 * 5. Element ist ein Integer, 0 oder 1:
 *     0 : Quest voruebergehend deaktiviert
 *     1 : Quest aktiviert
 */

// int DumpMiniQuests()
int DumpMiniQuests(object who)
{
  mixed keys;
  string *msg;
  int i, maxmq;

  if (!allowed())
    return 0;

  MQMLOG(sprintf("DumpMiniQuests: PO: %O, TI: %O", 
		 previous_object(),
//   this_interactive()));
     who));
		 
  rm(MQ_DUMP_FILE);
  
  keys=m_indices(miniquests);
  write_file(MQ_DUMP_FILE, "MINIQUESTS: ("+dtime(time())+")\n\n");
  msg = ({});
  
  for (i=sizeof(keys)-1; i>=0;i--) {
    msg += ({ sprintf("%4d %4d %s",
	       miniquests[keys[i]][1], miniquests[keys[i]][0],
	       keys[i]) });
    maxmq += miniquests[keys[i]][0];
  }

  write_file(MQ_DUMP_FILE, implode(sort_array(msg, #'> /*'*/), "\n"));

  write_file(MQ_DUMP_FILE, sprintf("\n\n"
             "============================================================\n"
	     +"MiniQuests: %d Miniquests mit %d Punkten.\n\n", 
              sizeof(keys), maxmq));

  return 1;
}

int AddMiniQuest(int mquestpoints, string allowedobj, mixed descr, 
		 int visible, int active)
{
  int nummer;
  
  if (!allowed()) 
    return 0;
  if (!stringp(allowedobj) || allowedobj=="")
    return -1;
  if (mquestpoints<1) 
    return -2;
  
  if (allowedobj[<2..] == ".c")
    allowedobj = allowedobj[0..<3];
  allowedobj = old_explode(allowedobj, "#")[0];
  allowedobj = (string)MASTER->_get_path(allowedobj,0);
  
  if (file_size(allowedobj+".c") <=0)
    return -3;

  if (miniquests[allowedobj])
    return -4;
  
  nummer = ++last_num;
  
  if (!stringp(descr))
    descr = 0;
  
  miniquests += 
    ([allowedobj:({mquestpoints, nummer, descr, visible, active})]);
  save_info();
  by_num += ([nummer : ({mquestpoints, allowedobj})]);
  
  MQMLOG(sprintf("AddMiniQuest: %s %O (%s)", allowedobj, miniquests[allowedobj],
		 getuid(this_interactive())));
 
  ClearUsersMQCache();
//call_out("DumpMiniQuests",0);
  call_out("DumpMiniQuests",0,this_interactive());
  return 1;
}

int RemoveMiniQuest(string name)
{
  if (!allowed())
    return 0;
  if (!miniquests[name])
    return -1;
  
  MQMLOG(sprintf("RemoveMiniQuest: %s %O (%s)", 
		 name, miniquests[name], getuid(this_interactive())));

  by_num = m_delete(by_num, miniquests[name][1]);
  miniquests = m_delete(miniquests, name);
  save_info();
  
  ClearUsersMQCache();
//call_out("DumpMiniQuests",0);
  call_out("DumpMiniQuests",0,this_interactive());
  return 1;
}

int ChangeMiniQuest(int mquestpoints, string allowedobj, mixed descr,
		    int visible, int active)
{
  int nummer;
  mixed altemq;
  
  if (!allowed())
    return 0;
  if (!stringp(allowedobj) || allowedobj == "")
    return -1;
  if (mquestpoints<1)
    return -2;
  
  if (allowedobj[<2..] == ".c")
    allowedobj = allowedobj[0..<3];
  allowedobj = old_explode(allowedobj, "#")[0];
  allowedobj = (string)"/secure/master"->_get_path(allowedobj,0);
  
  if (file_size(allowedobj+".c") <= 0)
    return -3;
  if (!miniquests[allowedobj])
    return -4;
  
  altemq = miniquests[allowedobj];
  nummer = miniquests[allowedobj][1];
  
  miniquests[allowedobj] = ({mquestpoints, nummer, descr, visible, active});
  by_num[nummer] = ({mquestpoints, allowedobj});
  save_info();
  
  MQMLOG(sprintf("ChangeMiniQuest: %s from %O to %O (%s)", allowedobj,
		 altemq, miniquests[allowedobj], 
		 getuid(this_interactive())));

  ClearUsersMQCache();
//call_out("DumpMiniQuests",0);
  call_out("DumpMiniQuests",0,this_interactive());
  return 1;
}

mixed QueryMiniQuestByName(string name)
{
  if (!allowed())
    return 0;
  
  if (extern_call())
    return ([ name:deep_copy(miniquests[name]) ]);

  return miniquests[name];
}

mixed QueryMiniQuestByNumber(int nummer)
{
  if (!allowed())
    return 0;
  
  if (extern_call())
    return ([by_num[nummer][1]:  
	     deep_copy(miniquests[by_num[nummer][1]]) ]);
  return miniquests[by_num[nummer][1]];
}

mixed QueryMiniQuests()
{
  if (!allowed())
    return 0;
  
  if (extern_call())
    return ({m_indices(miniquests), map(
	 m_values(miniquests), #'deep_copy /*'*/ )});
  
  return miniquests;
}

int SetMiniActive(string name, int flag)
{
  if (!allowed())
    return 0;
  if (!miniquests[name])
    return -1;
  if (flag < 0 || flag > 1)
    return -2;

  miniquests[name][4] = flag;
  save_info();
  
  MQMLOG(sprintf("%s: %s (%s)", (flag?"Activate":"Deactivate"),
		 name, getuid(this_interactive())));
  return 1;
}

int GiveMiniQuest(object winner)
{
  string name, mq;

  if (!winner || 
      (this_interactive() && (this_interactive() != winner)) ||
      ((this_player() == winner) && !query_once_interactive(winner)))
    return MQ_ILLEGAL_OBJ;
  if (winner->QueryGuest())
    return MQ_GUEST;
  if (!previous_object())
    return MQ_ILLEGAL_OBJ;

  name = load_name(previous_object());
  
  if (!miniquests[name])
    return MQ_KEY_INVALID;
  
  if (!miniquests[name][4])
    return MQ_IS_INACTIVE;

  mq = (MASTER->query_mq(getuid(winner)) || "");
  
  if (test_bit(mq, miniquests[name][1]))
    return MQ_ALREADY_SET;
  
  catch(mq = set_bit(mq, miniquests[name][1]);publish);
  MASTER->update_mq(getuid(winner), mq);
  
  MQSOLVEDLOG(sprintf("%s: %s, (#%d), (Stupse %d)",
	      name, geteuid(winner), 
	      miniquests[name][1], miniquests[name][0]));

  if (member(users_mq, getuid(winner)))
    users_mq = m_delete(users_mq, getuid(winner));

  return 1;
}

int QueryMiniQuestPoints(mixed pl)
{
  int i, j;
  string s, spieler;
  
  if (!allowed())
    return 0;
  
  if (!pl)
    return -1;

  if (!objectp(pl) && !stringp(pl))
    return -2;

  if (objectp(pl) && !query_once_interactive(pl))
    return -3;

  if (objectp(pl))
    spieler = getuid(pl);
  else
    spieler = pl;

  if (member(users_mq, spieler) == 0) {
    s = (MASTER->query_mq(spieler) || "");
    for (j=0, i=6*strlen(s)-1;i>0;i--) 
      if (test_bit(s,i))
	j+=by_num[i][0];
    users_mq += ([spieler:j]);
    return j;
  } else
    return users_mq[spieler];
}

int HasMiniQuest(mixed pl, mixed name)
{
  string mq, spieler;

  if (!pl || !name)
    return MQ_ILLEGAL_OBJ;
  
  if (!objectp(pl) && !stringp(pl))
    return MQ_ILLEGAL_OBJ;

  if (objectp(pl) && !query_once_interactive(pl))
    return MQ_ILLEGAL_OBJ;

  if (!objectp(name) && !stringp(name))
    return MQ_ILLEGAL_OBJ;

  if (objectp(name))
    name = old_explode(object_name(name), "#")[0];

  if (objectp(pl))
    spieler = getuid(pl);
  else 
    spieler = pl;
  
  if (!miniquests[name])
    return MQ_KEY_INVALID;

  mq = (MASTER->query_mq(spieler) || "");

  return test_bit(mq, miniquests[name][1]);
}


// Zum Von-Hand-Setzen der MiniQuests

int SetPlayerMiniQuest(string pl, string name)
{
  string mq;

  if(!allowed())
    return 0;
  if(!pl)
    return MQ_ILLEGAL_OBJ;
  if(!previous_object())
    return MQ_ILLEGAL_OBJ;

  if (!miniquests[name])
    return MQ_KEY_INVALID;

  mq = (MASTER->query_mq(pl) || "");

  if (test_bit(mq, miniquests[name][1]))
    return MQ_ALREADY_SET;

  catch (mq = set_bit(mq, miniquests[name][1]);publish);
  MASTER->update_mq(pl, mq);

  MQMLOG(sprintf("SetPlayerMiniQuest: %s %s (%s)", 
		 pl, name, getuid(this_interactive())));
  
  if (member(users_mq, pl))
    users_mq = m_delete(users_mq, pl);

  return 1;
}


int ClearPlayerMiniQuest(string pl, string name)
{
  string mq;
  
  if (!allowed())
    return 0;
  if (!pl)
    return MQ_ILLEGAL_OBJ;
  if (!previous_object())
    return MQ_ILLEGAL_OBJ;

  if (!miniquests[name])
    return MQ_KEY_INVALID;

  mq = (MASTER->query_mq(pl) || "");

  if (!test_bit(mq, miniquests[name][1]))
    return MQ_ALREADY_SET;

  catch (mq = clear_bit(mq, miniquests[name][1]);publish);
  MASTER->update_mq(pl, mq);

  MQMLOG(sprintf("ClearPlayerMiniQuest: %s %s (%s)",
		 pl, name, getuid(this_interactive())));

  if (member(users_mq, pl))
    users_mq = m_delete(users_mq, pl);

  return 1;
}
