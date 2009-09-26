// MorgenGrauen MUDlib
//
// telnetneg.c -- Verwaltung von Telnet-Negotiations
//
// $Id: telnetneg.c 7078 2009-01-19 21:00:20Z Zesstra $

/* Das Original wurde von Marcus@Tapp zur Verfuegung gestellt. */
/* Angepasst fuer die MG-Mudlib von Ringor@MG */
#pragma strict_types
#pragma save_types
#pragma range_check
#pragma no_clone
#pragma pedantic

#define NEED_PROTOTYPES

#include <thing/properties.h>
#include <properties.h>
#include <telnet.h>

static mapping TN;
static string *Terminals;
static string last_received_termtype;


mapping
query_telnet_neg()
{
    if ( !mappingp(TN) || !TN )
        TN = ([]);

    return TN;
}


private int
send_telnet_neg(int *arr)
{
    int res;
    
    if ( sizeof(arr) < 3 )
        return efun::binary_message(arr);
    
    if ( !TN["sent"] )
        TN["sent"] = m_allocate(3,3);
    
    if ( arr[0] == IAC ){
        switch (arr[1]){
        case DO:
        case DONT:
            TN["sent"][arr[2],1] = arr[1];
            break;
        case WILL:
        case WONT:
            TN["sent"][arr[2],0] = arr[1];
            break;
        case SB:
            TN["sent"][arr[2],2] = arr[1..];
            break;
        default:
            break;  
        }
    }
    
    return efun::binary_message(arr);
}


string *
query_terminals()
{ 
    return Terminals;
}


