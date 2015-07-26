// MorgenGrauen MUDlib
//
// death_room.c -- Der Todesraum
//
// $Id: death_room.c 7314 2009-09-28 13:28:14Z Zesstra $


#pragma strict_types

#include <defines.h>
#include <properties.h>
#include <moving.h>
#include <language.h>
#include <wizlevels.h>
#include <daemon.h>
#include <new_skills.h>

inherit "/std/room";

mixed *players;
mapping msgCache;


private static void flush( int unusedOnly );
private static string expand( string table, int value );
private static string parseText( string msg, object pl );
private static void do_remove();
private static varargs mixed get_sequence( string str );
void add_player( object pl );
static int filter_ldfied( string str );
public int SmartLog( string creat, string myname, string str, string date );
private static void wech_damit( object ob );
public mixed hier_geblieben( mixed dest, int methods, string direction,
                             string textout, string textin );
public void init()
{
  this_player()->move("/room/death/virtual/death_room_"+getuid(this_player()),
	     M_NOCHECK|M_SILENT|M_NO_SHOW);
  return;
}

public void create()
{
  if (IS_CLONE(this_object())) return;
    ::create();
    
    players = ({});
    flush(0);

    SetProp( P_NAME, "Lars" );
    SetProp( P_GENDER, MALE );
    SetProp( P_ARTICLE, 0 );
    SetProp( P_LIGHT,1 );
    SetProp( P_LOG_FILE, "TOD/Todesraum" );
    SetProp( P_INT_SHORT, "Arbeitszimmer des Todes" );
    SetProp( P_INT_LONG, break_string(
             "Ein dunkler Raum, erleuchtet von dunklem Licht, das sich der "
             "Dunkelheit nicht so sehr zu widersetzen scheint, indem es "
             "leuchtet, als dass es der dunkelste Punkt in einer weniger "
             "dunklen Umgebung ist. Im seltsamen Licht erkennst Du einen "+
             "zentral aufgestellten Schreibtisch, der mit Diagrammen und "
             "Buechern bedeckt ist. Die Waende verschwinden hinter Regalen, "
             "die gefuellt sind mit in Leder gebundenen, dunklen Waelzern, "
             "von denen geheimnisvolle Runen leuchten.\nTod.", 78, 0, 1 ) );
}


public void reset()
{
    ::reset();
    flush(1);
}

private static void flush( int unusedOnly )
{
    string *mi;
    int i;

    if ( unusedOnly ){
        if ( i = sizeof(mi = m_indices(msgCache)) ){
            for ( ; i--; )
                if ( msgCache[mi[i], 1] )
                    msgCache[mi[i], 1] = 0;
                else
                    msgCache = m_delete( msgCache, mi[i] );
        }
    }
    else
        msgCache = ([]);
}


private static string expand( string table, int value )
{
    int sz, wert, i;
    string *texte;

    sz = sizeof( texte = explode( table, "##" ) - ({""}) );
    
    for ( i = 0; i < sz; i++ )
        if ( i%2 ){
            sscanf( texte[i], "%d", wert );
            
            if ( value < wert )
                break;
        }
        else
            table = texte[i];

    return table;
}


#define TOS(s) s[<1]
#define STOS(s) s[<2]
#define PUSH(x,s) (s+= ({ x }))
#define POP(s)	(s=s[0..<2])

