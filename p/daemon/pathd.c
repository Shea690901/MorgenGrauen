// Daemon zum automatisierten Erstellen von Karten.
// Soll spaeter mal wichtige Wege berechnen koennen fuer Anfaenger.
//
// Noch ist er hoechst experimentell und im Pre-Alpha-Stadium.
//
// Wer mir ohne triftigen Grund daran herumpfuscht, der wird standesrechtlich
// mit faulen Eiern beworfen. ]:->
//
// Tiamak

#pragma strong_types,save_types

#include <wizlevels.h>
#include <player.h>
#include <properties.h>
#include <rooms.h>
#include <moving.h>

#define LOGNAME "PATHD"
#define SAVEFILE "/p/daemon/save/pathd"
#define DEBUG

// 10 verschiedene Leute werden gebraucht, um eine Verbindung zu verifizieren
#define NEEDED 10
#define TIMEOUT_NEW 604800  // 7 Tage
#define TIMEOUT_OLD 2592000 // 30 Tage

// Art des Ausgangs
#define NORMAL  0
#define SPECIAL 1
#define COMMAND 2

// Kosten fuer unterschiedliche Wege
#define COST_EXITS ({ 1, 10, 100 })
#define COST_MANY   3
#define COST_FEW    5
#define COST_ONE   10

#ifdef DEBUG
#  define DBG(x) if ( find_player("vanion") ) \
                   tell_object( find_player("vanion"), (x)+"\n" );
#else
#  define DBG(x)
#endif

#define LOG(x) log_file( "PATHD", (x), 100000 );

// Variablen
mapping pathmap;
static mapping searchlist;
static int time_to_clean_up, rooms_checked, conns_checked;

// Prototypes
public void create();
public void add_paths( string *connections );
public int search_for_path( string from, string to );
public varargs void show_pathmap( string path );
public varargs int show_statistic( int verbose, int silent );
public int change_pathmap( string pfad, mixed *verbindungen );
public mapping get_pathmap();
public void reset();
public varargs int remove( int silent );
public varargs int query_prevent_shadow( object ob );
static int insert_paths( mixed *path );
static mixed *format_paths( string path );
static void _search( string fname, string start );
private string _get_prefix( string path );
private void _add_deadend( string fname, string sackgasse );
private int _is_deadend( string fname, string room );
private void _rate_path( string fname, string *path, int *idx );
private int _connection_okay( string fname, mixed *connection, int idx,
                              string from );
static void cleanup_data( string *names, int idx );


public void create()
{
    // es darf nur einen daemon geben
    if ( clonep(this_object()) ){
        destruct(this_object());
        return;
    }
    
    // wir muessen das Savefile schreiben duerfen
    seteuid(getuid());
    
    if ( !restore_object(SAVEFILE) ){
        pathmap = ([]);
    }

    searchlist = ([]);
}


