// MorgenGrauen MUDlib
//
// www.finger.c
//
// $Id: www.finger.c 6181 2007-02-05 12:36:13Z Rumata $

#pragma strong_types
#pragma combine_strings

#include <properties.h>
#include <www.h>

string Request(mapping cmds)
{
  string result;
  string *tmp, tmp2, quoted;
  if(!sizeof(cmds) || !stringp(cmds[USER]))
    return ERROR("Kein Nutzer angegeben!");
  /*
   * Kann ja sein, dass ein Spieler auf die Idee kommt, HTML-Tags
   * in seine Beschreibung einzubauen. Unsere Seite ist aber schon
   * interaktiv genug. (Anm: Nur <.*>-Vorkommnisse zu ersetzen nutzt
   * nix, da man auch mit einzelnen Zeichen Schaden machen kann.
   */
  result = regreplace(FINGER(cmds[USER]), "<","\\&lt;",1); 
  result = regreplace(result, ">","\\&gt;",1);

  /*
   * Grund des kommenden Codeblocks ist , dass manche Spieler ihre
   * Homepage mit "http://mg.mud.de" angeben, andere nur"mg.mud.de" 
   * schreiben. Damit aber der Browser den Link als absolut interpretiert, 
   * muss das http:// davor stehen, und zwar nur einmal. 
   */
  tmp= regexp(explode(result,"\n"),"^Homepage:");
  if (sizeof(tmp)&&stringp(tmp[0])&&strlen(tmp[0])>16) {
	  quoted = regreplace(tmp[0],"([[\\]+*?.\\\\])","\\\\\\1", 1);
    if (tmp[0][10..16]=="http://")
      tmp2=sprintf("Homepage: <A HREF=\"%s\">%s</A>",
		      tmp[0][10..],tmp[0][10..]);
    else
      tmp2=sprintf("Homepage: <A HREF=\"http://%s\">%s</A>",
		      tmp[0][10..],tmp[0][10..]);
    result = regreplace(result,quoted,tmp2,1);
  }
  
  result = regreplace(result,
		      "E-Mail-Adresse: ([^\n]*)",
		      "E-Mail-Adresse: Bitte nachfragen...",1);

  result = regreplace(result,
		      "Messenger: ([^\n]*)",
		      "Messenger: Bitte nachfragen...", 1);

  result = regreplace(result,
		      "ICQ: ([^\n]*)",
		      "ICQ: Bitte nachfragen...", 1);

  return "<H2>Wer ist "+capitalize(cmds[USER])+"?</H2><HR>"
    +"<PRE>"+result+"</PRE>";
}
