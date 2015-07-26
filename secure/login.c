// MorgenGrauen MUDlib
//
// login.c -- Object for players just logging in
//
// $Id: login.c 7475 2010-02-20 18:50:03Z Zesstra $

 /*
 * secure/login.c
 *
 * This object is cloned for every user trying to log in
 * We are still running root.
 *
 * login.c looks up the username in the secure/PASSWD file. If it is
 * found, the password is checked. If the user is already logged in,
 * he will be reconnected to the running object. If the other object
 * is still interactive, that will be disconnected before the user is
 * reconnected to that object.
 *
 * If the user is not in PASSWD, a new entry with level 0 is created.
 * All PASSWD writing is done in secure/master.c.
 *
 */
#pragma strict_types
#pragma no_shadow
#pragma no_inherit
#pragma verbose_errors
#pragma combine_strings
//#pragma pedantic
//#pragma range_check
#pragma warn_deprecated

#include <config.h>
#include <properties.h>
#include <moving.h>
#include "/secure/wizlevels.h"
#include <telnet.h>
#include <defines.h>

inherit "/secure/mini_telnetneg.c";

#define SSL_GRRETING "REMOTE_HOST="
#define PROXIES ({"127.0.0.1","84.16.224.224"})
#define GUESTMASTER "/secure/guestmaster"

#ifndef DEBUG
#define DEBUG(x) if(find_player("tiamak")) tell_object(find_player("tiamak"),x)
#define DEBUGM(x) if(find_player("muadib")) tell_object(find_player("muadib"),x)
#endif


/* Variables of the secure save file */
int level, loginfails, creation_date;
string password, name, shell, ep, ek, mq;
string ektips;
string fptips;
string *domains, *guilds, *uidstotakecare;

static int invis, neu;
static string loginname;
static string cap_name;
static object myself;
static string *userentry;
static string banish;
static mixed *races;
static int hc_play;
static int newbie;
static int hc_confirm;
static string realip;

// Prototypes
static void SendTelopts();
public nomask string loginname();
public nomask int logon();
static int check_illegal( string str );
static int valid_name( string str );
static int logon2( string str );
static int new_password( string str );
static int again_password( string str );
static int check_password( string str );
static void select_race();
static void ask_race_question();
static void get_race_answer( string str );
static int load_player_object( int guestflag );
static void load_player_ob_2( string obname, int guestflag );
public void create();
public string short();
public string query_real_name();
public nomask int query_prevent_shadow();
static void time_out();
public int remove();
public mixed new_logon( string str);
static mixed new_load_player_object();
static mixed new_load_player_ob_2( string obname );
static void reask_hc_question();
static void ask_hc_question();
static void get_hc_answer(string str);
static void ask_mud_played_question();
static void get_mud_played_answer(string str);

//Wir zeigen, was wir alles koennen. Zumindest so Telnet-technisch gesehen ;-)
static void SendTelopts()
{
    efun::binary_message(({
        IAC,DO,TELOPT_LINEMODE,
            IAC,WILL,TELOPT_EOR,
            IAC,DO,TELOPT_NAWS,
            IAC,DO,TELOPT_TTYPE,
            IAC,DO,TELOPT_XDISPLOC,
            IAC,DO,TELOPT_ENVIRON,
            IAC,DO,TELOPT_NEWENV,
            IAC,DO,TELOPT_TSPEED,
            }),3);

    TN=(["sent":([
                  TELOPT_LINEMODE:0;DO;0,
                  TELOPT_EOR:WILL;0;0,
                  TELOPT_NAWS:0;DO;0,
                  TELOPT_TTYPE:0;DO;0,
                  TELOPT_ENVIRON:0;DO;0,
                  TELOPT_NEWENV:0;DO;0,
                  TELOPT_XDISPLOC:0;DO;0,
                  TELOPT_TSPEED:0;DO;0,
                  ])]);
}


public nomask string loginname()
{
    return loginname ? loginname : "";
}

static int check_too_many_logons()
{
    object *u;
    string ip;

    ip = query_ip_number(this_object());
    // users() nehmen, falls nicht-interaktive Clones von login.c existieren.
    u = filter( users(), function status (object ob, string addr) {
        return object_name(ob) == "/secure/login"
               && query_ip_number(ob) == addr;
    }, ip );

    if ( sizeof(u) > 5 ){
        write( "\nEs laufen schon zu viele Anmeldungen von Deiner Adresse "
               "aus.\nProbier es bitte in ein bis zwei Minuten noch "
               "einmal.\n" );
        
        log_file( "LOGIN_DENY", sprintf( "%s: >5 Logons von %-15s (%s)\n",
                                         ctime(time())[4..15],
                                         query_ip_number(this_object()),
                                         query_ip_name(this_object()) ) );
        return 1;
    }
    else
        return 0;
}


