/*
  
  Babelfisch.c von Fraggle
  Version 1.0
  
  Mit dem Babelfisch kann man auf einfache Weise Properties fuer das 
  Angeln in seine Raeume schreiben.

*/

inherit "std/thing";
#include <language.h>
#include <properties.h>
#include <fishing.h>
#include <wizlevels.h>
// #include "/d/ebene/fraggle/files.h"
// defines hier hinein kopiert
#define HOME(x) "/d/ebene/fraggle/"+x
#define TXT(x) HOME("txt/")+x+".txt"
// end
#define QueryDetail(o,x) member(o->QueryProp(P_DETAILS),x)
#define bs( x ) break_string( x, 78 )

private int i, first_setprop_line,first_include_line,startline, endline;
private string oldfname,waterstr,*file_lines, old_file, actual_file;

create()
{
 if (!clonep(this_object())) return;
 ::create();
 waterstr="FAILURE"; oldfname="";
 AddId(({"babelfisch","fisch"}));
 SetProp(P_NAME, "Babelfisch" );
 SetProp( P_GENDER , MALE );
 SetProp(P_ARTICLE, 1 );
 SetProp(P_SHORT, "Ein klitzekleiner Babelfisch" );
 SetProp(P_LONG, 
 "Du siehst einen klitzekleinen Babelfisch.\n"
 +"Er kann Dir helfen, in Deinen Raeumen das Gewaesser zu ermitteln\n"
 +"und die Flags zu setzen, damit man dort Angeln kann. Hilfe kannst\n"
 +"Du mit dem Befehl <babel> bekommen. Bei Problemen sag Fraggle bescheid.\n");
 SetProp(P_NOBUY,1);
 SetProp(P_VALUE,1);
 SetProp(P_WEIGHT,0 );
 SetProp(P_AUTOLOADOBJ,1);
 seteuid(getuid(this_object()));
}