// Wird von Spielern aufgerufen mit den Wegen, die sie gelaufen sind
public void add_paths( string *connections )
{
    mixed *paths;
    
    // keinen Aufruf per Hand bitte
    if ( !previous_object() || !this_player()
         || this_interactive() && getuid(this_interactive()) != "tiamak"
         || file_size( "/std/shells" +
                       explode( object_name(previous_object()), ":" )[0]
                       + ".c" ) < 1 )
        return;

    // Doppelte Wege rauswerfen
    paths = m_indices( mkmapping(connections) );
    // Daten zusammensuchen
    paths = map( paths, #'format_paths/*'*/ ) - ({0});
    // Neue Raeume eintragen
    filter( paths, #'insert_paths/*'*/ );
}


// Suchfunktion, um die kuerzeste Verbindung zwischen zwei Raeumen zu finden
public int search_for_path( string from, string to )
{
    // Es laeuft schon eine Suche fuer diesen Spieler
    if ( !this_interactive() || searchlist[getuid(this_interactive())] )
        return -1;

    // UID, Raeume, Ausgangsnummern, Zielraum, Wizlevel, Parawelt,
    // kuerzeste Verbindung, Ausgangsnummern fuer die Verbindung,
    // Kosten der Verbindung, Sackgassen (drei Arrays wegen der 10k-Grenze)
    searchlist += ([ getuid(this_interactive()): ({ }); ({ }); to;
                     (query_wiz_level(this_interactive()) ? 1 : 0);
                     this_interactive()->QueryProp(P_PARA);
                     ({}); ({}); 100000000; ({}); ({}); ({}) ]);

    // Die eigentliche Suche starten
    rooms_checked = 0;
    conns_checked = 0;
    _search( getuid(this_interactive()), from );
    
    return 1;
}


// Gespeicherte Informationen zu einem Raum oder einem kompletten Gebiet
// abfragen. Gebiete lassen sich aber nur als "Organisationseinheiten" abfragen.
// Dabei werden Gebiete unterhalb von /d/* als /d/region/magiername
// abgespeichert und der Rest unter den ersten beiden Teilpfaden.
public varargs void show_pathmap( string path )
{
    string pre;

    if ( path ){
        pre = _get_prefix(path);
    
        if ( pre == path )
            // Ganzen Teilbaum ausgeben
            printf( "Pathmap[%O]: %O\n", path, pathmap[path] );
        else
            // Nur Infos ueber einen Raum ausgeben
            printf( "Pathmap[%O]: %O\n", path,
                    pathmap[pre] ? pathmap[pre][path] : 0 );
    }
    else
        // Ohne Argument wird die gesamte Map ausgegeben.
        // Klappt aber eher nicht (mehr) wegen Buffer overflow. ;^)
        printf( "Pathmap: %O\n", pathmap );
}


// Statistic zu den gespeicherten Daten anzeigen.
// Mit 'verbose' wird jede Organisationseinheit einzeln aufgelistet, ohne
// wird grob nach Regionen zusammengefasst.
public varargs int show_statistic( int verbose, int silent )
{
    int i, ges;
    string *ind, name;
    mapping m;

    if ( verbose ){ // Stur jeden Eintrag auflisten
        ind = sort_array( m_indices(pathmap), #'</*'*/ );
        for ( i = sizeof(ind); i--; ){
            if ( !silent )
                printf( "%-30s: %4d\n", ind[i], sizeof(pathmap[ind[i]]) );
            ges += sizeof(pathmap[ind[i]]);
        }
    }
    else {          // Regionen zusammenfassen
        ind = m_indices(pathmap);
        m = ([]);

        for ( i = sizeof(ind); i--; ){
            if ( ind[i][0..8] == "/players/" )
                // Alle Playerverzeichnisse zusammenfassen ...
                name = "/players";
            else
                // ... den Rest jeweils nach zwei Teilpfaden
                name = implode( explode( ind[i], "/" )[0..2], "/" );
            
            if ( !m[name] )
                m[name] = sizeof( pathmap[ind[i]] );
            else
                m[name] += sizeof( pathmap[ind[i]] );
        }

        ind = sort_array( m_indices(m), #'</*'*/ );
        for ( i = sizeof(ind); i--; ){
            if ( !silent )
                printf( "%-30s: %4d\n", ind[i], m[ind[i]] );
            ges += m[ind[i]];
        }
    }

    if ( !silent )
        printf( "\nGesamt: %d Raeume.\n", ges );

    return ges;
}


// Manipulieren der internen Pathmap. Nur zum Debuggen und somit
// nur fuer Tiamak erlaubt. Sonst verhunzt mir noch wer meine Daten. ;^)
public int change_pathmap( string pfad, mixed *verbindungen )
{
    if ( !this_interactive() || getuid(this_interactive()) != "tiamak"
         || !previous_object() || getuid(previous_object()) != "tiamak" )
        return -1;

    if ( mappingp(verbindungen) ){
        if ( !pathmap[_get_prefix(pfad)] )
            return 0;
        
        pathmap[_get_prefix(pfad)] = verbindungen;
    }
    else {
        if ( !pathmap[_get_prefix(pfad)][pfad] )
            return 0;
        
        pathmap[_get_prefix(pfad)][pfad] = verbindungen;
    }

    return 1;
}

#define ALLOWED ({"zook","vanion","rumata","zesstra"})
#define MATCH(x,y) (sizeof(regexp(x,y)))

// Zum Debuggen. Nur für Tiamak. Bzw. nun fuer Zook, Vanion, Rumata
public mapping get_pathmap()
{
  if ( !this_interactive() || !MATCH(ALLOWED,getuid(this_interactive()))
       || !previous_object() || !MATCH(ALLOWED,getuid(previous_object())))
        return ([]);

    return pathmap;
}

#undef ALLOWED
#undef MATCH

// Die Funktion gibt alle dem pathd bekannten Raeume als Mapping von Arrays
// zurueck. Schlüssel des Mappings sind dabei Gebiete.
public mapping get_rooms()
{
  string *submaps;
  mapping roommap;
  int i;

  roommap=([]);

  submaps=m_indices(pathmap);

  i=sizeof(submaps);
  
  while (i--)
    if (sizeof(m_indices(pathmap[submaps[i]])))
	    roommap[submaps[i]]=m_indices(pathmap[submaps[i]]);
  
  return roommap;		    
}

public void reset()
{
    if ( time_to_clean_up <= 0 )
        // Einmal pro Tag Reset zum Aufraeumen der Datenbank
        call_out( "cleanup_data", 5,
                  sort_array(m_indices(pathmap), #'</*'*/), 0 );

    save_object(SAVEFILE);
    time_to_clean_up--;
    set_next_reset(900);
}

 
public varargs int remove( int silent )
{
    // Vor dem Entfernen noch schnell die Datenbank sichern
    save_object(SAVEFILE);
    destruct(this_object());

    return 1;
}


// keine Shadows bitte
public varargs int query_prevent_shadow( object ob )
{
    return 1;
}


// Daten zu einer Verbindung sammeln und aufbereiten
static mixed *format_paths( string path )
{
    string tmp, uid, *buf;
    object ob;
    mapping exits;
    int art;
    
    // Magier und Testspieler koennen auch in nicht angeschlossene Gebiete
    // und werden deshalb nicht beachtet.
    if ( IS_LEARNER(previous_object(1)) ||
         IS_LEARNER(lower_case( previous_object(1)->Query(P_TESTPLAYER)||"" )) )
        return 0;

    // Alle Daten kommen als ein String mit '#' als Trenner an.
    // Muster: Startraum#Zielraum#Verb#Methode der Bewegung#Parawelt
    buf = explode( path, "#" );
    // Falls im Verb auch # vorkam (unwahrscheinlich, aber moeglich):
    buf[2..<3] = ({ implode( buf[2..<3], "#" ) });

    // Beim Verb endstaendige Leerzeichen sowie " 0" (kommt bei
    // _unparsed_args(), wenn keine weiteren Argumente vorhanden waren)
    // abschneiden.
    tmp = buf[2];
    if ( tmp[<1] == ' ' )
        tmp = tmp[0..<2];
    else if ( tmp[<2..] == " 0" )
        tmp = tmp[0..<3];

    // Wenn der Zielraum als String angegeben wurde, kann der fuehrende
    // Slash fehlen!
    if ( buf[1][0] != '/' )
        buf[1] = "/" + buf[1];

    // Zum Abfragen der zusaetzlichen Daten brauchen wir das Objekt selber
    if ( !objectp(ob = find_object(buf[0])) ){
        catch( load_object( buf[0]);publish );
        ob = find_object(buf[0]);
    }

    // Kleiner Hack - bei Transportern etc. ist 'ob' die nicht initialisierte
    // Blueprint. Jede Abfrage von P_EXITS wuerde nett auf -Debug scrollen.
    // Da P_IDS im create() auf jeden Fall auf ein Array gesetzt wird,
    // missbrauche ich das hier als "Ist initialisiert"-Flag.
    if ( !objectp(ob) || !ob->QueryProp(P_IDS) )
        return 0;

    // Art des Ausgangs feststellen. 
    if ( mappingp(exits = ob->QueryProp(P_EXITS)) && exits[tmp] )
        art = NORMAL;   // "normaler" Ausgang
    else if ( mappingp(exits = ob->QueryProp(P_SPECIAL_EXITS)) && exits[tmp] )
        art = SPECIAL;  // SpecialExit
    else {
        // Kommandos, die einen in einen anderen Raum bringen
        art = (int) buf[3];

        // Es zaehlen aber nur Bewegungen, die halbwegs "normal" aussehen
        if ( art & (M_TPORT | M_NOCHECK) || !(art & M_GO) )
            return 0;
        else
            art = COMMAND;
    }

    // Die UID der Spieler/Seher wird verschluesselt.
    // Schliesslich brauchen wir sie nur fuer statistische Zwecke und nicht,
    // um Bewegungsprofile zu erstellen.
    uid = getuid(previous_object(1));
    uid = crypt( uid, "w3" );

    // Start, Ziel, Verb, Wizlevel(TP), UID(TP), Art des Ausgangs, Parawelt
    return ({ buf[0], buf[1], tmp, query_wiz_level(previous_object(1)) ? 1 : 0,
                  uid, art, (int) buf[4] });
}


// Neue Verbindung in der Datenbank eintragen
static int insert_paths( mixed *path )
{
    string pre;
    mixed *conn, *tmp, *tme;
    int i;

    pre = _get_prefix(path[0]);

    // Falls noch gar kein Eintrag existiert, neu initialisieren
    if ( !mappingp(pathmap[pre]) )
        pathmap[pre] = ([]);

    if ( !pointerp(pathmap[pre][path[0]]) )
        pathmap[pre][path[0]] = ({});

    // Aufbau von 'path':
    // ({ Start, Ziel, Verb, Wizlevel(TP), UID(TP), Art des Ausgangs, Para })
    // Aufbau von 'conn':
    // ({ Ziel, Verb, ({ Spieler-UIDs, Seher-UIDs }), Art des Ausgangs,
    //   ({ Letztes Betreten durch Spieler, durch Seher }), Parawelt })
    
    // Alle Verbindungen des Raumes durchgehen
    conn = pathmap[pre][path[0]];
    for ( i = sizeof(conn); i--; )
        // Wenn Zielraum, Verb und Parawelt passen ...
        if ( conn[i][0] == path[1] && conn[i][1] == path[2]
             && conn[i][5] == path[6] ){

            // Wenn schon genug Leute diese Verbindung genutzt haben, einfach
            // nur die Zeit der letzten Benutzung aktualisieren.
            if ( conn[i][2][path[3]] == -1 ){
                conn[i][4][path[3]] = time();
                break;
            }
            // Ansonsten die (neue?) UID hinzufuegen und die Zeit aktualisieren.
            else {
                conn[i][2][path[3]] =
                    conn[i][2][path[3]] - ({ path[4] }) + ({ path[4] });
                if ( sizeof(conn[i][2][path[3]]) >= NEEDED )
                    conn[i][2][path[3]] = -1;

                conn[i][4][path[3]] = time();
                break;
            }
        }

    // Falls keine Verbindung gepasst hat, eine neue erzeugen.
    if ( i < 0 ) {
        tmp = ({ ({}), ({}) });
        tmp[path[3]] = ({ path[4] });
        tme = ({ 0, 0 });
        tme[path[3]] = time();

        // Aufbau siehe oben
        conn += ({ ({ path[1], path[2], tmp, path[5], tme, path[6] }) });
    }

    // Ist eigentlich notwendig, da wir mit Referenzen arbeiten.
    // Aber sicher ist sicher. ;^)
    pathmap[pre][path[0]] = conn;

    // Ob 0 oder 1 ist eigentlich total egal, da das Ergebnis-Array sowieso
    // verworfen wird.
    return 0;
}


// Die eigentliche Suchroutine fuer die kuerzeste Verbindung zwischen zwei
// Raeumen in der Datenbank.
static void _search( string fname, string start )
{
    string from, to, *path;
    int lvl, *idx;
    mixed *tmp;

    from = start;
    path = searchlist[fname, 0];
    idx = searchlist[fname, 1];
    to = searchlist[fname, 2];
    lvl = searchlist[fname, 3];

    // Eigentlich macht man sowas mit einer handlichen kleinen eleganten
    // Rekursion. Leider begrenzt der Driver die maximale Rekursionstiefe
    // aber auf einen indiskutablen Wert. Deshalb bauen wir also eine
    // grosse lange haessliche Schleife ...
    do {
        if ( from == to )
            // Ist die gefundene Verbindung die kuerzeste?
            _rate_path( fname, path + ({ from }), idx );
        else if ( !sizeof(path & ({ from })) ){
            // Sofern wir keinen Kreis gelaufen sind, neuen Raum uebernehmen.
            DBG( sprintf( "Uebernehme Raum %O.", from ) );
            path += ({ from });
            idx += ({ -1 });
            rooms_checked++;
        }
        else
            DBG( sprintf( "In Raum %O war ich schon!", from ) );

        do {
            // So lange der aktuelle Raum keine noch nicht getesteten Ausgaenge
            // mehr hat, wieder einen Raum zurueck gehen.
            while ( sizeof(idx) && idx[<1] + 1 >=
                    sizeof(pathmap[ _get_prefix(path[<1]) ][ path[<1] ]) ){
                // Den Raum als "Sackgasse" markieren ...
                DBG( sprintf( "Markiere %O als Sackgasse.", path[<1] ) );
                _add_deadend( fname, path[<1] );
                // ... und einen Schritt zurueck gehen.
                path[<1..<1] = ({});
                idx[<1..<1] = ({});
            }

            // Den kleinsten noch nicht getesteten Ausgang nehmen.
            if ( sizeof(idx) ){
                tmp = pathmap[ _get_prefix(path[<1]) ][ path[<1] ];
                from = tmp[ ++idx[<1] ][0];
                DBG( sprintf( "Teste Verbindung nach %O.", from ) );
                conns_checked++;
            }
            
            // Das Spielchen geht so lange, bis wir einen 'gueltigen' Ausgang
            // gefunden haben.
         } while ( sizeof(idx) &&
                   !_connection_okay( fname, tmp, idx[<1], path[<1] ) );

        // Es wird so lange getestet, bis es keinen Raum mehr gibt, zu dem
        // man zurueckgehen koennte.
    } while ( sizeof(idx) && get_eval_cost() > 600000 );

    // Ist eigentlich nicht notwendig, da wir mit Referenzen arbeiten.
    // Aber sicher ist sicher. ;^)
    searchlist[fname, 0] = path;
    searchlist[fname, 1] = idx;

    // Erst, wenn wir saemtliche moeglichen Verbindungen durchgetestet haben,
    // sind wir fertig. Ansonsten sind nur die Evalticks ausgegangen.
    if ( sizeof(idx) ){
        DBG( sprintf( "Splitte bei %O Evalticks. Pfad hat momentan %O Raeume.",
                      get_eval_cost(), sizeof(path) ) );
        DBG( sprintf( "Getestete Raeume %O, Verbindungen %O, Sackgassen %O",
                      rooms_checked, conns_checked,
                      sizeof(searchlist[fname, 8] + searchlist[fname, 9]
                             + searchlist[fname, 10]) ) );
        call_out( "_search", 0, fname, from );
    }
    else {
        DBG( sprintf("Suche fertig! Gefundener Weg: %O, Kosten: %O\n",
                     searchlist[fname, 5], searchlist[fname, 7] ) );

        efun::m_delete( searchlist, fname );
    }
}


// Die Daten ueber Raeume werden quasi gehasht abgespeichert, damit wir nicht
// an die 10k-Begrenzung bei Arrays und Mappings stossen.
// Dabei ist der 'Hash' der Anfang des Pfades.
private string _get_prefix( string path )
{
    string *tmp;

    tmp = explode( path, "/" );

    // Bei Raeumen unterhalb von /d/* wird /d/region/magiername benutzt
    if ( path[0..2] == "/d/" )
        return implode( tmp[0..3], "/" );
    // Ansonsten die ersten beiden Teilpfade, falls soviele vorhanden sind
    else if ( sizeof(tmp) < 4 )
        return implode( tmp[0..1], "/" );
    else
        return implode( tmp[0..2], "/" );
}


// Die Sackgassen (schon erfolglos getestete Raeume) werden in drei Arrays
// gespeichert, weil eins zu klein werden koennte (10k-Grenze).
private void _add_deadend( string fname, string sackgasse )
{
    if ( sizeof(searchlist[fname, 8]) < 9999 )
        searchlist[fname, 8] += ({sackgasse});
    else if ( sizeof(searchlist[fname, 9]) < 9999 )
        searchlist[fname, 9] += ({sackgasse});
    else
        searchlist[fname, 10] += ({sackgasse});
}


// Test auf Sackgasse vereinfachen (wegen der drei Arrays).
private int _is_deadend( string fname, string room )
{
    return sizeof( ({room}) & (searchlist[fname, 8] + searchlist[fname, 9]
                               + searchlist[fname, 10]) );
}


// Kosten fuer eine Verbindung zwischen zwei Punkten berechnen
private void _rate_path( string fname, string *path, int *idx )
{
    int cost, size, i, tmp;
    mixed buf;

    for ( i = 0, size = sizeof(path) - 1; i < size; i++ ){
        buf = pathmap[ _get_prefix(path[i]) ][ path[i] ][ idx[i] ];
        // Die Art des benutzten Ausgangs zaehlt hinein ...
        tmp = COST_EXITS[ buf[3] ];

        // ... sowie die Zahl der Benutzer (selten benutzte Ausgaenge koennten
        // Haken haben)
        switch ( pointerp(buf[2][ searchlist[fname, 3] ]) ?
                 sizeof(buf[2][ searchlist[fname, 3] ]) : -1 ){
        case 0:
            // Wenn den Ausgang noch keiner benutzt hat, ist das keine gueltige
            // Verbindung. Also schoen teuer machen.
            tmp = 100000;
            break;

        case 1..2:
            tmp *= COST_ONE;
            break;

        case 3..5:
            tmp *= COST_FEW;
            break;

        case 6..10:
            tmp *= COST_MANY;
            break;

        default:
            // -1 fuer viel benutzte Ausgaenge sowie alles andere bei Bugs ;^)
            break;
        }
        
        cost += tmp;
    }

    DBG( sprintf( "Route mit %O Raeumen gefunden, Kosten %O.",
                  sizeof(path), cost ) );
    
    // Bei Erfolg die Route sowie ihre Kosten speichern
    if ( cost < searchlist[fname, 7] ){
        DBG( "Die Route ist die bisher kuerzeste!" );
        searchlist[fname, 5] = path[0..];
        searchlist[fname, 6] = idx[0..];
        searchlist[fname, 7] = cost;
    }
}


// Ist die gewuenschte Verbindung ueberhaupt geeignet?
private int _connection_okay( string fname, mixed *connections, int idx,
                              string from )
{
    int i;

    // Eine Verbindung in eine (schon getestete) Sackgasse brauchen wir
    // nicht noch einmal zu testen.
    if ( _is_deadend(fname, connections[idx][0]) ){
        DBG( sprintf( "Raum %O ->| %O!", from, connections[idx][0] ) );
        return 0;
    }

    // Kann der Spieler diese Verbindung ueberhaupt nutzen (Seherlevel)?
    if ( pointerp(connections[idx][2][ searchlist[fname, 3] ]) &&
         !sizeof(connections[idx][2][ searchlist[fname, 3] ]) )
        return 0;

    // Ist der Spieler in der richtigen (Para-)Welt?
    if ( connections[idx][5] != searchlist[fname, 4] )
        return 0;

    // Wenn es zu einem Verb mehrere Zielraeume gibt, kann mit dem Ausgang
    // keine Route geplant werden.
    for ( i = sizeof(connections); i--; )
        if ( i != idx && connections[i][1] == connections[idx][1]
             && connections[i][5] == connections[idx][5] ){
            DBG( sprintf( "Mehrdeutiges Verb %O in Raum %O!",
                          connections[i][1], from ) );
            return 0;
        }
    
    return 1;
}


// Datenbank aufraeumen
static void cleanup_data( string *names, int idx )
{
    int size, i, j, k, verbindungen;
    string *rooms;
    mixed *paths;

    size = sizeof(names);

    // Ein bisserl mitloggen, damit wir Schwachstellen im System auch finden.
    if ( !idx ){
        LOG( sprintf("=== %s: Starte cleanup_data(): %O Gebiete, %O Raeume "
                     + "...\n", dtime(time())[5..], size,
                     show_statistic(1, 1) ) );
    }
    else {
        LOG( sprintf("%s: Setze cleanup_data() fort.\n", dtime(time()))[5..] );
    }

    // Brav gesplittet, damit es kein Lag gibt.
    // Die Grenze ist recht hoch angesetzt, da immer gleich komplette
    // Teilbaeume aufgeraeumt werden.
    while ( get_eval_cost() > 600000 && idx < size ){
        rooms = sort_array( m_indices(pathmap[names[idx]]), #'</*'*/ );

        for ( i = sizeof(rooms); i--; ){
            paths = pathmap[names[idx]][rooms[i]];
            verbindungen = sizeof(paths);

            for ( j = verbindungen; j--; ){
                for ( k = 0; k < 2; k++ ){
                    // Diese Verbindung hat noch keiner genutzt bisher
                    if ( !paths[j][4][k] )
                        continue;
                    
                    if ( paths[j][2][k] == -1 // 'bekanntes' Gebiet
                         && time() - paths[j][4][k] > TIMEOUT_OLD ){
                        // LOG( sprintf("*** Loesche alte "
                        //             + (k ? "Seher" : "Spieler")
                        //             + "-Verbindung %O.\n", paths[j]) );
                        paths[j][2][k] = ({});
                        paths[j][4][k] = 0;
                    }
                    else if ( paths[j][2][k] != -1 // 'neues' Gebiet
                              && time() - paths[j][4][k] > TIMEOUT_NEW ){
                        paths[j][2][k] = ({});
                        paths[j][4][k] = 0;
                    }
                }
                
                // Wenn eine Verbindung weder von Spielern noch von Sehern
                // benutzt wurde in letzter Zeit, Verbindung ganz loeschen.
                if ( !paths[j][4][0] && !paths[j][4][1] ){
                    paths[j..j] = ({});
                    verbindungen--;
                }
            }

            // Ein Raum ohne Verbindungen frisst nur Platz in der Datenbank
            if ( !verbindungen ){
                // LOG( sprintf("*** Loesche kompletten Raum %O\n", rooms[i]) );
                efun::m_delete( pathmap[names[idx]], rooms[i] );
            }
            else
                pathmap[names[idx]][rooms[i]] = paths;
        }

        idx++;
    }

    if ( idx >= size ){
        time_to_clean_up = 95; // in ~24h wieder aufräumen
        LOG( sprintf("=== %s: Beende cleanup_data()! Uebrig: %O Raeume.\n",
                     dtime(time())[5..], show_statistic(1, 1) ) );
    }
    else {
        call_out( "cleanup_data", 20, names, idx );
        LOG( sprintf("%s: WARTE 20s bei Evalcost %O\n",
                     dtime(time())[5..], get_eval_cost()) );
    }
}