/*
 * This is the function that gets called by /secure/master for every user
 */
public nomask int logon()
{
    loginname = "logon";
    hc_play=0;
    newbie=0;
    realip="";
    
    SendTelopts();

    if ( check_too_many_logons() ){
        destruct(this_object());
        return 0;
    }

    // ist die Verbindung schon wieder weg?
    if (objectp(this_object()) && interactive(this_object())) {
      cat( "/etc/WELCOME" );
      write( "Wie heisst Du denn (\"neu\" fuer neuen Spieler)? " );
    }

    input_to( "logon2" );
    call_out( "time_out", 120 );
    return 1;
}


static int check_too_many_from_same_ip()
{
    object *u;
    string ip;

    ip = query_ip_number(this_object());
    u = filter(users(), function status (object ob, string addr, int a) {
        return query_ip_number(ob) == addr
               && ob->QueryProp(P_AGE) < a;
    }, ip, 12*60*60); // 24h in heart_beats

    if ( sizeof(u) > 25 ){
        write( "\nDa anscheinend gerade jemand von Deiner Adresse aus "
               "versucht, das \n"MUDNAME" mit neuen Charakteren zu "
               "ueberschwemmen, werden momentan \nnur aeltere Charaktere "
               "von dieser Adresse zugelassen.\nWenn Du meinst, dass es "
               "sich um einen Fehler handelt, logg Dich bitte als \n"
               "Gast ein und sprich einen Erzmagier oder Gott an.\n" );
        
        log_file( "LOGIN_DENY", sprintf( "%s: >10 Spieler von %-15s (%s)\n",
                                         ctime(time())[4..15],
                                         query_ip_number(this_object()),
                                         query_ip_name(this_object()) ) );
        
        destruct(this_object());
        return 1;
    }
    else
        return 0;
}


static int check_illegal( string str )
{
    string res;

    res = (string)master()->QuerySBanished(query_ip_number(this_object()));

    if ( res ){
        write( res );
        log_file( "LOGIN_DENY", sprintf( "%s: %-11s %-15s (%s)\n",
                                         ctime(time())[4..15], str,
                                         query_ip_number(this_object()),
                                         query_ip_name(this_object()) ) );
        this_object()->remove();
        return 1;
    }

    return 0;
}


/*
 * Check that a player name is valid. Only allow
 * lowercase letters.
 */
static int valid_name( string str )
{
    int i;

    if ( str == "logon" ){
        write( "Der Name wuerde nur Verwirrung stiften.\n" );
        return 0;
    }

    i = strlen(str);

    if ( i > 11 ){
        write( "Dein Name ist zu lang, nimm bitte einen anderen.\n" );
        return 0;
    }

    for ( ; i--; )
        if ( str[i] < 'a' || str[i] > 'z' ) {
            write( "Unerlaubtes Zeichen '" + str[i..i] + "' im Namen: " + str
			    + "\n" );
            write( "Benutze bitte nur Buchstaben ohne Umlaute.\n" );
            return 0;
        }

    return 1;
}


