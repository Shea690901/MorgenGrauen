//
// projektverwaltung.h
//
// Defines fuer den Projektverwaltungsmaster, die Projektliste und das Projekttool
//
// 11.6.2002	neu	Mandragon
//

#include <defines.h>
#include <properties.h>

// Files

#define PROJECTMASTERDIR "/p/service/mandragon/projektbrett/"
#define PROJECTMASTER	(PROJECTMASTERDIR+"master")
#define PROJECTBOARD	(PROJECTMASTERDIR+"board")
#define PROJECTTOOL	(PROJECTMASTERDIR+"tool")
#define PROJECTDOKU     (PROJECTMASTERDIR+"doku")
#define PROJSAVEFILE	(PROJECTMASTERDIR+"secure/projectmaster")
#define PROJHELPFILE    (PROJECTMASTERDIR+"bretthilfe.txt")

// Data Mapping Entries

// Number is Key 	
#define PM_Date		0
#define PM_Supervisor	1
#define PM_Wizards	2
#define PM_Type		3
#define PM_Short	4
#define PM_Long		5
#define PM_Size		6
#define PM_Job		7
#define PM_Wizlevel	8
#define PM_Status	9
#define PM_Change  10
#define PM_MAX	   10

// Project Size

#define PMS_Small	0
#define PMS_Medium	1
#define PMS_Large	2

// Project Status

#define PMU_Posted	0
#define PMU_Active      1
#define PMU_Complete	2
#define PMU_Cancelled	3
#define PMU_Planned	4
#define PMU_Closed	5
#define PMU_States      ({ "ausgeschrieben","vergeben","abgeschlossen","abgebrochen","planung","unterbrochen" })

// Project Types (Bits)

#define PMT_Mudlib	0
#define PMT_Region	1
#define PMT_Guild	2

// Project Job (Bits)

#define PMJ_Debug	0
#define PMJ_Details	1
#define PMJ_Rework	2

// Allgemeines

#define BS(x) break_string(x,77,0,BS_LEAVE_MY_LFS)
#define TP this_player()
#define TO this_object()

// Debugging

#define MAGIERNAME "mandragon"
#define DEBUG(x) if (find_player(MAGIERNAME)) tell_object(find_player(MAGIERNAME),x+"\n")
