// MorgenGrauen MUDlib
//
// thing/commands.h -- command handling header
//
// $Id: commands.h 5706 2006-09-23 10:37:01Z root $

#ifndef __THING_COMMANDS_H__
#define __THING_COMMANDS_H__

// properties

#define P_COMMANDS           "commands"

#endif // __THING_COMMANDS_H__

#ifdef NEED_PROTOTYPES

#ifndef __THING_COMMANDS_H_PROTO__
#define __THING_COMMANDS_H_PROTO__

// prototypes

// add commands to the command list of the object
varargs void AddCmd(mixed cmd, mixed func, mixed flag, mixed cmdid);

// remove a command
varargs int RemoveCmd(mixed cmd, int del_norule, mixed onlyid);
// Set- und Query-Methoden
static mapping _set_commands(mapping commands);
static mapping _query_commands();


// internal
static int _execute(mixed fun, string str, mixed *parsed);
static int _process_command(string str, string *noparsestr,mixed *fun, mixed *flag, mixed *rule);
static string _replace_personal(string str, mixed *obs);

public int _cl(string str);

#endif // __THING_COMMANDS_H_PROTO__

#endif	// NEED_PROTOYPES
