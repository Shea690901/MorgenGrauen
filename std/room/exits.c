// MorgenGrauen MUDlib
//
// room/exits.c -- room exits handling 
//
// $Id: exits.c 7481 2010-02-21 15:36:28Z Zesstra $

/*
 * Exits of the room (obvious ones, doors, and special ones)
 * we define the following function for easy reference:
 * GetExits() - return a string containing an "Obvious Exits" Statement
 *
 * The exits are implemented as properties P_EXITS
 * They are stored locally (_set_xx, _query_xx)
 * as mapping to speed up the routines in this module.
 *
 */

#pragma strong_types
#pragma save_types
#pragma pedantic
#pragma range_check
#pragma no_clone

#define NEED_PROTOTYPES

#include <sys_debug.h>
#include <config.h>
#include <thing/properties.h>
#include <properties.h>
#include <moving.h>
#include <defines.h>
#include <daemon.h>
#include <doorroom.h>
#include <routingd.h>
#include <exploration.h>
#include <room/exits.h>

#define NUMBERS ({ "zwei", "drei", "vier", "fuenf", "sechs", "sieben", "acht" })


// Hilfsfunktion, die bei kaputten Exits eine Notrettung betreibt, aber
// trotzdem auf Debug eine Meldung macht.
static mapping rescueExit()
{
  catch(raise_error(sprintf(
	  "room/exits.c: Forgotten ::create()? "
	  "P_EXITS in %O is 0!\n", this_object()));publish); 

  return ([]);
}


static mapping _set_exits( mapping map_ldfied ) 
{
    if( mappingp(map_ldfied) )
        return Set( P_EXITS, map_ldfied );
    return 0;
}


static mapping _query_exits() 
{
    mapping exits;
    mixed key;
    int i;

    if( (!previous_object() || object_name(previous_object()) != DOOR_MASTER)
        && QueryProp(P_DOOR_INFOS) )
        call_other( DOOR_MASTER, "init_doors" );
    
    key = m_indices( exits = (Query(P_EXITS) || rescueExit()) );
    
    for ( i = sizeof(key); i--; )
        if( !stringp(exits[key[i]]) )
            exits = m_delete( exits, key[i] );
    
    return exits;
}


static int _set_special_exits( mapping map_ldfied )
{
    return -1;
}


static mapping _query_special_exits() 
{
    mapping exits;
    mixed key;
    int i;

    key = m_indices( exits = (Query(P_EXITS) || rescueExit()) );
    
    for ( i = sizeof(key); i--; )
        if( stringp(exits[key[i]]) )
            exits = m_delete( exits, key[i] );
    
    return exits;
}


void reset()
{}


protected void create()
{
  SetProp( P_EXITS, ([]) );
}

protected void create_super() {
  set_next_reset(-1);
}

void AddExit( mixed cmd, mixed room )
{
    int i;
    mapping exita;
  
    exita = Query(P_EXITS) || rescueExit();
    
    if ( !closurep(room) ){
        int s;
        string msg;
        object router;
    
        if( (s = member(room, '#')) != -1 ){
            msg  = room[0..s];
            room = room[s+1..];
        }
        else
            msg = "";
        
        room = _MakePath(room);

        if ( !clonep(this_object()) && objectp(router = find_object(ROUTER)) )
            router->RegisterExit( object_name(this_object()), cmd, room );
    
        room = msg + room;
    }
    
    if( !pointerp(cmd) )
        exita += ([ cmd : room ]);
    else 
        for( i = sizeof(cmd); i--; )
            exita += ([ cmd[i] : room ]);

    Set( P_EXITS, exita );
}


void RemoveExit( mixed cmd )
{
    mapping exita;
    int i;
  
    if ( !cmd ) {
        SetProp(P_EXITS, ([]) );
        return;
    }

    if ( !pointerp(cmd) )
        cmd = ({ cmd });

    exita = Query(P_EXITS) || rescueExit();
    
    for( i = sizeof(cmd); i--; )
        exita = m_delete( exita, cmd[i] );
  
    Set( P_EXITS, exita );
}


void AddSpecialExit( mixed cmd, mixed functionname )
{
    
    if ( stringp(functionname) )
        functionname = symbol_function( functionname, this_object() );
    
    if ( !closurep(functionname) ) {
	catch(raise_error(sprintf( "method %O doesn't exist\n",
		functionname)); publish);
	return;
    }
    
    AddExit( cmd, functionname );
}


void RemoveSpecialExit( mixed cmd )
{
    RemoveExit( cmd );
}


varargs string GetExits( object viewer ) 
{
    int n, i;
    string exits, *indices, *hidden;
  
    if ( QueryProp(P_DOOR_INFOS) )
        call_other( DOOR_MASTER, "init_doors" );

    indices = m_indices( Query(P_EXITS) || rescueExit() );
    
    if ( pointerp(hidden = QueryProp(P_HIDE_EXITS)) )
        indices -= hidden;
  
    n = sizeof( indices );
    
    switch (n){
    case 0:
        return "Es gibt keine sichtbaren Ausgaenge.\n";
        
    case 1:
        return "Es gibt einen sichtbaren Ausgang: " + indices[0] + ".\n";
        
    case 2: case 3: case 4: case 5: case 6: case 7: case 8:
        exits = "Es gibt " + NUMBERS[n-2] + " sichtbare Ausgaenge: ";
        break;
        
    default:
        exits = "Es gibt viele sichtbare Ausgaenge: ";
    }
    
    for ( i = 0; i < n-2; i++ )
        exits += indices[i] + ", ";
    
    exits += indices[n-2] + " und " + indices[n-1] + ".";
    
    return break_string( exits, 78 );
}


// Richtungsbefehle nur interpretieren, wenn der Spieler *im* Raum steht und
// nicht davor (Transporter etc.)/o
void init() 
{
    if ( environment(this_player()) == this_object() )
        add_action( "_normalfunction", "", 1 );
}


/* not only normal exits are handled here */
int _normalfunction() 
{
    string verb, destroom;
    mixed message;
    int ret;

    verb = query_verb();
    destroom = (Query(P_EXITS) || ([]))[verb]; // hier kein rescueExit !!

    if ( !destroom )
        return 0;
    
    if ( closurep(destroom) ){
        ret = funcall( destroom, verb );
        
        if (ret)
            GiveEP( EP_EXIT, verb );
        
        return ret;
    }
    
    message = explode( destroom, "#" );
    
    if ( sizeof(message) == 2 ){
        destroom = message[1];
        message = message[0];
    } 
    else
        if( member( ({ "sueden", "suedwesten", "westen",
                       "nordwesten", "norden", "nordosten", "osten",
                       "suedosten" }), verb ) != -1 )
            message = "nach " + capitalize(verb);
        else if ( member( ({ "oben", "unten" }), verb ) != -1 )
            message = "nach " + verb;
        else
            message = verb;
    
    ret = this_player()->move( destroom, M_GO, message );
    
    if (ret)
        GiveEP( EP_EXIT, verb );
    
    return ret;
}


static string _MakePath( string str )
{
  string *comp;

  comp = explode( object_name(this_object()), "/" ) - ({""});
  
   switch( str[0] ){
   case '.':
       str = "/" + implode( comp[0..<2], "/" ) + "/" + str;
       break;
       
   case '~':
       str = "/" + comp[0] + "/" + (comp[0] == "d" ? comp[1] + "/" : "")
           +REAL_UID(this_object()) + str[1..];
       break;
   }
   
   return MASTER->_get_path( str, getuid(this_object()) );
}

