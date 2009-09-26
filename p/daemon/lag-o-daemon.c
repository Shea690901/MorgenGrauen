#pragma strong_types
#pragma no_clone
#pragma no_shadow
#pragma no_inherit
#pragma pedantic

#include <debug_info.h>

#ifndef DEBUG
#define DEBUG(x)        if (find_player("zesstra"))\
        tell_object(find_player("zesstra"),"LAG: "+x+"\n")
#endif

float round(float arg);
static void step();

int *zeit, *hb;
int pointer;
float lag_60, lag_15, lag_1, hbstat, obstat;
mapping lastwarning=([]);

void create()
{
  int i;
  if(clonep(this_object())) destruct(this_object());
  call_out("step",10);
  zeit = allocate(360, -1);
  hb = allocate(360, -1);
  zeit[0]=time();
  hb[0]=absolute_hb_count();
  pointer=0;
  lag_60=lag_15=lag_1=-1.0;
}

static void step()
{
  int pointer_60, pointer_15, pointer_1;

  call_out("step",10);
  zeit[pointer]=time();
  hb[pointer]=absolute_hb_count();
  pointer_60=(pointer==359?0:pointer+1);
  pointer_15=(pointer<90?pointer-90+359:pointer-90);
  pointer_1=(pointer<6?pointer-6+359:pointer-6);
  
  if(zeit[pointer_1]>=0)
  {
    lag_1 = ((float)hb[pointer]-(float)hb[pointer_1])*2.0;
    lag_1 /= (float)(zeit[pointer]-zeit[pointer_1]);
    lag_1 = 100.0*(1.0-lag_1);
  }
  if(zeit[pointer_15]>=0)
  {
    lag_15 = ((float)hb[pointer]-(float)hb[pointer_15])*2.0;
    lag_15 /= (float)(zeit[pointer]-zeit[pointer_15]);
    lag_15 = 100.0*(1.0-lag_15);
  }
  if(zeit[pointer_60]>=0)
  {
    lag_60 = ((float)hb[pointer]-(float)hb[pointer_60])*2.0;
    lag_60 /= (float)(zeit[pointer]-zeit[pointer_60]);
    lag_60 = 100.0*(1.0-lag_60);
  }
  if(pointer==359)
    pointer=0;
  else
    pointer++;

  // Heart-Beats, Objektzahl, etc ueberwachen
  hbstat=debug_info(DINFO_DATA,DID_STATUS,DID_ST_HBEAT_AVG_PROC);
  if (hbstat<0.99
      && (time()-lastwarning[DID_ST_HBEAT_AVG_PROC] > 3600)) {
      if (find_player("zesstra")) 
	  lastwarning[DID_ST_HBEAT_AVG_PROC]=time();
      DEBUG(sprintf(
	"Durchschnittlicher Anteil HB-Calls < 100%%: %.5f\n",hbstat));
      if (hbstat < 0.90) {
	lastwarning[DID_ST_HBEAT_AVG_PROC]=time();
	catch(raise_error(sprintf(
		"Warnung: Durchschnittlicher Anteil der "
		"abgearbeiteten Heartbeat-Calls < 90%%! (%.3f%%)",
		hbstat*100));publish);
      }
  }
  obstat=debug_info(DINFO_DATA,DID_STATUS,DID_ST_OBJECTS_AVG_PROC);
  if (obstat<0.99
      && (time()-lastwarning[DID_ST_OBJECTS_AVG_PROC] > 3600)) {
      if (find_player("zesstra")) 
	  lastwarning[DID_ST_OBJECTS_AVG_PROC]=time();
      DEBUG(sprintf(
	"Durchschnittlicher Anteil verarbeiteter Objekte < 100%%: %.5f\n",
	obstat));
      if (obstat < 0.90) {
	lastwarning[DID_ST_OBJECTS_AVG_PROC]=time();
	catch(raise_error(sprintf(
		"Warnung: Durchschnittlicher Anteil der "
		"verarbeiteten Objekte pro Zyklus < 90%%! (%.3f%%)",
		obstat*100));publish);
      }
  }
}

float *read_debug_stat() {
  return ({hbstat,obstat});
}

float *read_lag_data()
{
  return ({lag_60,lag_15,lag_1});
}