init()
{
  ::init();
  if(IS_WIZARD(this_player()))
   {
     add_action( "set_raum","writewater");
     add_action( "teste_wasser","wt");
     add_action( "teste_wasser","analysiere");
     add_action( "teste_wasser","watertest");
     add_action( "setwater","setwater");
     add_action( "wechdamit","brate");
     add_action( "zeige_etwas","view");
     add_action( "hilf_mir","babel");
   }
}
setwater(str)
{
 if(!str)str=waterstr;
write(
  "---------------------------------------------------------------------\n"
 +"Der Gewaessertyp kann manuell mit setwater <typ> gesetzt werden,\n"
 +"Es gibt folgende Salzwasser-Typen: meer  hafen strand\n"
 +" und  folgende  Suesswasser-Typen: fluss teich gebirgsbach bach see\n"
 +"---------------------------------------------------------------------\n");
 waterstr=str;
 if(DecodeType(waterstr)!="FAILURE")
 write("Der Typ ist zur Zeit gesetzt auf "+waterstr+".\n");
 if(DecodeType(waterstr)!="FAILURE")
  {
   write(
   "---------------------------------------------------------------------\n"
   +"Folgendes Zeilen muessen im Raum-source eingefuegt werden:\n"
   +" #include <fishing.h>  ->  zu den includes\n"
   +" SetProp(P_WATER,"+DecodeType(waterstr)+");  ->  im create()-block\n");
   return 1;
  }
  write("Es ist zur Zeit kein gueltiger Typ gesetzt!\n");
  return 1;
}
teste_wasser()
{
 int r1,r2;
 write("Teste auf Details:\n------------------\n");
 if(r1=environment(this_player())->QueryProp(P_WATER))
  {
  write("Der Typ des Gewaessers wurde bereits auf <"+DecodeFlag(r1)+"> gesetzt!\n");
  return 1;
  }
 if(!QueryDetail(environment(this_player()),"wasser"))write("Kein Detail Wasser zu sehen!\n");
 else write("Es wurde Wasser in diesem Raum gefunden!\n");
 r2=0;
 waterstr="FAILURE";
 if(QueryDetail(environment(this_player()),"fluss"))
  {
   r2++;
   write("Detail Fluss gefunden.\n");
   waterstr="fluss";
  }
 if(QueryDetail(environment(this_player()),"bach"))
  {
   r2++;
   write("Detail Bach gefunden.\n");
   waterstr="bach";
   if(QueryDetail(environment(this_player()),"gebirge"))
    {
     write("Detail Gebirge gefunden.\n");
     waterstr="gebirgsbach";
    }
  }
if(QueryDetail(environment(this_player()),"meer")||QueryDetail(environment(this_player()),"ozean"))
 {
  r2++;
  write("Detail Meer/Ozean gefunden.\n");
  waterstr="meer";
 }
if(QueryDetail(environment(this_player()),"hafen"))
  {
   r2++;
   write("Detail Hafen gefunden.\n");
   waterstr="hafen";
  }
if(QueryDetail(environment(this_player()),"teich"))
  {
   r2++;
   write("detail Teich gefunden.\n");
   waterstr="teich";
  }
if(QueryDetail(environment(this_player()),"strand"))
  {
   r2++;
   write("detail Strand gefunden.\n");
   waterstr="strand";
  }
if(QueryDetail(environment(this_player()),"see"))
  {
   r2++;
   write("detail See gefunden.\n");
   waterstr="see";
  }
if(r2>1) write(
 "Es wurden mehrere Details gefunden! Bitte ueberpruefe, ob gewaehlter Typ OK!\n");
if(r2==0) write(
 "Es wurde kein Detail im Raum gefunden, das einen Hinweis auf die Art\n"
 +"des Gewaessers schliessen laesst.\n");
if(waterstr!="FAILURE") write(
 "Der Gewaessertyp wurde als "+capitalize(waterstr)+" identifiziert.\n");
write(
 "---------------------------------------------------------------------\n"
+"Der Gewaessertyp kann auch manuell mit setwater <typ> gesetzt werden.\n"
+"Es gibt folgende Salzwasser-Typen: meer  hafen strand\n"
+"und folgende Suesswasser-Typen   : fluss teich gebirgsbach bach see\n"
+"---------------------------------------------------------------------\n");
if(DecodeType(waterstr)=="FAILURE")
{
write("Der Typ ist nicht gesetzt, bitte mit setwater <typ> setzen!\n");
return 1;
}
if(r2)write(
 "Folgende Zeilen muessen im Raum-Source eingefuegt werden:\n"
 +" #include <fishing.h>  ->  zu den includes\n"
 +" SetProp(P_WATER,"+DecodeType(waterstr)+");  ->  im create()-block\n"
 +"---------------------------------------------------------------------\n");
 return 1;
}
string DecodeType(string str)
 {
  switch(lower_case(str))
   {
    case "meer"  : return "W_OCEAN";
    case "fluss" : return "W_RIVER";
    case "hafen" : return "W_HARBOR";
    case "strand": return "W_BEACH";
    case "teich" : return "W_POOL";
    case "see"   : return "W_LAKE";
    case "bach"  : return "W_STREAM";
    case "gebirgsbach": return "W_ROCK";
    default: return "FAILURE";
   }
 }
DecodeFlag(i)
 {
  switch(i)
   {
    case W_STREAM: return "Bach";
    case W_OCEAN : return "Meer" ;
    case W_RIVER : return "Fluss";
    case W_HARBOR: return "Hafen";
    case W_BEACH : return "Strand";
    case W_POOL  : return "Teich";
    case W_LAKE  : return "See";
    case W_ROCK  : return "Gebirgsbach";
    case W_USER  : return "Userdefined";
    default      : return "*unbekannt*";
   }
  return "ERROR!!! Fraggle Bescheid sagen!";
  }
wechdamit(str)
{
 notify_fail("Was willst Du denn braten?\n");
 if(!id(str))return 0;
 write("Herzloserweise braetst Du den Babelfisch und isst ihn auf.\n");
 destruct(this_object());
 return 1;
}
hilf_mir()
 {
   return this_player()->more(TXT("babel"));
 }
zeige_etwas(str)
 {
   return this_player()->more(TXT(str));
   return 0;
 }
