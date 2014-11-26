/* ------------------------------
   $Id: sighandlers.c,v 1.1 2005/03/17 13:00:46 marquet Exp $
   ------------------------------------------------------------

   mshell - a job manager
   
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>


#include "jobs.h"
#include "common.h"
#include "sighandlers.h"


/*
 * Signal - wrapper for the sigaction function
 */
int
signal_wrapper(int signum, handler_t *handler) 
{
  struct sigaction action ;
  action.sa_flags=0;
  action.sa_handler= handler;
  action.sa_flags |= SA_RESTART;
  sigemptyset (&action.sa_mask) ;
  sigaction (signum,&action,NULL);
  return 1;
}


/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children
 */
void
sigchld_handler(int sig) 
{
  pid_t pid ;
  int status ;
  struct job_t *job;
  if (verbose)
    printf("sigchld_handler: entering\n");
    
  pid = waitpid(-1,&status, WNOHANG | WUNTRACED | WCONTINUED);
  if(WIFSTOPPED(status)) {
      job = jobs_getjobpid(pid) ;
      job->jb_state = ST;
    }else if(WIFEXITED(status)){
      jobs_deletejob(pid);
      if(verbose)
	printf("WIFEXITED\n");
    }else if(WIFSIGNALED(status)) {
      jobs_deletejob(pid);
      if(verbose)
	printf("WIFSIGNALED\n");
    }else if(WIFSTOPPED(status)) {
      jobs_deletejob(pid);
      if(verbose)
	printf("WIFSTOPPED\n");
    }
    else if(WIFCONTINUED(status)) {
      if(verbose)
	printf("WIFCONTINUED\n");
    }
    
  if (verbose)
    printf("sigchld_handler: exiting\n");
    
  return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void
sigint_handler(int sig) 
{
  int status ;
  pid_t pid ;
  if (verbose)
    printf("sigint_handler: entering\n");
  if((pid=jobs_fgpid())>0){
    kill(pid,SIGINT) ;
    status=jobs_deletejob(pid);
    assert(status);
    if(verbose)
      printf("sigint_handler: fg job [%d] killed\n", (int) pid);
  }

  if (verbose)
    printf("sigint_handler: exiting\n");
    
  return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void
sigtstp_handler(int sig) 
{
  int status ;
  pid_t pid ;
  status = -1;
  if (verbose)
    printf("sigstp_handler: entering\n");
  if((pid=jobs_fgpid())>0){
    kill(pid,SIGTSTP);
    assert(status);
    if(verbose)
      printf("sigstp_handler: fg job [%d] killed\n", (int) pid);
  }

  if (verbose)
    printf("sigstp_handler: exiting\n");
    
  return;
}