static int logon2( string str )
{
    int i, arg;
    mixed txt;

    if ( !str || str == "" ){
        write( "Abbruch!\n" );
        destruct( this_object() );
        return 0;
    }

    // Unterstuetzung fuer das Mud Server Status Protocol
    // (http://tintin.sourceforge.net/mssp/)
#ifdef MSSP_SUPPORT
    if (str == "MSSP-REQUEST") {
      "/secure/misc/mssp"->print_mssp_response();
      log_file( "MSSP.log", sprintf( "%s: %-15s (%s)\n",
                                         strftime("%c"),
                                         query_ip_number(this_object()),
                                         query_ip_name(this_object()) ) );
      write( "Wie heisst Du denn (\"neu\" fuer neuen Spieler)? " );
      input_to("logon2");
      return 1;
    }
#endif

    if(strstr(str,SSL_GRRETING)==0)
    {
      if( member(PROXIES,query_ip_number(this_object()))>-1 )
      {
        realip=str[strlen(SSL_GRRETING)..];
      } // andere IPs werden einfach ignoriert. -> log/PROXY.REQ ?
      input_to( "logon2" );
      return 1;
    }
    
    if ( loginname != "logon" ) {
        log_file( "ILLEGAL", sprintf( "%s Illegal patch of login: "
                                      "loginname = %O\n",
                                      dtime(time()), loginname ) );
        destruct( this_object() );
        return 0;
    }

    cap_name = capitalize(str);
    str = lower_case(str);

    if ( str == "neu" && !neu ){
        cat( "/etc/WELCOME_NEW" );
        neu = 1;
        input_to( "logon2" );
        return 1;
    }

    if ( !valid_name(str) ){
        if ( !neu )
            write( "Wie heisst Du denn (\"neu\" fuer neuen Spieler)? " );
        else
            write( "Bitte gib Dir einen anderen Namen: " );

        input_to( "logon2" );
        return 1;
    }

    if ( sscanf( str, "gast%d", arg ) == 1 ){
        write( "Du meinst wohl 'Gast' ...\n" );
        str = "gast";
    }

    loginname = str;

    /* read the secure save file to see if character already exists */
    if ( str != "gast" &&
         !restore_object( master()->secure_savefile(str) ) ){
        object *user;

        if ( !neu ){
            write( "Es existiert kein Charakter mit diesem Namen.\n" );
            write( "Falls Du einen neuen Charakter erschaffen moechtest, "
                   "tippe bitte \"neu\" ein.\n" );

            write( "Wie heisst Du denn (\"neu\" fuer neuen Spieler)? " );
            loginname = "logon";
            input_to( "logon2" );
            return 1;
        }

        for ( i = sizeof(user = users() - ({ 0, this_object() })); i--; )
            if ( object_name(user[i])[0..12] == "/secure/login" &&
                 ((string)user[i]->loginname()) == loginname ){
                write( "Eine Anmeldung fuer diesen Namen laeuft bereits.\n" );
                destruct( this_object() );
                return 1;
            }

        // Site-Banish checken
        if ( check_illegal(str) )
            return 1;

        if ( check_too_many_from_same_ip() )
            return 1;

        /* new character */
        if ( strlen(str) < 3 ){
            write( "Der Name ist zu kurz.\nVersuch einen anderen: " );
            loginname = "logon";
            input_to( "logon2" );
            return 1;
        }

        
        if ( (txt = (string)master()->QueryBanished(str)) ){
            if ( txt != "Dieser Name ist gesperrt." )
                txt = sprintf("Hoppla - dieser Name ist reserviert oder gesperrt "
                    "(\"gebanisht\")!\nGrund: %s\n",txt);
            else
                txt = "Hoppla - dieser Name ist reserviert oder gesperrt "
                    "(\"gebanisht\")!\n";
            write( txt + "Bitte gib Dir einen anderen Namen: " );
            loginname = "logon";
            input_to( "logon2" );
            return 1;
        }

        /* Initialize the new secure savefile */
        name = str;
        password = "";
        level = 0;
        domains = ({ });
	guilds = ({ }); 
        shell = "";
        ep = "";
        ek = "";
        mq = "";
		ektips="";
		fptips="";
        creation_date = time();

        write( "Waehle ein Passwort: " );
        input_to( "new_password", 1 );
        return 1;
    }
    else {
        if ( str == "gast" ){
            if ( check_illegal(str) )
                return 1;

            load_player_object(1);
            return 1;
        }

        if ( neu ){
            write( "Es existiert bereits ein Charakter dieses Namens.\n" );
            write( "Gib Dir einen anderen Namen: " );
            loginname = "logon";
            input_to( "logon2" );
            return 1;
        }

        if ( (int)master()->check_late_player(str) )
        {
            write( "Dieser Spieler hat uns leider fuer immer verlassen.\n" );
            write( "Wie heisst Du denn (\"neu\" fuer neuen Spieler)? " );
            loginname = "logon";
            input_to( "logon2" );
            return 1;
        }

        if ( txt = (string)master()->QueryTBanished(str) ){
            write( txt );
            write( "Wie heisst Du denn (\"neu\" fuer neuen Spieler)? " );
            loginname = "logon";
            input_to( "logon2" );
            return 1;
        }

        if ( creation_date > (time() - 30*24*60*60)
             && check_too_many_from_same_ip() )
            return 1;

        write( "Schoen, dass Du wieder da bist, "+capitalize(str)+"!\n" );

        if ( !stringp(password) || password == "" ) {
            write( "Du hast KEIN PASSWORD!\n" );
            write( "Benutze den \"password\"-Befehl, um das zu aendern !\n" );
            load_player_object(0);
            return 1;
        }

        write( "Passwort: " );
        input_to( "check_password", 1 );
        return 1;
    }
}


