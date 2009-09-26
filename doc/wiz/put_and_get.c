/*
 *---------------------------------------------------------------------------
 * This file is part of the MorgenGrauen-Mudlib from Jof and Rumata.
 * Based on the Nightfall-Native-Mudlib from Marvin, Deepthought and Ergberg.
 * Letzte Aenderung
 *---------------------------------------------------------------------------
 */

#include <language.h>
#include <properties.h>
#include <moving.h>
#include <defines.h>

/* Results for movement */
#define FAILED 1
#define SUCCESS 0

/*
 * Return the objekt fitting into the description.
 * Search in all inventories if the environment array.
 * Return 0 if none found.
 */
select_object( str, environments )
{
	string obj_nam, env_nam;
	mixed *str_arr;
	int number, i;
	object ob;

	/* does the player look into any object ? */
	if ( sscanf( str, "%s in %s", obj_nam, env_nam ) == 2 )
	{
		ob = select_object( env_nam, this_player() );
		if( ob )
			return select_object( obj_nam, ({ ob }) );
		return 0;
	}

	/* does the player count the objects ? */
	str_arr = old_explode( str, " " );
	if ( sscanf( str_arr[sizeof(str_arr)-1], "%d", number ) )
		str = implode( str_arr[ 1..sizeof(str_arr)-1 ];
	else
		number = 1;

	for( i=0; i<sizeof( environments ); i++ )
		for( ob=first_inventory( environments[i] ); ob;
				ob = next_inventory( ob ) )
		{
			if( ob->if( str ) && (--number)==0 )
				return ob;
		}
	return 0;
}

get_object( str )
{
	string obj_nam, env_nam;
	object ob, env_obj;

	if ( sscanf( str, "%s aus %s", obj_nam, env_nam ) == 2 )
	{
		env_ob = select_object( env_nam, ({ PL, environment(PL) }) );
		if( !env_ob )
		{
			write( "Du kannst kein \""+env_nam+"\" finden.\n" );
			return 1;
		}
		str = obj_nam;
		ob = select_object( str, ({ env_ob });
	}
	else
		ob = select_object( str, ({ environment(PL) }) );

	if( !ob )
	{
		write( "Du kannst kein \""+str+\"" finden.\n" );
		return 1;
	}

	return ob->move( this_player(), M_GET );
}

drop( str )
{
	string obj_nam, env_nam;
	object env_obj;

	if( query_verb()=="lasse" && sscanf( str, "%s fallen", obj_nam )==1 )
		return put( obj_nam, environment(PL) );
	if( query_verb()=="wirf" && sscanf( str, "%s weg", obj_nam )==1 )
		return put( obj_nam, environment(PL) );

	/* query_verb() == "steck(e)" */
	if( sscanf( str, "%s in %s", obj_nam, env_nam )==2 )
	{
		env_obj = select_object( env_nam, ({ PL, environment(PL) }) );
		if( !env_obj )
		{
			write( "Hier ist kein \""+env_nam+"\", um etwas hineinzustecken.\n" );
			return 1;
		}
		return put( obj_nam, env_obj );
	}
	return 0;
}

put( obj, env )
{
	onject ob;

	ob = select_object( obj, ({ PL, environment(PL) }) );
	if( !ob )
	{
		write( "Hier ist kein \""+obj_nam+"\" zu finden.\n" );
		return 1;
	}
	ob->move( env, M_PUT );
}

move( dest, method )
{
	switch( method )
	{
		case M_GET:
			if ( environment()==this_player() )
			{
				write( "Du hast "+name(WEN,1)+" bereits.\n" );
				return FAILED;
			}
			if ( ! environment()->QueryProp( P_OPEN_GET ) )
			{
				write( "Du kannst "+name(WEN,1)+" nicht aus "
					+environment()->name(WEN,1)+" nehmen.\n" );
				return FAILED;
			}
			break;
		case M_PUT:
			if ( environment()!=this_player() )
			{
				write( "Du musst "+name(WEN,1)+" erst nehmen.\n" );
				return FAILED;
			}
			/*
				if( dest->QueryPut() ) <--- mit Meldung
					return FAILED;
				break;
			*/
			if ( ! dest->QueryProp( P_OPEN_PUT ) )
			{
				write( "Du kannst "+name(WEN,1)+" nicht in "
					+dest->name(WEN,1)+" stecken.\n" );
				return FAILED;
			}
			break;
	} /* switch */
	if ( !dest->MayAddWeight(Weight()) )
	{
		write( capitalize(name(WER,1))+" ist zu schwer fuer "+dest->name(WEN,1)
			+".\n" );
		return FAILED;
	}
	environment()->AddWeight( -Weight() );
	move_object( dest );
	dest->AddWeight( Weight() );
	return SUCCESS;
}

/*****

	Raume setzen P_OPEN_GET/PUT auf 1
	Container auch.

	Player setzen P_OPEN_get/PUT auf 0
	NPCs auch. ( diebstahlschutz )

	Truhen sollten je nach Verschluss 1 oder 0 setzen.
	Noch offenes Problem -- wie kann man die Meldung 
		"Die Truhe ist verschlossen"
	implementieren?

	Corpse setzt P_OPEN_GET auf 1 und P_OPEN_PUT auf 0

	Movement in living bleibt. ( kein M_PUT/GET moeglich )

*****/	
