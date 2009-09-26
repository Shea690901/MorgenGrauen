// MorgenGrauen MUDlib
//
// sdefines.h -- Nuetzliche Definitionen von Sir.
//               Man sollte allerdings besser <defines.h> benutzen.
//
// $Id: sdefines.h 3059 2006-09-23 09:50:24Z root $

#include <language.h>
#include <properties.h>
#include <wizlevels.h>

#define bool int
#define FALSE 0
#define TRUE 1

#define ON  1
#define OFF 0

#define DEFAULT_PROBABILITY 30

#define TP             this_player()
#define this_room()    this_object()
#define this_monster() this_object()

#define NAME           TP -> QueryProp( P_NAME )
#define LEVEL          TP -> QueryProp( P_LEVEL )
#define TITLE          TP -> QueryProp( P_TITLE )
#define POSS           TP -> QueryPossPronoun( TP -> QueryProp( P_GENDER ), 3 )

#define QNAME( ob )    ob -> QueryProp( P_NAME )
#define QLEVEL( ob )   ob -> QueryProp( P_LEVEL )
#define QTITLE( ob )   ob -> QueryProp( P_TITLE )
#define QPOSS( ob )    ob -> QueryPossPronoun( ob -> QueryProp( P_GENDER ), 3 )
#define QPLAYER( ob )  query_once_interactive( ob )

#define SetHP( hp )    SetProp( P_MAX_HP, hp ) ; SetProp( P_HP, hp ) ;
#define SetSP( sp )    SetProp( P_MAX_SP, sp ) ; SetProp( P_SP, sp ) ;
#define SetNAME( n )   SetProp( P_NAME, capitalize( n )) ;  AddId( n ) ;\
                       set_living_name( n ) ;

#define CAP( str )     capitalize( str )
#define LOW( str )     lower_case( str )