static int new_password( string str )
{
    write( "\n" );

    if ( !str || str == "" )
        return remove();

    password = str;

    if ( !master()->good_password( str, loginname ) ){
        write( "Bitte gib ein Passwort an: " );
        input_to( "new_password", 1 );
        return 1;
    }

    write( "\nZur Erinnerung: Es ist  v e r b o t e n, andere Spieler "
           "anzugreifen!\n" );
    write( "Das gilt auch fuer Froesche, bei denen \"Ein Frosch namens "
           "XXXXX\" steht.\n\n" );
    write( "Passwort bitte nochmal eingeben: " );
    input_to( "again_password", 1 );
    return 1;
}

static int again_password( string str )
{
    write( "\n" );

    if ( str != password ){
        write( "Die Passwoerter stimmten nicht ueberein!\n" );
        destruct( this_object() );
        return 1;
    }

    while ( remove_call_out( "time_out" ) != -1 )
        ;
    call_out( "time_out", 600 );

    password = md5_crypt( password, 0 );
    save_object( SECURESAVEPATH + loginname[0..0] + "/" + loginname );
    master()->RemoveFromCache( loginname );

    load_player_object(0);
    return 1;
}

static int check_password( string str )
{
    write( "\n" );

    // Invis einloggen?
    if (strlen(str) > 1 && str[0] == '-') {
	invis = 1;
	str = str[1..];
    }

    // welcher Hash ists denn?
    if (strlen(password) > 13) {
	// MD5-Hash
	str = md5_crypt(str, password);
    }
    else if (strlen(password) > 2) {
	// Crypt-Hash
	str = crypt(str, password[0..1]);
    }
    else {
	// keiner von beiden Hashes -> ungueltiges PW
	str = 0;
    }

    if ( !stringp(str) || str != password ) {
	// Hashes stimmen nicht ueberein -> und schluss...
	write( "Falsches Passwort!\n" );
      
	log_file( (level < 60 ? "LOGINFAIL" : "ARCH/LOGINFAIL"),
	    sprintf( "PASSWORD: %-11s %s, %-15s (%s)\n",           
	              loginname, ctime(time())[4..15],
	              query_ip_number(this_object()),           
		      query_ip_name(this_object()) ), 200000 );
      
	loginfails++;
	save_object( SECURESAVEPATH + loginname[0..0] + "/" + loginname );
	master()->RemoveFromCache( loginname );
	destruct( this_object() );
	return 1;
    }

    if ( loginfails ) {
        write( loginfails + " fehlgeschlagene" + (loginfails == 1 ? "r" : "") +
               " Login-Versuch" + (loginfails == 1 ? "" : "e") +
               " seit dem letzten erfolgreichen Login.\n" );
        loginfails = 0;
    }

    save_object( SECURESAVEPATH + loginname[0..0] + "/" + loginname );
    master()->RemoveFromCache( loginname );

    load_player_object(0);
    return 1;
}


static void select_race()
{
    int i;
    string s;

    races = get_dir( "/std/shells/*.c" );

    // Mensch soll immer als erstes in der Auswahlliste stehen.
    if (member(races,"human.c")!=-1)
	    races=({"human.c"})+(races-({"human.c"}));
    
    for ( i = sizeof(races); i--; ){
        races[i] = "/std/shells/" + races[i][0..<3];
        s = 0;

        if ( catch(s = (string)call_other( races[i], "QueryAllowSelect" ); publish) 
	    || !s)
            s = 0;
        else if ( catch(s = (string)call_other( races[i], "QueryProp", P_RACE );publish) )
	    s = 0;

        if ( !strlen(s) )
            races[i..i] = ({});
        else
            races[i] = ({ races[i], s });
    }

    if ( sizeof(races) == 1 ){
        write( "Es gibt nur eine Rasse, Du hast also keine Wahl.\n" );

        shell = races[0][0];
        master()->set_player_object( loginname, shell );

        return load_player_ob_2( shell, 0 );
    }

    return ask_mud_played_question();
}