// ziemlich vom htmld abgekupfert ;)
private static string
parseText( string msg, object pl ) 
{
  string *words, *texte, *todo, *done, tmp, cmd;
  int i, k, sz;
  int endFlag;

  sz = sizeof( words = regexplode(msg, "[<][^>]*[>]") );
  todo = ({ });
  done = ({""});

  for ( i=1; i<sz; i+=2 ){
      cmd = words[i][1..<2];
      TOS(done) += words[i-1];

      if ( cmd[0] == '/' ){
          endFlag = 1;
          cmd = cmd[1..];
      }
      else
          endFlag = 0;
      
      switch( cmd[0] ){
      case 'A': /*** Alignment ersetzen ***/
          if (!endFlag){
              PUSH( cmd, todo );
              PUSH( "", done );
          }
          else
              if ( todo[<1] == "A" ){
                  STOS(done) += expand(TOS(done), (int) pl->QueryProp(P_ALIGN));
                  done = done[0..<2];
                  todo = todo[0..<2];
              }
          break;
          
      case 'D': /*** Tode ersetzen ***/
          if ( !endFlag ){
              PUSH( cmd, todo );
              PUSH( "", done );
          }
          else
              if ( todo[<1] == "D" ){
                  STOS(done) += expand(TOS(done), (int) pl->QueryProp(P_DEADS));
                  POP(done);
                  POP(todo);
              }
          break;
          
      case 'L': /*** Level ersetzen ***/
          if ( !endFlag ){
              PUSH( cmd, todo );
              PUSH( "", done );
          }
          else
              if ( todo[<1] == "L" ){
                  STOS(done) += expand(TOS(done), (int) pl->QueryProp(P_LEVEL));
                  POP(done);
                  POP(todo);
              }
          break;
          
      case 'Z': /*** Zufall ersetzen ***/
          if ( !endFlag ){
              PUSH( cmd, todo );
              PUSH( "", done );
          }
          else{
              if ( todo[<1][0] == 'Z'){
                  int cnt, rnd, wert, sz2;
                  
                  if ( !sscanf(todo[<1], "Z=%d", rnd) )
                      STOS(done) += "\n###\n### Syntax Error in <Z>!\n###\n\n";
                  else {
                      rnd = random(rnd);
                      sz2 = sizeof( texte = explode(TOS(done), "##") );
                      wert=0;
                      cnt=0;
                      
                      for ( k = 1; k < sz2; k += 2 ){
                          sscanf( texte[k], "%d", wert );
                          cnt += wert;
                          if ( rnd < cnt ) {
                              STOS(done) += texte[k+1];
                              break;
                          }
                      }
                  }
                  POP(done);
                  POP(todo);
              }
          }
          break;

      case 'G': /*** Gender ersetzen ***/
          if ( !endFlag ){
              PUSH( cmd, todo );
              PUSH( "", done );
          }
          else{
              if( sizeof( texte = regexplode(TOS(done), ":") ) == 3 )
                  STOS(done) += texte[2*((int) pl->QueryProp(P_GENDER)
                                         == FEMALE)];
              POP(done);
              POP(todo);
          }
          break;

      case 'R': /*** Rasse ersetzen ***/
          if ( !endFlag ){
              PUSH( cmd, todo );
              PUSH( "", done );
          }
          else{
              int race;

              texte = regexplode( TOS(done), "\\|" );
              race = 2 * (member( ({ "Mensch", "Elf", "Zwerg", "Hobbit",
                                         "Feline", "Dunkelelf" }),
                                  (string) pl->QueryProp(P_RACE) ) + 1);
              
              if ( race >= sizeof(texte) )
                  race = 0;

              STOS(done) += texte[race];
              POP(done);
              POP(todo);
          }
          break;

      case 'n': /*** Name, normal geschrieben ***/
          TOS(done) += (string) (pl->name(RAW));
          break;

      case 'N': /*** Name, in Grossbuchstaben ***/
          TOS(done) += (string) upperstring(pl->name(RAW));
          break;
      }
  }
  PUSH( words[<1], done );
  return implode( done, "" );
}


public void heart_beat()
{
    int j, nr;
    string msg;

    j=0;

    for ( j = sizeof(players); j--; )
        if ( !objectp(players[j][0]) ||
             environment(players[j][0]) != 
	     find_object("/room/death/virtual/death_room_"+getuid(players[j][0])) )
            players[j] = 0;

    players -= ({0});

    if ( !sizeof(players) ) {
        set_heart_beat(0);
        return;
    }

  for ( j = sizeof(players); j--; ){
      nr = ++players[j][1];

      if ( mappingp(players[j][2]) )
          msg = players[j][2][nr];
      else
          msg = 0;
      
      if ( !msg )
          msg = players[j][3][1][nr];
      
      if ( msg )
          tell_object( players[j][0], parseText( msg, players[j][0] ) );
  }
  
  do_remove();
}

private static void
do_remove()
{
    int j, res;
    string prayroom;
    object plobj, pl;
    
    for ( j = sizeof(players); j--; ){
        if ( players[j][1] >= players[j][3][0]){
            pl = players[j][0];
            while ( plobj = present("\ndeath_mark", pl) )
                plobj->remove();
            
            if ( !(prayroom = (string) pl->QueryPrayRoom()) )
                prayroom="/room/pray_room";

            pl->Set( P_TMP_MOVE_HOOK, 0 );
            pl->Set( P_NO_ATTACK, 0, F_QUERY_METHOD );
            pl->Set( P_LAST_KILLER, 0 );
            pl->Set( P_KILLER, 0 );
            pl->Set( P_ENEMY_DEATH_SEQUENCE, 0 );
            pl->Set( P_POISON, 0, F_QUERY_METHOD );
            
            if ( catch( res = (int) pl->move(prayroom, M_GO|M_SILENT|M_NOCHECK) )
                 || res < 1 )
                pl->move( "/room/pray_room", M_GO|M_NOCHECK );

            players[j] = 0;
        }
    }

    players -= ({0});

    if ( !sizeof(players) )
        set_heart_beat(0);
}

