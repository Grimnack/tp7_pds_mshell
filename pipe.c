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
  int i;
  printf("%d commandes\n", nbcmd);
  pipe(fds[0]);
  /*1er Fork*/
  switch(fork()) 
    {
    case -1:
      exit(EXIT_FAILURE);
    case 0:
      /*2e fork*/
      switch(fork()) 
	{
	case -1: /*erreur*/
	  exit(EXIT_FAILURE);
	case 0:	  
	  /*1er fils*/
	  dup2(fds[0][1], STDOUT_FILENO);
	  close(fds[0][0]);
	  close(fds[0][1]);
	  execvp(cmds[0][0], cmds[0]);
	  break;
	default:/*2e fils*/
	  dup2(fds[0][0], STDIN_FILENO);
	  close(fds[0][0]);
	  close(fds[0][1]);
	  execvp(cmds[1][0], cmds[1]);;
	}
      break;
    default: ;
    }
  close(fds[0][0]);
  close(fds[0][1]);
  if(!bg)
    for(i = 0; i < nbcmd; i++)
      wait(0);
  return;
}
