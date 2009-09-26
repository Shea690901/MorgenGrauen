#ifndef __UNIQ_H__
#define __UNIQ_H__

#define UNIQDB "/p/daemon/uniqdb"
#define UNIQDB_SAVE "/p/daemon/save/uniqdb"
#define UNIQ_MAXNUM 0
#define UNIQ_OBS 1

#define P_MAX_CLONES "uniq_max_clones"

#define CLONE_UNIQ_OBJECT(x) call_other(UNIQDB,"CloneUniqObject",x)
#define ALT_CLONE_UNIQ_OBJECT(x,y) call_other(UNIQDB,"AltCloneUniqObject",x,y)
#define QUERY_UNIQ_CLONES(x) call_other(UNIQDB,"QueryUniqClones",x)
  
#endif