private static varargs mixed
get_sequence( string str )
{
    string *sequences, seq;
    int i, len, cacheable;
    mapping m;
    
    if ( !stringp(str) || catch( len = file_size(str) ) || len <= 0 ){
        sequences = get_dir( "/room/death/sequences/*" ) - ({ ".", "..", ".svn" });
        str = "/room/death/sequences/" + sequences[random( sizeof(sequences) )];
    }

    if ( cacheable = ((strlen(str) > 21) &&
                      (str[0..21] == "/room/death/sequences/")) ){
        if ( member(msgCache, str) ){
            msgCache[str, 1] = 1;  // Touch it!
            return ({ msgCache[str], str });
        }
    }
    
    sequences = explode( read_file(str), "\n" );
    sscanf( sequences[0], "%d", len );
    seq = implode( sequences[1..], "\n" );
    sequences = regexplode( seq, "[0-9][0-9]*:" );
    m = ([]);
    
    for ( i = 1; i < sizeof(sequences)-1; i += 2 )
        m[(int) sequences[i]] = sequences[i+1];

    if ( cacheable )
        msgCache += ([ str: ({ len, m }); 1 ]);

    return ({ ({ len, m }), str });
}

void add_player( object pl )
  {
// Description:   Adds a player to the list
    int i, kart, kgen;
    int escaped,magiertestie;
    object pre, kill_liv, kill_ob;
    mixed dseq, act_seq, killer_name, killer_msg;
    string fn, kanal,testplayer;
 
    set_heart_beat(1);
    kill_liv = 0;
    kill_ob = 0;
    dseq = 0;
    kart = 0;
    kgen = MALE;

    foreach(pre : caller_stack(1)) { 
        if ( !objectp(pre) || pre == pl )
            continue;

        fn = object_name(pre);

        if ( fn[0..12] == "/secure/login" && !kill_liv ){
            escaped = 1;
            break;
        }

        if ( fn[0..7] == "/secure/" && fn[0..13] != "/secure/merlin" )
            continue;

        if ( fn[0..21] == "/room/death/death_mark" )
            continue;

        if ( living(pre) ){
            kill_liv = pre; // Killer
            break;
        }

        kill_ob = pre; // killendes Objekt
    }

    if ( objectp( pre = ((object) pl->QueryProp(P_KILLER)) ) ){
        dseq = (mixed) pre->QueryProp(P_ENEMY_DEATH_SEQUENCE);

        if( !(killer_name = (mixed) pre->QueryProp(P_KILL_NAME)) ){
            killer_name = (mixed) pre->QueryProp(P_NAME);
            kart = (int) pre->QueryProp(P_ARTICLE);
            kgen = (int) pre->QueryProp(P_GENDER);
        }

        killer_msg = (mixed)pre->QueryProp(P_KILL_MSG);  
    }

    if ( !killer_name && kill_liv && function_exists( "QueryProp", kill_liv ) ){
        dseq = (mixed) kill_liv->QueryProp(P_ENEMY_DEATH_SEQUENCE);

        if( !(killer_name = (mixed) kill_liv->QueryProp(P_KILL_NAME)) ){
            killer_name = (mixed) kill_liv->QueryProp(P_NAME);
            kart = (int) kill_liv->QueryProp(P_ARTICLE);
            kgen = (int) kill_liv->QueryProp(P_GENDER);
        }

        killer_msg = (mixed) kill_liv->QueryProp(P_KILL_MSG);
        pre = kill_liv;
    }

    if ( !killer_name && kill_ob && function_exists( "QueryProp", kill_ob ) ){
        dseq = (mixed) kill_ob->QueryProp(P_ENEMY_DEATH_SEQUENCE);

        if( !(killer_name = (mixed) kill_ob->QueryProp(P_KILL_NAME)) ){
            killer_name = (mixed) kill_ob->QueryProp(P_NAME);
            kart = (int) kill_ob->QueryProp(P_ARTICLE);
            kgen = (int) kill_ob->QueryProp(P_GENDER);
        }

        killer_msg = (mixed) kill_ob->QueryProp(P_KILL_MSG);
        pre = kill_ob;
   }

    act_seq = 0;

    if ( mappingp(dseq) )
        act_seq = get_sequence( "/room/death/sequences/lars" );
    else if ( pointerp(dseq) )  // ganze Todessequenz...
        act_seq = ({ dseq, 0 });
    else if ( stringp(dseq) )
        act_seq = get_sequence(dseq);

    if(pl->query_hc_play()>1)
    {
        act_seq=({({22,([1:"Du faellst und faellst...\n",
                        5:"und faellst...\n",
                        10:"und faellst...\n",
                        12:"direkt in die Arme von TOD.\n",
                        14:"Triumphierend laechelt er Dich an.\n",
                        16:"NUN GEHOERST DU FUER IMMER MIR!\n",
                        18:"HAHHHAHAHAAAAAAAAAAHAAAAAAAAA!\n",
                        20:"TOD schlaegt Dir mit seiner Sense den Kopf ab.\n"])}),0});
    }
    if ( !act_seq )
        act_seq = get_sequence();

    if ( !mappingp(dseq) )
        dseq = 0;

    for ( i = sizeof(players); i--; )
        if ( players[i][0] == pl )
            break;

    if ( i == -1 )
        players += ({ ({ pl, 0, dseq, act_seq[0], act_seq[1], pre }) });
    else
        players[i][5] = pre;


        if ( escaped ){
            killer_name = "";
            killer_msg = upperstring(getuid(pl)) + " VERSUCHTE, MIR ZU "
                "ENTKOMMEN - JETZT HABE ICH WIEDER EXTRA-ARBEIT MIT "+
                ((int) pl->QueryProp(P_GENDER) != 2 ? "IHM" : "IHR") +
                " ...";
        }
        else if ( !killer_name ) {
                if ( (string) pl->QueryProp(P_KILLER) == "gift" ){
                    killer_name = "Vergiftung";
                    kgen = FEMALE;
                    kart = 1;
                }
                else{
                    killer_name = "Etwas Geheimnisvolles und Unbekanntes";
                    kgen = NEUTER;
                    kart = 0;
                }
        }

        if ( !pointerp(killer_msg) )
            killer_msg = ({ killer_msg, 0, 0 });
        else if ( sizeof(killer_msg) < 3 )
            killer_msg += ({ 0, 0, 0 });

        if ( stringp(killer_msg[0]) )
            killer_msg[0] = sprintf( killer_msg[0], capitalize(getuid(pl)) );

        SetProp( P_NAME, killer_name );
        SetProp( P_ARTICLE, kart );
        SetProp( P_GENDER, kgen );
        fn = Name(WER);
        SetProp( P_NAME, "Lars" );
        SetProp( P_ARTICLE, 0 );
        SetProp( P_GENDER,MALE );

        testplayer=(string) pl->QueryProp(P_TESTPLAYER);
        if (strlen(testplayer))
        {
          if (testplayer[<5..<1]=="Gilde")
            magiertestie=0;
          else 
            magiertestie=1;
        }
        else 
          magiertestie=0;
        if  (magiertestie || IS_LEARNING(pl))
            kanal = "TdT";
        else
            kanal = "Tod";

        CHMASTER->join( kanal, this_object() );

        if ( (!stringp(killer_name) || killer_name != "") &&
             (sizeof(killer_msg) < 4 || !killer_msg[3]) ){
            if ( killer_msg[2] == PLURAL )
                CHMASTER->send( kanal, this_object(),
                                fn + " haben gerade " +
                                capitalize(getuid(pl)) + " umgebracht." );
            else
                CHMASTER->send( kanal, this_object(),
                                fn + " hat gerade " +
                                capitalize(getuid(pl)) + " umgebracht." );
        }

        i = (int) pl->QueryProp(P_DEADS);
        if ( i && (getuid(pl) == "key" || i%100 == 0 || i%250 == 0) ){
            SetProp( P_NAME, "Tod" );
            CHMASTER->send( kanal, this_object(),
                            sprintf( "DAS WAR SCHON DAS %dTE MAL!", i ) );
            SetProp( P_NAME, "Lars" );
        }

        if( killer_msg[0] ){
            if ( stringp(killer_name) && killer_name == "" ){
                CHMASTER->send( kanal, this_object(),
                                break_string( funcall(killer_msg[0]), 78,
                                              "["+kanal+":] " )[0..<2],
                                MSG_EMPTY );
                return; 
            }
            else {
                if ( (killer_msg[1] < MSG_SAY) || (killer_msg[1] > MSG_GEMOTE) )
                    killer_msg[1] = MSG_SAY;

                SetProp( P_NAME, killer_name );
                SetProp( P_ARTICLE, kart );
                SetProp( P_GENDER, kgen );
                CHMASTER->send( kanal, this_object(), funcall(killer_msg[0]),
                                killer_msg[1] );
                SetProp( P_NAME, "Lars" );
                SetProp( P_ARTICLE, 0 );
                SetProp( P_GENDER, MALE );
            }
        }

        if ( pointerp(killer_msg = (mixed) pl->QueryProp(P_DEATH_MSG)) &&
             sizeof(killer_msg) == 2 && stringp(killer_msg[0]) &&
             intp(killer_msg[1]) ){
            SetProp( P_NAME, capitalize(getuid(pl)) );
            SetProp( P_ARTICLE, 0 );
            SetProp( P_GENDER, pl->QueryProp(P_GENDER) );
            CHMASTER->send( kanal, this_object(), killer_msg[0],
                            killer_msg[1] );
            SetProp( P_NAME, "Lars" );
            SetProp( P_ARTICLE, 0 );
            SetProp( P_GENDER, MALE );
        }

        if (pl->query_hc_play()>1){
            SetProp( P_NAME, "Tod" );
            CHMASTER->send( kanal, this_object(),"NUN GEHOERST DU FUER EWIG MIR!" );
            SetProp( P_NAME, "Lars" );
        }
}

