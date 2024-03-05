
// Copyright Timothy Miller, 1999

#include "pseudo_classic.hpp"

#include <stdlib.h>
#include <unistd.h>
#ifndef __FreeBSD__
#include <stropts.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>

#ifdef USE_UTMP
#include <utmp.h>
#include <time.h>
#include <pwd.h>
#endif

# include <grp.h>

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
		chown(ptsname, tty_stat.st_uid, tty_stat.st_gid);
		chmod(ptsname, tty_stat.st_mode);
	}
	master = -1;
	slave = -1;
}

bool PseudoTerminal::init()
{
	/* init only once */
	if (master!=-1 || slave!=-1)
		return false;
	int i;
	const char *ptr;
	struct stat statbuf;
	const char ptychar[] = "pqrstuvwxyzPQRST";
	const char hexdigit[] = "0123456789abcdef";

	struct group *gptr; // for tty group look up
	int gid;            // for tty group id


	for (ptr = ptychar; *ptr; ptr++) {
		strcpy(ptsname, "/dev/ptyXY");
		ptsname[8] = *ptr;
		ptsname[9] = '0';

		if (stat(ptsname, &statbuf) < 0) break;

		for (i=0; i<16; i++) {
			ptsname[9] = hexdigit[i];
			master = open(ptsname, O_RDWR);
			if (master >= 0)
				goto master_ok;
		}
	}
	goto finish_error;
	master_ok:;
	ptsname[5] = 't';
	if ((gptr = getgrnam("tty")) != 0) {
		gid = gptr->gr_gid;
	} else {
		gid = -1;
	}
	slave = open(ptsname, O_RDWR);
	if(slave<0)
		goto close_master;
	if(chown(ptsname, getuid(), gid)<0 ||
	   chmod(ptsname, S_IRUSR|S_IWUSR|S_IWGRP)<0)
		goto close_slave;
	return true;
	
	close_slave:
		close(slave);
	close_master:
		close(master);
	finish_error:
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
