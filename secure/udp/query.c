// MorgenGrauen MUDlib
//
// query.c
//
// $Id: query.c 6081 2006-10-23 14:12:34Z Zesstra $

#include <udp.h>

#ifdef ZEBEDEE
#include <system.h>
#elif !defined(INETD)
#define INETD	"/secure/inetd"
#endif

/* Mud / Admin email address. */
#define EMAIL	"mud@mg.mud.de"

void udp_query(mapping data) {
    mapping ret;

    switch(data[DATA]) {
	case "commands":
	    ret = INETD->query("hosts");
	    if (ret[lower_case(data[NAME])])
		ret = ([
		DATA: implode(ret[lower_case(data[NAME])][LOCAL_COMMANDS], ":")
		]);
	    else
		ret = ([ DATA: implode(INETD->query("commands"), ":") ]);
	    break;
	case "email":
	    ret = ([ DATA: EMAIL ]);
	    break;
	case "hosts":
	{
	    mapping hosts;
	    string *list;
	    string tmp;
	    int i;

	    tmp = "";
	    for(i = sizeof(list = m_indices(hosts = INETD->query("hosts")));
	    i--; ) {
		tmp +=
		hosts[list[i]][HOST_NAME] + ":" +
		hosts[list[i]][HOST_IP] + ":" +
		hosts[list[i]][HOST_UDP_PORT] + ":" +
		implode(hosts[list[i]][LOCAL_COMMANDS], ",") + ":" +
		implode(hosts[list[i]][HOST_COMMANDS], ",");
		if (i)
		    tmp += "\n";
	    }
	    ret = ([ DATA: tmp ]);
	    break;
	}
	case "inetd":
	    ret = ([ DATA: INETD_VERSION ]);
	    break;
	case "list":
	    /* List of thingsthat can be queried. */
	    ret = ([ DATA: "commands:email:hosts:inetd:mud_port:time:version" ]);
	    break;
	case "mud_port":
	    ret = ([ DATA: query_mud_port() ]);
	    break;
	case "time":
	    ret = ([ DATA: time() ]);
	    break;
	case "version":
	    ret = ([ DATA: version() ]);
	    break;
	default:
	    /* Just ignore it for the time being. */
	    return;
    }
    INETD->_send_udp(data[NAME], ret + ([
	REQUEST: REPLY,
	RECIPIENT: data[SENDER],
	ID: data[ID],
	"QUERY": data[DATA]
    ]) );
}