public int
SmartLog( string creat, string myname, string str, string date )
{
    int i;
    string fn;
    
    for ( i = sizeof(players); i--; )
        if ( players[i][0] == this_player() )
            break;

    // Spieler (Magier?) ist in keiner Todessequenz -> normales Repfile
    if ( i == -1 )
        return 0;

    if ( !(fn = players[i][4]) ){
        // Spieler hat eine unbekannte Todessequenz (kein Filename, Sequenz
        // wurde komplett in P_ENEMY_DEATH_SEQUENCE abgelegt)
        creat = "TOD/unbekannt.rep";
        fn = "unbekannte Todessequenz";
    }
    else
        // Jede Sequenz mit nem eigenen Repfile
        creat = "TOD/" + explode( fn, "/" )[<1] + ".rep";

    log_file( creat, myname + " von " + getuid(this_interactive())
              + " ["+fn+"] (" + date + "):\n" );
    log_file( creat, str + "\n" );

    return 1;
}

public mixed hier_geblieben( mixed dest, int methods, string direction,
                             string textout, string textin ) 
{
    int i;

    // Magier duerfen Spieler heraustransen
    if ( this_interactive() && IS_LEARNER(this_interactive()) &&
         (this_interactive() != previous_object() ||
          IS_DEPUTY(this_interactive())) ){
        previous_object()->Set( P_TMP_MOVE_HOOK, 0 );
        return ({ dest, methods, direction, textout, textin });
    }

    // Spieler haengt noch in der Todessequenz
    for ( i = sizeof(players); i--; )
        if ( objectp(players[i][0]) && previous_object() == players[i][0] &&
             environment(previous_object()) == find_object(
	  "/room/death/virtual/room_death_" + getuid(previous_object()))&&
             interactive(previous_object()) ) {
	    // Move nur erlaubt, wenn das Ziel wieder der Todesraum ist.
	    // wenn mal fuer nen bestimmten Zwecks Bewegungen raus aus dem
	    // Todesraum erforderlich sind, sollten hier entsprechende
	    // Ausnahmen eingebaut werden.
	    if ( (stringp(dest) && 
		  dest == object_name(environment(previous_object()))) ||
		 (objectp(dest) &&
		  dest == environment(previous_object())) ) {
		previous_object()->Set( P_TMP_MOVE_HOOK, 0 );
		return ({ dest, methods, direction, textout, textin });
	    }
            else
                return -1;
        }

    // Spieler ist nicht mehr im Raum oder eingeschlafen
    if ( previous_object() )
        previous_object()->Set( P_TMP_MOVE_HOOK, 0 );
    
    return ({ dest, methods, direction, textout, textin });
}
