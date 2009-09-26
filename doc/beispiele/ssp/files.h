#define SM(x) ("/doc/beispiele/ssp/"+x)
#define L1(x) (SM("l1/"+x))
#define L2(x) (SM("l2/"+x))
#define L3(x) (SM("l3/"+x))
#define L4(x) (SM("l4/"+x))
#define L5(x) (SM("l5/"+x))
#define L6(x) (SM("l6/"+x))
#define MON(x) (SM("mon/"+x))
#define OBJ(x) (SM("obj/"+x))

#define LOG_OBJ "/players/boing/obj/logobj"
#define PFLANZE "/doc/beispiele/ssp/mon/pflanze"
#define MMONST "/doc/beispiele/ssp/mon/metallmonster"

#define GUARDED_EXIT(direction, dest, monster, message)\
  AddSpecialExit(direction, \
		 lambda(({}), \
({#',,\
  ({ #'?, \
    ({#'present, monster }),\
    ({ #'write, message }),\
    ({ #'call_other, \
      ({ #'this_player }), "move", dest, M_GO, \
        ({#'+, "nach ", \
          ({ #'?, \
            ({ #'||, \
              ({#'==, direction, "oben"}), \
	      ({#'==, direction, "unten"}) \
            }),\
            direction, \
            ({#'capitalize, direction }) \
	  })\
	})\
    }) \
  }), 1 \
})\
))


/* --- Von Paracelsus geklaut --- */
#define MURDER(message)\
    SetProp(P_MURDER_MSG, lambda(({}),\
        ({ #'sprintf,message, \
            ({ #'capitalize, ({#'call_other,\
                ({#'call_other, ({#'this_object}), "QueryProp", P_KILLER }),\
                     "name" }) }) }) ), F_QUERY_METHOD);
