// MorgenGrauen MUDlib
//
// simul_efun.c -- simul efun's
//
// $Id: spare_simul_efun.c 7252 2009-08-07 14:10:52Z Zesstra $

#pragma strict_types
#pragma no_clone
#pragma no_shadow
#pragma no_inherit
#pragma verbose_errors
#pragma combine_strings
#pragma pedantic
//#pragma range_check
#pragma warn_deprecated

// Absolute Pfade erforderlich - zum Zeitpunkt, wo der Master geladen
// wird, sind noch keine Include-Pfade da ...

#define SNOOPLOGFILE "SNOOP"
#define ASNOOPLOGFILE "ARCH/SNOOP"

#include "/secure/config.h"
#include "/secure/wizlevels.h"
#include "/sys/snooping.h"
#include "/sys/language.h"
#include "/sys/thing/properties.h"
#include "/sys/wizlist.h"
#include "/sys/erq.h"
#include "/sys/lpctypes.h"
#include "/sys/daemon.h"
#include "/sys/properties.h"
#include "/sys/defines.h"
#include "/sys/telnet.h"
#include "/sys/objectinfo.h"
#include "/sys/files.h"
#include "/sys/strings.h"
#include "/sys/debug_info.h"
#include "/sys/time.h"
#include "/sys/lpctypes.h"
#include "/sys/notify_fail.h"
#include "/sys/tls.h"

#define IP_VALID_CACHE 86400 // Eintraege im IP-Name-Cache nach 24h erneuern
#define IP_RETRY 28800	     // Retry after 8 hours if name not found

#define TO	 efun::this_object()
#define TI	 efun::this_interactive()
#define TP	 efun::this_player()
#define PO	 efun::previous_object(0)
#define LEVEL(x) query_wiz_level(x)
#define NAME(x)  capitalize(getuid(x))

#define DEBUG(x) if (find_player("zesstra")) \
  tell_object(find_player("zesstra"),x)

/* function prototypes
 */
string dtime(int wann);
varargs int log_file(string file, string txt, int size_to_break);
int query_wiz_level(mixed player);
mixed *remove_alist(mixed key, mixed *alist);
nomask varargs int snoop(object me, object you);
nomask object query_snoop(object who);
varargs string country(mixed ip, string num);
int query_wiz_grp(mixed wiz);
void set_living_name(string name);
object find_living(string name);
object find_player(string name);
public varargs object deep_present(mixed what, object ob);
nomask int secure_level();
nomask string secure_euid();
public nomask int process_call();
nomask mixed __create_player_dummy(string name);
varargs string replace_personal(string str, mixed *obs, int caps);


//replacements for dropped efuns in LD
#if !__EFUN_DEFINED__(extract)
varargs string extract(string str, int from, int to);
#endif
#if !__EFUN_DEFINED__(slice_array)
varargs mixed *slice_array(mixed *array, int from, int to);
#endif
#if !__EFUN_DEFINED__(member_array)
int member_array(mixed item, mixed arraystring);
#endif

#undef SOMMERZEIT
#ifdef SOMMERZEIT
string ctime(int t);
#endif

mapping ip_map;
mixed dtime_cache = ({-1,""});

#if __BOOT_TIME__ < 1234125656
nomask void destruct(mixed ob)
{
  string fname;
  object tmp;

  if ( !objectp(ob) && !objectp(ob = find_object(ob)) )
      return;

  //bestimmte Objekte duerfen nicht so ohne weiteres zerstoert werden.
  fname = object_name(ob);
  if( member(({ "/secure/master", "/secure/simul_efun", "/obj/shut",
	"/secure/shut", "/room/void", "/room/netztot", "/secure/errord",
	"/secure/spare_simul_efun", "/secure/explorationmaster", }), 
	fname ) >= 0 ) {
    if( previous_object()!=ob && !ARCH_SECURITY || process_call() ) {
      write("You have no permission to destruct the specified object!\n");
      return;
    }
  }
  
  // Nicht-EMs sollen keine EMs zerstoeren koennen, woraufhin auch evtl.
  // EM-Tools rumliegen koennten.
  if ( query_once_interactive(ob) && IS_ARCH(ob) && previous_object()!=ob &&
      (process_call() || !ARCH_SECURITY) )
    return;

  // Das Zerstoeren von Spielern wird ggf. geloggt.
  if ( query_once_interactive(ob) && previous_object() != ob
       && previous_object()
       && object_name(previous_object())[0..7] != "/secure/" )
      log_file( "PLAYERDEST",
		sprintf( "%s: %O VERNICHTET von PO %O, TI %O, TP %O\n",
			 dtime(time()), ob, previous_object(),
			 this_interactive(), this_player() ) );

  set_this_object(previous_object());
  efun::destruct(ob);
}
#endif

// Nicht jeder Magier muss die simul_efun entsorgen koennen.
string NotifyDestruct(object caller) {
    if( (caller!=this_object() && !ARCH_SECURITY) || process_call() ) {
      return "Du darfst das simul_efun Objekt nicht zerstoeren!\n";
    }
}

public nomask void remove_interactive( object ob )
{
    if ( objectp(ob) && previous_object()
	 && object_name(previous_object())[0..7] != "/secure/"
	 && ((previous_object() != ob
	      && (ob != this_player() || ob != this_interactive()))
	     || (previous_object() == ob
		 && (this_player() && this_player() != ob
		     || this_interactive() && this_interactive() != ob)) ) )

	log_file( "PLAYERDEST",
		  sprintf( "%s: %O ausgeloggt von PO %O, TI %O, TP %O\n",
			   dtime(time()), ob, previous_object(),
			   this_interactive(), this_player() ) );

    efun::remove_interactive(ob);
}


void ___updmaster()
{
    object ob;

    //sollte nicht jeder duerfen.
    if (process_call() || !ARCH_SECURITY)
      raise_error("Illegal use of ___updmaster()!");

    write("Removing old master ... ");
    foreach(string file: get_dir("/secure/master/*.c",GETDIR_NAMES|GETDIR_UNSORTED)) {
      if (ob = find_object(file))
	efun::destruct(ob);
    }
    efun::destruct(efun::find_object("/secure/master"));
    write("done.\nLoading again ... ");
    load_object("/secure/master");

    write("done.\n");
}

// * Herkunfts-Ermittlung
string query_ip_number(object ob)
{
  ob= ob || this_player();
  if (!objectp(ob) || !interactive(ob)) return 0;
  if(ob->query_realip() && (string)ob->query_realip()!="")
  {
    return (string)ob->query_realip();
  }
  return efun::query_ip_number(ob);
}

