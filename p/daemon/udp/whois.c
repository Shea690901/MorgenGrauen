// WHOIS -- query the whois database

#pragma strong_types
#pragma no_shadow

#include "/secure/config.h"
#include <wizlevels.h>
#include <daemon.h>

#define SAVE_FILE "/p/daemon/save/whois"

#define PORT         99943
//#define CLEANUP_TIME 172800  // 2 days
//#define VALID_CACHE  1209600 // 14 days
#define CLEANUP_TIME 86400  // 1 day
#define VALID_CACHE  604800 // 7 days

#define DEBUG(msg) if(find_player("vanion")) \
                     tell_object(find_player("vanion"), sprintf("%O\n",msg))
#define D_MSG(msg) if(find_player("vanion")) \
                     tell_object(find_player("vanion"), msg)

// contains the whois database cache
mapping domains = m_allocate(0,4);
mapping numbers = m_allocate(0,4);
// next cleanup
int next_cleanup;
// cache for cleanup
static string *indices;
static int *cleanup_stats;

// a static mapping of country ID -> country NAME
#define CM ([ \
   "at": "Oesterreich", \
   "au": "Australien", \
   "br": "Brasilien", \
   "ca": "Kanada", \
   "ch": "Schweiz", \
   "de": "Deutschland", \
   "es": "Spanien", \
   "fi": "Finnland", \
   "fr": "Frankreich", \
   "gb": "Grossbritannien", \
   "germany": "Deutschland", \
   "hk": "Hong Kong", \
   "il": "Israel", \
   "ir": "Irland", \
   "jp": "Japan", \
   "mx": "Mexico", \
   "nl": "Niederlande", \
   "no": "Norwegen", \
   "pl": "Polen", \
   "se": "Schweden", \
   "uk": "Grossbritannien", \
   "us": "USA", \
   "za": "Suedafrika", \
   "com":"Com", \
   "edu":"USA", \
   "gov":"USA", \
   "org":"Org", \
])


// initialize whois server
public void create()
{
    if ( clonep(this_object()) )
        return destruct(this_object());
    
    seteuid(getuid());
    restore_object(SAVE_FILE);
    
    if ( !mappingp(domains) || widthof(domains) != 4)
        domains = m_allocate( 0, 4 );
    
    if ( !mappingp(numbers) || widthof(numbers) != 4)
        numbers = m_allocate( 0, 4 );
    
    if ( !next_cleanup )
        next_cleanup = time() + CLEANUP_TIME;
    else if ( next_cleanup <= time() )
        next_cleanup = time() + 60;

    //set_next_reset( next_cleanup - time() );
}


private string check_int( string v )
{
    return "" + to_int(v);
}


private void check_outdated( mapping map_ldfied, string key )
{
    if ( map_ldfied[key, DATE] >= 0 && map_ldfied[key, DATE] < time() - VALID_CACHE )
        send_udp( UDPSERV, PORT, key );
}


