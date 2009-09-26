// MorgenGrauen MUDlib
//
// npc/sequencer.c -- Scripte und Trigger fuer NPCs :)
//
// $Id: sequencer.c 7084 2009-01-19 21:06:40Z Zesstra $

// seq2.c
// Version 2.0 des Sequencers von Don Rumata 28.06.93
// 
// Sinn:
//   Bereitstellung von Funktionen, ddamit ein npc mehrere Befehle
//   bei Eintritt eines bestimmten Ereignisses nacheinander
//   automatisch durchfuehrt.
//
// Ereignisse:
//   TellEvent: Es wird etwas in dem Raum, in dem der npc sich
//              befindet, gesagt.
//   GiveEvent: Es wird dem npc etwas gegeben.
//
// Programm:
//   Ein Programm ist eine Liste von Befehlen.
//   Jeder Befehl ist eine Liste, bestehend aus einem Kommando
//   und einer Zahl.
//   Das Kommendo wird aehnlich der Befehle, die ein Spieler ein-
//   gibt ausgefuehrt.
//   Vorsicht: NPCs koennen nur einen Teil der Befehle, die ein
//   Spieler kann, dafuer aber immer 'echo' und 'emote'.
//   Die Zahl gibt die Anzahl der Sekunden an, in der der naechste
//   Befehl ausgefuehrt wird.
//
// Funktionen:
//   RegisterTell( funktion, programm )
//     Wenn dem npc etwas gesagt wird, so wird die gesagte Meldung
//     an die Funktion uebergeben. Gibt die Funktionen nicht 0
//     zurueck, wird das Programm gestartet.
//   RegisterGive( funktion, programm )
//     Wird dem npc etwas gegeben, so wird das Objekt an die
//     Funktion uebergeben. Gibt die Funktion nicht 0 zurueck, so
//     wird das Programm gestartet.
//   Load( programm )
//     Starte das angegebene Programm.
//
//   give_notify() gibt eine 1 zurueck, wenn das Objekt akzeptiert
//     wurde. (Es muss - falls gewuenscht - dann von Hand zuruech-
//     gegeben werden. (give_obj(ob,this_player())) in der Funk.)
//   mittels add_action() kann man im create() ds NPCs eigene
//     Verben fuer den NPC einfuehren.
//
// Es kann immer nur eine Funktion (egal ob Tell oder Give) angemeldet
// sein. Es kann immer nur ein Programm gleichzeitig laufen.
//
// Ideen und Bugreports an Rumata

string givefun, tellfun;
mixed *program;
int isRunning;

// zur sicherheit nicht von aussen aufrufbar !!!!
static RegisterGive( fun, prog )
{
  if( isRunning ) return;
  program = prog;
  tellfun = 0;
  return givefun = fun;
}

// zur sicherheit nicht von aussen aufrufbar !!!!
static RegisterTell( fun, prog )
{
  if( isRunning ) return;
  program = prog;
  givefun = 0;
  return tellfun = fun;
}

// zur sicherheit nicht von aussen aufrufbar !!!!
static Start()
{
  if( isRunning || !objectp(this_object())) return;
  isRunning = 1;
  call_out( "nextStep", 0, 0 );
}

static Load( prog )
{
  if( isRunning ) return;
  program = prog;
  Start();
}

catch_tell( str )
{
  if( isRunning || previous_object()==this_object()) return;
  if( stringp(tellfun) && call_other( this_object(), tellfun, str ) )
    Start();
}

give_notify( ob )
{
  if( isRunning ) return;
  if( stringp(givefun) && call_other( this_object(), givefun, ob ) )
  {
    Start();
    return 1;
  }
}

static nextStep( line )
{
  if( !isRunning ) return;
  if( line >= sizeof( program ) )
  {
    isRunning = 0;
    return;
  }
  command( process_string( program[line][0] ) );
  if( !isRunning || !objectp(this_object()) ) return;
  call_out( "nextStep", program[line][1], line+1 );
}

QueryProg()
{
  return program[0..];
} 