static void reask_hc_question()
{
  hc_confirm=1;
  write( break_string(
            "Du moechtest den Gefahren dieser Welt mit nur einem Leben "
            "entgegentreten. Doch bedenke wohl, dies ist ein schwerer Weg "
            "und bist Du erstmal in das Reich des Todes eingetreten, so gibt "
            "es kein Entrinnen.\n"
            "Als Neuling solltest Du Dir ueberlegen, ob Du das "
            MUDNAME" vielleicht nicht doch lieber mit der Moeglichkeit zur "
            "Wiedergeburt erkunden moechtest.\n"
            "Solltest Du Dir noch nicht ganz ueber die Folgen Deiner Entscheidung "
            "im Klaren sein, bekommst du mit \"?\" Hilfe.\n\n",78,0,BS_LEAVE_MY_LFS));

    write( "\nNun entscheide Dich, willst Du wirklich nur ein Leben?(ja,nein,?): " );

    input_to( "get_hc_answer" );
    return;
}

static void ask_hc_question()
{
  write( break_string(
            "\nDu musst Dich nun entscheiden, ob Du dieser Welt mit den Mitteln "+
            "der Wiedergeburt entgegentreten willst, oder ob Dir ein Leben "+
            "ausreicht. Triff diese Wahl sorgfaeltig, es gibt keine zweite "+
            "Wahl. Sollte Dir nicht klar sein, was diese Frage bedeutet, so "+
            "bekommst du mit \"?\" Hilfe.\n\n",78,0,BS_LEAVE_MY_LFS));

  if (newbie)
  {
	  write("Du solltest hier unbedingt \"nein\" eingeben.\n"
			  "Eine andere Wahl bringt keine Vorteile!\n");
  }
    write( "\nNun entscheide Dich, willst Du nur ein Leben?(ja,nein,?): " );

    input_to( "get_hc_answer" );
    return;
}

static void ask_mud_played_question()
{
	write(break_string(
		"\nWenn Du ein absoluter Neuling in diesem Spiel bist moechten "
		"wir Dir mit einigen Tips zu Beginn beiseite stehen.\n\n"
		"Hast Du schon einmal in einem MUD gespielt?(ja,nein): ",78,
		0,BS_LEAVE_MY_LFS));
	input_to( "get_mud_played_answer" );
	return;
}

static void ask_race_question()
{
    int i, j;

    write( break_string( "Du musst Dich jetzt entscheiden, welcher Rasse Du "
                         "in dieser Welt angehoeren moechtest. Alle Rassen "
                         "haben verschiedene Vor- und Nachteile, insgesamt "
                         "aber gleich gute Chancen. Auch das Startgebiet "
                         "haengt von der ausgewaehlten Rasse ab. Im "
                         "Normalfall kann die Rasse nicht mehr gewechselt "
                         "werden, nachdem sie einmal ausgewaehlt wurde. "
                         "Ueberlege Dir Deine Entscheidung also gut. Derzeit "
                         "stehen folgende Rassen zur Auswahl:\n\n", 78 ) );

    for ( i = 0, j = sizeof(races); i < j; i++ )
        printf( "% 2d. %-30s   %s", i+1, capitalize(races[i][1]),
                (i % 2 ? "\n" : "| ") );

    if ( sizeof(races) % 2 )
        write( "\n" );

    write( break_string( "\nDurch Eingabe einer Ziffer waehlst Du die Rasse "
                         "aus, durch Eingabe eines \"\?\" gefolgt von einer "
                         "Ziffer erhaelst Du naehere Informationen ueber "
                         "eine Rasse. Ein \"\?\" allein wiederholt diese "
                         "Liste.", 78, 0, 1 ) );

    if (newbie)
    {
	    write(break_string("\nAls Neuling solltest Du Dich NICHT fuer "
				    "die Dunkelelfen entscheiden. Diese "
				    "Rasse hat einige Probleme im Umgang "
				    "mit den anderen Rassen und mit dem "
				    "Sonnenlicht.",78,0,BS_LEAVE_MY_LFS));
    }
    write( "\nWas willst Du tun: " );

    input_to( "get_race_answer" );
    return;
}


static void get_race_answer( string str )
{
    int num;

    if ( str == "?" )
        return ask_race_question();

    if ( sscanf( str, "?%d", num ) ){
        if ( num < 1 || num > sizeof(races) ){
            write( "Das geht nicht.\n\nWas willst Du tun: " );
            input_to( "get_race_answer" );
            return;
        }

        write( call_other( races[num - 1][0], "QueryProp", P_RACE_DESCRIPTION )
               + "\nWas willst Du tun: " );
        input_to( "get_race_answer" );
        return;
    }

    if ( sscanf( str, "%d", num ) && num >= 1 && num <= sizeof(races) ){
        write( "Ok, Du bist jetzt ein " + capitalize(races[num-1][1]) + "\n" );

        shell = races[num-1][0];
        hc_confirm=0;
        return ask_hc_question();
    }

    write(" Wie bitte?\n\nWas willst Du tun: " );
    input_to( "get_race_answer" );
}

