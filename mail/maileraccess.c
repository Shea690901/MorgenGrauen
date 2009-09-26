#include "/players/loco/lib/post.h"
#include <properties.h>
#include <language.h>

inherit "std/thing";

#define ALLOWED "loco"
#define FROMDIR "/players/loco/mail/"
#define TODIR   "/mail/"

create() {
  (::create());
  seteuid(getuid());
  SetProp(P_NAME,"Debuggingtool");
  SetProp(P_SHORT,"Ein Post-Debuggingtool");
  SetProp(P_LONG,"mailmv filename\n");
  SetProp(P_GENDER,NEUTER);
  SetProp(P_NEVERDROP,1);
  SetProp(P_IDS,({"posttool","mailtool"}));
  SetProp(P_AUTOLOADOBJ,1);
}

init() {
  (::init());
  if (geteuid(this_player())==ALLOWED) 
    add_action("mailmv","mailmv");
}

mailmv(str) {
  mixed x,file;
//  printf("%O %O %O\n",previous_object(0),previous_object(1),previous_object(2));
  if (geteuid(this_interactive())!=ALLOWED) return remove();
  if (this_player()!=this_interactive()) 
    return write("player!=interactive\n");
  

  file=read_file(FROMDIR+str);
  if (!file) {
    notify_fail("File "+str+" not found.\n");
    return 0;
  }
  if (sizeof(old_explode(str,"/"))!=1) return;
  if (strlen(str)<3) return;
  rm(TODIR+str+".old");
  x=read_file(TODIR+str);
  if (!x) write("No old file to backup.\n");
  else {
    write_file(TODIR+str+".old",x);
    rm(TODIR+str);
  }
  write_file(TODIR+str,file);
  return 1;
}
