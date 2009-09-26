// MorgenGrauen MUDlib
//
// explorationmaster.c -- Verwaltung der ExplorationPoints (FP)
//
// $Id: explorationmaster.c 7155 2009-02-25 21:20:50Z Zesstra $
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

#include <config.h>
#include <wizlevels.h>
#include <exploration.h>
#include <properties.h>
#include <new_skills.h>

#define DOMAIN_INFO 1

private mapping obs;
private mapping tipList;
private int cnt, avg;
private string alloc,bonus;

private static string vc_ob;
private static string output;

private static int changed;
private static int dumping;
private static mapping dumpMap, lastfound, querytime;

private static int stat_done;

nomask int addTip(mixed key,string tip);
nomask int changeTip(mixed key,string tip);
nomask int removeTip(mixed key);
nomask static string getTipFromList(mixed key);

void create()
{
  seteuid(getuid(this_object()));
  tipList=([]);
  if (!restore_object(EPSAVEFILE)) {
    obs = ([]);
    cnt = avg = 0;
    alloc = " ";
    bonus = " ";
    tipList=([]);
    save_object(EPSAVEFILE);
  }
  if (!bonus) bonus=" ";
  lastfound=([]);
  querytime=([]);
}

public nomask int query_prevent_shadow()
{
    return 1;
}

nomask int remove()
{
  save_object(EPSAVEFILE);
  destruct(this_object());
  return 1;
}

// Statistik erstellen
nomask void make_stat()
{
  if (stat_done)
    return;

  stat_done = 1;

  if (file_size(EPSTAT_INFO) <= 0 || file_size(MAKE_EPSTAT ".c") <= 0)
    return;

  // Falls das Skript noch roedelt...
  if (file_time(EPSTAT_INFO)+1800 > time())
    call_out("make_stat", 1800);
  else
    catch(load_object(MAKE_EPSTAT);publish);
  return;
}