// return whois data, whithout type the description is returned
varargs public mixed whois( string query, int type )
{
    mixed parts, data;
    string match;
    int i, j;

    if ( !stringp(query) )
        return 0;

    parts = explode( lower_case(query), "." );

    // check if we find the information in our domain database
    if ( member( domains, query ) ){
        check_outdated( domains, query );
        return domains[query, type];
    }
    
    if ( member( domains, match = implode( parts[1..], "." ) ) ){
        check_outdated( domains, match );
        return domains[match, type];
    }

    // see if we have ip number information
    if ( sizeof(parts) == 4 &&
         query == implode( map( parts, #'check_int/*'*/ ), "." ) )
        for ( i = 3; i >= 0; i-- )
            if ( member( numbers, match = implode( parts[0..i], "." ) ) ){
                check_outdated( numbers, match );
                return numbers[match, type];
            }

    // if we did not even find ip information try domain database again
    for ( i = 2, j = sizeof(parts); i < j; i++ )
        if ( member( domains, match = implode( parts[i..], "." ) ) ){
            check_outdated( domains, match );
            return domains[match, type];
        }

    // there was no entry in our database
    send_udp( UDPSERV, PORT, query );
    return "...";
}


// return the country origin
varargs public string country( string query )
{
    mixed ctry;

    if ( !stringp(query) )
        return 0;
    
    if ( stringp(ctry = whois( query, COUNTRY )) && ctry != "..." )
        ctry = lower_case(ctry);
    else
        ctry = lower_case(explode( query, "." )[<1]);
    
    if ( to_int(ctry) )
        ctry = "<Unbekannt>";
    
    return capitalize( CM[ctry] || ctry );
}


varargs public string city( string query )
{
    mixed descr, tmp;
    int i, flag;

    if ( stringp(descr = whois( query, COUNTRY )) && member( descr, '.' ) > 0 )
        return capitalize(explode( descr, "." )[<2]);
    
    if ( pointerp(descr = whois( query, DESCR )) ){
        descr -= regexp( descr, "^ *$" );
        descr -= regexp( descr, "[Vv][Ee][Rr][Ss][Ii][Oo][Nn]" );
        descr -= regexp( descr, "restricted by copyright" );
        descr -= regexp( descr, "[Ss]trasse" );
        descr -= regexp( descr, "^[A-Z0-9 -.][A-Z0-9 -.][A-Z0-9 -.][A-Z0-9 -.]"
                         "[A-Z0-9 -.]*$" );
        descr -= regexp( descr, "[Gg][Mm][Bb][Hh]" );
        descr -= regexp( descr, " AG" );
        descr -= regexp( descr, "@" );

        if ( !sizeof(descr) )
            return 0;

        if ( (sizeof(tmp = regexp( descr, "^[A-Z][A-Z]*[ -][ -]*[0-9][0-9]*" ))
              || sizeof(tmp = regexp( descr, "[0-9][0-9]*[ ]" ))) &&
             strstr( tmp[0], " " ) != -1 ){
            tmp = regexplode( tmp[0],
              "[,; ]*[A-Z]*[ -]*[0-9][0-9][0-9]*(-[ ]*[0-9][0-9][0-9]*)*[,; ]*"
              "|[0-9A-Z][0-9A-Z][0-9A-Z]([- ]*[0-9A-Z][0-9A-Z][0-9A-Z])*");
	      
            if ( sizeof(tmp) >= 3 )
                if ( !strlen(tmp[<1]) )
                    tmp = tmp[<3];
                else
                    tmp = tmp[<1];
            
            if ( stringp(tmp) && strlen(tmp) )
                return capitalize(regexplode( tmp, "([,;]| [0-9][0-9]*)" )[0]);
        }
        else {
            for ( i = sizeof(descr); i--; ){
		if (!stringp(descr[i]) || !strlen(descr[i]))
		    continue;
                if (  sizeof(tmp = regexplode( descr[i],
                                              " (of|at|in|zu|von) " )) > 1 ){
                    descr[i] = tmp[<1];
                    flag = i+1;
                }
                
                if ( sizeof(tmp = regexplode( descr[i],
                                              "([Uu]niversitaet|[Ss]chule|"
                                              "Standort|Stadt|TU|HU|FH|RWTH|"
                                              "Fachhochschule) "
                                              )) > 1 ){
                    descr[i] = tmp[<1];
                    flag = i+1;
                }

            if ( sizeof(tmp = regexplode( descr[i],
                                          "[^ -][^ -]*[ -][ -]*[^ -][^ -]*"
                                          "[;,] " )) > 1 &&
                 strlen(tmp = implode( tmp[2..], ", " )) < 20 )
                descr[i] = tmp;

            descr[i] = regexplode( descr[i], "[;,] *" )[0];
            
            if ( sizeof(tmp = regexplode( descr[i], "\\(.*\\)" )) > 1 )
                descr[i] = tmp[1][1..<2];

            if ( sizeof(regexp( ({ descr[i] }), " [0-9][0-9]*$" )) )
                descr[i] = 0;
            
            if ( strlen(descr[i]) > 20 || strlen(descr[i]) < 3 )
                descr[i] = 0;
            }

            if ( flag && descr[flag-1] )
                return capitalize(descr[flag-1]);
            else {
                descr -= ({0});
                
                if ( sizeof(descr) )
                    return capitalize(descr[<1]);
            }
        } 

        log_file( "WHOIS_FAILED", sprintf( "%s: %s - %O\n",
                                           ctime(time())[4..15],
                                           query, whois( query, DESCR ) ) );
    }
    
    return 0;
}


varargs public string locate( string query, string ip )
{
    string tmp;
    
    if ( !stringp(tmp = city(query)) &&
         (query == ip || !stringp(tmp = city(ip))) )
        tmp = country(query);
    
    return tmp;
}


public void _receive_udp( string message )
{
    mixed data, number, descr, tmp;
    string domain, country;
    mixed from, to, i, class;

    if ( !previous_object() ||
         object_name(previous_object()) != "/secure/udp/external" )
        return;

    data = explode( message, "\n" );
    
    if ( !sizeof(data) || message[0..5] != "WHOIS:" )
        return;

    // check for network number information
    if ( sizeof(tmp = regexp( data, "^inetnum=" )) ){
        if ( strstr( number = tmp[0][8..], " - " ) != -1 )
            number = explode( number, " - " );

        descr = map( regexp( data, "^descr=" ), #'[../*'*/, 6 );
        if (sizeof(tmp=regexp( data, "^country="))) 
	  country = tmp[0][8..];
	else country="Unbekannt";

        if ( pointerp(number) ){

            from = map( explode( number[0], "." ), #'to_int/*'*/ );
            to = map( explode( number[<1], "." ), #'to_int/*'*/ );
            
            if ( from[1] != to[1] )
                class = 1;
            else if( from[2] != to[2] )
                class = 2;
            else if( from[3] != to[3] )
                class = 3;

            if ( !from[class] && to[class] == 255 )
                --class;
                
            for( i = from[class]; i <= to[class]; i++ ){
                string num;
                
                switch(class){
                case 0:
                    num = sprintf( "%d", i );
                case 1:
                    num = sprintf( "%d.%d", from[0], i );
                    break;
                    
                case 2:
                    num = sprintf( "%d.%d.%d", from[0], from[1], i );
                    break;
                    
                case 3:
                    num = sprintf( "%d.%d.%d.%d",
                                   from[0], from[1], from[2], i );
                    break;
                    
                default:
                    continue;
                }

                if ( numbers[num, DATE] < 0 )
                    return;
                
                numbers[num, DESCR] = descr;
                numbers[num, COUNTRY] = country;
                numbers[num, DATE] = time();
            }
        }
        else {
            from = explode( number, "." );
            
            if ( !to_int(from[1]) )
                class = 0;
            else if ( !to_int(from[2]) )
                class = 1;
            else if ( !to_int(from[3]) )
                class = 2;

            if ( numbers[number, DATE] < 0 )
                return;
            
            numbers[number = implode(from[0..class], "."), DESCR] = descr;
            numbers[number, COUNTRY] = country;
            numbers[number, DATE] = time();
        }
    }
    else if ( sizeof(tmp = regexp( data, "^domain=" )) ){
        domain = lower_case(tmp[0][7..]);
        descr = map( regexp( data, "^descr=" ), #'[../*'*/, 6 );

        if ( domains[domain, DATE] < 0 )
            return;
        
        domains[domain, DESCR] = descr;
        domains[domain, COUNTRY] = explode( domain, "." )[<1];
        domains[domain, DATE] = time();
    }
    else {
        if ( data[0][0..11] == "WHOIS:QUERY=" ){
            domain = lower_case( data[0][12..] );
            descr = ({ data[1][0..strstr( data[1], "(" )-2] });
            
            for ( i = 2, tmp = sizeof(data); i < tmp && strlen(data[i]); i++ )
                descr += ({ data[i][3..] });

            descr -= regexp( descr, "^ *$" );
            descr -= regexp( descr, "[Vv][Ee][Rr][Ss][Ii][Oo][Nn]" );
            descr -= regexp( descr, "restricted by copyright" );
            
            if ( domain == implode( map( explode(domain, "."),
                                               #'check_int/*'*/ ), "." ) ){
                if ( numbers[domain, DATE] < 0 )
                    return;
                
                numbers[domain, DESCR] = sizeof(descr) ? descr : 0;
                numbers[domain, DATE] = time();
                
                if ( sizeof(descr) && strstr( descr[<1], " " ) == -1 )
                    numbers[domain, COUNTRY] = descr[<1];
                else
                    numbers[domain, COUNTRY] = 0;
            }
            else {
                if ( domains[domain, DATE] < 0 )
                    return;
                
                domains[domain, DESCR] = sizeof(descr) ? descr : 0;
                domains[domain, COUNTRY] = explode( domain, "." )[<1];
                domains[domain, DATE] = time();
            }
        }
        else {
            domain = lower_case( data[0][strstr( data[0], "=" )+1..] );
            
            if ( domain == implode( map( explode( domain, "." ),
                                               #'check_int/*'*/ ), "." ) ){
                if ( numbers[domain, DATE] < 0 )
                    return;
                
                numbers[domain, DESCR] = 0;
                numbers[domain, COUNTRY] = 0;
                numbers[domain, DATE] = time();
            }
             else {
                 if ( domains[domain, DATE] < 0 )
                     return;
                 
                 domains[domain, DESCR] = 0;
                 domains[domain, COUNTRY] = explode( domain, "." )[<1];
                 domains[domain, DATE] = time();
                 
                 if ( to_int(domains[domain, COUNTRY]) )
                     domains[domain, DESCR] = "<Unbekannt>";
             }
        }
    }

    // save database
    //save_object(SAVE_FILE);
}


// kill a domain or number
public void kd( string k )
{
    if( !stringp(k) || k == "" || this_player() != this_interactive() ||
        !IS_ARCH(this_interactive()) )
        return;

    efun::m_delete( domains, k );
}


public void kn( string k )
{
    if( !stringp(k) || k == "" || this_player() != this_interactive() ||
        !IS_ARCH(this_interactive()) )
        return;

    efun::m_delete( numbers, k );
}


// set a specific value
public varargs int sd( string d, mixed descr, string ctry, int exp )
{
    if( !stringp(d) || d == "" || this_player() != this_interactive() ||
        !IS_ARCH(this_interactive()) )
        return -1;

    d = lower_case(d);
    
    if ( stringp(descr) )
        descr = ({ descr });

    if ( pointerp(descr) )
        domains[d, DESCR] = descr;
    
    if ( stringp(ctry) )
        domains[d, COUNTRY] = ctry;

    if ( exp )
        domains[d, DATE] = -1;
    else
        domains[d, DATE] = time();
    
    //save_object(SAVE_FILE);

    return 1;
}


public varargs int sn( string n, mixed descr, string ctry, int exp )
{
    if ( !stringp(n) || n == "" || this_player() != this_interactive() ||
         !IS_ARCH(this_interactive()) )
        return -1;

    if ( stringp(descr) )
        descr = ({ descr });

    if ( pointerp(descr) )
        numbers[n, DESCR] = descr;
    
    if ( stringp(ctry) )
        numbers[n, COUNTRY] = ctry;

    if ( exp )
        numbers[n, DATE] = -1;
    else
        numbers[n, DATE] = time();
    
    //save_object(SAVE_FILE);

    return 1;
}


private void cleanup_domains( int index )
{
    while ( index >= 0 && get_eval_cost() > 400000 ){
        if ( domains[indices[index], DATE] > 0 &&
             domains[indices[index], DATE] < time() - VALID_CACHE )
            efun::m_delete( domains, indices[index] );

        index--;
    }

    if ( index >= 0 )
        call_out( #'cleanup_domains, 10, index );
    else {
        log_file( "WHOIS", sprintf( "%s: Whois-Datenbank aufgeraeumt.\n"
                                    "Eintraege vorher: %d/%d, "
                                    "nachher: %d/%d\n", ctime(time())[4..15],
                                    cleanup_stats[0], cleanup_stats[1],
                                    sizeof(numbers), sizeof(domains) ) );
        indices = 0;
        cleanup_stats = 0;
        next_cleanup = time() + CLEANUP_TIME;
        save_object(SAVE_FILE);
    }
}


private void cleanup_numbers( int index )
{
    while ( index >= 0 && get_eval_cost() > 400000 ){
        if ( numbers[indices[index], DATE] > 0 &&
             numbers[indices[index], DATE] < time() - VALID_CACHE )
            efun::m_delete( numbers, indices[index] );

        index--;
    }

    if ( index >= 0 )
        call_out( #'cleanup_numbers, 10, index );
    else {
        indices = m_indices( domains );
        call_out( #'cleanup_domains, 10, sizeof(indices)-1 );
    }
}


public void reset()
{
    // only the driver may call reset()
    //if ( previous_object() )
    //    return;
   
  // Cleanup machen? wenn nicht, nur speichern.
  if (time() > next_cleanup) {
    cleanup_stats = ({ sizeof(numbers), sizeof(domains) });
    indices = m_indices(numbers);
    call_out( #'cleanup_numbers, 2, sizeof(indices)-1 );
  }
  else
    save_object(SAVE_FILE);
}


// return the domain or number mappings
public mixed d() { return deep_copy(domains); }
public mixed n() { return deep_copy(numbers); }

// reset the cache
public int DeleteCache() {
  
  if (!IS_ELDER(this_interactive()))
    return 0;
  
  domains = m_allocate( 0, 4 );
  numbers = m_allocate( 0, 4 );
  save_object(SAVE_FILE);
  return 1;
}

