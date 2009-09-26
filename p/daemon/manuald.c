// MorgenGrauen MUDlib
//
// MANUALD.C -- searches in manpages and caches
//
// $Date: 2004/04/09 06:07:19 $                                            
// $Revision: 6081 $             
/* $Log: manuald.c,v $
 * Revision 1.2  2004/04/09 06:07:19  Hate
 * *** empty log message ***
 *
 * Revision 1.1  1995/03/31  13:30:33  Hate
 * Initial revision
 *
 */

#pragma strong_types

#include <sys_debug.h>
#include "html.h"
#include <ansi.h>
#include <properties.h>

private static mapping cache;

void create()
{
  cache = ([]);
}

private void split(string f, string p, string *dir, string *page)
{
  if(file_size(p+"/"+f) == -2) dir += ({ p+"/"+f });
  else if(f[<5..] == ".html") page += ({ f[0..<6], p+"/"+f });
}

private mixed SearchPage(string path, string word)
{
  string *dir, *page;
  int i;
  dir = ({}); page = ({});
  map(get_dir(path+"/*")-({".",".."}),#'split,path,&dir,&page);
  if((i = member(page, word)) != -1) return page[i+1];
  while(sizeof(dir))
  {
    string file;
    if(file = SearchPage(dir[0], word)) return file;
    dir = dir[1..];
  }
}

varargs mixed ReadPage(string what, string section)
{
  string file;
  if(!what) return -1;
  if(!member(cache, what+(section ? section : "")))
  {
    file = SearchPage("/htmldoc/"+section, what);
    cache[what+(section ? section : "")] = file;
  }
  else
    file = cache[what+(section ? section : "")];
  DEBUG(file);
  return HTMLD->ReadPage(file, ([]), 
                         member(({"ansi","vt100"}),
			        previous_object()->QueryProp(P_TTY)));
}