void reset()
{
  if (changed && !dumping) {
    catch(rm(DUMPFILE);publish);
    dumping = 1;
    call_out("dumpEPObjects", 0, sort_array(m_indices(obs),#'> /*'*/));
  }
  if (time()%86400 < 4000)
    stat_done = 0;

  make_stat();

  changed = 0;
}

private static string strArr(string *s)
{
  string ret;
  int i;

  ret = ("\""+s[<1]+"\"");
  for (i=sizeof(s)-2; i>=0; i--)
    ret += (", \""+s[i]+"\"");

  return ret;
}

static void dumpEPObjects(string *doit)
{
  string *toGo, id;
  int i,j;

  if (!mappingp(dumpMap))
    dumpMap = ([]);

  toGo = 0;
  if (sizeof(doit) > 200) {
    toGo = doit[200..];
    doit = doit[0..199];
  }

  j = sizeof(doit);

  for (i=0; i<j; i++) {
    id = (string)__MASTER_OBJECT__->creator_file(doit[i]);
    if (member(dumpMap, id))
      dumpMap[id] += ({ doit[i] });
    else
      dumpMap += ([ id : ({ doit[i] }) ]);
  }
  if (toGo)
    call_out("dumpEPObjects", 1, toGo);
  else {
    int step;

    step = 0;
    id = "";
    toGo = sort_array(m_indices(dumpMap),#'> /*'*/ );
    for (i=0, j=sizeof(toGo); i<j; i++) {
      int k,l;
      doit = dumpMap[toGo[i]];
      id += sprintf("### %s %s\n", toGo[i], "#########################"[strlen(toGo[i])..]);
      for (k=0, l=sizeof(doit); k<l; k++) {
        id += sprintf("%s %4d %s %s.c ({ %s })\n",
                      EP_TYPES[obs[doit[k], MPOS_TYPE]],
                      obs[doit[k], MPOS_NUM],
                      test_bit(bonus,obs[doit[k], MPOS_NUM])?"b":"n",
                      doit[k],
                      strArr(obs[doit[k]]));
        if (!(++step % 50)) {
          write_file(DUMPFILE, id);
          id = "";
        }
      }
      id += "\n";
    }
    write_file(DUMPFILE,id);
    if (dumping == 2)
      write("Fertig! Anfrage bitte wiederholen.\n");
    dumping = 0;
    changed = 0;
    dumpMap = 0;
  }
}

private static string validOb(mixed ob)
{
  string fn, fpart;

  if (!objectp(ob))
    return 0;

  fn = old_explode(object_name(ob),"#")[0];
  fpart = old_explode(fn,"/")[<1];
 /*
  if (query_once_interactive(ob))
    return 0;

  if ((file_size(fn+".c") <= 0) &&
      this_player() &&
      (strstr(fpart, getuid(this_player())) >= 0))
    return 0;
 */
  return fn;
}

private static int allowed()
{
  if (previous_object() && geteuid(previous_object())==ROOTID)
    return 1;
  if ( !process_call() && previous_object() && this_interactive()
        && ARCH_SECURITY )
    return 1;

  return 0;
}

nomask varargs int AddEPObject(object ob, mixed keys, int type, int bonusflag)
{
  string fn;

  if (!allowed())
    return EPERR_NOT_ARCH;

  if (!(fn = validOb(ob)))
    return EPERR_INVALID_OB;

  if (stringp(keys))
    keys = ({ keys });

  if (member(obs, fn)) {
    if (type == obs[fn, MPOS_TYPE])
      obs[fn] = keys;
    else
      obs += ([ fn : keys; obs[fn, MPOS_NUM]; type ]);
    if (bonusflag) bonus = set_bit(bonus, obs[fn, MPOS_NUM]);
	else bonus = clear_bit(bonus, obs[fn, MPOS_NUM]);
  }
  else {
    int nr, i;

    nr = 0;
    while (test_bit(alloc,nr))
      nr++;

    obs += ([ fn : keys; nr; type ]);
    cnt++;
    alloc = set_bit(alloc,nr);
    if (bonusflag) bonus = set_bit(bonus,nr);
	else bonus = clear_bit(bonus,nr);
  }

  changed = 1;
  save_object(EPSAVEFILE);
  return cnt;
}

nomask int RemoveEPObject(object ob)
{
  string fn;

  if (!allowed())
    return EPERR_NOT_ARCH;

  if (!objectp(ob))
    return EPERR_INVALID_OB;

  fn = old_explode(object_name(ob),"#")[0];
  if (!member(obs,fn))
    return EPERR_NO_ENTRY;

  alloc = clear_bit(alloc, obs[fn, MPOS_NUM]);
  bonus = clear_bit(bonus, obs[fn, MPOS_NUM]);

  obs = m_delete(obs, fn);
  removeTip(fn);
  changed = 1;
  cnt--;
  save_object(EPSAVEFILE);
  return cnt;
}

nomask int ChangeEPObject(object ob, int what, mixed new)
{
  string fn, fn2,tmp;
  mapping entry;

  if (!allowed())
    return EPERR_NOT_ARCH;

  if (!(fn = validOb(ob)))
    return EPERR_INVALID_OB;

  if (!member(obs,fn))
    return EPERR_NO_ENTRY;

  switch(what) {
  case CHANGE_OB:
    if (!(fn2 = validOb(new)))
      return EPERR_INVALID_ARG;
    entry = ([ fn2: obs[fn]; obs[fn,MPOS_NUM]; obs[fn,MPOS_TYPE] ]);
    obs = m_delete(obs, fn);
    obs += entry;
    break;
  case CHANGE_BONUS:
    if (!(intp(new)))
      return EPERR_INVALID_ARG;
    if (new) bonus=set_bit(bonus,obs[fn,MPOS_NUM]);
	else bonus=clear_bit(bonus,obs[fn,MPOS_NUM]);
    break;
  case CHANGE_KEY:
    if (!stringp(new) && !pointerp(new))
      return EPERR_INVALID_ARG;
	
	tmp=getTipFromList(fn);
	removeTip(fn);
    if (stringp(new))
      new = ({ new });

    obs[fn] = new;
    if(tmp && tmp!="")
	    addTip(fn,tmp);
    break;
  case CHANGE_TYPE:
    if (!intp(new) || new < 0 || new > EP_MAX)
      return EPERR_INVALID_ARG;

    obs[fn, MPOS_TYPE] = new;
    break;
  default:
    return EPERR_INVALID_ARG;
  }
  changed = 1;
  save_object(EPSAVEFILE);
  return 1;
}

nomask mixed QueryEPObject(object ob)
{
  string fn;

  if (!allowed())
    return EPERR_NOT_ARCH;

  if (!(fn = validOb(ob)))
    return EPERR_INVALID_OB;

  if (!member(obs, fn))
    return 0;

  return ({ obs[fn], obs[fn,MPOS_NUM], obs[fn, MPOS_TYPE], test_bit(bonus,obs[fn, MPOS_NUM]) });
}

private static string getMatch(string m)
{
  string *res;
  int i;

  res = regexp(sort_array(m_indices(obs), #'> /*'*/), m);
  for (i=sizeof(res)-1; i>=0; i--)
    res[i] = sprintf("%s %s %s.c ({ %s })",
                     EP_TYPES[obs[res[i], MPOS_TYPE]],
                     test_bit(bonus,obs[res[i], MPOS_NUM])?"b":"n",
                     res[i],
                     strArr(obs[res[i]]));
  return implode(res, "\n");
}

private static string getMatchArch(string m)
{
  string *res;
  int i;

  res = regexp(sort_array(m_indices(obs), #'> /*'*/), m);
  for (i=sizeof(res)-1; i>=0; i--)
    res[i] = sprintf("%s %4d %s %s.c ({ %s })",
                     EP_TYPES[obs[res[i], MPOS_TYPE]],
                     obs[res[i], MPOS_NUM],
                     test_bit(bonus,obs[res[i], MPOS_NUM])?"b":"n",
                     res[i],
                     strArr(obs[res[i]]));
  return implode(res, "\n");
}

/*
 *  Anleitung fuer >=EMs:
 *    ShowEPObjects() zeigt das gesamte Dumpfile an
 *    ShowEPObjects("dump") erzeugt ein neues Dumpfile
 *    ShowEPObjects("string") liefert alle EPs, die "/string/"
 *                            im Filenamen enthalten
 *    ShowEPObjects("string1","string2") liefert alle EPs, die
 *                            "/string1/string2/" im Filenamen enthalten
 *  Anleitung fuer RMs:
 *    ShowEPObjects() liefert all Deine EPs
 *    ShowEPObjects("domainname") liefert Deine EPs in Deiner Domanin
 *    ShowEPObjects("domainname","magiername") liefert die EPs des
 *                                Mitarbeiters in Deiner Region
 *
 *  Anleitung fuer <RMs:
 *    ShowEPObjects() liefert Deine EPs
 *
 *  Alle EPObjects einer ganzen Domain kann man nicht mehr auf einmal
 *  ziehen und sollte man auch nicht. Alle Zugriffe auf diese Funktion
 *  werden geloggt. Auch die der EMs!
 *
 *                                             Rikus
 */

nomask varargs void ShowEPObjects(string what, string magname)
{
  if (allowed()) {
    if (what == "dump") {
      if (!dumping) {
        dumping = 2;
        catch(rm(DUMPFILE);publish);
        call_out("dumpEPObjects", 0, sort_array(m_indices(obs),#'>/*'*/));
      }
      printf("Liste wird erstellt und in '%s' abgelegt!\n", DUMPFILE);
      return;
    }
    if (!what || what == "") {
      this_interactive()->More(DUMPFILE, 1);
      log_file("ARCH/EPZugriffe", ctime(time())+": "+
        capitalize(getuid(this_interactive()))+" schaute sich das DUMPFILE an.\n");
      return;
    }
    what="/"+what+"/";
    if (magname) what+=magname+"/";
  }
  else
#ifdef DOMAIN_INFO
    if (IS_LORD(this_interactive())) {
      if (!what || what == "")
	what = "/"+getuid(this_interactive())+"/";
      else {
	if (!__MASTER_OBJECT__->domain_master(getuid(this_interactive()), what)) {
	  write("Sorry, Du kannst nur Objekte in Deiner eigenen Region abfragen!\n");
	  return;
	}
	if (!magname || magname=="")
	  magname = getuid(this_interactive());
//        if (!__MASTER_OBJECT__->domain_member(magname, what)) {
//         write(capitalize(magname)+" ist gar kein Mitarbeiter in Deiner Region!\n");
//          return;
//	}
	what = "/d/"+what+"/"+magname+"/";
      }
    }
    else
#endif
      {
	if (!what || what == "")
	  what = getuid(this_interactive());
	else if (what != getuid(this_interactive())) {
	  write("Sorry, Du kannst nur Deine eigenen Objekte abfragen!\n");
	  return;
	}
	what="/"+what+"/";
      }
  if (allowed())
    this_interactive()->More(getMatchArch(what));
  else
    this_interactive()->More(getMatch(what));
  log_file("ARCH/EPZugriffe", ctime(time())+": "+
    capitalize(getuid(this_interactive()))+" schaute sich "+what+" an.\n");
  return;
}

nomask void PrepareVCQuery(string file)
{
  string path, *parts;

  vc_ob = 0;

  if (!previous_object() || !stringp(file))
    return;

  parts = explode(object_name(previous_object()),"/");

  if (parts[<1] == "virtual_compiler") {
    path = implode(parts[0..<2]+({ file }), "/");
    if (file_size(path+".c") < 0)
      vc_ob = path;
  }
}

nomask mixed *QueryExplore()
{
  string fn;

  if (!previous_object())
    return 0;

  if (!member(obs, fn = old_explode(object_name(previous_object()),"#")[0]))
    if (!vc_ob || !member(obs, fn = vc_ob)) {
      vc_ob = 0;
      return 0;
    }

  vc_ob = 0;
  return ({ obs[fn, MPOS_TYPE], obs[fn] });
}

nomask int QueryMaxEP()
{
  return (cnt||1);
}

nomask int QueryAverage()
{
  return (avg||1);
}

nomask int SetAverage(int x)
{
  if (getuid(previous_object()) == ROOTID) {
    avg = x;
    save_object(EPSAVEFILE);
  }
  return avg;
}

static int check_arch(object u)
{
   return query_wiz_level(u)>=ARCH_LVL;
}

private static int check_to_fast(string name, string fn, int gesetzt)
{
    if (gesetzt) return 1; // Rikus, sonst arger scroll :)

    log_file(FP_LOG,sprintf("%s : %s : %s\n",name,fn,dtime(time())),500000);

    if ( !member( lastfound, name ) )
        lastfound += ([ name: time(); 1; ({ fn + "#*#" + dtime(time()) + "#*#" +gesetzt }) ]);
    else if ( time() <= lastfound[name, 0] + LF_TIME ){
        lastfound[name, 1]++;
        lastfound[name, 2] = ({ fn + "#*#" + dtime(time()) + "#*#" +gesetzt })
            + lastfound[name,2];
    }
    else {
        lastfound[name, 1] = 1;
        lastfound[name, 2] = ({ fn + "#*#" + dtime(time()) + "#*#" +gesetzt });
    }

    lastfound[name, 0] = time();

    if ( lastfound[name, 1] >= LF_WARN ){
        object *u;
        int i;
        string *tmp;

//        u = filter( users(), "check_arch" );
//        map( u, #'tell_object/*'*/, "**** FP-Liste/Script " +
//                   capitalize(name) + " (" + dtime(time()) + ") ****\n" );

        for ( i = sizeof(lastfound[name, 2]); i--; ){
            tmp = explode( lastfound[name, 2][i], "#*#" );
            log_file( LF_LOG, sprintf( "%s : %s : %s : %s\n",
                                         tmp[1], name, tmp[2], tmp[0] ), 500000 );
        }

        lastfound[name, 2] = ({});
    }
    return 1;
}

nomask int GiveExplorationPoint(string key)
{
  string fn;
  string ep;
  int gesetzt;

  if (!previous_object() || !this_interactive() || !this_player() ||
       this_player() != this_interactive() ||
       this_player()->QueryProp(P_KILLS) ||
       this_player()->QueryGuest()       )
    return 0;

  fn = old_explode(object_name(previous_object()), "#")[0];

  if (!member(obs, fn))
    return 0;

  if (member(obs[fn],key) < 0)
    return 0;

  ep = (MASTER->query_ep(getuid(this_interactive())) || "");

  gesetzt=test_bit(ep,obs[fn,1]);
  check_to_fast(getuid(this_player()),fn,gesetzt);
  if (gesetzt) return 0;
  
  catch(ep = set_bit(ep, obs[fn,1]));

  return (int)MASTER->update_ep(getuid(this_interactive()),ep);
}

nomask int GiveExplorationPointObject(string key, object ob)
{
  string fn;
  string ep;
  int gesetzt;
  
  if (!objectp(ob) || ob->QueryProp(P_KILLS ))
    return 0;

  fn = old_explode(object_name(previous_object()), "#")[0];

  if (!member(obs, fn))
    return 0;

  if (member(obs[fn],key) < 0)
    return 0;

  ep = (MASTER->query_ep(getuid(ob)) || "");

  gesetzt=test_bit(ep,obs[fn,1]);
  check_to_fast(getuid(this_player()),fn,gesetzt);
  if (gesetzt) return 0;

  catch(ep = set_bit(ep, obs[fn,1]));

  return (int)MASTER->update_ep(getuid(ob),ep);
}


private static int QueryRealExplorationPoints(string pl)
{
  return count_bits(MASTER->query_ep(pl) || " ");
}

nomask int QueryExplorationPoints(mixed pl)
{
  mixed val;
  int ep;

  if (!stringp(pl)) pl=getuid(pl);
  ep=QueryRealExplorationPoints(pl);

  if (allowed() || !ep) return ep;

  val=querytime[pl];

  if (!pointerp(val) || sizeof(val)<2)
    val=({0,time()});

  if (time()>=val[1]) {
    val = ({ ep + random(6)-3, time()+300+random(300) });
    if (val[0]<0) val[0]=0;
    querytime+=([pl:val]);
  }
  return val[0];
}

private static int remove_fp(int num, string pl)
{
  int i,j,k,t,maxEP;
  string ep;
  ep = (MASTER->query_ep(pl) || "");

  maxEP = QueryMaxEP();
  for( i=0; i<num; i++)
  {
    t = random(maxEP);
    for( j=0; j<maxEP; j++ ) {
      if( test_bit(ep, k=(t+j)%maxEP ) ) break;
    }
    if( j==maxEP ) break;
    ep = clear_bit(ep, k);
  }
  MASTER->update_ep(pl,ep);
  return i;
}

/* */

// quoted from /sys/mail.h
#define MSG_FROM 0
#define MSG_SENDER 1
#define MSG_RECIPIENT 2
#define MSG_CC 3
#define MSG_BCC 4
#define MSG_SUBJECT 5
#define MSG_DATE 6
#define MSG_ID 7
#define MSG_BODY 8

nomask int RemoveFP(int num, string pl, string grund)
{
  int i;
  string text;
  mixed* mail;

  if (!allowed()) return -1;
  if( num<0 ) return -3;
  if (!grund || grund=="") grund="<unbekannt>";
  if (!pl) return -2;
  i=remove_fp(num, pl);
  log_file("ARCH/fp_strafen", ctime(time())+": "
    +this_interactive()->Name(WER)+" loescht "+pl+" "+i
    +" FPs\nGrund:"+grund+"\n");
  if( i>0 ) {
     text =
     "Hallo "+capitalize(pl)+",\n\n"+
     break_string(
      this_interactive()->Name(WER)+" hat soeben veranlasst, dass Dir "+i
      +" FPs abgezogen wurden.\nGrund:"+grund+"\n", 78 );

     mail = allocate(9);
     mail[MSG_FROM] = getuid(this_interactive());
     mail[MSG_SENDER] = "MorgenGrauen";
     mail[MSG_RECIPIENT] = pl;
     mail[MSG_CC]=0;
     mail[MSG_BCC]=0;
     mail[MSG_SUBJECT]="FP-Reduktion";
     mail[MSG_DATE]=dtime(time());
     mail[MSG_ID]="MorgenGrauen:"+time();
     mail[MSG_BODY]=text;

     "/secure/mailer"->DeliverMail(mail,1);
  }
  return i;
}
/* */

private static int add_fp(int num, string pl)
{
  int i,j,k,t,maxEP;
  string ep;
  ep = (MASTER->query_ep(pl) || "");

  maxEP = QueryMaxEP();
  for( i=0; i<num; i++)
  {
    t = random(maxEP);
    for( j=0; j<maxEP; j++ ) {
      if( !test_bit(ep, k=(t+j)%maxEP ) ) break;
    }
    if( j==maxEP ) break;
    ep = set_bit(ep, k);
  }
  MASTER->update_ep(pl,ep);
  return i;
}

nomask int AddFP(int num, string pl)
{
  int i;
  if (!allowed()) return -1;
  if ( num<0 ) return -3;
  if (!pl) return -2;
  i=add_fp(num, pl);
  log_file("ARCH/fp_strafen", ctime(time())+": "
    +this_interactive()->Name(WER)+" gibt "+pl+" "+i
    +" FPs\n");

  return i;
}

nomask int SetFP(int num, string pl)
{
  int maxEP;
  string ep;
  if (!allowed()) return -1;
  if ( num<0 ) return -3;
  if (!pl) return -2;
  ep = (MASTER->query_ep(pl) || "");

  maxEP = QueryMaxEP();
  if (num<0 || num>=maxEP) return -4;
  ep = set_bit(ep, num);
  MASTER->update_ep(pl,ep);
  return num;
}

nomask int ClearFP(int num, string pl)
{
  int maxEP;
  string ep;
  if (!allowed()) return -1;
  if ( num<0 ) return -3;
  if (!pl) return -2;
  ep = (MASTER->query_ep(pl) || "");

  maxEP = QueryMaxEP();
  if (num<0 || num>=maxEP) return -4;
  ep = clear_bit(ep, num);
  MASTER->update_ep(pl,ep);
  return num;
}

private static void printep( int nr, string key, int kind, string* det )
{
  output+=sprintf("%4d %s %s.c %s ({ %s })\n",nr,test_bit(bonus,nr)?"b":"n",key,EP_TYPES[kind],
		  strArr(det));
}

nomask varargs int ShowPlayerEPs(string pl,string pattern)
{
  string ep,teststring;
  if (!allowed()) return -1;
  ep = (MASTER->query_ep(pl) || "");

  output="";
  if (!pattern || pattern=="")
    teststring="%s";
  else teststring="%s"+pattern+"%s";
  walk_mapping( obs, lambda( ({ 'key, 'v1, 'v2, 'v3 }),
			     // v1 -- details, v2 -- Nummer, v3 -- art
			     // key -- Filename
			     ({ #'if, ({ #'test_bit, ep, 'v2 }),
			     ({#'if,({#'sscanf,'key,teststring,'v4,'v5}),
			     ({ #'printep, 'v2, 'key, 'v3, 'v1 })
			     }) }) ));
  this_interactive()->More(output);
  return 1;
}

// Hier kommen Funktionen fuer die Levelpunkte
// Funktion fuer Levelpunkte steht im LEPMASTER!

nomask int QueryLEP(int lep) {
    raise_error("Bitte QueryLEP() im LEPMASTER abfragen, nicht im "
	"EPMASTER!");
    return(-1); //never reached
}

string QueryForschung()
{
  int max, my, avg;
  string ret;

  if ((my=QueryRealExplorationPoints(getuid(previous_object()))) < MIN_EP)
    return "Du kennst Dich im MorgenGrauen so gut wie gar nicht aus.\n";

  my *= 100;
  max = my/QueryMaxEP();
  avg = my/QueryAverage();

  ret = "Verglichen mit Deinen Mitspielern, kennst Du Dich im MorgenGrauen ";
  switch(avg) {
  case 0..10:
    ret += "kaum";
    break;
  case 11..40:
    ret += "aeusserst schlecht";
    break;
  case 41..56:
    ret += "sehr schlecht";
    break;
  case 57..72:
    ret += "schlecht";
    break;
  case 73..93:
    ret += "unterdurchschnittlich";
    break;
  case 94..109:
    ret += "durchschnittlich gut";
    break;
  case 110..125:
    ret += "besser als der Durchschnitt";
    break;
  case 126..145:
    ret += "recht gut";
    break;
  case 146..170:
    ret += "ziemlich gut";
    break;
  case 171..210:
    ret += "gut";
    break;
  case 211..300:
    ret += "sehr gut";
    break;
  case 301..400:
    ret += "ausserordentlich gut";
    break;
  case 401..500:
    ret += "unheimlich gut";
    break;
  default:
    ret += "einfach hervorragend";
    break;
  }
  ret += " aus.\nAbsolut gesehen ";

  switch(max) {
  case 0..5:
    ret += "kennst Du nur wenig vom MorgenGrauen.";
    break;
  case 6..10:
    ret += "solltest Du Dich vielleicht noch genauer umsehen.";
    break;
  case 11..17:
    ret += "bist Du durchaus schon herumgekommen.";
    break;
  case 18..25:
    ret += "hast Du schon einiges gesehen.";
    break;
  case 26..35:
    ret += "bist Du schon weit herumgekommen.";
    break;
  case 36..50:
    ret += "koenntest Du eigentlich einen Reisefuehrer herausbringen.";
    break;
  case 51..75:
    ret += "hast Du schon sehr viel gesehen.";
    break;
  default:
    ret += "besitzt Du eine hervorragende Ortskenntnis.";
  }
  return break_string(ret, 78, 0, 1);
}

nomask status hasFP(object player,string key)
{
  string ep;
  
  if(!allowed() || !player || !query_once_interactive(player) || !key || !member(obs,key)){
	return -1;
  }
  
  ep = (MASTER->query_ep(getuid(player)) || "");

  return test_bit(ep,obs[key,1]);
	
}

nomask mapping getFreeFPsForPlayer(object player)
{
	mapping freeFPs;
	string* indices;
	int i;
	
	freeFPs=([]);
	if(!allowed() || !player || !query_once_interactive(player)){
		return freeFPs;
	}
	
	freeFPs=copy(obs);
	indices=m_indices(freeFPs);
	i=sizeof(indices)-1;
	for(i;i>=0;i--){
		if(hasFP(player,indices[i])){
			efun::m_delete(freeFPs,indices[i]);
		}
	}
	return freeFPs;
}

nomask int addTip(mixed key,string tip)
{
  string fn;
  
  if (!allowed())
    return EPERR_NOT_ARCH;

  if (!tip || (!objectp(key) && !stringp(key)))
    return EPERR_INVALID_ARG;

  if (objectp(key))
    fn=old_explode(object_name(key),"#")[0];
  else
    fn=old_explode(key,"#")[0];

  if (!member(obs, fn)) return EPERR_INVALID_ARG;
  tipList+=([fn:tip]);
  save_object(EPSAVEFILE);
    
  return 1;
}

nomask int changeTip(mixed key,string tip)
{
	return addTip(key,tip);
}

nomask int removeTip(mixed key)
{
  string fn;
  
  if (!allowed())
    return EPERR_NOT_ARCH;
  
  if ((!objectp(key) && !stringp(key)))
    return EPERR_INVALID_ARG;

  if (objectp(key))
    fn=old_explode(object_name(key),"#")[0];
  else
    fn=old_explode(key,"#")[0];
  
  if (!member(tipList, fn)) return EPERR_INVALID_ARG;
    
  efun::m_delete(tipList,fn);
  save_object(EPSAVEFILE);
    
  return 1;  
}

nomask static string getTipFromList(mixed key)
{
  string fn;
  
  if (!allowed())
    return "";
  
  if ((!objectp(key) && !stringp(key)))
    return "";

  if (objectp(key))
    fn=old_explode(object_name(key),"#")[0];
  else
    fn=old_explode(key,"#")[0];
  
  if (!member(tipList, fn)) return "";
        
  return tipList[fn];  
}

// return valid tips from database or existing 
nomask string getTip(mixed key)
{
  string fn;
  string tip;
  string* path;
  
  if (!allowed())
    return "";
  
  if ((!objectp(key) && !stringp(key)))
    return "";

  if (objectp(key))
    fn=old_explode(object_name(key),"#")[0];
  else
    fn=old_explode(key,"#")[0];
  
  if(!member(obs,fn)) return "";
  
  tip=getTipFromList(fn);
  if(!tip || tip==""){
  	path=old_explode(fn,"/");
  	if(sizeof(path)<3) return "";
  	if(path[0]=="players") return "Schau Dich doch mal bei "+capitalize(path[1])+" um.";
	
	if(path[0]=="d"){
		tip+="Schau Dich doch mal ";
		
		if(file_size("/players/"+path[2])==-2){
			tip+="bei "+capitalize(path[2]+" ");
		}
		
		if(path[1]=="anfaenger")
			tip+="in den Anfaengergebieten ";
		if(path[1]=="fernwest")
			tip+="in Fernwest ";
		if(path[1]=="dschungel")
			tip+="im Dschungel ";
		if(path[1]=="schattenwelt")
			tip+="in der Welt der Schatten ";
		if(path[1]=="unterwelt")
			tip+="in der Unterwelt ";
		if(path[1]=="gebirge")
			tip+="im Gebirge ";
		if(path[1]=="seher")
			tip+="bei den Sehergebieten ";
		if(path[1]=="vland")
			tip+="auf dem Verlorenen Land ";
		if(path[1]=="ebene")
			tip+="in der Ebene ";
		if(path[1]=="inseln")
			tip+="auf den Inseln ";
		if(path[1]=="wald")
			tip+="im Wald ";
		if(path[1]=="erzmagier")
			tip+="bei den Erzmagiern ";
		if(path[1]=="polar")
			tip+="im eisigen Polar ";
		if(path[1]=="wueste")
			tip+="in der Wueste ";
		tip+="um.";
	}
  }
  return tip;
}

nomask static string* makeTiplistFromBitString(string bitstr)
{
	string* ret;
	string* keys;
	int* nums;
	int i,tmp;
	string key;
	
	keys=m_indices(obs);
	nums=({});
	for(i=0;i<sizeof(keys);i++){
		nums+=({obs[keys[i],MPOS_NUM]});
	}
	
	ret=({});
    for (i=6*strlen(bitstr)-1;i>0;i--){
      if (test_bit(bitstr,i)){
      	key=0;
      	tmp=member(nums,i);
      	if(tmp!=-1){
      		key=keys[tmp];
      	}
		if(key) ret+=({key});
      }
    }
	
	return ret;
}

nomask string allTipsForPlayer(object player)
{
	string ret,tipstr,tmp;
	string* tips;
	int i;
	
	ret="";
	
	if(!player || !this_interactive() 
	    || (this_interactive()!=player && !IS_ARCH(this_interactive())) )
		return "";		
	
	tipstr=(string)MASTER->query_fptips(getuid(player) || "");
	tips=makeTiplistFromBitString(tipstr);
		
	for(i=0;i<sizeof(tips);i++){
		tmp=getTip(tips[i]);
		if(tmp && tmp!="") ret+=tmp+"\n";
	}
	
	return ret;
}

nomask status playerMayGetTip(object player)
{
	int numElegible;
	int numReceived;
	int lvl;
	int i;
	string tips;
	
    if(!allowed() || !player || !query_once_interactive(player))
  	    return 0;

	if(!player || !query_once_interactive(player))
		return 0;
	lvl=(int)player->QueryProp(P_LEVEL);
	numElegible=0;
	i=sizeof(FPTIPS_LEVEL_LIMITS)-1;

	if(lvl>FPTIPS_LEVEL_LIMITS[i])
		numElegible+=(lvl-FPTIPS_LEVEL_LIMITS[i]);

	for(i;i>=0;i--){
		if(lvl>=FPTIPS_LEVEL_LIMITS[i]) numElegible++;
	}
	
	tips=MASTER->query_fptips(getuid(player)) || ""; 
	numReceived=count_bits(tips);

	return numElegible>numReceived;
}

nomask string giveTipForPlayer(object player)
{
  string* tmp;
  mapping free;
  string* tips;
  string tip,pl,fptip;
  int i,index;
  
  if(!allowed() || !player || !query_once_interactive(player) || !playerMayGetTip(player))
  	return "";
  
  pl=getuid(player);
  free=getFreeFPsForPlayer(player);
  if(!free || sizeof(free)==0)
  	return "";

  tmp=m_indices(free);
  fptip=MASTER->query_fptips(pl) || "";
  tips=makeTiplistFromBitString(fptip);
  if(tips==0)
  	tips=({});
  tmp-=tips;
  if(sizeof(tmp)==0)
  	return "";

  i=FPTIPS_MAX_RETRY;
  while(i>0){
  	i--;
	index=random(sizeof(tmp));
	tip=getTip(tmp[index]);
	if(tip!=""){
		i=0;
		fptip=set_bit(fptip,obs[tmp[index],MPOS_NUM]);
		MASTER->update_fptips(pl,fptip);
	}
	tmp-=({tmp[index]});
  }
  tips=makeTiplistFromBitString(fptip);
  return tip;
  
}

// Nicht jeder Magier muss den EPMASTER entsorgen koennen.
string NotifyDestruct(object caller) {
    if( (caller!=this_object() && !ARCH_SECURITY) || process_call() ) {
      return "Du darfst den Exploration Master nicht zerstoeren!\n";
    }
}

/* */
