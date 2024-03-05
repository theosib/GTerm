
// Copyright Timothy Miller, 1999

#ifndef INCLUDED_PSEUDO_H
#define INCLUDED_PSEUDO_H

#ifdef USE_UTMP
# include <utmp.h>
#endif

class PseudoTerminal
{
public:
    	PseudoTerminal();
    	bool init(); // try to get PT
    	void done(); // close all open fds
    	bool spawn(char *exe); // try to exec "exe"
    	~PseudoTerminal();
    	int get_master()
    	{
		return master;
    	}
    	int get_slave()
    	{
		return slave;
    	}
    	int get_pid()
    	{
		return pid;
    	}
protected:
#ifdef USE_UTMP
    	void add_utmp();
    	void remove_utmp();
#endif

private:
    	int master,slave; // fd of master and slave pty, default -1
    	int pid; // initial pid of subprocess, default 0
#ifdef USE_UTMP
    	struct utmp ut_entry;
#endif
};

#endif
