/*
 * nix.c
 *
 * System interface for *nix systems.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define DOTDIR ".gnuboy"

char homedir[128];
char savesdir[192];
char statesdir[192];
#ifndef HAVE_USLEEP
static void my_usleep(unsigned int us)
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = us;
	select(0, NULL, NULL, NULL, &tv);
}
#endif

void *sys_timer()
{
	struct timeval *tv;
	
	tv = malloc(sizeof(struct timeval));
	gettimeofday(tv, NULL);
	return tv;
}

int sys_elapsed(struct timeval *prev)
{
	struct timeval tv;
	int secs, usecs;
	
	gettimeofday(&tv, NULL);
	secs = tv.tv_sec - prev->tv_sec;
	usecs = tv.tv_usec - prev->tv_usec;
	*prev = tv;
	if (!secs) return usecs;
	return 1000000 + usecs;
}

void sys_sleep(int us)
{
	if (us <= 0) return;
#ifdef HAVE_USLEEP
	usleep(us);
#else
	my_usleep(us);
#endif
}

void sys_checkdir(char *path, int wr)
{
	char *p;
	if (access(path, X_OK | (wr ? W_OK : 0)))
	{
		if (!access(path, F_OK))
		{
			printf("cannot access %s: %s\n", path, strerror(errno));
			exit(EXIT_FAILURE);
			return;
		}
		p = strrchr(path, '/');
		if (!p) 
		{
			printf("descended to root trying to create dirs\n");
			exit(EXIT_FAILURE);
			return;
		}
		*p = 0;
		sys_checkdir(path, wr);
		*p = '/';
		if (mkdir(path, 0777))
		{
			printf("cannot create %s: %s\n", path, strerror(errno));
			exit(EXIT_FAILURE);
			return;
		}
	}
}

char *sys_gethome()
{
	return homedir;
}

char *sys_getsavedir()
{
	return savesdir;
}

void sys_initpath()
{
	// static char homedir[128];
	snprintf(homedir, sizeof(homedir), "%s/.gnuboy", getenv("HOME"));
	snprintf(savesdir, sizeof(savesdir), "%s/.gnuboy/saves", getenv("HOME"));
	snprintf(statesdir, sizeof(statesdir), "%s/.gnuboy/savestates", getenv("HOME"));
	if (access( homedir, F_OK ) == -1)
	{
		mkdir(homedir, 0755);
	}
	if (access(savesdir, F_OK ) == -1)
	{
		mkdir(savesdir, 0755);
	}
	if (access(statesdir, F_OK ) == -1)
	{
		mkdir(statesdir, 0755);
	}
}