int reset_vars()
{
  i =first_include_line= first_setprop_line = startline = endline = 0;
  file_lines =  ({});
  old_file = actual_file;
  return 1;
}
int set_raum(string str )
 {
 int r1;
 r1=0;
 if(r1=environment(this_player())->QueryProp(P_WATER))
  {
   write("Der Typ des Gewaessers wurde bereits auf <"+DecodeFlag(r1)+"> gesetzt!\n"
   +"Ich kann diesen Raum nicht mehr aendern! Bitte manuell aendern!\n");
   return 1;
  }

 reset_vars();

 if(!str)actual_file=object_name(environment(this_player()))+".c";
 else
  {
   if ( str[0..0] == "/" )
    {
     if ( str[<2..] != ".c" )
      actual_file = str+".c";
     else
      actual_file = str;
     }
    else
     {
       if ( str[<2..] != ".c" )
            actual_file = this_player()->QueryProp( P_CURRENTDIR )+"/"+str+".c";
       else
           actual_file = this_player()->QueryProp( P_CURRENTDIR )+"/"+str;
       }
    }
   if ( file_size( actual_file )==-1 )
     {
       write( "File: "+actual_file+" nicht gefunden!\n" );
       return 1;
     }
 call_other( actual_file, "???" );
 write( "\n------------------------------------------------------------------\n");
 write( "Aktuelles File:   "+actual_file+"\n" );
 write( "Filegroesse:      "+file_size( actual_file )+"\n" );
 write( "Userid des Files: "+getuid( find_object( actual_file ) )+"\n" );
 first_setprop_line = first_include_line = 0;
 old_file = read_file( actual_file );
 file_lines = ({ "dummy" }) + old_explode( old_file, "\n" );
 startline = 1;
 endline = sizeof( file_lines )-1;
 for( i = startline; i <= endline; i++ )
  {
   if ( strstr( file_lines[i], "SetProp" ) != -1 )
    {
     if ( !first_setprop_line ) first_setprop_line = i;
    }
   if ( strstr( file_lines[i], "#include" ) != -1 )
    {
     if ( !first_include_line ) first_include_line = i;
    }
  }
 if ( !first_setprop_line )
  {
   write( bs( "Es wurden im File "+actual_file+" keine "+
   "Properties gefunden! Um Details hinzufuegen zu koennen, "+
   "muss mindestens ein SetProp bereits vorhanden sein!" ) );
   write( "-----------------------------------------------------------------\n");
   return 1;
  }
 if ( !first_include_line )
   {
    write( bs( "Es wurden im File "+actual_file+" keine "+
    "Includes gefunden! Um Details hinzufuegen zu koennen, "+
    "muss mindestens ein #include bereits vorhanden sein!" ) );
    write( "-----------------------------------------------------------------\n");
    return 1;
    }
 write("First #include @line "+first_include_line
    +"\nFirst SetProp  @line "+first_setprop_line+"\n");
 if(DecodeType(waterstr)=="FAILURE")
  {
   write("Abbruch, kein Gewaessertyp gesetzt! Starte stattdessen Test-Funktion!\n"
   +"Zum Schreiben musst Du diese Funktion erneut aufrufen!\n");
   teste_wasser();
   return 1;
  }
 write(
  "---------------------------------------------------------------------\n"
  +"Folgendes Zeilen werden in "+actual_file+" eingefuegt:\n"
  +" #include <fishing.h>\n"
  +" SetProp(P_WATER,"+DecodeType(waterstr)+");\n"
  +"---------------------------------------------------------------------\n"
  +"Das Gewaesser wird ab dann als "+capitalize(waterstr)+" identifiziert!\n\n"
  +"OK? [ja | irgendwas anderes fuer Abbruch]\n");
  input_to("DoItNow");
 return 1;
}
DoItNow(str)
{
  if(str!="ja")
   {
    write("Abbruch !\n");
    return 1;
   }
  write("writing file "+actual_file+"\n");
 if ( rename( actual_file,actual_file+".old") )
   {
    write( "File: "+actual_file+" nicht gefunden/no write permission.\n");
    return 1;
   }
  write("File "+actual_file+" copied to: "+actual_file+".old\n");
  write("First file line: "+file_lines[1]+"\n");
  write("Adding #include <fishing.h>\nbefore "
        +file_lines[first_include_line]+"\n");
  write("Adding SetProp(P_WATER,"+DecodeType(waterstr)+");\nbefore "
          +file_lines[first_setprop_line]+"\n");
  write("Last file line: "+file_lines[endline]+"\n");
  write("...writing");

  for(i=1;i<first_include_line;i++)write_file(actual_file,file_lines[i]+"\n");
  write_file(actual_file,"#include <fishing.h>\n");
  for(;i<first_setprop_line;i++)write_file(actual_file,file_lines[i]+"\n");
  write_file(actual_file,"  SetProp(P_WATER,"+DecodeType(waterstr)+");\n");
  for(;i<=endline;i++)write_file(actual_file,file_lines[i]+"\n");

  write("...uploading...");
  this_player()->command_me("here make");
  write("Ready! (Download via ftp nicht vergessen! ;)\n");
 return 1;
}