static void get_mud_played_answer (string str)
{
	if ( str == "ja" || str=="j")
		{
		newbie=0;
		return ask_race_question();
		}
	if ( str != "nein" && str!="n")
	{
		write("\n\nAntworte bitte mit ja oder nein.\n\n");
				
		return ask_mud_played_question();
	}
        newbie=1;
	write("\n\nEine kleine Einfuehrung in das "MUDNAME" bekommst "
			"Du auch hier:\n\n"
			"http://mg.mud.de/newweb/hilfe/tutorial/inhalt.shtml\n\n");
	return ask_race_question();
	
}

static void get_hc_answer( string str )
{

    if ( str == "?" )
    {
      write( break_string(
            "Stirbst Du, so wirst Du normalerweise wiedergeboren und kannst "+
            "mit dem Spielen fortfahren. Waehlst Du hier nur ein Leben, so ist "+
            "der Tod das Ende des Spielens. Als Neuling solltest Du diese Frage "+
            "mit \"nein\" beantworten!\n\n",78,0,BS_LEAVE_MY_LFS));
        return ask_hc_question();
    }
    
    str=lower_case(str);
    if (str=="ja" || str=="j" )
    {
      if(hc_confirm==0)
      {
        return reask_hc_question();
      }
      else
      {
        hc_play=1;
        write( "Ok, Du hast nun nur ein Leben.\n" );
      }
    }
    else if (str=="nein" || str="n" )
    {
      hc_play=0;
      write( "Ok, Du erfaehrst die Gnade der Wiedergeburt.\n" );
    }
    else
    {
      write("Was willst Du tun?\n");
      return ask_hc_question();
    }


    master()->set_player_object( loginname, shell );
    load_player_ob_2( shell, 0 );
    return;
}


