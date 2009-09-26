// MorgenGrauen MUDlib
//
// living/moving.h -- Props/Prototypen fuer living/moving.c
//
// $Id: moving.h 7280 2009-09-01 19:40:05Z Zesstra $

#ifndef __LIVING_MOVING_H__
#define __LIVING_MOVING_H__

#define P_PURSUERS       "pursuers"

#endif // __LIVING_MOVING_H__

#ifdef NEED_PROTOTYPES

#ifndef __LIVING_MOVING_H_PROTO__
#define __LIVING_MOVING_H_PROTO__

public void AddPursuer(object ob);
public void RemovePursuer(object ob);

public void TakeFollowers();

public varargs int move( mixed dest, int methods, string direction,
                         string textout, string textin );
public varargs int remove();

// internal
public void _SetPursued(object ob);
public void _RemovePursued(object ob);

private void kampfende( object en );
private int _is_learner(object pl);


#endif // __LIVING_MOVING_H_PROTO__

#endif // NEED_PROTOTYPES

