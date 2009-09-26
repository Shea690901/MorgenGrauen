#include <udp.h>

int udp_channel(mapping data) {
    object *list;
    string msg;
    int i;
    string prefix;
    string *message;

    switch(data["CMD"]) {
	case "emote":
	    /* A channel emote. */
			prefix = "["+capitalize(data["CHANNEL"]+"]:";
			message=old_explode(break_string(capitalize(data[SENDER]+"@"+
			                capitalize(data[NAME])+" "+data[data],79-strlen(prefix)),
			                "\n");
			msg="\n";
			for (i=0;i<sizeof(message);i++)
				msg+=prefix+message[i]+"\n");
	    break;
	case "list":
	    /* Request for a list of people listening to a certain channel. */
	    list = filter(
	    users(), "filter_listeners", this_object(), data["CHANNEL"]);
	    if (i = sizeof(list)) {
		msg = "[" + capitalize(data["CHANNEL"]) + "@" +
		LOCAL_NAME + "] Listening:\n";
		while(i--)
		    msg +=
		    "    " + capitalize(list[i]->query_real_name()) + "\n";
	    }
	    else
		msg = "[" + capitalize(data["CHANNEL"]) + "@" +
		LOCAL_NAME + "] Nobody Listening.\n";
	    INETD->_send_udp(data[NAME], ([
		REQUEST: REPLY,
		RECIPIENT: data[SENDER],
		ID: data[ID],
		DATA: msg
	    ]) );
	    return 1;
	default:
	    /* A regular channel message. */
	    msg = sprintf("%-80=s", "[" + capitalize(data["CHANNEL"]) + ":" +
	    capitalize(data[SENDER]) + "@" + capitalize(data[NAME]) +
	    "] " + data[DATA]) + "\n";
	    break;
    }
    for(i = sizeof(list = users()); i--; ) {
	/* If a wizard and earmuffs at 1 then send the message. */
	if ((int)list[i]->query_level() >= 21 &&
	(int)list[i]->listen_shout() == 1)
	    tell_object(list[i], msg);
    }
    /* Never generate an error. Unknown channels can be ignored later. */
    return 1;
}
