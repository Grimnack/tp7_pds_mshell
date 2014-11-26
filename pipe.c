/* ------------------------------
   $Id: pipe.c,v 1.2 2005/03/29 09:46:52 marquet Exp $
   ------------------------------------------------------------

   mshell - a job manager
   
*/

#include <stdio.h>
#include <stdlib.h>
#include "pipe.h"
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>

void do_pipe(char *cmds[MAXCMDS][MAXARGS], int nbcmd, int bg) {
  int fds[MAXCMDS][2];
  int i, j;
  if(verbose)
    printf("%d commands\n", nbcmd);
  pipe(fds[0]);

  /*1er Fork*/
  if(verbose)
    printf("%de fork (first one)\n", 0);
  switch(fork()) 
    {
    case -1:
      exit(EXIT_FAILURE);
    case 0:	
      dup2(fds[0][1], STDOUT_FILENO);
      close(fds[0][0]);
      close(fds[0][1]);
      execvp(cmds[0][0], cmds[0]);
      break;
    }

  for(i = 1; i < nbcmd-1; i++)
    {
      pipe(fds[i]);
      if(verbose)
	printf("%de fork\n", i);
      /*nbcmds Fork*/
      switch(fork())
	{
	case -1:
	  exit(EXIT_FAILURE);
	case 0:	
	  dup2(fds[i-1][0], STDIN_FILENO);
	  dup2(fds[i][1], STDOUT_FILENO);

	  for(j = 0; j <= i; j++)
	    {
	      close(fds[j][0]);
	      close(fds[j][1]);
	    }
	  execvp(cmds[i][0], cmds[i]);
	  break;
	}
    }
  /*Dernier Fork*/
  if(verbose)
    printf("%de fork (last one)\n", i);
  switch(fork()) 
    {
    case -1: /*erreur*/
      exit(EXIT_FAILURE);
    case 0:
      /*2e fork*/
      dup2(fds[i-1][0], STDIN_FILENO);
      for(j = 0; j < i; j++)
	{
	  close(fds[j][0]);
	  close(fds[j][1]);
	}
      execvp(cmds[i][0], cmds[i]);
      break;
    }

  /* Fermeture des fds */
  for(j = 0; j < i; j++)
    {
      close(fds[j][0]);
      close(fds[j][1]);
    }

  /* Wait si bg est false*/
  if(!bg)
    for(i = 0; i < nbcmd; i++)
    wait(0);
      return;
}
