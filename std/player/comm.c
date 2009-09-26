// MorgenGrauen MUDlib
//
// player/comm.c-- basic player communiction commands
//
// $Id: comm.c 7256 2009-08-10 19:21:04Z Zesstra $

#pragma strong_types
#pragma save_types
#pragma no_clone
#pragma pedantic
//#pragma range_check

inherit "/std/player/channel";
inherit "/std/player/comm_structs";

#include <input_to.h>

//#define NEED_PROTOTYPES
#include <sys_debug.h>

#include <thing/properties.h>
#include <player/comm.h>
#include <player/base.h>

#include <properties.h>
#include <config.h>
#include <ansi.h>
#include <wizlevels.h>
#include <language.h>
#include <udp.h>
#include <defines.h>
#include <daemon.h>
#include <strings.h>

#define TELLHIST_DISABLED   0
#define TELLHIST_NO_MESSAGE 1
#define TELLHIST_ENABLED    2
#define TELLHIST_LONGLIFE   3

#define ECHO_COST 50
#define ERWIDER_PARAM ","

#define ZDEBUG(x)  if (find_player("zesstra")\
                       && find_player("zesstra")->QueryProp("zdebug")>3)\
                     tell_object(find_player("zesstra"),"CommDBG: "+x+"\n")

private int tell_history_enabled = TELLHIST_NO_MESSAGE;
private nosave mapping tell_history=([]);
private nosave string *commreceivers = ({});
private nosave string last_comm_partner;
private nosave int last_beep_time;

varargs string name(int casus, int demonst);

//local property prototypes
static int _query_intermud();
// erzeugt sortierte Liste an Kommunikationspartnern
private string *sorted_commpartners(int reversed);

private static string *buffer = ({});

void create()
{
  ::create();
  Set(P_EARMUFFS, 0);
  Set(P_EARMUFFS, SAVE, F_MODE);
  Set(P_EARMUFFS, SECURED, F_MODE);
  Set(P_INTERMUD, SAVE, F_MODE);
  Set(P_IGNORE, ({}), F_VALUE);
  Set(P_IGNORE, SAVE, F_MODE);
  Set(P_BUFFER, SAVE, F_MODE);
  Set(P_MESSAGE_PREPEND, SAVE, F_MODE_AS);
  Set(P_MESSAGE_BEEP, SAVE, F_MODE_AS);
}

void create_super()
{
  set_next_reset(-1);
}

private string permutate(string msg) {
  object ob;
  if (!objectp(ob=QueryProp(P_PERM_STRING)))
    return msg;

  return (string)ob->permutate_string(msg)||"";
}

varargs int _flush_cache(string arg)
{
  switch(arg)
  {
    case "ein":
      SetProp(P_BUFFER, 1);
      printf("Der Kobold merkt sich jetzt alles!\n"); break;
    case "aus":
      SetProp(P_BUFFER, 0);
      printf("Der Kobold wird Dich nicht stoeren!\n"); break;
    default: if(arg) printf("Der Kobold sagt: kobold ein oder kobold aus\n");
  }
  if(sizeof(buffer))
  {
    tell_object(this_object(),
    sprintf("Ein kleiner Kobold teilt Dir folgendes mit:\n%s",
      implode(buffer, "")));
    buffer = ({});
  }
  return 1;
}

public int TestIgnore(string *arg)
{   string *ignore;

    if (!pointerp(arg) || !pointerp(ignore=QueryProp(P_IGNORE)))
        return 0;
    return sizeof(arg&ignore);
}

private int check_ignore(mixed ignore, string verb, string name)
{
  return (ignore == verb) ||
    ((sizeof(ignore = explode(ignore, ".")) > 1) &&
     (name == ignore[0] && member(ignore[1..], verb) != -1));
}

private int comm_beep() {
  if (QueryProp(P_VISUALBELL)) return 0; // kein ton
  int beep_interval=(int)QueryProp(P_MESSAGE_BEEP);
  if (!beep_interval || ((time()-last_beep_time) < beep_interval)) return 0;
  last_beep_time=time();
  return 1;
}