string query_ip_name(mixed ob)
{
  string ipnum, *tmp;
  mixed ipname;

  if ( !ob || objectp(ob) )
      ipnum=query_ip_number(ob);
  else if ( stringp(ob) && sizeof(tmp = efun::explode(ob, ".")) == 4 &&
	    ob == implode( map( map( tmp, #'to_int ), #'to_string ),
				      "." ) )
      ipnum = ob;
  else
      return 0;

  ipname=ip_map[ipnum,0];
  if (stringp(ipname))
  {
    if ((time()-ip_map[ipnum,1])>IP_VALID_CACHE)
    {
      ip_map[ipnum,1]=time();
      "/secure/master"->get_ip_name(ipnum);
    }
    return ipname;
  }
  if (++ipname>3)
  {
    ip_map[ipnum,1]=time()-IP_VALID_CACHE+IP_RETRY;
    return ip_map[ipnum]=ipnum;
  }
  "/secure/master"->get_ip_name(ipnum);
  ip_map[ipnum,0]=ipname;
  return ipnum;
}

varargs string country(mixed ip, string num) {
  mixed ret, *strs, twn;
  int a,i;

  if (objectp(ip))
  {
    num=query_ip_number(ip);
    ip=query_ip_name(ip);
  }

  if(ret = (string)"/p/daemon/udp/whois"->locate(ip, num)) {
    if(pointerp(ret)) return ret[0];
    return ret;
  } else return "???";
}


// * Snoopen und was dazugehoert
static object find_snooped(object who)
{
  object *u;
  int i;

  for (i=sizeof(u=users())-1;i>=0;i--)
    if (who==efun::query_snoop(u[i]))
      return u[i];
  return 0;
}

private static string Lcut(string str) {
  return str[5..11]+str[18..];
}

nomask varargs int snoop( object me, object you )
{
    int ret;
    object snooper0, snooper, snooper2, snooper3;

    if( !objectp(me) || me == you || !PO )
	return 0;

    snooper0 = find_snooped(me);

     if(you){
	 if ( PO != me && query_wiz_grp(me) >= query_wiz_grp(geteuid(PO)) )
	     return 0;

	 if ( query_wiz_grp(me) <= query_wiz_grp(you) &&
	      !(you->QueryAllowSnoop(me)) )
	     if ( !IS_DEPUTY(me) || IS_ARCH(you) )
		 return 0;

	 if ( (snooper = efun::query_snoop(you)) &&
	      query_wiz_grp(snooper) >= query_wiz_grp(me) ){
	     if ( (int)snooper->QueryProp(P_SNOOPFLAGS) & SF_LOCKED )
		 return 0;

	     tell_object( snooper, sprintf( "%s snooped jetzt %s.\n",
					    me->name(WER), you->name(WER) ) );

	     snooper2 = me;

	     while ( snooper3 = query_snoop(snooper2) ){
		 tell_object( snooper,
			      sprintf( "%s wird seinerseits von %s gesnooped.\n"
				       ,snooper2->name(WER),
				       snooper3->name(WEM) ) );
		 snooper2 = snooper3;
	     }

	     efun::snoop( snooper, snooper2 );

	     if ( efun::query_snoop(snooper2) != snooper )
		 tell_object( snooper, sprintf( "Du kannst %s nicht snoopen.\n",
						snooper2->name(WEN) ) );
	     else{
		 tell_object( snooper, sprintf( "Du snoopst jetzt %s.\n",
						snooper2->name(WEN) ) );
		 if ( !IS_DEPUTY(snooper) ){
		     log_file( SNOOPLOGFILE, sprintf("%s: %O %O %O\n",
						     dtime(time()),
						     snooper,
						     snooper2,
						     environment(snooper2) ),
			       100000 );
		     if (snooper0)
			 CHMASTER->send( "Snoop", snooper,
					 sprintf( "%s *OFF* %s (%O)",
						  capitalize(getuid(snooper)),
						  capitalize(getuid(snooper0)),
						  environment(snooper0) ) );

		     CHMASTER->send( "Snoop", snooper,
				     sprintf("%s -> %s (%O)",
					     capitalize(getuid(snooper)),
					     capitalize(getuid(snooper2)),
					     environment(snooper2)));
		 }
		 else{
		     log_file( ASNOOPLOGFILE, sprintf( "%s: %O %O %O\n",
						       dtime(time()),
						       snooper,
						       snooper2,
						       environment(snooper2) )
			       ,100000 );
		 }
	     }
	 }
	 else
	     if (snooper)
		 if ( !me->QueryProp(P_SNOOPFLAGS) & SF_LOCKED ){
		     printf( "%s wird bereits von %s gesnooped. Benutze das "
			     "\"f\"-Flag, wenn du dennoch snoopen willst.\n",
			     you->name(WER), snooper->name(WEM) );
		     return 0;
		 }

	 ret = efun::snoop( me, you );

	 if ( !IS_DEPUTY(me) && efun::query_snoop(you) == me){
	     log_file( SNOOPLOGFILE, sprintf( "%s: %O %O %O\n",
					      Lcut(dtime(time())),
					      me, you, environment(you) ),
		       100000 );

	     if (snooper0)
		 CHMASTER->send( "Snoop", me,
				 sprintf( "%s *OFF* %s (%O).",
					  capitalize(getuid(me)),
					  capitalize(getuid(snooper0)),
					  environment(snooper0) ) );

	     CHMASTER->send( "Snoop", me, sprintf( "%s -> %s (%O).",
						   capitalize(getuid(me)),
						   capitalize(getuid(you)),
						   environment(you) ) );
	 }
	 else{
	     if ( efun::query_snoop(you) == me ){
		 log_file( ASNOOPLOGFILE, sprintf( "%s: %O %O %O\n",
						   Lcut(dtime(time())),
						   me, you, environment(you) ),
			   100000 );
	     }
	 }

	 if ( ret && query_wiz_grp(me) <= query_wiz_grp(you) &&
	      !IS_DEPUTY(me) )
	     tell_object( you, "*** " + NAME(me) + " snoopt Dich!\n" );

	 return ret;
     }
     else {
	 if ( (me == PO ||
	       query_wiz_grp(geteuid(PO)) > query_wiz_grp(me) ||
	       (query_wiz_grp(geteuid(PO)) == query_wiz_grp(me) &&
		query_snoop(PO) == me)) && snooper0 ){
	     if ( !IS_DEPUTY(me) ){
		 log_file( SNOOPLOGFILE, sprintf( "%s: %O %O %O *OFF*\n",
						  Lcut(dtime(time())), me,
						  snooper0,
						  environment(snooper0) ),
			   100000 );

		  CHMASTER->send( "Snoop", me,
				  sprintf( "%s *OFF* %s (%O).",
					   capitalize(getuid(me)),
					   capitalize(getuid(snooper0)),
					   environment(snooper0) ) );
	     }
	     else{
		 log_file( ASNOOPLOGFILE, sprintf( "%s: %O %O %O *OFF*\n",
						   Lcut(dtime(time())), me,
						   snooper0,
						   environment(snooper0) ),
			   100000 );
	     }

	     return efun::snoop(me);
	 }
     }
}

nomask object query_snoop(object who) {
  object snooper;

  snooper=efun::query_snoop(who);
  if (!snooper) return 0;
  if (query_wiz_grp(snooper)>query_wiz_grp(getuid(previous_object())) &&
      IS_ARCH(snooper)) return 0;
  return snooper;
}

// * Emulation des 'alten' explode durch das neue
string *old_explode(string str, string del) {
  int s, t;
  string *strs;

  if (!stringp(str)) {
      set_this_object(previous_object());
      raise_error(sprintf(
	  "Invalid argument 1 to old_explode()! Expected <string>, got: "
	  "%.30O\n",str));
  }
  if (!stringp(del)) {
      set_this_object(previous_object());
      raise_error(sprintf(
	  "Invalid argument 2 to old_explode()! Expected <string>, got: "
	  "%.30O\n",del));
  }
  if(del == "")
    return ({str});
  strs=efun::explode(str, del);
  t=sizeof(strs)-1;
  while(s<=t && strs[s++] == "");s--;
  while(t>=0 && strs[t--] == "");t++;
  if(s<=t)
    return strs[s..t];
  return ({});
}

int file_time(string path) {
  mixed *v;

  set_this_object(previous_object());
  if (sizeof(v=get_dir(path,GETDIR_DATES))) return v[0];
  return(0); //sonst
}

// * Bei 50k Groesse Log-File rotieren
varargs int log_file(string file, string txt, int size_to_break) {
  mixed *st;

  file="/log/"+file;
  file=implode((efun::explode(file,"/")-({".."})),"/");
//  tell_object(find_player("jof"),sprintf("LOG FILE: %O -> %O\n",previous_object(),file));
  if (!funcall(bind_lambda(#'efun::call_other,PO),"secure/master",//')
	       "valid_write",file,geteuid(PO),"log_file",PO))
      return 0;
  if ( size_to_break >= 0 & (
      sizeof(st = get_dir(file,2) ) && st[0] >= (size_to_break|MAX_LOG_SIZE)))
      catch(rename(file, file + ".old");publish); /* No panic if failure */
  return(write_file(file,txt));
}

// * Magier-Level abfragen
int query_wiz_level(mixed player) {
  return (int)"/secure/master"->query_wiz_level(player);
}

// * Element aus Alist loeschen (by key)
mixed *remove_alist(mixed key,mixed *alist)
{
  int i,j;

  if (!pointerp(alist) || !sizeof(alist))
    return alist;
  if (!pointerp(alist[0]))
  {
    if ((i=assoc(key,alist))<0)
      return alist;
    return alist[0..i-1]+alist[i+1..];
  }
  i = assoc(key,alist[0]);
  if ((i=assoc(key,alist[0]))<0)
    return alist;
  alist=alist[0..];
  for (j=sizeof(alist)-1;j>=0;j--)
    alist[j]=alist[j][0..i-1]+alist[j][i+1..];
  return alist;
}

// * Element aus Alist loeschen (by pos)
mixed *exclude_alist(int i,mixed *alist)
{
  int j;
  if (!pointerp(alist) || !sizeof(alist) || i<0)
    return alist;
  if (!pointerp(alist[0]))
    return alist[0..i-1]+alist[i+1..];
  alist=alist[0..]; /* Create PHYSICAL copy of alist */
  for (j=sizeof(alist)-1;j>=0;j--)
    alist[j]=alist[j][0..i-1]+alist[j][i+1..];
  return alist; /* order_alist is NOT necessary - see /doc/LPC/alist */
}

// * German version of ctime()
#define TAGE ({"Son","Mon","Die","Mit","Don","Fre","Sam"})
#define MONATE ({"Jan","Feb","Mar","Apr","Mai","Jun","Jul","Aug",\
                 "Sep","Okt","Nov","Dez"})
string dtime(int wann) {
  
  if (wann == dtime_cache[0])
    return(dtime_cache[1]);

  int *lt = localtime(wann);
  return sprintf("%s, %2d. %s %d, %02d:%02d:%02d",
      TAGE[lt[TM_WDAY]], lt[TM_MDAY], MONATE[lt[TM_MON]], 
      lt[TM_YEAR],lt[TM_HOUR], lt[TM_MIN], lt[TM_SEC]);
}

// wenn strftime im Driver nicht existiert, ist dies hier ein Alias auf dtime(),
// zwar stimmt das Format dann nicht, aber die Mudlib buggt nicht und schreibt
// ein ordentliches Datum/Uhrzeit.
#if !__EFUN_DEFINED__(strftime)
varargs string strftime(mixed fmt, int clock, int localized) {
  if (intp(clock) && clock >= 0)
    return dtime(clock);
  else if (intp(fmt) && fmt >= 0)
    return dtime(fmt);
  
  return dtime(time());
}
#endif //!__EFUN_DEFINED__(strftime)

// * Shutdown mit zusaetzlichem logging
nomask int shutdown(string reason)
{
  string name;
  string obname;
  string output;

  if (!reason)
    return 0;
  if ( !ARCH_SECURITY && getuid(previous_object())!=ROOTID &&
	   object_name(previous_object())!="/obj/shut" )
  {
    write("You have no permission to shut down the gamedriver!\n");
    return 0;
  }
  if ((this_interactive())&&(name=getuid(this_interactive())))
  {
    name=capitalize(name);
    filter(users(),#'tell_object,//'
		 capitalize(name)+" faehrt das Spiel herunter!\n");
  }
  else
    name="ANONYMOUS";
  if (previous_object()) obname=capitalize(getuid(previous_object()));
  output=name;
  if (obname && name!=obname) output=output+" ("+obname+")";
  if (previous_object()&&object_name(previous_object())=="/obj/shut"){
    output+=" faehrt das Spiel via Armageddon herunter.\n";
    output=dtime(time())+": "+output;
    log_file("GAME_LOG",output+"\n",-1);
    efun::shutdown();
    return 1;
  }
  output=ctime(time())+": "+output+" faehrt das Spiel herunter.\n";
  output+="    Grund: "+reason;
  log_file("GAME_LOG",output+"\n",-1);
  efun::shutdown();
  return 1;
}

// * lowerchar

int lowerchar(int char) {
  if (char<'A' || char>'Z') return char;
  return char+32;
}

// * upperstring

string upperstring(string s)
{
#if __EFUN_DEFINED__(upper_case)
  return(upper_case(s));
#else
  int i;
  if (!stringp(s)) return 0;
  for (i=strlen(s)-1;i>=0;i--) s[i]=((s[i]<'a'||s[i]>'z')?s[i]:s[i]-32);
  return s;
#endif
}

// * lowerstring

string lowerstring(string s)
{
  if (!stringp(s)) return 0;
  return lower_case(s);
}


// * GD version
string version()
{
  return __VERSION__;
}

// * break_string
// stretch() -- stretch a line to fill a given width
private string stretch(string s, int width) {
  int len=strlen(s);
  if (len==width) return s;

  // reine Leerzeilen, direkt zurueckgeben
  string trimmed=trim(s,TRIM_LEFT," ");
  if (trimmed=="") return s; 
  int start_spaces = len - strlen(trimmed);

  string* words = explode(trimmed, " ");
  // der letzte kriegt keine Spaces
  int word_count=sizeof(words) - 1;
  // wenn Zeile nur aus einem Wort, wird das Wort zurueckgegeben
  if (!word_count)
    return " "*start_spaces + words[0];

  int space_count = width - len;

  int space_per_word=(word_count+space_count) / word_count;
  // Anz.Woerter mit Zusatz-Space
  int rest=(word_count+space_count) % word_count; 
  // Rest-Spaces Verteilen
  foreach (int pos : rest) words[pos]+=" ";
  return (" "*start_spaces) + implode( words, " "*space_per_word );
}

// aus Geschwindigkeitsgruenden hat der Blocksatz fuer break_string eine
// eigene Funktion bekommen:
private varargs string block_string(string s, int width, int flags) {
  if (!(flags & BS_NO_PARINDENT)) {
    if (flags & BS_LEAVE_MY_LFS)
      s = " "+regreplace(s,"\n","\n ",1);
    else s=" "+s;
 }

  s = sprintf( "%-*=s", width, s);

  string *tmp=explode(s, "\n");
  return implode( map( tmp[0..<2], #'stretch/*'*/, width ), "\n" ) 
    + "\n" + tmp[<1];
}

public varargs string break_string(string s, int w, mixed indent, int flags)
{
    if ( !s || s == "" ) return "";

    if ( !w ) w=78;

    if( intp(indent) )
	indent = indent ? " "*indent : "";

    int indentlen=stringp(indent) ? strlen(indent) : 0;

    if (indentlen>w) {
      set_this_object(previous_object());
      raise_error(sprintf("break_string: indent longer %d than width %d\n",
		    indentlen,w));
      // w=((indentlen/w)+1)*w;
    }

    if (!(flags & BS_LEAVE_MY_LFS)) 
      s=regreplace( s, "\n", " ", 1 );

    if ( flags & BS_SINGLE_SPACE )
	s = regreplace( s, "(^|\n| )  *", "\\1", 1 );
 
    string prefix="";
    if (indentlen && flags & BS_PREPEND_INDENT) {
      if (indentlen+strlen(s) > w || 
	  (flags & BS_LEAVE_MY_LFS) && strstr(s,"\n")>-1) {
	prefix=indent+"\n";
	indent=(flags & BS_NO_PARINDENT) ? "" : " ";
	indentlen=strlen(indent);
      }
    }

    if ( flags & BS_BLOCK ) {
      /*
	    s = implode( map( explode( s, "\n" ),
				   #'block_string, w, indentlen, flags),
			 "" );
      */
      s = block_string( s , w - indentlen, flags );
    }
    else {
      s = sprintf("%-1.*=s",w-indentlen,s);
    }
    if ( s[<1] != '\n' ) s += "\n";

    if ( !indentlen ) return prefix + s;
    
    string indent2 = ( flags & BS_INDENT_ONCE ) ? (" "*indentlen) :indent;
      
    return prefix + indent + 
      regreplace( s[0..<2], "\n", "\n"+indent2, 1 ) + "\n";
      /*
	string *buf;

	buf = explode( s, "\n" );
	return prefix + indent + implode( buf[0..<2], "\n"+indent2 ) + buf[<1] + "\n";
      */
}

// * Elemente aus mapping loeschen - mapping vorher kopieren

mapping m_delete(mapping m, mixed key) {
  return efun::m_delete(copy(m), key);
}

// * die Breite eines Mappings bestimmen, also die Anzahl der Values pro Key
int m_width(mapping m) {
  return(widthof(m));
}


// * times
int last_reboot_time()
{
  return __BOOT_TIME__;
}

int first_boot_time()
{
  return 701517600;
}

int exist_days()
{
  return (((time()-first_boot_time())/8640)+5)/10;
}

// * uptime :)
string uptime()
{
  int t;
  int tmp;
  string s;

  t=time()-__BOOT_TIME__;
  s="";
  if (t>=86400)
    s+=sprintf("%d Tag%s, ",tmp=t/86400,(tmp==1?"":"e"));
  if (t>=3600)
    s+=sprintf("%d Stunde%s, ",tmp=(t=t%86400)/3600,(tmp==1?"":"n"));
  if (t>60)
    s+=sprintf("%d Minute%s und ",tmp=(t=t%3600)/60,(tmp==1?"":"n"));
  return s+sprintf("%d Sekunde%s",t=t%60,(t==1?"":"n"));
}

// * Was tun bei 'dangling' lfun-closures ?
void dangling_lfun_closure() {
  raise_error("dangling lfun closure\n");
}

// * Sperren ausser fuer master/simul_efun

#if __EFUN_DEFINED__(set_environment)
nomask void set_environment(object o1, object o2) {
  raise_error("Available only for root\n");
}
#endif

nomask void set_this_player(object pl) {
  raise_error("Available only for root\n");
}

// * actions auf den neusten Stand bringen
public void update_actions()
{
  int objs;

  if (environment(previous_object()))
    previous_object()->move(environment(previous_object()),1);
  filter(all_inventory(previous_object()),"move_inv",previous_object());
}

static void move_inv(object ob)
{
  if (living(ob)) ob->move(previous_object(),1);
}

// * Jetzt auch closures
int process_flag;

public nomask int process_call()
{
  if (process_flag>0)
    return process_flag;
  else return(0);
}

private nomask string _process_string(string str,object po) {
		set_this_object(po);
		return(efun::process_string(str));
}

nomask string process_string( mixed str )
{
  string tmp, err;
  int flag; 

  if ( closurep(str) ) {
      set_this_object( previous_object() );
      return funcall(str);
  }
  else if (str==0)
      return((string)str);
  else if ( !stringp(str) ) {
      return to_string(str);
  }

  if ( !(flag = process_flag > time() - 60))			
      process_flag=time();

  err = catch(tmp = funcall(#'_process_string,str,previous_object()); publish);

  if ( !flag )
    process_flag=0;

  if (err) {
    // Verarbeitung abbrechen
    set_this_object(previous_object());
    raise_error(err);
  }
  return tmp;
}


// * Properties ggfs. mitspeichern
mixed save_object(mixed name)
{
  mapping properties;
  mapping save;
  mixed index, res;
  int i;

  // nur Strings und 0 zulassen
  if ((!stringp(name) || !strlen(name)) && 
      (!intp(name) || name!=0)) {
      set_this_object(previous_object());
      raise_error(sprintf(
	  "Only non-empty strings and 0 may be used as filename in "
	  "sefun::save_object()! Argument was %O\n",name));
  }
  save = m_allocate(0, 2);
  properties = (mapping)previous_object()->QueryProperties();

  if(mappingp(properties))
  {
    // delete all entries in mapping properties without SAVE flag!
    index = m_indices(properties);
    for(i = sizeof(index)-1; i>=0;i--)
    {
      if(properties[index[i], F_MODE] & SAVE)
      {
	save[index[i]] = properties[index[i]];
	save[index[i], F_MODE] =
	properties[index[i], F_MODE] &
		      (~(SETMNOTFOUND|QUERYMNOTFOUND|QUERYCACHED|SETCACHED));
      }
    }
  }
  else save = ([]);

  // save object!
  previous_object()->_set_save_data(save);
  // format: wie definiert in config.h
  if (stringp(name))
    res = funcall(bind_lambda(#'efun::save_object, previous_object()), name,
	__LIB__SAVE_FORMAT_VERSION__);
  else
    res = funcall(bind_lambda(#'efun::save_object, previous_object()),
	__LIB__SAVE_FORMAT_VERSION__);
  previous_object()->_set_save_data(0);

  return res;
}

// * Auch Properties laden
int restore_object(string name)
{
  int result;
  mixed index;
  mixed save;
  mapping properties;
  int i;
  closure cl;

  // get actual property settings (by create())
  properties = (mapping)previous_object()->QueryProperties();

//  DEBUG(sprintf("RESTORE %O\n",name));
  // restore object
  result=funcall(bind_lambda(#'efun::restore_object, previous_object()), name);
  //'))
  //_get_save_data liefert tatsaechlich mixed zurueck, wenn das auch immer ein 
  //mapping sein sollte.
  save = (mixed)previous_object()->_get_save_data();
  if(mappingp(save))
  {
    index = m_indices(save);
    for(i = sizeof(index)-1; i>=0; i--)
    {
      properties[index[i]] = save[index[i]];
      properties[index[i], F_MODE] = save[index[i], F_MODE]
				&~(SETCACHED|QUERYCACHED);
    }
  }
  else properties = ([]);

  // restore properties
  (void)funcall(
		bind_lambda(
			    unbound_lambda(({'arg}), //'})
					   ({#'call_other,({#'this_object}),
					   "SetProperties",'arg})),//')
			    previous_object()),properties);
  previous_object()->_set_save_data(0);
  return result;
}

// * HB eines Objektes ein/ausschalten
int set_object_heart_beat(object ob, int flag)
{
  if (objectp(ob))
    return funcall(bind_lambda(#'efun::set_heart_beat,ob),flag);
}

// * Magierlevelgruppen ermitteln
int query_wiz_grp(mixed wiz)
{
  int lev;

  lev=query_wiz_level(wiz);
  if (lev<SEER_LVL) return 0;
  if (lev>=GOD_LVL) return lev;
  if (lev>=ARCH_LVL) return ARCH_GRP;
  if (lev>=ELDER_LVL) return ELDER_GRP;
  if (lev>=LORD_LVL) return LORD_GRP;
  if (lev>=SPECIAL_LVL) return SPECIAL_GRP;
  if (lev>=DOMAINMEMBER_LVL) return DOMAINMEMBER_GRP;
  if (lev>=WIZARD_LVL) return WIZARD_GRP;
  if (lev>=LEARNER_LVL) return LEARNER_GRP;
  return SEER_GRP;
}

mixed *wizlist_info()
{
  if (ARCH_SECURITY || !extern_call())
	     return efun::wizlist_info();
  return 0;
}

// * wizlist ausgeben
varargs void wizlist(string name, int sortkey ) {
  int i, pos, total_cmd;
  int *cmds;
  mixed *a;
  mixed *b;

  if (!name) {
    if (this_player())
			name = getuid(this_player());
    if (!name)
      return;
  }

  // Schluessel darf nur in einem gueltigen Bereich sein
  if (sortkey<=WL_NAME||sortkey>WL_EXTRA) sortkey=WL_COMMANDS;

  a = transpose_array(wizlist_info());

  // Sortieren der Liste nach dem sortierschluessel
  if (sortkey==0)
  {
    a = order_alist(a);
  } else {
    cmds = a[sortkey];
    a[sortkey] = a[0];
    a[0] = cmds;
    a = order_alist(a);
    cmds = a[0];
    a[0] = a[sortkey];
    a[sortkey] = cmds;
  }

  // cmds wurde missbraucht - wieder zuruecksetzen
  cmds = a[WL_COMMANDS];

  if ((pos = member(a[WL_NAME], name)) < 0 && name != "ALL")
    return;
  b = allocate(sizeof(cmds));
  for (i = sizeof(cmds); i;) {
    b[<i] = i;
    total_cmd += cmds[--i];
  }
  a = transpose_array(a + ({b}) );
  if (name != "ALL") {
    if (pos + 18 < sizeof(cmds)) {
      a = a[pos-2..pos+2]+a[<15..];
    } else if (pos < sizeof(cmds) - 13) {
      a = a[pos-2..];
    } else {
      a = a[<15..];
    }
  }
  write("\nWizard top score list\n\n");
  if (total_cmd == 0)
    total_cmd = 1;
  for (i = sizeof(a); i; ) {
    b = a[<i--];
    printf("%-20s %6d %2d%% %6s [%6dk,%6d] %6d %d\n",
	   b[WL_NAME], b[WL_COMMANDS], b[WL_COMMANDS] * 100 / total_cmd,
     "("+(string)b[<1]+")", b[WL_TOTAL_COST] / 1000,
	   b[WL_HEART_BEATS], b[WL_EXTRA], b[WL_ARRAY_TOTAL]
	   );
  }
  printf("\nTotal	  %7d	  (%d)\n\n", total_cmd, sizeof(cmds));
}

static int _cost_sort(mixed *a, mixed *b)
{
  return a[WL_TOTAL_COST]<b[WL_TOTAL_COST];
}

#ifndef TESTMUD
static int CallOutStopped = 0;
static int CallOutCount;
static int CountMessages = 0;
static string LastMessage;

void StopCallOut(int arg)
{
  if (arg && extern_call() && object_name(previous_object())!="/secure/master")
    return 0;
  if (arg && !CallOutStopped)
    write_file("/log/call_out_stop",sprintf("__CALLED %s (%d)\n__(CUT HERE)__\n",
					    ctime(time()),arg));
  CallOutStopped=arg;
}


// Ab hier folgen Hilfsfunktionen fuer call_out() bzw. fuer deren Begrenzung

// ermittelt das Objekt des Callouts.
protected object _call_out_obj( mixed call_out ) {
    return pointerp(call_out) ? call_out[0] : 0;
}

protected void _same_object( object ob, mapping m ) {
  // ist nicht so bloed, wie es aussieht, s. nachfolgede Verwendung von m
  if ( m[ob] )
    m[ob] += ({ ob });
  else
    m[ob] = ({ ob }); 
}

// alle Objekte im Mapping m zusammenfassen, die den gleichen Loadname (Name der
// Blueprint) haben, also alle Clones der BP, die Callouts laufen haben.
// Objekte werden auch mehrfach erfasst, je nach Anzahl ihrer Callouts.
protected void _same_path( object key, object *obs, mapping m ) {
  string path;
  if (!objectp(key) || !pointerp(obs)) return;

  path = load_name(key);

  if ( m[path] )
    m[path] += obs;
  else
    m[path] = obs;
}


protected int _too_many( string key, mapping m, int i ) {
    return sizeof(m[key]) >= i;
}

// alle Objekte in obs zerstoeren und Fehlermeldung ausgeben. ACHTUNG: Die
// Objekte werden idR zu einer BP gehoeren, muessen aber nicht! In dem Fall
// wird auf der Ebene aber nur ein Objekt in der Fehlermeldung erwaehnt.
protected void _destroy( string key, object *obs, string text, int uid ) {
    if (!pointerp(obs)) return;
    // Array mit unique Eintraege erzeugen.
    obs = m_indices( mkmapping(obs) );
    // Fehlermeldung auf der Ebene ausgeben, im catch() mit publish, damit es
    // auf der Ebene direkt scrollt, der backtrace verfuegbar ist (im
    // gegensatz zur Loesung mittels Callout), die Ausfuehrung aber weiter
    // laeuft.
    catch( efun::raise_error(	    
	  sprintf( text,		     
	    uid ? (string)"/secure/master"->creator_file(key) : key,		     
	    sizeof(obs), object_name(obs[<1]) ) );publish);
    // Und weg mit dem Kram...
    filter( obs, #'efun::destruct/*'*/ );
}

// Alle Objekt einer UID im Mapping m mit UID als KEys zusammenfassen. Objekt
// sind dabei nicht unique.
protected void _same_uid( string key, object *obs, mapping m, closure cf ) {
  string uid;

  if ( !pointerp(obs) || !sizeof(obs) )
    return;

  uid = funcall( cf, key );

  if ( m[uid] )
    m[uid] += obs; // obs ist nen Array
  else
    m[uid] = obs;
}


#ifdef STOP_CALL_OUTS_HARD
nomask varargs void call_out( mixed what, int delay, mixed arg1, mixed arg2,
			      mixed arg3, mixed arg4, mixed arg5, mixed arg6,
			      mixed arg7, mixed arg8, mixed arg9, mixed arg10,
			      mixed arg11, mixed arg12, mixed arg13,
			      mixed arg14, mixed arg15 )
{
    object tmp;
    int num;
    string message;

    if (CallOutStopped)
  {
    message=sprintf("%O %O\n",previous_object(),what);
    if(LastMessage==message)
      CountMessages++;
    else
    {
      if (CountMessages)
      {
	write_file("/log/call_out_stop",
		   sprintf("Last message repeated %d times.\n",CountMessages));
	CountMessages=0;
      }
      write_file("/log/call_out_stop",message);
      LastMessage=message;
    }
    return;
  }
  if (!(++CallOutCount%50))
  {
    if((num=sizeof(efun::call_out_info()))>CALL_OUT_HARD)
    {
      write("!!! EMERGENCY !!! MAX NUMBER OF CALL_OUTS EXCEEDED !!! CALL_OUT STOPPED !!!\n");
      filter(users(),#'tell_object,
	"!!! EMERGENCY !!! MAX NUMBER OF CALL_OUTS EXCEEDED !!! CALL_OUT STOPPED !!!\n");
      StopCallOut(1);
      return;
    }
    if (num>CALL_OUT_SOFT)
    {
      filter(users(),#'tell_object,sprintf(
	"WARNING! NUM OF CALL_OUTS exceeded (%d)! WILL STOP AT %d!\n",CALL_OUT_SOFT,CALL_OUT_HARD));
      write(sprintf("WARNING! NUM OF CALL_OUTS exceeded (%d)! WILL STOP AT %d!\n",
	CALL_OUT_SOFT,CALL_OUT_HARD));
    }
  }
  tmp=this_object();
  set_this_object(previous_object());
  efun::call_out(what,delay,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,
		 arg10,arg11,arg12,arg13,arg14,arg15);
  set_this_object(tmp);
  return;
}
#else // STOP_CALL_OUTS_HARD
nomask varargs void call_out( varargs mixed *args )
{
    mixed tmp, *call_outs;

    // Bei >600 Callouts alle Objekte killen, die mehr als 30 Callouts laufen
    // haben.
    if ( efun::debug_info(DINFO_DATA,DID_STATUS,DID_ST_CALLOUTS) > 600 
	 && geteuid(previous_object()) != ROOTID ) {
	// Objekte aller Callouts ermitteln
	call_outs = map( efun::call_out_info(), #'_call_out_obj );
	mapping objectmap = ([]);
	filter( call_outs, #'_same_object, &objectmap );
	// Master nicht grillen...
	efun::m_delete( objectmap, master(1) );
	// alle Objekte raussuchen, die zuviele haben...
	mapping res = filter_indices( objectmap, #'_too_many, objectmap, 29 );
	// und ueber alle Keys gehen, an _destroy() werden Key und Array mit
	// Objekten uebergeben (in diesem Fall sind Keys und Array mit
	// Objekten jeweils das gleiche Objekt).
	if ( sizeof(res) )	
	    walk_mapping(res, #'_destroy, "CALL_OUT overflow by single "	      
		"object [%O]. Destructed %d objects. [%s]\n", 0 );

	// Bei (auch nach dem ersten Aufraeumen noch) >800 Callouts alle
	// Objekte killen, die mehr als 50 Callouts laufen haben - und
	// diesmal zaehlen Clones nicht eigenstaendig! D.h. es werden alle
	// Clones einer BP gekillt, die Callouts laufen haben, falls alle
	// diese Objekte _zusammen_ mehr als 50 Callouts haben!
	if ( efun::debug_info(DINFO_DATA,DID_STATUS,DID_ST_CALLOUTS) > 800 ) {
	    // zerstoerte Objekte von der letzten Aktion sind in objectmap nicht
	    // mehr drin, da sie dort als Keys verwendet wurden.
	    mapping pathmap=([]);
	    // alle Objekt einer BP in res sortieren, BP-Name als Key, Arrays
	    // von Objekten als Werte.
	    walk_mapping( objectmap, #'_same_path, &pathmap);
	    // alle BPs (und ihre Objekte) raussuchen, die zuviele haben...
	    res = filter_indices( pathmap, #'_too_many/*'*/, pathmap, 50 );
	    // und ueber alle Keys gehen, an _destroy() werden die Clones
	    // uebergeben, die Callouts haben.
	    if ( sizeof(res) )
		walk_mapping( res, #'_destroy/*'*/, "CALL_OUT overflow by file "
			      "'%s'. Destructed %d objects. [%s]\n", 0 );

	    // Wenn beide Aufraeumarbeiten nichts gebracht haben und immer
	    // noch >1000 Callouts laufen, werden diesmal alle Callouts
	    // einer UID zusammengezaehlt.
	    // Alle Objekte einer UID, die es in Summe aller ihrer Objekt mit
	    // Callouts auf mehr als 100 Callouts bringt, werden geroestet.
	    if (efun::debug_info(DINFO_DATA,DID_STATUS,DID_ST_CALLOUTS)>1000){
		// das nach BP-Namen vorgefilterte Mapping jetzt nach UIDs
		// zusammensortieren. Zerstoerte Clones filter _same_uid()
		// raus.
		mapping uidmap=([]);
		walk_mapping( pathmap, #'_same_uid, &uidmap,
			      symbol_function( "creator_file",
					       "/secure/master" ) );
		// In res nun UIDs als Keys und Arrays von Objekten als Werte.
		// Die rausfiltern, die mehr als 100 Objekte (non-unique, d.h.
		// 100 Callouts!) haben.
		res = filter_indices( uidmap, #'_too_many, uidmap, 100 );
		// und erneut ueber die Keys laufen und jeweils die Arrays mit
		// den Objekten zur Zerstoerung an _destroy()...
		if ( sizeof(res) )
		    walk_mapping( res, #'_destroy, "CALL_OUT overflow by "
				  "UID '%s'. Destructed %d objects. [%s]\n",
				  1 );
	    }
	}
    }

    // Falls das aufrufende Objekt zerstoert wurde beim Aufraeumen
    if ( !previous_object() )
	return;

    set_this_object( previous_object() );
    apply( #'efun::call_out, args );
    return;
}
#endif // !STOP_CALL_OUTS_HARD
#endif // !TESTMUD

mixed call_out_info() {
  
  object po = previous_object();
  mixed coi = efun::call_out_info();

  // ungefilterten Output nur fuer bestimmte Objekte, Objekte in /std oder
  // /obj haben die BackboneID.
  if (query_wiz_level(getuid(po)) >= ARCH_LVL
       || (string)master()->creator_file(load_name(po)) == BACKBONEID ) {
      return coi;
  }
  else {
      return filter(coi, function mixed (mixed arr) {
              if (pointerp(arr) && arr[0]==po)
                 return 1;
              else return 0; });
  }
}

// * Zu einer closure das Objekt, an das sie gebunden ist, suchen
mixed query_closure_object(closure c) {
  return
    CLOSURE_IS_UNBOUND_LAMBDA(get_type_info(c, 1)) ?
      0 :
  (to_object(c) || -1);
}

// * Wir wollen nur EIN Argument ... ausserdem checks fuer den Netztotenraum
varargs void move_object(mixed what, mixed where)
{
  object po,tmp;

  po=previous_object();
  if (!where)
  {
    where=what;
    what=po;
  }
  if (((stringp(where) && where==NETDEAD_ROOM ) ||
       (objectp(where) && where==find_object(NETDEAD_ROOM))) &&
       objectp(what) && object_name(what)!="/obj/sperrer")
  {
    if (!query_once_interactive(what))
    {
      what->remove();
      if (what) destruct(what);
      return;
    }
    if (living(what) || interactive(what))
    {
      log_file("NDEAD2",sprintf("TRYED TO MOVE TO NETDEAD: %O\n",what));
      return;
    }
    set_object_heart_beat(what,0);
  }
  tmp=what;
  while (tmp=environment(tmp))
      // Ja. Man ruft die _set_xxx()-Funktionen eigentlich nicht direkt auf.
      // Aber das Lichtsystem ist schon *so* rechenintensiv und gerade der
      // P_LAST_CONTENT_CHANGE-Cache wird *so* oft benoetigt, dass es mir
      // da um jedes bisschen Rechenzeit geht.
      // Der Zweck heiligt ja bekanntlich die Mittel. ;-)
      //
      // Tiamak
    tmp->_set_last_content_change();
  (void)funcall(bind_lambda(#'efun::move_object,po),what,where);
  if (tmp=what)
    while (tmp=environment(tmp))
      tmp->_set_last_content_change();
}

// * additional message handling
private int living_or_interactive(object o)
{
  return (living(o) || query_once_interactive(o));
}

static int _shout_filter( object ob, string pat )
{
    string *ignore;

    if ( !environment(ob) )
	return 0;

    // shout may be ignored
    if ( member(ob->QueryProp(P_IGNORE)||({}),"shout")!=-1)
	    return 0;

    return sizeof( regexp( ({ object_name( environment(ob) ) }), pat ) );
}

varargs void shout( string s, mixed where ){
    object *u;
    string *pfade;

    if ( !sizeof( u = users() - ({ this_player(), 0 }) ) )
	return;

    if ( !where )
	pfade = ({ "/" });
    else if ( intp(where) )
	pfade =
	    ({ implode( efun::explode( object_name( environment(this_player()) ),
				       "/" )[0..2], "/" ) + "/" });
    else if ( stringp(where) )
	pfade = ({ where });
    else
	pfade = where;

    filter( filter( u, "_shout_filter", ME, implode( pfade, "|" ) ),
		  #'tell_object/*'*/, to_string(s) );
}

private nosave mapping living_name_m, name_living_m, netdead;

// * living_name-Behandlung

void start_simul_efun() {
  mixed *info;

  // Falls noch nicht getan, extra_wizinfo initialisieren
  if ( !pointerp(info = get_extra_wizinfo(0)) )
    set_extra_wizinfo(0, info = allocate(BACKBONE_WIZINFO_SIZE));
 
  // living_name ist ein Pointer der auch in der extra_wizinfo gehalten 
  // wird, so kann die simul_efun neu geladen werden, ohne dass dieses
  // Mapping verloren geht
  if (!mappingp(living_name_m = info[LIVING_NAME]))
    living_name_m = info[LIVING_NAME] = m_allocate(0, 1);

  // Gleiches gilt fuer das Mapping Name-Living
  if (!mappingp(name_living_m = info[NAME_LIVING]))
    name_living_m = info[NAME_LIVING] = m_allocate(0, 1);
  
  // Netztote sind ebenfalls in der extra_wizinfo
  if (!mappingp(netdead = info[NETDEAD_MAP]))
    netdead = info[NETDEAD_MAP] = ([]);

  // genau wie die IP-Map
  if (!mappingp(ip_map = info[IP_NAMES]))
    ip_map = info[IP_NAMES] = m_allocate(0,2);

  set_next_reset(10); // direkt mal aufraeumen
}

void __clear_ip_map()
{
  mixed *info;

  info = get_extra_wizinfo(0);
  ip_map=info[IP_NAMES]=m_allocate(0,2);
}


// * regelmaessig Listen von Ballast befreien
#define clean_log(s)
//#define clean_log(s) log_file("CLEAN_SIM",ctime(time())[4..18]+": "+(s));

protected void clean_name_living_m(string *keys, int left, int num)
{
  int i, j;
  mixed a;

  while (left && num--)
  {
    if (pointerp(a = name_living_m[keys[--left]]) && member(a, 0)>=0)
    {
      a-=({0});
      name_living_m[keys[left]] = a;
    }
    if (!a || (pointerp(a) && !sizeof(a)))
    {
      clean_log("Toasting "+keys[left]+"\n");
      efun::m_delete(name_living_m, keys[left]);
    } else clean_log(sprintf("KEEPING %s (%O)\n",keys[left],pointerp(a)?sizeof(a):a));
  }
  if (left)
    efun::call_out(#'clean_name_living_m, 1, keys, left, 80);
  else
    clean_log("Clean process finished\n");
}

protected void clean_netdead() {
  int i;
  string *s;
  object ob;

  s=m_indices(netdead);
  for (i=sizeof(s)-1;i>=0;i--)
    if (!objectp(ob=netdead[s[i]]) || interactive(ob))
      efun::m_delete(netdead,s[i]);
}

protected void reset() {
  set_next_reset(7200);
  if (find_call_out(#'clean_name_living_m) < 0) {
    clean_log("Starting clean process\n");
    efun::call_out(#'clean_name_living_m,
		   1,
		   m_indices(name_living_m),
		   sizeof(name_living_m),
		   80
		   );
  }
  efun::call_out(#'clean_netdead,2);
}

void set_object_living_name(string name, object obj)
{
  string old;
  mixed a;
  int i;

  if (previous_object()==obj || previous_object()==this_object() ||
      object_name(previous_object())==__MASTER_OBJECT__)
  {
    if(!name || !stringp(name)) {
      set_this_object(previous_object());
      raise_error(sprintf("%O: illegal living name: %O\n", obj, name));
    }
    if (old = living_name_m[obj]) {
      if (pointerp(a = name_living_m[old])) {
	a[member(a, obj)] = 0;
      } else {
	efun::m_delete(name_living_m, old);
      }
    }
    living_name_m[obj] = name;
    if (a = name_living_m[name]) {
      if (!pointerp(a)) {
	name_living_m[name] = ({a, obj});
	return;
      }
      /* Try to reallocate entry from destructed object */
      if ((i = member(a, 0)) >= 0) {
	a[i] = obj;
	return;
      }
      name_living_m[name] = a + ({obj});
      return;
    }
    name_living_m[name] = obj;
  }
}

void set_living_name(string name)
{
  set_object_living_name(name,previous_object());
}

void remove_living_name()
{
  string name;

  if (!previous_object())
    return;
  if (name=living_name_m[previous_object()])
  {
    efun::m_delete(living_name_m,previous_object());
    if (objectp(name_living_m[name]))
    {
      if (name_living_m[name]==previous_object())
	efun::m_delete(name_living_m,name);
      return;
    }
    if (pointerp(name_living_m[name]))
    {
      name_living_m[name]-=({previous_object()});
      if (!sizeof(name_living_m[name]))
	efun::m_delete(name_living_m,name);
    }
  }
}

void _set_netdead()
{
  if (query_once_interactive(previous_object()))
    netdead[getuid(previous_object())]=previous_object();
}

void _remove_netdead()
{
  efun::m_delete(netdead,getuid(previous_object()));
}

object find_netdead(string name)
{
  return netdead[name];
}

string *dump_netdead()
{
  return m_indices(netdead);
}

object find_living(string name) {
  mixed *a, r;
  int i;

  if (pointerp(r = name_living_m[name])) {
    if (member(r,0)>=0)
      r-=({0});
    if (!sizeof(r)){
      efun::m_delete(name_living_m,name);
      clean_log(sprintf("find_living loescht %s\n",name));
      return 0;
    }
    if ( !living(r = (a = r)[0])) {
      for (i = sizeof(a); --i;) {
	if (living(a[<i])) {
	  r = a[<i];
	  a[<i] = a[0];
	  return a[0] = r;
	}
      }
    }
    return r;
  }
  return living(r) && r;
}

object *find_livings(string name)
{
  mixed r;

  if (pointerp(r=name_living_m[name]))
    r-=({0});
  else
    if (objectp(r))
      r=({r});
  if (!pointerp(r)||!sizeof(r))
    return 0;
  return r;
}

object find_player(string name) {
  mixed *a, r;
  int i;

  if (pointerp(r = name_living_m[name])) {
    if (member(r,0)>=0)
      r-=({0});
    if (!sizeof(r)){
      efun::m_delete(name_living_m,name);
      clean_log(sprintf("find_player loescht %s\n",name));
      return 0;
    }
    if ( !(r = (a = r)[0]) || !query_once_interactive(r)) {
      for (i = sizeof(a); --i;) {
	if (a[<i] && query_once_interactive(a[<i])) {
	  r = a[<i];
	  a[<i] = a[0];
	  return a[0] = r;
	}
      }
    }
    return r && query_once_interactive(r) && r;
  }
  return r && query_once_interactive(r) && r;
}

private static int check_match( string str, int players_only )
{
    mixed match;

    if ( !(match = name_living_m[str]) )
	return 0;

    if ( !pointerp(match) )
	match = ({ match });

    match -= ({0});

    if ( sizeof(match) ){
	if ( players_only )
	    return sizeof(filter( match, #'query_once_interactive/*'*/ ))
		> 0;
	else
	    return 1;
    }

    efun::m_delete( name_living_m, str );
    clean_log( sprintf("check_match loescht %s\n", str) );
    return 0;
}

varargs mixed match_living( string str, int players_only, string *exclude )
{
    int i, s;
    mixed match, *user;

    if ( !str || str == "" )
	return 0;

    if ( !pointerp(exclude) )
	exclude = ({ exclude });

    if ( member(exclude, str) < 0 && check_match(str, players_only) )
	return str;

    user = m_indices(name_living_m);
    s = strlen(str);
    match = 0;

    for ( i = sizeof(user); i--; )
	if ( str == user[i][0..s-1] && member( exclude, user[i] ) < 0 )
	    if ( match )
		return -1;
	    else
		if ( check_match(user[i], players_only) )
		    match = user[i];

    if ( !match )
	return -2;

    return match;
}

mixed *dump_livings()
{
  return sort_array(m_indices(name_living_m),#'>);
}

//TODO: nach Reboot die beiden da entfernen.
#if __BOOT_TIME__ < 1233606680
#if __EFUN_DEFINED__(send_udp)
int send_udp(string host, int port, string message) {
  return efun::send_udp(host,port,message);
}
#endif

#if __EFUN_DEFINED__(send_erq)
void cindent(string name)
{
    string valid;

    valid = funcall(
      bind_lambda(#'call_other, previous_object()),
      __MASTER_OBJECT__,
      "valid_write",
      name,
#if __EFUN_DEFINED__(geteuid)
      geteuid(previous_object()),
#else
      0,
#endif
      "cindent", previous_object()
    );
    if (valid)
      send_erq(ERQ_FORK, "indent_wrapper " + (stringp(valid) ? valid : name));
}
#else
void cindent(string name)
{
  set_this_object(previous_object());
  raise_error("Sorry, no CINDENT available.\n");
}
#endif
#endif //__BOOT_TIME__

#if !__EFUN_DEFINED__(query_comm_stat)
mixed *query_comm_stat()
{
  return ({0,0});
}
#endif

#if !__EFUN_DEFINED__(set_next_reset)
void set_next_reset(int time)
{    }
#endif

#if !__EFUN_DEFINED__(absolute_hb_count)
int absolute_hb_count() {
  return efun::debug_info(DINFO_DATA,DID_STATUS,DID_ST_HBEAT_CALLS_TOTAL);
}
#endif

void __set_environment(object ob, mixed target)
{
  string path;
  object obj;

  if (!objectp(ob))
    return;
  if (!IS_ARCH(geteuid(previous_object())) || !ARCH_SECURITY )
    return;
  if (objectp(target))
  {
    efun::set_environment(ob,target);
    return;
  }
  path=(string)MASTER->_get_path(target,this_interactive());
  if (stringp(path) && file_size(path+".c")>=0 &&
      !catch(load_object(path);publish) )
  {
    obj=find_object(path);
    efun::set_environment(ob,obj);
    return;
  }
}

void _dump_wizlist(string file, int sortby) {
  int i;
  mixed *a;

  if (!LORD_SECURITY)
    return;
  if (!__MASTER_OBJECT__->valid_write(file,geteuid(previous_object()),"write_file"))
  {
    write("NO WRITE PERMISSION\n");
    return;
  }
  a = wizlist_info();
  a = sort_array(a, lambda( ({'a,'b}),
			   ({#'<,
			     ({#'[,'a,sortby}),
			     ({#'[,'b,sortby})
			    })));
  rm(file);
  for (i=sizeof(a)-1;i>=0;i--)
    write_file(file,sprintf("%-11s: eval=%-8d cmds=%-6d HBs=%-5d array=%-5d mapping=%-7d\n",
      a[i][WL_NAME],a[i][WL_TOTAL_COST],a[i][WL_COMMANDS],a[i][WL_HEART_BEATS],
      a[i][WL_ARRAY_TOTAL],a[i][WL_CALL_OUT]));
}

public varargs object deep_present(mixed what, object ob) {

  if(!objectp(ob))
    ob=previous_object();
  // Wenn ein Objekt gesucht wird: Alle Envs dieses Objekts ermitteln und
  // schauen, ob in diesen ob vorkommt. Dann ist what in ob enthalten.
  if(objectp(what)) {
    object *envs=all_environment(what);
    // wenn ob kein Environment hat, ist es offensichtlich nicht in what
    // enthalten.
    if (!pointerp(envs)) return 0;
    if (member(envs, ob) != -1) return what;
  }
  // sonst wirds teurer, ueber alle Objekte im (deep) Inv laufen und per id()
  // testen. Dabei muss aber die gewuenschte Nr. ("flasche 3") abgeschnitten
  // werden und selber gezaehlt werden, welche das entsprechende Objekt ist.
  else if (stringp(what)) {
      int cnt;
      string newwhat;
      if(sscanf(what,"%s %d",newwhat,cnt)!=2)
	cnt=1;
      else
	what=newwhat;
      foreach(object invob: deep_inventory(ob)) {
	if (invob->id(what) && !--cnt)
	    return invob;
      }
  }
  else {
    set_this_object(previous_object());
    raise_error(sprintf("Wrong argument 1 to deep_present(). "
	  "Expected \"object\" or \"string\", got %.50O.\n",
	  what));
  }
  return 0;
}

mapping dump_ip_mapping()
{
  return copy(ip_map);
}

nomask void swap(object obj)
{
  write("Your are not allowed to swap objects by hand!\n");
  return;
}

nomask varargs void garbage_collection(string str)
{
  if(previous_object()==0 || !IS_ARCH(geteuid(previous_object())) 
      || !ARCH_SECURITY)
  {
    write("Call GC now and the mud will crash in 5-6 hours. DONT DO IT!\n");
    return;
  }
  else if (stringp(str))
  {
    return efun::garbage_collection(str);
  }
  else 
    return efun::garbage_collection();
}

varargs void notify_fail(mixed nf, int prio) {
  object po,oldo;
  int oldprio;
  
  if (!PL || !objectp(po=previous_object())) return;
  if (!stringp(nf) && !closurep(nf)) {
      set_this_object(po);
      raise_error(sprintf(
	  "Only strings and closures allowed for notify_fail! "
	  "Argument was: %.50O...\n",nf));
  }

  // falls ein Objekt bereits nen notify_fail() setzte, Prioritaeten abschaetzen
  // und vergleichen.
  if (objectp(oldo=query_notify_fail(1)) && po!=oldo) {
    if (!prio) {	
      //Prioritaet dieses notify_fail() 'abschaetzen'
      if (po==PL) // Spieler-interne (soul-cmds)
        prio=NF_NL_OWN;
      else if (living(po))
        prio=NF_NL_LIVING;
      else if ((int)po->IsRoom())
        prio=NF_NL_ROOM;
      else
        prio=NF_NL_THING;
    }
    //Prioritaet des alten Setzers abschaetzen
    if (oldo==PL)
      oldprio=NF_NL_OWN;
    else if (living(oldo))
      oldprio=NF_NL_LIVING;
    else if ((int)oldo->IsRoom())
      oldprio=NF_NL_ROOM;
    else
      oldprio=NF_NL_THING;
  }
  else // wenn es noch kein Notify_fail gibt:
    oldprio=NF_NL_NONE;

  //vergleichen und ggf. setzen
  if (prio >= oldprio) { 
    set_this_object(po);
    efun::notify_fail(nf);
  }

  return;
}

void _notify_fail(string str)
{
  //query_notify_fail() benutzen, um das Objekt
  //des letzten notify_fail() zu ermitteln
  object o;
  if ((o=query_notify_fail(1)) && o!=previous_object())
    return;
  //noch kein notify_fail() fuer dieses Kommando gesetzt, auf gehts.
  set_this_object(previous_object());
  efun::notify_fail(str);
  return;
}


private object _shadow(object ob, int flag)
{
  set_this_object(previous_object(1));
  return efun::shadow(ob, flag);
}

public object shadow(object ob, int flag)
{
  object res = funcall(#'_shadow,ob, flag);
  if (flag)
    "/secure/shadowmaster"->RegisterShadow(previous_object());
  return res;
}

private void _unshadow() {
  set_this_object(previous_object(1));
  efun::unshadow();
}

public void unshadow() {
  funcall(#'_unshadow);
  "/secure/shadowmaster"->UnregisterShadow(previous_object());
}


string time2string( string format, int time )
{
  int days,hours,mins,secs,i,ch,max,abbr,dummy;
  string *parts, fmt;

  secs = time;
  mins = (time/60);
  hours = (time/3600);
  days = (time/86400);
  abbr = 0;

  parts = regexplode( format, "\(%\(-|\)[0-9]*[dhmsxDHMSX]\)|\(%%\)" );

  max = 0;
  for( i=1; i<sizeof(parts); i+=2 )
  {
    ch = parts[i][<1];
    switch( parts[i][<1] )
    {
    case 'x': case 'X':
	abbr = sscanf( parts[i], "%%%d", dummy ) && dummy==0;
	// NO break !
    case 'd': case 'D':
	max |= 7;
	break;
    case 'h': case 'H':
	max |= 3;
	break;
    case 'm': case 'M':
	max |= 1;
	break;
    }
  }
  if( max & 4 ) hours %= 24;
  if( max & 2 ) mins %= 60;
  if( max ) secs %= 60;

  for( i=1; i<sizeof(parts); i+=2 )
  {
    fmt = parts[i][0..<2];
    ch = parts[i][<1];
    if( ch=='x' )
    {
      if( days>0 ) ch='d';
      else if( hours>0 ) ch='h'; else ch = (mins>0) ? 'm' : 's';
    } else if( ch=='X' )
    {
      if( days>0 ) ch='D';
      else if( hours>0 ) ch='H'; else ch = (mins>0) ? 'M' : 'S';
    }
    switch( ch )
    {
      case 'd': parts[i] = sprintf( fmt+"d", days ); break;
      case 'h': parts[i] = sprintf( fmt+"d", hours ); break;
      case 'm': parts[i] = sprintf( fmt+"d", mins ); break;
      case 's': parts[i] = sprintf( fmt+"d", secs ); break;
      case 'D':
	if(abbr) parts[i] = "d"; else
	parts[i] = sprintf( fmt+"s", (days==1) ? "Tag" : "Tage" );
	break;
      case 'H':
	if(abbr) parts[i] = "h"; else
	parts[i] = sprintf( fmt+"s", (hours==1) ? "Stunde" : "Stunden" );
	break;
      case 'M':
	if(abbr) parts[i] = "m"; else
	parts[i] = sprintf( fmt+"s", (mins==1) ? "Minute" : "Minuten" );
	break;
      case 'S':
	if(abbr) parts[i] = "s"; else
	parts[i] = sprintf( fmt+"s", (secs==1) ? "Sekunde" : "Sekunden" );
	break;
      case '%':
	parts[i] = "%";
	break;
      }
    }
    return implode( parts, "" );
}

#if !__EFUN_DEFINED__(query_ip_port)
nomask varargs int query_ip_port(object pl)
{
  return 23;
}
#endif

nomask mixed debug_info(int flag, varargs int * args) {

//  if ( (flag == 2) &&
//       (!this_player() || !IS_ARCH(this_player()) || process_call()) ) return;

  set_this_object(previous_object());
  return(apply(#'efun::debug_info,flag, args));
}

#ifdef SOMMERZEIT
string ctime(int t) {
	return efun::ctime(t);
}
#endif

nomask mixed __create_player_dummy(string name)
{
  string err;
  object ob;
  mixed m;
  //hat nen Scherzkeks die Blueprint bewegt?
  if ((ob=find_object("/secure/login")) && environment(ob))
      catch(destruct(ob);publish);
  err = catch(ob = clone_object("secure/login");publish);
  if (err)
  {
    write("Fehler beim Laden von /secure/login.c\n"+err+"\n");
    return 0;
  }
  if (objectp(m=(mixed)ob->new_logon(name))) netdead[name]=m;
  return m;
}

nomask int secure_level()
{
  int *level;
  //kette der Caller durchlaufen, den niedrigsten Level in der Kette
  //zurueckgeben. Zerstoerte Objekte (Selbstzerstoerer) fuehren zur Rueckgabe
  //von 0.
  //caller_stack(1) fuegt dem Rueckgabearray this_interactive() hinzu bzw. 0,
  //wenn es keinen Interactive gibt. Die 0 fuehrt dann wie bei zerstoerten
  //Objekten zur Rueckgabe von 0, was gewuenscht ist, da es hier einen
  //INteractive geben muss.
  level=map(caller_stack(1),function int (object caller)
      {if (objectp(caller))
	return(query_wiz_level(geteuid(caller)));
       return(0); // kein Objekt da, 0.
      } );
  return(min(level)); //den kleinsten Wert im Array zurueckgeben (ggf. 0)
}

nomask string secure_euid()
{
  string euid;

  if (!this_interactive()) // Es muss einen interactive geben
     return 0;
  euid=geteuid(this_interactive());
  // ueber alle Caller iterieren. Wenn eines davon eine andere euid hat als
  // der Interactive und diese nicht die ROOTID ist, wird 0 zurueckgeben.
  // Ebenso, falls ein Selbstzerstoerer irgendwo in der Kette ist.
  foreach(object caller: caller_stack()) {
      if (!objectp(caller) ||
	(geteuid(caller)!=euid && geteuid(caller)!=ROOTID))
	  return 0;
  }
  return euid; // 'sichere' euid zurueckgeben
}

varargs void input_to( mixed fun, int onoff, varargs mixed *args )
{
    mixed *arr;
    mapping TN;
    int i;

    if ( !this_player() || !previous_object() )
	return;

    arr = ({ fun, onoff }) + args;

    for ( i = sizeof(arr) - 1; i > 1; i-- )
	if ( pointerp(arr[i]) )
	    arr[i] = quote(arr[i]);

    TN = (mapping) this_player()->query_telnet_neg();

    if ( mappingp(TN) && mappingp(TN["received"]) &&
	 (TN["received"][TELOPT_EOR,1] == DO) )
	funcall( bind_lambda( #'efun::binary_message/*'*/, this_player() ),
		 ({ IAC, EOR }) );

    funcall( bind_lambda( unbound_lambda( ({}),
					  ({ #'efun::input_to/*'*/ }) + arr ),
			  previous_object() ) );
}

nomask int set_light(int i)
// erhoeht das Lichtlevel eines Objekts um i
// result: das Lichtlevel innerhalb des Objekts
{
    object ob, *inv;
    int lall, light, dark, tmp;

    if (!(ob=previous_object())) return 0; // ohne das gehts nicht.

    // aus kompatibilitaetsgruenden kann man auch den Lichtlevel noch setzen
    if (i!=0) ob->SetProp(P_LIGHT, ob->QueryProp(P_LIGHT)+i);

    // Lichtberechnung findet eigentlich in der Mudlib statt.
    return (int)ob->QueryProp(P_INT_LIGHT);
}


public string iso2ascii( string str )
{
    if ( !stringp(str) || !strlen(str) )
	return "";

    str = regreplace( str, "ä", "ae", 1 );
    str = regreplace( str, "ö", "oe", 1 );
    str = regreplace( str, "ü", "ue", 1 );
    str = regreplace( str, "Ä", "Ae", 1 );
    str = regreplace( str, "Ö", "Oe", 1 );
    str = regreplace( str, "Ü", "Ue", 1 );
    str = regreplace( str, "ß", "ss", 1 );
    str = regreplace( str, "[^ -~]", "?", 1 );

    return str;
}


public varargs string CountUp( string *s, string sep, string lastsep )
{
    string ret;

    if ( !pointerp(s) )
	return "";
    
    if (!sep) sep = ", ";
    if (!lastsep) lastsep = " und ";

    switch (sizeof(s))  {
	case 0: ret=""; break;
	case 1: ret=s[0]; break;
	default:
		ret = implode(s[0..<2], sep);
		ret += lastsep + s[<1];
    }
    return ret;
}


public varargs string getuuid( object ob )
{
    mixed *ret;

    if ( !objectp(ob) )
	ob = previous_object();

    if ( !query_once_interactive(ob) )
	return getuid(ob);

    ret = (mixed)__MASTER_OBJECT__->get_userinfo( getuid(ob) );

    if ( !pointerp(ret) || sizeof(ret) < 5 )
	return getuid(ob);

    // Username + "_" + CreationDate
    return ret[0] + "_" + ret[5];
}

nomask varargs int query_next_reset(object ob) {
    
    // Typpruefung: etwas anderes als Objekte oder 0 sollen Fehler sein.
    if (ob && !objectp(ob))
      raise_error(sprintf("Bad arg 1 to query_next_reset(): got %.20O, "
	    "expected object.\n",ob));

    // Defaultobjekt PO, wenn 0 uebergeben.
    if ( !objectp(ob) )
      ob = previous_object();

    return object_info(ob,OINFO_BASIC)[OIB_NEXT_RESET];
}


#if !__EFUN_DEFINED__(copy_file)
#define MAXLEN 50000
nomask int copy_file(string source, string dest)
{

  int ptr;
  string bytes;

  set_this_object(previous_object());
  if (!strlen(source)||!strlen(dest)||source==dest||(file_size(source)==-1)||
      (!call_other(__MASTER_OBJECT__,"valid_read",source,
                   getuid(this_interactive()||
                 previous_object()),"read_file",previous_object()))||
      (!call_other(__MASTER_OBJECT__,"valid_read",source,
                   getuid(this_interactive()||
                 previous_object()),"write_file",previous_object())))
    return 1;
  switch (file_size(dest))
  {
  case -1:
    break;
  case -2:
    if (dest[<1]!='/') dest+="/";
    dest+=efun::explode(source,"/")[<1];
    if (file_size(dest)==-2) return 1;
    if (file_size(dest)!=-1) break;
  default:
    if (!rm(dest)) return 1;
    break;
  }
  do
  {
    bytes = read_bytes(source, ptr, MAXLEN); ptr += MAXLEN;
    if (!bytes) bytes="";
    write_file(dest, bytes);
  }
  while(strlen(bytes) == MAXLEN);
  return 0;
}
#endif //!__EFUN_DEFINED__(copy_file)


// ### Ersatzaufloesung in Strings ###
varargs string replace_personal(string str, mixed *obs, int caps) {
  int i;
  string *parts;

  parts = regexplode(str, "@WE[A-SU]*[0-9]");
  i = sizeof(parts);

  if (i>1) {
    int j, t;
    closure *name_cls;

    t = j = sizeof(obs);

    name_cls  =  allocate(j);
    while (j--)
      if (objectp(obs[j]))
        name_cls[j] = symbol_function("name", obs[j]);
      else if (stringp(obs[j]))
        name_cls[j] = obs[j];

    while ((i-= 2)>0) {
      int ob_nr;
      // zu ersetzendes Token in Fall und Objektindex aufspalten
      ob_nr = parts[i][<1]-'1';
      if (ob_nr<0 || ob_nr>=t) {
        set_this_object(previous_object());
        raise_error(sprintf("replace_personal: using wrong object index %d\n",
                    ob_nr));
        return implode(parts, "");
      }

      // casus kann man schon hier entscheiden
      int casus;
      string part = parts[i];
      switch (part[3]) {
        case 'R': casus = WER;    break;
        case 'S': casus = WESSEN; break;
        case 'M': casus = WEM;    break;
        case 'N': casus = WEN;    break;
        default:  continue; // passt schon jetzt nicht in das Hauptmuster
      }

      // und jetzt die einzelnen Keywords ohne fuehrendes "@WE", beendende Id
      mixed tmp;
      switch (part[3..<2]) {
        case "R": case "SSEN": case "M": case "N":               // Name
          parts[i] = funcall(name_cls[ob_nr], casus, 1);  break;
        case "RU": case "SSENU": case "MU": case "NU":           // unbestimmt
          parts[i] = funcall(name_cls[ob_nr], casus);     break;
        case "RQP": case "SSENQP": case "MQP": case "NQP":       // Pronoun
          if (objectp(tmp = obs[ob_nr]))
            parts[i] = (string)tmp->QueryPronoun(casus);
          break;
        case "RQA": case "SSENQA": case "MQA": case "NQA":       // Article
          if (objectp(tmp = obs[ob_nr]))
            tmp = (string)tmp->QueryArticle(casus, 1, 1);
          if (stringp(tmp) && !(tmp[<1]^' ')) 
            tmp = tmp[0..<2];                // Extra-Space wieder loeschen
          break;
        case "RQPPMS": case "SSENQPPMS": case "MQPPMS": case "NQPPMS":
          if (objectp(tmp = obs[ob_nr]))
            parts[i] = (string)tmp->QueryPossPronoun(MALE, casus, SINGULAR);
          break;
        case "RQPPFS": case "SSENQPPFS": case "MQPPFS": case "NQPPFS":
          if (objectp(tmp = obs[ob_nr]))
            parts[i] = (string)tmp->QueryPossPronoun(FEMALE, casus, SINGULAR);
          break;
        case "RQPPNS": case "SSENQPPNS": case "MQPPNS": case "NQPPNS":
          if (objectp(tmp = obs[ob_nr]))
            parts[i] = (string)tmp->QueryPossPronoun(NEUTER, casus, SINGULAR);
          break;
        case "RQPPMP": case "SSENQPPMP": case "MQPPMP": case "NQPPMP":
          if (objectp(tmp = obs[ob_nr]))
            parts[i] = (string)tmp->QueryPossPronoun(MALE, casus, PLURAL);
          break;
        case "RQPPFP": case "SSENQPPFP": case "MQPPFP": case "NQPPFP":
          if (objectp(tmp = obs[ob_nr]))
            parts[i] = (string)tmp->QueryPossPronoun(FEMALE, casus, PLURAL);
          break;
        case "RQPPNP": case "SSENQPPNP": case "MQPPNP": case "NQPPNP":
          if (objectp(tmp = obs[ob_nr]))
            parts[i] = (string)tmp->QueryPossPronoun(NEUTER, casus, PLURAL);
          break;
        default:
          continue;
      }
      
      // wenn tmp ein String war, weisen wir es hier pauschal zu
      if (stringp(tmp))
        parts[i] = tmp;

      // auf Wunsch wird nach Satzenden gross geschrieben
      if (caps)
        switch (parts[i-1][<2..]) {
          case ". ":  case "! ":  case "? ":
          case ".":   case "!":   case "?":
          case ".\n": case "!\n": case "?\n":
          case "\" ": case "\"\n":
            parts[i] = capitalize(parts[i]);
            break;
        }
    }
    return implode(parts, "");
  }
  return str;
}


//replacements for dropped efuns in LD
#if !__EFUN_DEFINED__(extract)
varargs string extract(string str, int from, int to) {

  if(!stringp(str)) {
    set_this_object(previous_object());
    raise_error(sprintf("Bad argument 1 to extract(): %O",str));
  }
  if (intp(from) && intp(to)) {
    if (from>=0 && to>=0)
      return(str[from .. to]);
    else if (from>=0 && to<0)
      return(str[from .. <abs(to)]);
    else if (from<0 && to>=0)
      return(str[<abs(from) .. to]);
    else
      return(str[<abs(from) .. <abs(to)]);
  }
  else if (intp(from)) {
    if (from>=0)
      return(str[from .. ]);
    else
      return(str[<abs(from) .. ]);
  }
  else {
    return(str);
  }
}
#endif // !__EFUN_DEFINED__(extract)

#if !__EFUN_DEFINED__(slice_array)
varargs mixed *slice_array(mixed array, int from, int to) {

  if(!pointerp(array)) {
    set_this_object(previous_object());
    raise_error(sprintf("Bad argument 1 to slice_array(): %O",array));
  }
  if (intp(from) && intp(to)) {
    if (from>=0 && to>=0)
      return(array[from .. to]);
    else if (from>=0 && to<0)
      return(array[from .. <abs(to)]);
    else if (from<0 && to>=0)
      return(array[<abs(from) .. to]);
    else
      return(array[<abs(from) .. <abs(to)]);
  }
  else if (intp(from)) {
    if (from>=0)
      return(array[from .. ]);
    else
      return(array[<abs(from) .. ]);
  }
  else {
    return(array);
  }
}
#endif // !__EFUN_DEFINED__(slice_array)

#if !__EFUN_DEFINED__(member_array)
int member_array(mixed item, mixed arraystring) {

  if (pointerp(arraystring)) {
    return(efun::member(arraystring,item));
  }
  else if (stringp(arraystring)) {
    return(efun::member(arraystring,to_int(item)));
  }
  else {
    set_this_object(previous_object());
    raise_error(sprintf("Bad argument 1 to member_array(): %O",arraystring));
  }
}
#endif // !__EFUN_DEFINED__(member_array)

// The digit at the i'th position is the number of bits set in 'i'.
string count_table =
    "0112122312232334122323342334344512232334233434452334344534454556";
int broken_count_bits( string s ) {
    int i, res;
    if( !stringp(s) || !(i=strlen(s)) ) return 0;
    for( ; i-->0; ) {
        // We are counting 6 bits at a time using a precompiled table.
        res += count_table[(s[i]-' ')&63]-'0';
    }
    return res;
}
    
#if !__EFUN_DEFINED__(count_bits)
int count_bits( string s ) {
    return(broken_count_bits(s));
}
#endif


// * Teile aus einem Array entfernen *** OBSOLETE
mixed *exclude_array(mixed *arr,int from,int to)
{
  if (to<from)
    to = from;
  return arr[0..from-1]+arr[to+1..];
}

#if __EFUN_DEFINED__(hash)
string md5(mixed arg, varargs mixed* iterations)
{
    if (extern_call())
         set_this_object(previous_object());

    return hash(TLS_HASH_MD5, arg, iterations...);
}
#endif //__EFUN_DEFINED__(hash)


#if __VERSION__ > "3.3.718"
// This sefun replaces the deprecated efun cat().
#define CAT_MAX_LINES 50
varargs int cat(string file, int start, int num)
{
    if (extern_call())
        set_this_object(previous_object());

    int more;

    if (num < 0 || !this_player())
        return 0;

    if (!start)
        start = 1;

    if (!num || num > CAT_MAX_LINES) {
        num = CAT_MAX_LINES;
        more = strlen(read_file(file, start+num, 1));
    }

    string txt = read_file(file, start, num);
    if (!txt)
        return 0;

    tell_object(this_player(), txt);

    if (more)
        tell_object(this_player(), "*****TRUNCATED****\n");

    return strlen(txt & "\n");
}
#undef CAT_MAX_LINES
#endif // __VERSION__ > "3.3.718"