static int load_player_object( int guestflag )
{
    object ob;
    string fname;
    int was_interactive;

    if ( sizeof(users()) >= 195 && !IS_WIZARD(loginname) ){
        write( "Die maximale Spielerzahl wurde bereits erreicht!!!\n"
               "Aus technischen Gruenden duerfen sich nur noch Magier "
               "einloggen.\nVersuch es spaeter noch einmal ...\n" );
        destruct( this_object() );
        return 1;
    }
    else if ( sizeof(users()) >= 198 && !IS_ARCH(loginname) ){
        write( "Die maximale Spieler- und Magierzahl wurde bereits erreicht!!!"
               "\nAus technischen Gruenden duerfen sich nur noch Erzmagier "
               "einloggen.\nVersuch es spaeter noch einmal ...\n" );
        destruct( this_object() );
        return 1;
    }

    if ( file_size("/etc/NOLOGIN")>=0 )
    {
      if (file_size("/etc/NOLOGIN.info")>0) {
        //NOLOGIN.info enthaelt evtl. weitergehende Informationen fuer 
        //Spieler, z.B. vorrauss. Wiederverfuegbarkeit.
        write(break_string(read_file("/etc/NOLOGIN.info"),78,"",
              BS_LEAVE_MY_LFS|BS_SINGLE_SPACE));
      }
      else {
        //sonst Standardmeldung ausgeben.
        write ("\nAufgrund von technischen Problemen ist das Einloggen ins "
                MUDNAME" zur \nZeit nicht moeglich. Bitte versuch es "
	              "spaeter noch einmal.\n\n");
      }
      if ( IS_ARCH(loginname) || 
           member(explode(read_file("/etc/NOLOGIN")||"","\n"),
                  loginname)!=-1 )
      {
	write("Im Moment koennen nur Erzmagier einloggen. Um Spieler "
              "wieder ins Spiel zu lassen, muss die Datei '/etc/NOLOGIN' "
	      "geloescht werden.\n\n ");
      } else {
        destruct( this_object() );
        return 1;			
      }
    }

    if ( (fname = (string)master()->secure_isavefile(loginname)) != "" ) {
        save_object( SECURESAVEPATH + loginname[0..0] + "/" + loginname );
        "/secure/master"->RemoveFromCache( loginname );

        // Just to be sure ...
        rm( fname + ".o" );

        log_file( "REACTIVATE", ctime(time()) + ": " + capitalize(loginname) +
                  " reactivated\n" );
    }

    if ( guestflag ){
        if ( catch(guestflag = (int)GUESTMASTER->new_guest();publish) || !guestflag ){
            write( "Derzeit kein Gastlogin moeglich!\n" );
            destruct( this_object() );
        }

        loginname = "gast" + guestflag;
        cap_name = capitalize(loginname);
        name = cap_name;

        if ( !(ob = find_player(loginname) || find_netdead(loginname)) ){
            object *user;
            int i;

            // gegen Horden von Gast1 - wenn ein Gast noch am Prompt fuer
            // das Geschlecht haengt, ist er ueber find_player() noch nicht
            // zu finden ...
            for ( i = sizeof(user = users() - ({ 0, this_object() })); i--; )
                if ( object_name(user[i])[0..11] == "/std/shells/" &&
                     getuid(user[i]) == loginname ){
                    ob = user[i];
                    break;
                }
        }

        if ( ob ){
            tell_object( ob, "Ein anderer Spieler moechte diesen Gastzugang "
                         "jetzt benutzen. Wenn es Dir hier\ngefallen hat, "
                         "ueberleg Dir doch einen Charakternamen und komm "
                         "unter diesem\nNamen wieder!\n" );
            destruct(ob);
        }

        load_player_ob_2( "std/shells/human", guestflag );

        return 1;
    }
    else {
        /* Test if we are already playing */
        was_interactive = 0;
        ob = find_player(loginname) || find_netdead(loginname);

        if (ob) {
            write( "Du nimmst schon am Spiel teil!\n" );
            write( "Verwende Deine alte sterbliche Huelle ...\n" );

            if ( interactive(ob) ) {
                /* The other object is still interactive; reconnect that "soul"
                   to a dummy object and destruct that, thus disconnecting the
                   other probably linkdead user. The real "body" is still
                   there for reconnecting by login.c */
                remove_interactive(ob);
                was_interactive = 1;
            }

            /* Now reconnect to the old body */
            exec( ob, this_object() );
	    /* NewbieIntroMsg? */
	    ob->NewbieIntroMsg();
	    ob->set_realip(realip);
            if ( ((int)ob->QueryProp(P_LEVEL)) == -1 )
                ob->start_player( cap_name );
            else
                ob->Reconnect( was_interactive );

            call_out( "remove", 1 );
            return 1;
        }
    }

    /* read player object from passwd file */
    if ( stringp(shell) && shell != "" )
        load_player_ob_2 ( shell, 0 );
    else
        select_race();

    return 1;
}


static void load_player_ob_2( string obname, int guestflag )
{
    object blueprint;
    string err, ob_name;
    object ob, old_ob;

    /* start player activity */
    log_file( "ENTER", sprintf( "%-11s %s, %-15s (%s).\n",
                                capitalize(name), ctime(time())[4..15],
                                query_ip_number(this_object()),
                                query_ip_name(this_object()) ), 200000 );

    seteuid(loginname);

    /* load the "real" player object */
    /* If some asshole has moved the blueprint */
    if ( objectp(blueprint = find_object(obname))  && environment(blueprint) )
        destruct(blueprint);

    if ( err = catch(ob = clone_object(obname);publish) ){
        log_file( "SHELLS", "Failed to load shell " + obname + ", " +
                  dtime(time()) + ", " + loginname + "\n" + err + "\n\n" );

        write( "Konnte das passende Playerobjekt nicht laden. Lade "
               "stattdessen\ndas Objekt Mensch. BITTE ERZMAGIER "
               "BENACHRICHIGEN !\n" );
        err = catch(ob = clone_object("std/shells/human");publish);
    }

    if ( !ob || err ) {
        write( "Error on loading " + shell + "\nError = " + err + "\n" );
        destruct( this_object() );
        return;
    }

    if ( guestflag )
        catch( GUESTMASTER->set_guest( guestflag, ob );publish );

    ob_name = explode( object_name(ob), "#" )[0];

    if ( strlen(ob_name) > 11 && ob_name[0..11] == "/std/shells/" )
        ob_name = ob_name[11..];

    ob_name = ob_name + ":" + lower_case(cap_name);

  if( !guestflag ){
      if ( old_ob = find_object(ob_name) ){
          catch(old_ob->remove();publish);

          if ( old_ob )
              destruct( old_ob );
      }

      rename_object( ob, ob_name );
      ob->__reload_explore();
  }

  if(hc_play)
  {
    ob->set_hc_play(cap_name,1);
    log_file("HCPLAY",dtime(time())+" "+cap_name+" ist sehr mutig!\n");
  }
  
  exec( ob, this_object() );
  ob->set_realip(realip);
  ob->start_player( cap_name );
  ob->SetProp( "creation_date", creation_date );
  ob->Set( "creation_date", SAVE|SECURED|PROTECTED, F_MODE_AS );

  if ( invis && IS_WIZARD(ob) ){
      ob->SetProp( P_INVIS, ob->QueryProp(P_AGE) );
      tell_object( ob, "DU BIST UNSICHTBAR!\n" );
  }
  destruct( this_object() );
}


