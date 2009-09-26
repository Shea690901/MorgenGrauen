// MorgenGrauen MUDlib
//
// utils/executer.c - Helfer zum Ausfuehren vom Kram
//
// $Id: skills.c 6673 2008-01-05 20:57:43Z Zesstra $
#pragma strict_types
#pragma save_types
#pragma range_check
#pragma no_clone
#pragma pedantic

protected mixed execute_anything(mixed fun, mixed args)
{ object ob;

  if ( closurep(fun) && objectp(query_closure_object(fun)) )
    return funcall(fun,args);

  if ( !pointerp(fun) || sizeof(fun)<2 )
    return 0;

  if ( stringp(fun[0]) )
    ob=find_object(fun[0]);
  else
    ob=fun[0];

  if ( !objectp(ob) || !stringp(fun[1]) )
    return 0;

  return call_other(ob,fun[1],args);
}