private varargs void add_to_tell_history( string uid, int sent, int recv,
                                 string message, string indent, int flags )
{
  /* tell_history ist ein Mapping mit UIDs der Gespraechspartner als Key.
     Als Wert ist eine Strukur vom Typ chat_s eingetragen.
     Strukturen chat_s und msg_s sind in /std/player/comm_structs.c definiert.
     TODO fuer spaeter, gerade keine Zeit fuer:
     Als Wert ist ein Array von chat_s enthalten, wobei das 0. Element das
     jeweils juengste Gespraech mit diesem Gespraechspartner ist und alle
     weiteren Elemente in der zeitlichen Reihenfolge kommen (also letztes
     Element ist aeltestes Gespraech).
     */

  // letzten Gespraechspartner fuer erwidere.
  if (!(flags & MSGFLAG_REMOTE))
    last_comm_partner = uid;

  // ist ein sortiertes Array von max. MAX_SAVED_CHATS Groesse, welches die
  // Spieler enthaelt, denen man schon was mitgeteilt hat. Aktuellste am
  // Anfang.
  if (sent) {
    if (!sizeof(commreceivers))
      commreceivers = ({uid});
    else if (commreceivers[0] != uid) {
      // nur wenn der aktuelle Partner nicht am Anfang steht, muss man hier was
      // tun. Comm-Partner an den Anfang stellen und ggf. alten Eintrag
      // entfernen.
      // TODO: Effizienter gestalten.
      commreceivers = ({uid}) + (commreceivers-({uid}));
      // ggf. kuerzen. (wenn !tell_history_enabled, wird es ggf. unten
      // gemacht, denn die Hist muss min. alle UID enthalten, die auch in
      // commreceivers drin sind.)
      if (!tell_history_enabled && sizeof(commreceivers) > MAX_SAVED_CHATS)
        commreceivers = commreceivers[0..MAX_SAVED_CHATS];
    }
  }

  if (!tell_history_enabled)
    return;

  if (!indent && message[<1] == 10)
      message = message[..<2];

  struct chat_s chat;
  // Gespraechspartner unbekannt?
  if (!member(tell_history, uid)) {
    // zuviele Gespraeche in Hist? >= ist Absicht weil ja gleich noch eins
    // dazu kommt.
    if (sizeof(tell_history) >= MAX_SAVED_CHATS) {
      string deluid;
      int zeit = __INT_MAX__;
      foreach(string tuid, chat : tell_history) {
        // aeltestes Gespraech suchen
        if (zeit > chat->time_last_msg) {
          deluid = tuid;
          zeit = chat->time_last_msg;
        }
      }
      // aeltestes Gespraech raus.
      efun::m_delete(tell_history, deluid);
      if (member(commreceivers,deluid)>-1)
        commreceivers-=({deluid});
    }
    // neues Gespraech anlegen
    chat = (<chat_s> uid: uid, time_first_msg: time(), 
               time_last_msg: time(),
         sentcount: sent, recvcount: recv,
         msgbuf: 0, ptr: 0 );
    tell_history[uid] = chat;
  }
  else {
    // Gespraechspartner bekannt, altes Gespraech weiterbenutzen
    chat = tell_history[uid];
    chat->time_last_msg = time();
    chat->sentcount += sent;
    chat->recvcount += recv;
  }

  if (tell_history_enabled < TELLHIST_ENABLED)
    return;

  // ggf. Array fuer Messages anlegen
  if (!pointerp(chat->msgbuf))
    chat->msgbuf = allocate(MAX_SAVED_MESSAGES);

  // Message-Struktur ermitteln oder neu anlegen
  struct msg_s msg;
  if (!structp(chat->msgbuf[chat->ptr])) {
    // neue Struct ins Array schreiben
    chat->msgbuf[chat->ptr] = msg = (<msg_s>);
  }
  else {
    // alte Struct ueberschreiben
    msg = chat->msgbuf[chat->ptr];
  }
  // Index auf naechste Messagestruktur ermitteln
  chat->ptr = (chat->ptr + 1) % MAX_SAVED_MESSAGES;
  // Message speichern
  msg->msg = message;
  msg->indent = indent;
  msg->timestamp = time();
}

protected void clear_tell_history()
{
  /* Nach einem "schlafe ein" werden die gespeicherten Mitteilungen geloescht,
     sofern der Spieler nichts abweichendes eingestellt hat. */

#ifdef TELLHIST_LONGLIFE
  if (tell_history_enabled == TELLHIST_LONGLIFE)
    return;
#endif

  foreach (string uid, struct chat_s chat: tell_history)
    if (pointerp(chat->msgbuf)) {
      chat->msgbuf = 0;
      chat->ptr = 0;
    }
}

protected void reset(void)
{
  /* Wird 15 Minuten nach dem Verlust der Verbindung aufgerufen. Falls der
     Spieler nicht inzwischen eine Verbindung wiederhergestellt hat, werden
     wie bei einem "schlafe ein" die Mitteilungen geloescht. */

  if (!interactive())
    clear_tell_history();

}

private varargs int _send(object ob, string message, int flag, string indent )
{
  mixed res;
  if(!call_resolved(&res, ob, "Message", message, flag, indent))
    tell_object(ob, break_string(message, 78, indent,
      ob->QueryProp(P_MESSAGE_PREPEND) ? BS_PREPEND_INDENT : 0));
  return res;
}

protected varargs int _recv(object ob, string message, int flag, string indent)
{
  write(break_string(message, 78, indent,
        QueryProp(P_MESSAGE_PREPEND) ? BS_PREPEND_INDENT : 0));
  if ((flag & MSGFLAG_TELL || flag & MSGFLAG_REMOTE) &&
      query_once_interactive(ob))
    if (flag & MSGFLAG_WHISPER)
      add_to_tell_history(getuid(ob), 1, 0,
        "Du fluesterst " + ob->name(WEM) + " aus der Ferne etwas zu.", 0,
	flag);
    else
      add_to_tell_history(getuid(ob), 1, 0, message, indent, flag);
}

