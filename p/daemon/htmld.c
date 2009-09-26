// MorgenGrauen MUDlib
//
// HTML.C -- interpretes HTML encoded files
//
// $Date: 1995/03/31 13:30:33 $                                            
// $Revision: 6142 $             
/* $Log: htmld.c,v $
 * Revision 1.1  1995/03/31  13:30:33  Hate
 * Initial revision
 *
 */

#pragma strong_types

#include <sys_debug.h>
#include "html.h"
#include <ansi.h>

#define LOG(msg)	return (printf("LOG: %s\n", msg), "")

int width;		// text width
mixed *ind;		// indentation
string *tagS, *text;	// tag and text stack

#define INITSTACK(s)	(s = ({}))
#define PUSH(s,x)	(s += ({ (x) }))
#define POP(s,x)	(x = s[<1], s = s[0..<2])
#define POPX(s)		(s = s[0..<2])
#define TOP(s)		((s)[<1])
#define SP(s)		(sizeof(s))

private string expand(string token)
{
  switch(token)
  {
    case "lt"  : return "<";
    case "gt"  : return ">";
    case "amp" : return "&";
    case "quot": return "\"";
    default    : return token; break;
  }
}

#define ENDPARA		0x257

private mixed htmlParse(string tag, mapping page_info, int ANSI)
{
  string t;
  switch(tag[0])
  {
  case '&':
    switch(tag[1..<2])
    {
    case "lt"  : tag = "<"; break;
    case "gt"  : tag = ">"; break;
    case "amp" : tag = "&"; break;
    case "quot": tag = "\""; break;
    default    : break;
    }
    break;
  case '<': 
    if(strlen(tag)>2)
    {
      string *cmd, ltag, para;
      int EndTag;
      cmd = old_explode(tag[1..<2], " ");
      if(cmd[0][0] == '/') { EndTag = 1; cmd[0] = cmd[0][1..]; }
      switch(cmd[0] = lower_case(cmd[0]))
      {
      case H_H1: case H_H2: case H_H3: case H_H4: case H_H5: case H_H6:
        if(EndTag) 
          if(SP(tagS) && TOP(tagS) == cmd[0])
            POPX(tagS);				// remove tag from stack
          else LOG(sprintf("</%s>: missing <%s>", cmd[0]));
        else 
        {
          int level;
          PUSH(tagS, cmd[0]);			// push header type
          sscanf(cmd[0], "h%d", level);		// set indentation
          INITSTACK(ind); PUSH(ind, level);
        }
        PUSH(tagS, H_BREAK); tag = "";
        break;
      case H_TITLE:
        if(EndTag) 
          if(SP(tagS) && TOP(tagS) == H_TITLE) 
          {  
            POPX(tagS); 			// remove title tag
            if(!stringp(page_info[H_TITLE]))
              page_info[H_TITLE] = "";		// initialize title field
            page_info[H_TITLE] += TOP(text); 	// add info to list
            POPX(text); 			// remove info text
          }
          else LOG("</TITLE>: missing <TITLE>");
        else PUSH(tagS, H_TITLE);		// push begin tag on stack 
        return "";				// do not add text
      case H_ANCHOR:
        if(EndTag) 
          if(SP(tagS) && TOP(tagS) == H_ANCHOR)
            if(SP(text))
            {
               POPX(tagS);			// remove <A> from stack
               POP(tagS, t);			// pop reference type
               if(SP(tagS)) 			// check for reference
               {
                 if(!mappingp(page_info[t = lower_case(t)]))
                   page_info[t] = ([]);
                 page_info[t][TOP(text)] = TOP(tagS);// add reference to list
                 POPX(tagS);			// remove reference data
		             POP(text, tag);		// pop reference for add
                 if(ANSI) tag = ANSI_UNDERL + tag + ANSI_NORMAL;
               }
               else LOG(sprintf("</A>: missing information (%s=..)", t));
            }
            else LOG("</A>: missing reference keyword"); 
          else LOG("<A>: missing </A>");
        else
        {
          cmd = old_explode(implode(cmd[1..], " "), "=") -({"", " "});
          if(cmd[1][0]=='"' && cmd[1][<1]=='"')
            cmd[1] = cmd[1][1..<2];		// remove quotes
          PUSH(tagS, cmd[1]);			// push reference
          PUSH(tagS, lower_case(cmd[0]));	// push type of reference
          PUSH(tagS, H_ANCHOR);			// push <A>
          PUSH(text, "");			// push text fragment
          return "";
        }
        break;
      case H_PRE:
        if(EndTag) 
          if(SP(tagS) && TOP(tagS) == H_PRE)
            POPX(tagS);				// remove <PRE> from stack
          else LOG("</PRE>: missing <PRE>");
        else 
          PUSH(tagS, H_PRE);			// push <PRE>
        PUSH(text, ""); return "";
        break;
      case H_PARAGRAPH:
        PUSH(tagS, H_BREAK); tag = ""; break;
      case H_ULIST:
      case H_OLIST:
      case H_DLIST: 
        if(EndTag) PUSH(tagS, H_BREAK); 		// break when finished
        tag = ""; break;
      case H_LIST: 
        PUSH(tagS, H_BREAK); tag = "*"; PUSH(ind, "  "); break;
      default: return ""; 			// just ignore it
      }
    }
    else LOG("empty tag not allowed");
  }

  if(SP(text)) 
  {
    POP(text, t);
    if(SP(tagS) && TOP(tagS) == H_BREAK)
    {
      POPX(tagS);				// remove BREAK
      if(SP(tagS) && TOP(tagS) == H_PRE) 
        t = break_string(t, width, TOP(ind), 1);
      else 
      {
        t = implode(regexplode(t, "[\n ]")-({"", " ", "\n"}), " ");
        t = break_string(t, width, TOP(ind));
      }
      PUSH(text, t);
    }
    else tag = t + tag;
  }
  PUSH(text, tag);
}

mixed ReadPage(string file, mapping page_info, int ANSI)
{
  string page, src; page_info = ([]);
  if(strlen(file) < 6 || file[<5..] != ".html") return NO_HTML_FILE;
  if(!stringp(src = read_file(file)) || !strlen(src)) return EMPTY_FILE;

  INITSTACK(tagS); INITSTACK(text); INITSTACK(ind); width = 78;
  map(regexplode(src, HTML_TAGS)-({""}), #'htmlParse, &page_info, ANSI);
  page = "";
  while(SP(text)) 
  { 
    string top; POP(text, top);
    if(top[<1] == '\n' && strlen(page) && page[0] == '\n')
      top = top[0..<2];
    page = top + page; 
  }
  if(ANSI && page_info[H_TITLE]) 
    page_info[H_TITLE] = ANSI_BOLD + page_info[H_TITLE] + ANSI_NORMAL;
  return page;
}
