/* ------------------------------
   $Id: pipe.c,v 1.2 2005/03/29 09:46:52 marquet Exp $
   ------------------------------------------------------------

   mshell - a job manager
   
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include "pipe.h"
#include "jobs.h"
#include "cmd.h"

void do_pipe(char *cmds[MAXCMDS][MAXARGS], int nbcmd, int bg) {
  int fds[MAXCMDS][2];
  int i, j;
  pid_t pid;
  if(verbose)
    printf("%d commands\n", nbcmd);
  pipe(fds[0]);

  /*1er Fork*/
  if(verbose)
    printf("First command : %s\n", cmds[0][0]);
  pid = fork();
  switch(pid)
    {
    case -1:
      exit(EXIT_FAILURE);
    case 0:	
      dup2(fds[0][1], STDOUT_FILENO);
      close(fds[0][0]);
      close(fds[0][1]);
    default:
      execute(cmds[0], bg, pid);
    }
  for(i = 1; i < nbcmd-1; i++)
    {
      pipe(fds[i]);
      if(verbose)
	printf("%de command : %s\n", i+1, cmds[i][0]);
      /*nbcmds Fork*/
      pid = fork();
      switch(pid)
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
	default:
	  execute(cmds[i], bg, pid);
	}
    }
  /*Dernier Fork*/
  if(verbose)
    printf("Last command : %s \n", cmds[i][0]);
  pid = fork();
  switch(pid) 
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
    default:
      execute(cmds[i], bg, pid);
    }

  /* Fermeture des fds */
  for(j = 0; j < i; j++)
    {
      close(fds[j][0]);
      close(fds[j][1]);
    }

  /* Wait si bg est false
  if(!bg)
    for(i = 0; i < nbcmd; i++)
    wait(0);*/   
  if (!bg) 
    waitfg(pid);
    return;
}

void execute(char **cmd, int bg, pid_t pid)
{
  sigset_t mask;       /* signal mask */
  assert(pid != -1);
  if(pid == 0)
    {	
      sigprocmask(SIG_UNBLOCK, &mask, NULL);
      if (setpgid(0, 0) < 0) 
	unix_error("setpgid error"); 
      if(execvp(cmd[0], cmd) < 0) {
	printf("%s: Command not found\n", cmd[0]);
	exit(EXIT_FAILURE);
      } 
    }
  jobs_addjob(pid, (bg == 1 ? BG : FG), cmd[0]);
  sigprocmask(SIG_UNBLOCK, &mask, NULL);
  return;
}
