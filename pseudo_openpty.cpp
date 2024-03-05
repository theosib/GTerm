
// Copyright Timothy Miller, 1999

#include "pseudo_openpty.hpp"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>

#ifdef USE_UTMP
#include <utmp.h>
#include <time.h>
#include <pwd.h>
#endif

#ifdef __FreeBSD__
#include <libutil.h>
#elif defined(linux)
#include <pty.h>
#endif

PseudoTerminal::PseudoTerminal():master(-1),slave(-1)
{
	ptsname = new char[12];
} 

PseudoTerminal::~PseudoTerminal()
{
	done();
	delete ptsname;
}

void PseudoTerminal::done()
{
	if(master!=-1)
		close(master);
	if(slave!=-1)
		close(slave);
	if(master!=-1 || slave!=-1) {
#ifdef USE_UTMP
		remove_utmp();
#endif
	}
	master = -1;
	slave = -1;
}

bool PseudoTerminal::init()
{
	/* init only once */
	if (master!=-1 || slave!=-1)
		return false;
	if(openpty(&master,&slave,ptsname,NULL,NULL)==0)
	    return true;
	master = -1;
	slave = -1;
	return false;
}

bool PseudoTerminal::spawn(char *exe)
{
	int pid;
	pid = fork();
	if (pid<0)
		return false;
	if (!pid)
	{
		close(master);

		setuid(getuid());
		setgid(getgid());
		
		if (setsid()<0) 
			fprintf(stderr, "Could not set session leader\n");
		if (ioctl(slave, TIOCSCTTY, NULL)) 
			fprintf(stderr, "Could not set controlling tty\n");

		dup2(slave, 0);
		dup2(slave, 1);
		dup2(slave, 2);

		if (slave>2)
			close(slave);

		execl(exe, exe, NULL);
		exit(0);
	}	

	close(slave); // not necessary any more, comment out?
	slave = -1;
	this->pid = pid;
#ifdef USE_UTMP
	add_utmp();
#endif
	
	return true;	
}

#ifdef USE_UTMP
void PseudoTerminal::add_utmp()
{
	ut_entry.ut_type = USER_PROCESS;
	ut_entry.ut_pid = pid;
	strcpy(ut_entry.ut_line, ptsname+5);
	memset(ut_entry.ut_id,' ',4);
	time(&ut_entry.ut_time);
	strcpy(ut_entry.ut_user, getpwuid(getuid())->pw_name);
	strcpy(ut_entry.ut_host, getenv("DISPLAY"));
	ut_entry.ut_addr = 0;
	setutent();
	pututline(&ut_entry);
	endutent();
}

void PseudoTerminal::remove_utmp()
{
	ut_entry.ut_type = DEAD_PROCESS;
	memset(ut_entry.ut_line, 0, UT_LINESIZE);
	ut_entry.ut_time = 0;
	memset(ut_entry.ut_user, 0, UT_NAMESIZE);
	setutent();
	pututline(&ut_entry);
	endutent();
}
#endif /* USE_UTMP */