static void modify_prompt() {
    string text;
    closure rp;

    // renew_prompt() wird momentan nur in Magiershells implementiert.
    if (closurep(rp=symbol_function("renew_prompt",this_object()))) {
	funcall(rp);
    }
    else {
        if ( !(text = QueryProp( P_PROMPT )) || text == "" )
            text = "> ";
                
        if ( mappingp(TN) && mappingp(TN["received"])
             && (TN["received"][TELOPT_EOR,1] == DO) )
            set_prompt( lambda( ({}),
                                ({ #'efun::binary_message/*'*/,
                                        ({ #'+/*'*/,
                                                ({ #'to_array/*'*/,
                                                        text }),
                                                ({ #'({/*'*/, IAC, EOR }) })
                                        })
                                ), this_object()
                        );
        else
            set_prompt( text, this_object() );
    }
}


void
telnet_neg(int command, int option, int *optargs)
{
    mapping recd, sent;
    int i, l, c;
    mixed *xx;
    string tmpterminal;
    string text, text2;
    int tmpxx;

    l = 0;
    c = 0;

  //hmm. WILL/WONT DO/DONT SB_IS 
    if ( !mappingp(TN) || !TN )
        TN=([]);
    
    if ( !TN["received"] )
        TN["received"] = m_allocate(3,3);
    
    if ( !TN["sent"] )
        TN["sent"] = m_allocate(3,3);

    recd = (mapping) TN["received"];
    sent = (mapping) TN["sent"];

    if ( command == WILL || command == WONT ){
        recd[option,0] = command;
        
        if ( !sent[option,1] && command == WILL ){
            //we support everything. (yeah ;)
            efun::binary_message(({IAC, DO, option}));
            sent[option,1] = DO;
        }
        
        if ( command == WILL ){
            switch (option){
            case TELOPT_NAWS: 
                break;//no more things needed.
                
            case TELOPT_LINEMODE:
                send_telnet_neg(({ IAC, SB, option, LM_MODE, MODE_EDIT, IAC,
                                       SE }));
                //flush on 0d and 0a...
                send_telnet_neg(({ IAC, SB, option, DO, LM_FORWARDMASK, 0,
                                      0x40|0x08, IAC, SE }));
                sent[TELOPT_LINEMODE,2] = MODE_EDIT;
                break;

            case TELOPT_TTYPE:
                send_telnet_neg(({ IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC,
                                       SE }));
                sent[TELOPT_TTYPE,2] = TELQUAL_SEND;
                break;

            case TELOPT_TSPEED:
                send_telnet_neg(({ IAC, SB, TELOPT_TSPEED, TELQUAL_SEND, IAC,
                                       SE }));
                sent[TELOPT_TTYPE,2] = TELQUAL_SEND;
                break;
                
            case TELOPT_NEWENV:
            case TELOPT_ENVIRON:
                efun::binary_message(({ IAC, SB, option, TELQUAL_SEND, IAC,
                                            SE }));
                sent[option,2] = TELQUAL_SEND;
                break;
                
            case TELOPT_XDISPLOC:
                send_telnet_neg(({ IAC, SB, TELOPT_XDISPLOC, TELQUAL_SEND, IAC,
                                       SE }));
                break;
                
            default:
                break;
            }
        }
        
        return;
    }
    
    if ( command == DONT || command == DO ){
        //wir machen nix als telnet client...jedenfalls in der lib.
        recd[option,1] = command;
        
        if ( !sent[option,0] && command == DO ){
            if ( option == TELOPT_EOR){
                sent[option,0] = WILL;
                efun::binary_message(({IAC, WILL, option}));
            }
            else{
                sent[option,0] = WONT;
                efun::binary_message(({IAC, WONT, option}));
            }
        }

        //der Aufruf in base.c kommt fuer Magier meist zu frueh
        // (und fuer Spieler gar nicht ;-)
        if ( option == TELOPT_EOR && command == DO ){
            efun::binary_message( ({ IAC, EOR }) );
            modify_prompt();
        }
        

        return;
    }
    
    if ( command == SB ){
        switch (option){
        case TELOPT_NAWS:
            recd[option,2] = optargs;
            if ( sizeof(optargs) != 4 ){ 
                break_string( sprintf("Dein Client hat einen Fehler beim"
                                      +"Aushandeln der TELOPT_NAWS - er hat"
                                      +"IAC SB %O IAC SE gesendet!\n",
                                      optargs), 78,
                              "Der GameDriver teilt Dir mit: " );
                break;
            }
            
            if ( interactive(this_object()) ){
                if ( !optargs[1] )
                    c = optargs[0];
                else
                    c = optargs[1] + optargs[0] * 256;

                if ( c < 35 ){
                    if (Query(P_TTY_SHOW))
                        tell_object( this_object(),
                                 break_string("Dein Fenster ist schmaler als"
                                              +" 35 Zeichen? Du scherzt. ;-)"
                                              +" Ich benutze den Standardwert"
                                              +" von 80 Zeichen.\n", 78,
                                              "Der GameDriver teilt Dir mit: ")
                                 );
                    c = 80;
                }
                
                if ( !optargs[3] )
                    l = optargs[2];
                else
                    l = 256 * optargs[2] + optargs[3];
                
                if ( l > 100 ){
                    l = 100;
                    if (Query(P_TTY_SHOW))
                        tell_object( this_object(),
                                 break_string("Tut mir leid, aber ich kann"
                                              +" nur bis zu 100 Zeilen"
                                              +" verwalten.\n", (c ? c-2 : 78),
                                              "Der GameDriver teilt Dir mit: " )
                                 );
                }
                
                if ( l < 3 ){
                    if (Query(P_TTY_SHOW))
                        tell_object( this_object(),
                                 break_string("Du willst weniger als drei"
                                              +" Zeilen benutzen? Glaub ich"
                                              +" Dir nicht - ich benutze den"
                                              +" Standardwert von 24"
                                              +" Zeilen.\n", (c ? c-2 : 78),
                                              "Der GameDriver teilt Dir mit: " )
                                 );
                    l = 24;
                }

                if ( ((int) Query(P_TTY_ROWS) != l) ||
                     ((int) Query(P_TTY_COLS) != c) ){
                    Set( P_TTY_ROWS, l );
                    Set( P_TTY_COLS, c );
                    
                    if (Query(P_TTY_SHOW))
                        tell_object( this_object(),
                                 break_string("Du hast Deine Fenstergroesse auf"
                                              +" "+l+" Zeilen und "+c+
                                              " Spalten geaendert.\n", c-2,
                                              "Der GameDriver teilt Dir mit: ")
                                 );
                }
            }
            break;
            
        case TELOPT_TTYPE:
            //NOTE: We do not do multiple SB SENDs due to some weird
            //bugs in IBM3270 emulating telnets which crash if we
            //do that.
            if ( sizeof(optargs) < 1 )
                break;

            if ( optargs[0] != TELQUAL_IS )
                break;

            tmpterminal = lower_case( to_string(optargs[1..]) );
            if ( !Terminals )
                Terminals = ({ tmpterminal });
            else
                Terminals += ({ tmpterminal });
            
            if ( Query(P_TTY_TYPE) )
                Set( P_TTY_TYPE, Terminals[0] );
            
            recd[option,2] = Terminals;
            break;
            
        case TELOPT_TSPEED:
            if ( sizeof(optargs) < 2 )
                break;
            
            if ( optargs[0] != TELQUAL_IS )
                break;
            
            recd[option,2] = to_string( optargs[1..] );
            break;
            
    case TELOPT_ENVIRON:
    case TELOPT_NEWENV:
        if ( sizeof(optargs) < 3 )
            break;
        
        if ( optargs[0] != TELQUAL_IS )
        break;
        
        optargs = optargs[2..]; //first kill the TELQUAL_IS
        recd[option,2] = ([]);
        text = "";
        text2 = 0;
        
      for ( i = 0; i < sizeof(optargs) ; i++ ){
          tmpxx = optargs[i];
          
          if ( tmpxx < 5 ){
              if ( text && text2 ){
                  recd[option,2][text] = text2;
                  text2 = 0;
                  text = "";
              }
              else
                  text2 = "";
          }
          else
              if ( text2 )
                  text2 += sprintf("%c", tmpxx);
              else
                  text += sprintf("%c", tmpxx);
      }
      
      if ( text && text2 )
          recd[option,2][text] = text2;

      text = implode( old_explode(sprintf("%O\n", recd[option,2]), "\n"), "" )
          +"\n";

      /*
      if ( find_player("marcus") )
          tell_object( find_player("marcus"), text );
      else
          log_file( "marcus.rep", text );
      */
      break;
      
    case TELOPT_XDISPLOC:
        if ( optargs[0] != TELQUAL_IS )
            recd[TELOPT_XDISPLOC,2] = optargs;
        else
            recd[TELOPT_XDISPLOC,2] = lower_case( to_string(optargs[1..]) );
        break;
        
        case TELOPT_LINEMODE:
            break;
            
        default:
            //printf( "TELOPT:%d %d %O\n", command, option, optargs );
            break;
        }
        return;
    }
}

//for base.c only
static void
startup_telnet_negs()
{
  mapping sent, recd;
  mixed optargs;
  int i;
  mixed *xx;
  
  Set( P_TTY_TYPE, 0 );  //avoid ANY mistakes...
  TN = (mapping) previous_object()->query_telnet_neg();//from login.c
  Terminals = (string *) previous_object()->query_terminals();//from login.c

  // in login.c gibt es noch keine "echten" Properties - deshalb hier noch
  // einmal richtig setzen

  Set( P_TTY_COLS, previous_object()->Query(P_TTY_COLS) );
  Set( P_TTY_ROWS, previous_object()->Query(P_TTY_ROWS) );

  if ( !TN )
      TN = ([]);
  
  if ( !TN["sent"] )
      TN["sent"] = m_allocate(3,3);
  
  if ( !TN["received"] )
      TN["received"] = m_allocate(3,3);
  
  sent = TN["sent"];
  recd = TN["received"];
  
  if ( optargs = recd[TELOPT_NAWS,2] ){
      int c, l;
      
      if ( sizeof(optargs) == 4 ){
          if ( !optargs[1] )
              c = optargs[0];
          else
              c = optargs[1] + optargs[0] * 256;
          
          if ( c < 20 ){
              c = 80;
        
              if (Query(P_TTY_SHOW))
                   tell_object( this_object(),
                           break_string("Dein Fenster ist schmaler als 20"
                                        +" Zeichen? Du scherzt. ;-) Ich"
                                        +" benutze den Standardwert von 80"
                                        +" Zeichen.\n", 78,
                                        "Der GameDriver teilt Dir mit: ") );
          }
      
          if ( !optargs[3] )
              l = optargs[2];
          else
              l = 256 * optargs[2] + optargs[3];
          
          if ( l > 100 ){
              l = 100;
              if (Query(P_TTY_SHOW))
                  tell_object( this_object(),
                           break_string("Tut mir leid, aber ich kann nur bis zu"
                                        +" 100 Zeilen verwalten.\n",
                                        (c ? c-2 : 78),
                                        "Der GameDriver teilt Dir mit: " ) );
          }
          
          if ( l < 3 ){
              if (Query(P_TTY_SHOW))
                  tell_object( this_object(),
                           break_string("Du willst weniger als drei Zeilen"
                                        +" benutzen? Glaub ich Dir nicht - ich"
                                        +" benutze den Standardwert von 24"
                                        +" Zeilen.\n", (c ? c-2 : 78),
                                        "Der GameDriver teilt Dir mit: " ) );
              l = 24;
          }
      
          if ( ((int) Query(P_TTY_ROWS) != l) ||
               ((int) Query(P_TTY_COLS) != c) ){
              Set( P_TTY_ROWS, l );
              Set( P_TTY_COLS, c );
              if (Query(P_TTY_SHOW))
                  tell_object( this_object(),
                           break_string("Du hast Deine Fenstergroesse auf "+l
                                        +" Zeilen und "+c
                                        +" Spalten geaendert.\n", c-2,
                                        "Der GameDriver teilt Dir mit: ") );
          }
      }
  }
  
  if ( recd[TELOPT_TTYPE,2] ){
      if ( !sizeof(Terminals) )
          return;
      
      if ( Terminals[0][0..3] == "dec-" )
          Terminals[0] = Terminals[0][4..];
      
      if ( Terminals[0] == "linux" )
          Terminals[0] = "vt100";
      
      Set( P_TTY_TYPE, Terminals[0] );
  }
}

protected void send_telnet_timing_mark() {
  binary_message( ({IAC, DO, TELOPT_TM}), 1 );
}

