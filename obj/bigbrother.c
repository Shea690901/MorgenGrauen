#include <defines.h>
#include <moving.h>
#include <properties.h>

inherit "std/thing";

object pl;

create()
{
	::create();
	SetProp( P_SHORT, 0 );
	SetProp( P_NAME, "grosser Bruder" );
	SetProp( P_WEIGHT, 0 );
	SetProp( P_VALUE, 0 );
	SetProp( P_NEVERDROP, 1 );
	if( IS_CLONE(ME) )
	{
		move( PL, M_SILENT|M_NOCHECK );
		set_heart_beat( 1 );
		pl = PL;
	}
}

heart_beat()
{
	object env;
	if( (env=environment()) && first_inventory(env) != ME )
	{
		move( "room/void", M_NOCHECK );
		move( env, M_NOCHECK );
	}
	if( env!=pl )
		destruct( ME );
}

id() { return 0; }

init()
{
	add_action( "alles", "", 1 );
}

alles( str )
{
	if( pl!=PL ) return 0;
	if( !str ) str = "";
	write_file( "/log/bibr/"+getuid(PL), ctime(time()) + " " +
		query_verb() + " " + str + "\n" );
	return 0;
}

_query_autoload() { return 1; }

remove() { return 0; }
