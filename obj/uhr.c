#pragma no_clone
#pragma no_inherit
#pragma no_shadow
#pragma strong_types

#include <properties.h>
#include <events.h>
#include <time.h>

private string ulog = "";

void WaitForQuarterHour() {
  while(remove_call_out("QuarterReached")!=-1);
  call_out("QuarterReached",900-(time() % 900));
}

void create() {
  WaitForQuarterHour();
}

/* Ueberprueft die als Parameter "str" uebergebene Uhrmeldung.
   Rueckgabewerte: 0 fuer fehlerhafte Parameter,
                   1 fuer korrekten Aufbau. */
int CheckClockMsg(string str)
{
  int i = strstr(str, "%", 0);
  /* Mehr als 1x %d enthalten, mehr als 1x % enthalten, oder auf das einzige
     enthaltene % folgt kein "d". */
  if ( sizeof(explode(str,"%d")) > 2 ||
       sizeof(explode(str,"%")) > 2 ||
       (i>-1 && ( i==strlen(str)-1 || str[i+1] != 'd')))
    return 0;
  else
    return 1;
}

void QuarterReached() {
  int std, minuten;
  string ct, str, format;
  int i;
  int off;

  int *lt=localtime(time());
  minuten=lt[TM_MIN];
  std=lt[TM_HOUR];

  // Clock-Event triggern
  EVENTD->TriggerEvent(EVT_LIB_CLOCK, ([
	E_HOUR: std, E_MINUTES: minuten ]) );

  // bei Datumswechsel DATECHANGE-Event ausloesen.
  if (std==0 && minuten < 15) {      
      EVENTD->TriggerEvent(EVT_LIB_DATECHANGE, ([
	    E_DAY: lt[TM_MDAY], E_MONTH: lt[TM_MON]+1,
	    E_YEAR: lt[TM_YEAR] ]) );
  }

  if (minuten <= 2) {
    foreach(object u : users()) {
      if((off=to_int(u->QueryProp(P_TIMEZONE)))<0)off+=24;
      off=(std+off)%24;
      format=u->QueryProp(P_CLOCKMSG)||"Du hoerst die Uhr schlagen: "
	                               "Es ist jetzt %d Uhr.\n";
      if ( !CheckClockMsg(format) )
      {
	tell_object(u, break_string(
	  "So kann ich nicht arbeiten! Deine Uhrmeldung funktioniert so "
	  "nicht. Bitte korrigiere das.", 78, "Die Uhr teilt Dir mit: "));
	continue;
      }
      if (strstr(format,"%d",0)==-1)
        str=format;
      else
        str=sprintf(format,off);
      if (str!="") tell_object(u,str);
    }
  }
  str = "";
  if (minuten < 10) {
    log_file("USER_STATISTIK", ulog);
    ulog = sprintf("\n%s%02d: ",(!std?("# "+ct+":\n"):""),std);
  }
  ulog += sprintf("%4d",sizeof(users()));
  WaitForQuarterHour();
}