varargs int Message(string msg, int flag, string indent,
                    string cname, object sender)
{
  object ti;
  string verb, reply, *ignore, tin;
  int em, te;
  mixed deaf;

  // TODO: Kann nach der ersten Uptime raus
  if (objectp(cname))
    raise_error("Message() hat einen neuen Parameter!\n");

  // Bei den Kanaelen 'Debug' und 'Entwicklung' kann man gezielt Bugs
  // einzelner Magier ignorieren. Dazu wird der Kanalname zum 'verb',
  // damit 'ignoriere name.debug' funktioniert.
  if( flag == MSGFLAG_CHANNEL ){
      if((msg[1..5] == "Debug" || msg[1..11] == "Entwicklung"
	    || msg[1..9]=="Warnungen"))
      {
        // Missbrauch der Variable 'ignore' als Zwischenspeicher
        ignore = regexplode( msg, ":| |\\]" );
        verb = lower_case(ignore[0][1..]);
        tin = lower_case(ignore[2]);
      }
      else
      {
        if(cname)
          verb=lower_case(cname);
        else
          verb=query_verb();
        if( ti = this_interactive() )
        {
          tin = getuid(this_interactive());
        }
        else
        {
	  //falls doch kein Objekt...
          if (objectp(sender))
            tin=lower_case(sender->name(RAW)||"<Unbekannt>");
        }
      }
  }
  else {
    if( ti = this_interactive() )
      tin = getuid(this_interactive());
    verb = query_verb();
  }

  te = flag & (MSGFLAG_TELL | MSGFLAG_WHISPER);

  // fuer "erwidere"
  if (ti && (flag & MSGFLAG_TELL || flag & MSGFLAG_REMOTE)) {
    if (!ti->QueryProp(P_INVIS)||IS_LEARNER(ME)) {
      if (flag & MSGFLAG_WHISPER)
        add_to_tell_history(getuid(ti), 0, 1,
          capitalize((((IS_LEARNER(ti) && !ti->QueryProp(P_INVIS) &&
            (ti->QueryProp(P_CAN_FLAGS) & CAN_PRESAY)) ?
            ti->QueryProp(P_PRESAY) : "") + ti->name()) || "") +
          " fluestert Dir aus der Ferne etwas zu.", 0, flag, 0);
      else
        add_to_tell_history(getuid(ti), 0, 1, msg, indent, flag, 0);
    }
  }
  // Hoert der Spieler nicht?
  em = (ti &&
        (te || flag & MSGFLAG_SHOUT) &&
        (QueryProp(P_EARMUFFS) &&
          (query_wiz_level(ti) < QueryProp(P_EARMUFFS))));
  ignore = (pointerp(ignore = QueryProp(P_IGNORE)) ? ignore : ({}));

  // Werden der Sender oder das Verb ignoriert?
  if(!ti && tin && flag == MSGFLAG_CHANNEL)
  {
     if((member(ignore, tin) != -1))
     {
       return MESSAGE_IGNORE_YOU;
     }
     if(verb &&  sizeof(filter(ignore, #'check_ignore, verb, tin)) )
     {
       return MESSAGE_IGNORE_YOU;
     }
  }
  if (ti && (member(ignore, getuid(ti)) != -1)) {
    if(te && (IS_LEARNER(ti)||!QueryProp(P_INVIS)))
      tell_object(ti, capitalize(name())+
                      " hoert gar nicht zu, was Du sagst.\n");
    return MESSAGE_IGNORE_YOU;
  }
  if(tin && verb &&
     sizeof(filter(ignore, #'check_ignore/*'*/, verb, tin)))
  {
    if(ti && verb[0..2] != "ruf" && verb[0..3] != "mruf" &&
       verb[0..3] != "echo" && verb[0] != '-' && !(flag & MSGFLAG_CHANNEL) )
      tell_object(ti, name()+" wehrt \""+verb+"\" ab.\n");
    return MESSAGE_IGNORE_VERB;
  }
  if (flag & MSGFLAG_RTELL) {
    int at;

    verb = lower_case(old_explode(msg, " ")[0][1..]);
    at = member(verb, '@');
    /* verb wird hier eh missbraucht, also auch fuer ein intermud-erwidere*/
    add_to_tell_history(verb, 0, 1, msg, indent, flag, 0);

    if ((member(ignore, verb) >= 0) || (member(ignore,verb[0..at]) >= 0))
      return MESSAGE_IGNORE_YOU;
    else if (at > 0 && member(ignore, verb[at..]) >= 0)
      return MESSAGE_IGNORE_MUD;
  }

  // Taubheit/Oropax
  te |= (flag & MSGFLAG_SAY);

  if (QueryProp(P_DEAF) && (flag & MSGFLAG_DEAFCHK) && !(flag & MSGFLAG_CHIST)) {
    deaf = QueryProp(P_DEAF);
    if (te)
      reply = stringp(deaf) ?
        capitalize(sprintf(deaf, name())) :
        capitalize(name())+" ist momentan leider taub.\n";
  }
  else if (em)
    reply = capitalize(name())+" hat Oropax in den Ohren.\n";

  msg = break_string(msg, 78, indent,
    (QueryProp(P_MESSAGE_PREPEND) ? BS_PREPEND_INDENT : 0) | BS_LEAVE_MY_LFS);

  if(QueryProp(P_BUFFER) &&
     (deaf ||
      query_editing(this_object()) ||
      query_input_pending(this_object())))
  {
    deaf = MESSAGE_DEAF;
    if(flag & MSGFLAG_CACHE)
    {
      if(!stringp(reply))
        reply = name()+" moechte gerade nicht gestoert werden.\n";

      if(sizeof(buffer) < 20)
      {
        buffer += ({ msg[0..<2]+" [" + strftime("%H:%M",time()) + "]\n" });
        reply += "Die Mitteilung wurde von einem kleinen Kobold in Empfang "+
                 "genommen.\nEr wird sie spaeter weiterleiten!";
        deaf = MESSAGE_CACHE;
      }
      else {
        reply += "Die Mitteilung ging verloren, denn "+
                 "der Kobold kann sich nichts mehr merken!";
        deaf = MESSAGE_CACHE_FULL;
      }
      if(ti && (IS_LEARNER(ti)||!QueryProp(P_INVIS)))
        tell_object(ti, reply+"\n");
    }
    return deaf;
  }
  else if((deaf || em) &&
          ( (flag & MSGFLAG_RTELL) ||
            (ti && (IS_LEARNER(ti)||!QueryProp(P_INVIS))))) {
    if (te && ti)
      tell_object(ti, reply);
    return MESSAGE_DEAF;
  }

  _flush_cache();
  if(te && QueryProp(P_AWAY))
    msg = msg[0..<2]+" [" + strftime("%H:%M",time()) + "]\n";

  if (flag & (MSGFLAG_SAY | MSGFLAG_TELL) && comm_beep()) {
    msg=MESSAGE_BEEP+msg;
  }
  tell_object(ME, msg);
  return MESSAGE_OK;
}

static int ignoriere(string str)
{
  mixed ignore;
  int i;

  str = _unparsed_args(1);
  if(!(ignore = Query(P_IGNORE))) ignore = ({});

  if (!str)
  {
      if (!sizeof(ignore)) write("Du ignorierst niemanden.\n");
      else
        write( "Du ignorierst:\n"
               + CountUp( map( sort_array( ignore, #'>/*'*/ ),
                                     #'capitalize/*'*/ ) ) + ".\n" );
      return 1;
  }
  // trim spaces from args and convert to lower case.
  str = lower_case(trim(str, TRIM_BOTH));
  
  if (member(ignore, str)!=-1)
  {
    Set(P_IGNORE, ignore -= ({ str }));
    printf("Du ignorierst %s nicht mehr.\n", capitalize(str));
    return 1;
  }
  if (sizeof(ignore)>1000)
  {
   printf("Du ignorierst schon genuegend!\n");
   return 1;
  }
  Set(P_IGNORE, ignore += ({ str }));
  printf("Du ignorierst jetzt %s.\n", capitalize(str));
  return 1;
}

static int _msg_beep(string str) {
  int beep_interval;
  notify_fail("Syntax: klingelton <1 bis 3600 Sekunden> oder klingelton aus\n");
  if (stringp(str)) {
    if (str=="aus")
      SetProp(P_MESSAGE_BEEP,0);
    else if ((beep_interval=to_int(str)) > 0 && beep_interval<=3600)
      SetProp(P_MESSAGE_BEEP,beep_interval);
    else return 0;
  }

  beep_interval=(int)QueryProp(P_MESSAGE_BEEP);
  write("Ton bei Mitteilungen: "+ 
	(beep_interval ? "aller "+beep_interval+" Sekunden.\n" : "aus.\n"));
  return 1;
}

static int _msg_prepend(string str) {
  int beep_interval;
  notify_fail("Syntax: senderwiederholung ein/aus\n");
  if (stringp(str)) {
    if (str=="aus")  
      SetProp(P_MESSAGE_PREPEND,1);
    else if (str=="ein")  
      SetProp(P_MESSAGE_PREPEND,0);
    else return 0;
  }

  write("Senderwiederholung bei Mitteilungen: "+ 
	((int)QueryProp(P_MESSAGE_PREPEND) ?  "aus" : "ein")+".\n");
  return 1;
}

static int _communicate(mixed str, int silent)
{
  string  verb;
  string  myname;
  string  msg;

  if (!str || extern_call()) str=_unparsed_args()||"";
  /* str=_unparsed_args()||""; */
  verb = query_verb();
  if(stringp(verb) && verb[0] == '\'') str = verb[1..] + " " + str;
  if (str==""||str==" "||!str)
  {
    write("Was willst Du sagen?\n");
    return 1;
  }
  msg=permutate(str);

  myname=(((QueryProp(P_INVIS)||!IS_LEARNER(ME))||
     !(QueryProp(P_CAN_FLAGS)&CAN_PRESAY)?
    "":QueryProp(P_PRESAY))+name())||"";

  object* players = filter(all_inventory(environment()||ME),#'interactive);
  string sag_text=break_string(msg, 78, capitalize(myname)+" sagt: ");
  string prepended_text;
  foreach (object target : players-({ME})) {
    if (target->QueryProp(P_DEAF)) continue;
    if (target->QueryProp(P_MESSAGE_PREPEND)) {
      if (!stringp(prepended_text)) 
	prepended_text=break_string(msg, 78, capitalize(myname)+" sagt: ",
	    BS_PREPEND_INDENT);
      _send(target,prepended_text,MSGFLAG_SAY);
    } else {
      _send(target,sag_text,MSGFLAG_SAY);
    }
  }
  if(!silent)
    _recv(0, str, MSGFLAG_SAY, "Du sagst: ");
  say(sag_text,players);
  return 1;
}

static int _shout_to_all(mixed str)
{
  string pre, myname, realname, str1, str2;
  int i,chars;

  if (!(str=_unparsed_args()))
  {
    write("Was willst Du rufen?\n");
    return 1;
  }
  chars=strlen(str)/2;
  if (chars<4) chars=4;
  pre = (!IS_LEARNER(ME) ||
   QueryProp(P_INVIS) ||
   !(QueryProp(P_CAN_FLAGS) & CAN_PRESAY)) ? "" : QueryProp(P_PRESAY);
  realname = capitalize((pre + capitalize(getuid()))||"");
  myname = capitalize(pre + name()||"");
  if (QueryProp(P_INVIS))
    realname = "("+realname+")";

  str1=permutate(str);

  if(QueryProp(P_FROG))
    str2 = break_string("Quaaak, quaaaaak, quuuuaaaaaaaaaaaaaaaaaaaak !!",
           78,myname+" quakt: ");
  else str2=break_string(str1,78,myname+" ruft: ");

  if(!IS_LEARNER(this_player()))
  {
    if(QueryProp(P_GHOST)) {
	write("So ganz ohne Koerper bekommst Du keinen Ton heraus.\n");
	return 1;
    }
    if (QueryProp(P_SP) <(chars+20))
    {
      write("Du must erst wieder magische Kraefte sammeln.\n");
      write("Tip: Benutz doch mal die Kanaele (Hilfe dazu mit 'hilfe kanaele').\n");
      return 1;
    }
    SetProp(P_SP, QueryProp(P_SP) - chars - 20);
  }

  _recv(0, str, MSGFLAG_SHOUT, "Du rufst: ");
  str1=break_string(str1,78,realname+" ruft: ");

  foreach ( object ob : users()-({this_object()}) )
    if ( IS_LEARNER(ob) )
      _send(ob,str1,MSGFLAG_SHOUT);
    else
      _send(ob,str2,MSGFLAG_SHOUT);

  return 1;
}

varargs int _tell(string who, mixed msg, int intcall)
{
  object    ob;
  string    away,myname,ret;
  mixed     ignore,it,*xname;
  int       i,visflag;

  if (extern_call()&&this_interactive()!=ME) return 1;
  if (!who || !msg) {
    write("Was willst Du mitteilen?\n");
    return 1;
  }

  if(who == ERWIDER_PARAM)
  {
    if (!last_comm_partner)
    {
      _notify_fail("Du hast aber noch keine Mitteilungen erhalten, auf die "
          "Du was erwidern\nkoenntest.\n");
      return 0;
    }
    who=last_comm_partner;
  }

  // teile .x mit teilt bisherigen Gespraechspartnern etwas mit.
  if (who == ".")
   who = ".1";

  if ( sscanf(who, ".%d", i) == 1 ) {
    if(i > 0 && i <= sizeof(commreceivers))
      who = commreceivers[i-1];
    else {
      _notify_fail("So vielen Leuten hast Du noch nichts mitgeteilt!\n");
      return 0;
    }
  }

  xname = old_explode(who, "@");

  if (sizeof(xname) == 2) {
    if (ret=(string)INETD->_send_udp(xname[1],
                                    ([ REQUEST:   "tell",
                                       RECIPIENT: xname[0],
                                       SENDER:    getuid(ME),
                                       DATA:     msg ]), 1))
      write(ret);
    else
    {
      write("Nachricht abgeschickt.\n");
      add_to_tell_history(who, 1, 0, msg,
        "Du teilst " + capitalize(who) + " mit: ", MSGFLAG_TELL, 1);
    }
    return 1;
  }

  if (!ob=find_player(it = lower_case(who)))
  {
    it = match_living(it, 0);
    if (!stringp(it))
      switch(it){
      case -1:
        write("Das war nicht eindeutig!\n");
        return 1;
      case -2:
        write("Kein solcher Spieler!\n");
        return 1;
      }
    ob = find_player(it);
    if(!ob) ob = find_living(it);
    if(!ob){
      write("Kein solcher Spieler!\n");
      return 1;
    }
  }

  if(QueryProp(P_INVIS)){
    if(!IS_LEARNER(ob))
      myname = name();
    else
      myname="("+
             ((QueryProp(P_CAN_FLAGS) & CAN_PRESAY)?QueryProp(P_PRESAY):"")+
             capitalize(getuid()) + ")";
  }
  else
    myname=((IS_LEARNER(ME) && (QueryProp(P_CAN_FLAGS) & CAN_PRESAY)) ?
              QueryProp(P_PRESAY):"") + name();
  if (myname && strlen(myname)) myname=capitalize(myname);

  if (visflag = !ob->QueryProp(P_INVIS) || IS_LEARNER(this_player()))
    _recv(ob, msg, MSGFLAG_TELL, "Du teilst " + capitalize(it) + " mit: ");

  if (!visflag&&interactive(ob))
    write("Kein solcher Spieler!\n");
  else if (away = (string) ob->QueryProp(P_AWAY))
      write( break_string( away, 78, capitalize(it)
                           + " ist gerade nicht da: ", BS_INDENT_ONCE ) );
  else
    if(interactive(ob) && (i=query_idle(ob))>=600){ //ab 10 Mins
      if(i<3600) away=time2string("%m %M",i);
      else away=time2string("%h %H und %m %M",i);
      printf("%s ist seit %s voellig untaetig.\n",capitalize(it),away);
    }
  _send(ob, permutate(msg), MSGFLAG_TELL, myname + " teilt Dir mit: ");
  return 1;
}

static int _teile(string str)
{
  string who, message;
  if (!(str=_unparsed_args())) return 0;
  if (sscanf(str, "%s mit %s", who, message) == 2)
    return _tell(who, message,1);
  return 0;
}

static int _erzaehle(string str)
{
  string who, message;

  if (!(str=_unparsed_args())) return 0;
  if (sscanf(str, "%s %s", who, message) == 2)
    return _tell(who, message,1);
  return 0;
}

static int _whisper(string str)
{
  object    ob;
  string    who;
  string    msg;
  string    myname;

  if (!(str=_unparsed_args()) ||
       (sscanf(str, "%s zu %s", who, msg) != 2 &&
  sscanf(str, "%s %s", who, msg) !=2 )) {
    write("Was willst Du wem zufluestern?\n");
    return 1;
  }
  if (!(ob = present(who, environment(this_player()))) || !living(ob)) {
    write(capitalize(who)+" ist nicht in diesem Raum.\n");
    return 1;
  }

  myname = capitalize((((IS_LEARNER(ME) &&
       !QueryProp(P_INVIS) &&
       (QueryProp(P_CAN_FLAGS) & CAN_PRESAY))?
      QueryProp(P_PRESAY) : "") + name()) || "");
  _send(ob, permutate(msg), MSGFLAG_WHISPER, myname + " fluestert Dir zu: ");
  _recv(ob, msg, MSGFLAG_WHISPER, "Du fluesterst " + ob->name(WEM) + " zu: ");
  say(myname + " fluestert " + ob->name(WEM, 1) + " etwas zu.\n", ob);
  return 1;
}

static int _remote_whisper(string str)
{
  /* Wie 'teile mit', nur mit MSGFLAG_WHISPER. Dadurch wird der Inhalt der
     Nachricht nicht in der tell_history verewigt. */

  object    ob;
  string    who, it;
  string    msg;
  string    myname;

  if (!(str=_unparsed_args()) ||
       (sscanf(str, "%s zu %s", who, msg) != 2 &&
  sscanf(str, "%s %s", who, msg) !=2 )) {
    write("Was willst Du wem aus der Ferne zufluestern?\n");
    return 1;
  }

  if (!ob=find_player(it = lower_case(who)))
  {
    it = match_living(it, 0);
    if (!stringp(it))
      switch(it){
      case -1:
        write("Das war nicht eindeutig!\n");
        return 1;
      case -2:
        write("Kein solcher Spieler!\n");
        return 1;
      }
    ob = find_player(it);
    if(!ob) ob = find_living(it);
    if(!ob){
      write("Kein solcher Spieler!\n");
      return 1;
    }
  }
  if (ob->QueryProp(P_INVIS) && IS_LEARNER(this_player())) {
    write("Kein solcher Spieler!\n");
    return 1;
  }
  if (environment(ob) == environment()) {
    write("Wenn jemand neben Dir steht, nimm fluester.\n");
    return 1;
  }

  myname = capitalize((((IS_LEARNER(ME) &&
       !QueryProp(P_INVIS) &&
       (QueryProp(P_CAN_FLAGS) & CAN_PRESAY))?
      QueryProp(P_PRESAY) : "") + name()) || "");
  _send(ob, permutate(msg), MSGFLAG_WHISPER | MSGFLAG_REMOTE,
        myname + " fluestert Dir aus der Ferne zu: ");
  _recv(ob, msg, MSGFLAG_WHISPER | MSGFLAG_REMOTE,
        "Du fluesterst " + ob->name(WEM) + " aus der Ferne zu: ");
  return 1;
}

static int _converse(string arg)
{
  write("Mit '**' wird das Gespraech beendet.\n");
  if (stringp(arg) && strstr(arg, "-s") == 0)
    input_to("_converse_more", INPUT_PROMPT, "]", 1);
  else
    input_to("_converse_more", INPUT_PROMPT, "]", 0);
  return 1;
}

static int _converse_more(mixed str, int silent)
{
  string    cmd;
  int     i;
  string    myname;

  if (str == "**") {
    write("Ok.\n");
    return 0;
  }
  myname=capitalize((((IS_LEARNER(ME) && !QueryProp(P_INVIS) && 
	    (QueryProp(P_CAN_FLAGS)&CAN_PRESAY))?
          QueryProp(P_PRESAY):"")+name())||"");
  if(str != "") _communicate(str, silent);
  input_to("_converse_more", INPUT_PROMPT, "]", silent);
}

private int is_learner(object o) { return IS_LEARNER(o); }

static int _shout_to_wizards(mixed str)
{
  int     i, j;
  string    myname;
  object   *u;

  str = _unparsed_args();
  if (!str||!strlen(str)) {
    write("Was willst Du den Magiern zurufen?\n");
    return 1;
  }
  myname = capitalize(getuid(this_object()));
  if (!IS_LEARNER(this_object()))
    _recv(0, str, MSGFLAG_MECHO, "Du teilst allen Magiern mit: ");
  map(filter(users(), #'is_learner/*'*/),
        #'_send/*'*/,
            break_string(str, 78, myname+" an alle Magier: "),MSGFLAG_MECHO);
  return 1;
}

static int _echo(string str) {
  if (!IS_SEER(ME) || (!IS_LEARNER(ME) &&
           !(QueryProp(P_CAN_FLAGS)&CAN_ECHO)))
    return 0;
  if (!(str=_unparsed_args())) {
    write ("Was moechtest Du 'echoen'?\n");
    return 1;
  }
  if (!IS_LEARNER(this_interactive()))
  {
    if (QueryProp(P_GHOST))
    {
      _notify_fail("Ohne Koerper fehlt Dir dazu die noetige magische Kraft.\n");
      return 0;
    }
    if (QueryProp(P_SP)<ECHO_COST)
    {
      _notify_fail("Du musst erst wieder magische Kraefte sammeln.\n");
      return 0;
    }
    SetProp(P_SP,QueryProp(P_SP)-ECHO_COST);
    str=">\b"+str;
    log_file("ARCH/ECHO_SEHER", sprintf("%s %s: %s\n", dtime(time()), getuid(),
           str));
  }
  say ( str + "\n");
  write ( str + "\n");
  return 1;
}

static mixed _set_ignore(mixed arg)
{
  if(stringp(arg)) arg = ({ arg });
  if(pointerp(arg)) return Set(P_IGNORE, arg);
}

static int _query_intermud()
{
  mixed tmp;
  return member(pointerp(tmp=Query(P_CHANNELS))?tmp:({}), "Intermud") > -1;
}


int erwidere(string str)
{
  str=_unparsed_args();
  if (!str) return 0;
  return _tell(ERWIDER_PARAM, str ,1);
}

static int tmhist(string str)
{

  if (str == "aus") {
    tell_history_enabled = TELLHIST_DISABLED;
    write("Ok, es wird nichts mehr gespeichert.\n");
    if (sizeof(tell_history)) {
      tell_history = ([]);
      commreceivers = ({});
      write("Deine Mitteilungsgeschichte wurde geloescht.\n");
    }
    return 1;
  }

  if (str == "namen") {
    int flag;
    tell_history_enabled = TELLHIST_NO_MESSAGE;
    write("Ok, die Namen zukuenftiger Gespraechspartner werden gespeichert.\n");
    foreach (string uid, struct chat_s chat: tell_history)
      if (pointerp(chat->msgbuf)) {
        chat->msgbuf = 0;
        chat->ptr = 0;
        flag = 1;
      }
    if (flag)
      write("Der Inhalt Deiner Mitteilungen wurde geloescht.\n");
    return 1;
  }

  if (str == "ein" || str == "an") {
    tell_history_enabled = TELLHIST_ENABLED;
    write("Ok, zukuenftige Mitteilungen werden gespeichert.\n");
    return 1;
  }

#ifdef TELLHIST_LONGLIFE
  if (str == "langlebig") {
    tell_history_enabled = TELLHIST_LONGLIFE;
    write("Ok, zukuenftige Mitteilungen werden jeweils bis zum naechsten "
          "Ende/Crash/\nReboot gespeichert.\n");
    return 1;
  }
#endif

  if (str == "status") {
    switch (tell_history_enabled) {
      case TELLHIST_DISABLED:
        write("Die Namen Deiner Gespraechspartner werden nicht gespeichert.\n");
        break;
      case TELLHIST_NO_MESSAGE:
        write("Die Namen Deiner Gespraechspartner werden gespeichert.\n");
        break;
      case TELLHIST_ENABLED:
        write("Deine Mitteilungen werden gespeichert.\n");
        break;
#ifdef TELLHIST_LONGLIFE
      case TELLHIST_LONGLIFE:
        write("Deine Mitteilungen werden jeweils bis zum naechsten Ende/"
              "Crash/Reboot\ngespeichert.\n");
        break;
#endif
    }
    return 1;
  }

  if (tell_history_enabled == TELLHIST_DISABLED) {
    _notify_fail("Deine Gespraechspartner werden nicht gespeichert.\n");
    return 0;
  }

  if (!sizeof(tell_history)) {
    _notify_fail("Du hast noch keinem etwas mitgeteilt "
                 "und noch keine Mitteilungen erhalten.\n");
    return 0;
  }

  if (str && strlen(str)) {

    if (tell_history_enabled < TELLHIST_ENABLED) {
      _notify_fail("Der Inhalt Deiner Mitteilungen wird nicht gespeichert.\n");
      return 0;
    }

    string uid;
    if (member(tell_history, str)) {
      // Name gewuenscht, da der String in der History vorkommt.
      uid = str;
    }
    else {
      // evtl. ne Zahl angegeben.
      int i;
      string *partners = sorted_commpartners(0);
      if ((i = to_int(str) - 1) >= 0 && i < sizeof(partners))
        uid = partners[i];
      else {
        notify_fail("Mit so vielen Leuten hast Du nicht gesprochen!\n");
        return 0;
      }
    }

    mixed *data = tell_history[uid]->msgbuf;
    if (!data) {
      _notify_fail(
        "Der Inhalt dieser Mitteilung ist nicht (mehr) gespeichert.\n");
      return 0;
    }

    int ptr = tell_history[uid]->ptr;

    More(sprintf("%@s", map(data[ptr..MAX_SAVED_MESSAGES-1] +
                              data[0..ptr-1],
         function string (struct msg_s msg) {
             if (!structp(msg)) return "";
               return break_string( msg->msg + " <"
                 + strftime("%H:%M:%S",msg->timestamp) + ">", 78,
                 msg->indent || "", msg->indent ? BS_LEAVE_MY_LFS : 0);
         } ) ) );
    return 1;
  }

  string history = "Folgende Gespraeche hast Du bereits gefuehrt:\n";
  int i;
  foreach (string uid : sorted_commpartners(0) ) {
    int j;
    struct chat_s chat = tell_history[uid];
    history += sprintf("%2d.%-4s %s  %-11s  %d gesendet/%d empfangen\n", ++i,
      ((j=member(commreceivers,uid))>-1 ? sprintf("/%2d.",j+1) : ""),
      strftime("%a, %e.%m.%y",chat->time_last_msg),
      capitalize(chat->uid), chat->sentcount, chat->recvcount);
  }

  More(history);

  return 1;
}

static string *_query_localcmds()
{
  return ({
    ({"kobold", "_flush_cache",0,0}),
     ({"sag","_communicate",0,0}),
     ({"sage","_communicate",0,0}),
     ({"'","_communicate",1,0}),
     ({"mruf","_shout_to_wizards",0,0}),
     ({"mrufe","_shout_to_wizards",0,0}),
     ({"ruf","_shout_to_all",0,0}),
     ({"rufe","_shout_to_all",0,0}),
     ({"erzaehl","_erzaehle",0,0}),
     ({"erzaehle","_erzaehle",0,0}),
     ({"teil","_teile",0,0}),
     ({"teile","_teile",0,0}),
     ({"fluester","_whisper",0,0}),
     ({"fluestere","_whisper",0,0}),
     ({"rfluester","_remote_whisper",0,0}),
     ({"rfluestere","_remote_whisper",0,0}),
     ({"gespraech","_converse",0,0}),
     ({"echo","_echo",0,0}),
     ({"ignorier","ignoriere",0,0}),
     ({"ignoriere","ignoriere",0,0}),
     ({"tmhist","tmhist",0,0}),
     ({"erwider","erwidere",0,0}),
     ({"erwidere","erwidere",0,0}),
     ({"klingelton","_msg_beep",0,0}),
     ({"senderwiederholung","_msg_prepend",0,0}),
    })+channel::_query_localcmds();
}

private string *sorted_commpartners(int reversed) {
  return sort_array(m_indices(tell_history),
      function int (string uid1, string uid2) {
          if (reversed)
            return tell_history[uid1]->time_last_msg >
                   tell_history[uid2]->time_last_msg;
          else
            return tell_history[uid1]->time_last_msg <=
                   tell_history[uid2]->time_last_msg;
      } );
}