/*
 *   With arg = 0 this function should only be entered once!
 */
public void create()
{
    if( myself )
        return;

    loginname = "logon";
    hc_play=0;
    creation_date = -1;
    catch( load_object( "/secure/merlin");publish );
    loginfails = 0;
    realip="";
}


public string short()
{
    return "<Einloggender Teilnehmer>.\n";
}


public string query_real_name()
{
    return "<logon>";
}


public nomask int query_prevent_shadow()
{
    return 1;
}


static void time_out()
{
    if (this_player())
      tell_object(this_player(),"Time out!");
    destruct( this_object() );
}


public int remove()
{
    destruct( this_object() );
    return 1;
}


public mixed new_logon( string str)
{
    seteuid(getuid()); // sonst funkt ARCH_SECURITY nicht

    if ( !ARCH_SECURITY || process_call() ){
        write( "Nur fuer Erzmagier erlaubt!\n" );
        destruct( this_object() );
        return -1;
    }

    if ( !str || str == "" ){
        write( "Kein Name angegeben!\n" );
        destruct( this_object() );
        return 0;
    }

    cap_name = capitalize(str);
    str = lower_case(str);
    loginname = str;
    seteuid(ROOTID);

    /* read the secure save file to see if character already exists */
    if ( !restore_object( master()->secure_savefile(str) ) ){
        write( "Kein solcher Spieler!\n" );
        destruct( this_object() );
        return 0;
    }
    else {
        write( "Ok, der Spieler " + capitalize(str) + " existiert!\n" );
        return new_load_player_object();
    }
}


static mixed new_load_player_object()
{
    if ( find_player(loginname) || find_netdead(loginname) ){
        write( "Der Spieler ist bereits online oder netztot!\n" );
        destruct( this_object() );
        return 2;
    }

    /* read player object from passwd file */
    if ( stringp(shell) && shell != "" )
        return new_load_player_ob_2( shell );
    else {
        write( "Keine Shell angegeben!\n" );
        destruct( this_object() );
        return 0;
    }
}


static mixed new_load_player_ob_2( string obname )
{
    object blueprint;
    string err, ob_name;
    object ob, old_ob;

    seteuid(loginname);

    /* load the "real" player object */
    /* If some asshole has moved the blueprint */
    if ( objectp(blueprint = find_object(obname)) && environment(blueprint) )
        destruct( blueprint );

    err = catch(ob = clone_object(obname);publish);

    if ( err ){
        log_file( "SHELLS", "Failed to load shell " + obname + ", " +
                  dtime(time()) + ", " + loginname + "\n" + err + "\n\n" );

        write( "Konnte das passende Playerobjekt nicht laden. "
               "Lade stattdessen\ndas Objekt Mensch!\n" );

        err = catch(ob = clone_object( "std/shells/human" );publish );
    }

    if ( !ob || err ){
        write( "Error on loading " + shell + "\nError = " + err + "\n" );
        destruct( this_object() );
        return 0;
    }

    ob_name = explode( object_name(ob), "#" )[0];

    if ( strlen(ob_name) > 11 && ob_name[0..11] == "/std/shells/" )
        ob_name = ob_name[11..];

    ob_name = ob_name + ":" + lower_case(cap_name);

    if ( old_ob = find_object(ob_name) ){
        catch( old_ob->remove(); publish );

        if ( old_ob )
            destruct( old_ob );
    }

    rename_object( ob, ob_name );
    ob->__reload_explore();
    ob->set_realip(realip);
    ob->start_player(cap_name);
    ob->SetProp( "creation_date", creation_date );
    ob->Set( "creation_date", SAVE|SECURED|PROTECTED, F_MODE_AS );

    ob->move( "/room/nowhere", M_NOCHECK );
    set_object_heart_beat( ob, 0 );
    destruct( this_object() );

    return ob;
}

string query_realip()
{
  return realip ? realip : "";
}
