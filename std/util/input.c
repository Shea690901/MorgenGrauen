// MorgenGrauen MUDlib
//
// util/input.c -- generic input handling
//
// $Id: input.c 6371 2007-07-17 22:46:50Z Zesstra $

#pragma strong_types
#pragma save_types
#pragma pedantic
#pragma range_check
#pragma no_clone

varargs void input(mixed prompt, mixed pargs, mixed ctrl, mixed ctrlargs)
{
  mixed tmp;
  if(closurep(prompt))
    tmp = apply(prompt, pointerp(pargs) ? pargs : ({}));
  else tmp = prompt;
  write(stringp(tmp) ? tmp : "");
  input_to("done", 0, prompt, pargs, ctrl, ctrlargs);
}

void done(string in, mixed prompt, mixed pargs, mixed ctrl, mixed ctrlargs)
{
  if(closurep(ctrl) &&
     apply(ctrl, ({ in }) + (pointerp(ctrlargs) ? ctrlargs : ({})))) 
    return;
  input(prompt, pargs, ctrl, ctrlargs);
}
