// bier.c
//
//   $Id: bier.c 6437 2007-08-18 22:58:58Z Zesstra $

#include <properties.h>
#include <defines.h>
#include <language.h>

inherit "std/thing";

mapping sorten;
mapping badsorten;
mapping alisorten;
mixed wahl;

void ungewaehlt()
{
	SetProp( P_SHORT, "Eine magische Bierflasche" );
	SetProp( P_LONG,
		"Diese magische Bierflasche ist momentan leer. Und das ist gut so, denn\n"
	+ "Du solltest Dich erst fuer Deine Lieblingssorte entscheiden, damit die\n"
	+	"magische Flasche Deinen Wuenschen entspricht. (Eine Speisekarte er-\n"
	+	"haeltst du mit \"bierliste\".)\n"
	);
	SetProp( P_IDS, ({ "bier","flasche","humpen","bierflasche" }) );
}

void initSorten()
{
	sorten = ([
		"krombacher" : "Krombacher Pilsner"; "",
		"becks"      : "Becks"; "",
		"jever"      : "Jever"; "",
		"flensburger": "Flensburger Pilsner"; "",
		"warsteiner" : "Warsteiner"; "",
		"veltins"    : "Veltins"; "",
		"bitburger"  : "Bitburger Pilsner"; "",
		"koenig"     : "Koenig Pilsner"; "Du fuehlst Dich wie ein Koenig.\n",
		"rolinck"    : "Rolinck Pilsner"; "Es war fuer Dich allein gebraut.\n",
		"iserlohner" : "Iserlohner Pilsner"; "",
		"paulaner"   : "Paulaner"; "",
		"erdinger"   : "Erdinger"; "",
		"radler"     : "Radler"; "Erfrischend!\n",
		"krefelder"  : "Krefelder"; "",
		"sion"       : "Sion Koelsch"; "",
		"dom"        : "Dom Koelsch"; "",
		"pinkus"     : "Pinkus Alt"; ""
	]);
	alisorten = ([
		"flens" : "flensburger",
		"flepi" : "flensburger",
		"koepi" : "koenig",
		"bit"   : "bitburger"
	]);
	badsorten = ([
		"rolink" : "Du meinst doch Rolinck, oder?\n",
		"clausthaler" : "Die Bierflasche schluettelt sich vor Ekel.\n"
	]);
}

void create()
{
	::create();
	SetProp( P_GENDER, FEMALE );
	SetProp( P_NAME, "Bierflasche" );
	SetProp( P_WEIGHT, 800 );
	SetProp( P_VALUE, 300 );
	SetProp( P_NOBUY, 1 );
	ungewaehlt(); // <-- setzt P_SHORT und P_IDS
  if (!clonep(this_object())) return;
	AddCmd( "wuensche", "wuenschen" );
	AddCmd( "bierliste", "liste" );
	AddCmd( ({"trinke","trink"}), "trinken" );
	AddCmd( ({"proste","prost","prosit"}), "prosten" );
	initSorten();
	wahl = -1;
}


int liste(string str )
{
	string ans;
	int i;

	write( "Es stehen momentan folgende Sorten zur Verfuegung: \n" );
	walk_mapping( sorten, lambda( ({ 'k, 'txt }), ({
		#'printf, "  %s\n", 'txt }) ));
	return 1;
}

int wuenschen( string str )
{
	int i;
	string bad;

	notify_fail( "WAS wuenschst Du Dir denn?\n" );
	if( !str ) return 0;
	str = lower_case(str);
	if( stringp(alisorten[str]) )
	{ // Aliase sollen auch als id erlaubt sein.
		AddId( str );
		str = alisorten[str];
	}

	if( stringp(sorten[str,0]) )
	{
		wahl = str;
		write( "Eine gute Wahl! Und schon fuellt sich die Flasche mit frischem "
			+	sorten[wahl,0] + ".\n" );
		SetProp( P_SHORT, "Eine Flasche voll " + sorten[wahl,0] );
		SetProp( P_LONG,
			"Du haelst ein schoenes, grosses und vor allem kuehles " 
		+ sorten[wahl,0] + " in\nDeinen Haenden.\n"
		);
		AddId( wahl );
		return 1;
	}

	if( bad = badsorten[str] )
	{
		write( bad );
		return 1;
	}

	if( member_array( str, ({ "pilz","bier","pils","alt","koelsch",
		"hefe","weizen","weizenbier","altbier" }) ) != -1
	)
	{
		write( "Ein bischen genauer sollte es schon sein.\n" );
		return 1;
	}
	write( "So ein Pech, die Sorte ist gerade aus.\n" );
	log_file( "rumata/biere", ctime(time()) + ": "
		+ PL->name(WER) + " moechte ein " + capitalize(str) + ".\n"
	);
	return 1;
}

int trinken( string str )
{
	if( !str || !id(str) ) return 0;
	if( !stringp(wahl) )
	{
		write( "Du nuckelst an einer leeren Bierflasche.\n" );
		say( PL->name(WER,2)
			+ " nuckelt an einer leeren Bierflasche herum (So ein Kind!)\n"
		);
		return 1;
	}
	write( "Ahhh, Du trinkst Dein " + sorten[wahl,0]
		+ " mit Genuss in einem Zug auf.\n" + sorten[wahl,1]
	);
	if( random(3) == 0 )
	{
		write( "Du hoerst den Teufel leise lachen:\n"
			+	"  'Nichts geht schneller an die Niere,\n"
			+	"   als der Rumata ihre Biere.'\n"
		);
	}
	say( PL->name(WER,2) + " trinkt " + PL->QPP( NEUTER, WEN ) + " "
		+	sorten[wahl,0] + ".\n" + capitalize(PL->QueryPronoun(WER))
		+	" scheint es sichtlich zu geniessen.\n"
	);
	ungewaehlt();
	wahl = -1;
	return 1;
}

int prosten( string str )
{
	string arg;
	object obj;

	if( !stringp(wahl)  )
	{
		write( "Mit einer leeren Flasche?\n" );
		return 1;
	}
	if( !str )
	{
		write( "Du prostest der Gemeinde zu.\n" );
		say( PL->name(WER,2) + " sagt Prost!\n" );
		return 1;
	}
	if( sscanf( str, "%s zu", arg ) == 1 )
		str = arg;
	if( !(obj=present(str,environment(PL))) || !living(obj) )
	{
		write( "Keinen Saufbruder gefunden (So'n Frust).\n" );
		return 1;
	}
	write( "Du prostest " + obj->name(WEM,2) + " zu.\n" );
	tell_object( obj, PL->name(WER,2) + " prostet Dir zu.\n" );
	say( PL->name(WER,2) + " prostest " + obj->name(WEM,2) + " zu.\n", obj );
	return 1;
}
