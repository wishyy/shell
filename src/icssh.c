#include "icssh.h"
#include <readline/readline.h>
#include "helpers.h"

int main(int argc, char* argv[]) {
	int exec_result;
	int exit_status;
	pid_t pid;
	pid_t wait_result;
	char* line;
	//bglist
	struct List_t bglist;
	bglist.comparator = comparator;
	bglist.head = NULL;
	bglist.length = 0;


	// Setup segmentation fault handler
	if (signal(SIGSEGV, sigsegv_handler) == SIG_ERR) {
		perror("Failed to set signal handler");
		exit(EXIT_FAILURE);
	}

	if(signal(SIGCHLD, sigchld_handler) == SIG_ERR)	{
		perror("Failed to set signal handler");
		exit(EXIT_FAILURE);
	}

    // print the prompt & wait for the user to enter commands string
	while ((line = readline(SHELL_PROMPT)) != NULL) {
        // MAGIC HAPPENS! Command string is parsed into a job struct
        // Will print out error message if command string is invalid
		job_info* job = validate_input(line);
        if (job == NULL) { // Command was empty string or invalid
			free(line);
			continue;
		}

        //Prints out the job linked list struture for debugging
        debug_print_job(job);

		// example built-in: exit
		if (strcmp(job->procs->cmd, "exit") == 0) {
			// Terminating the shell
			free(line);
			free_job(job);
			return 0;
		}

		//built-in: cd
		if(strcmp(job->procs->cmd, "cd") == 0)	{
			char s[100];
			if(job->procs->argv[1] == NULL)
				chdir(getenv("HOME"));
			else if(chdir(job->procs->argv[1]) < 0)	{
				printf(DIR_ERR);
			}
			printf("%s\n", getcwd(s, 100)); 
			continue;
		}

		//built-in: estatus
		if(strcmp(job->procs->cmd, "estatus") == 0)	{
			printf("%d\n", pid);
			continue;
		}

		//built-in: bglist
		if(strcmp(job->procs->cmd, "estatus") == 0)	{

		}

		// example of good error handling!
		if ((pid = fork()) < 0) {
			perror("fork error");
			exit(EXIT_FAILURE);
		}
		if (pid == 0) {  
			//If zero, then it's the child process
            //get the first command in the job list
		    proc_info* proc = job->procs;
			exec_result = execvp(proc->cmd, proc->argv);
			if (exec_result < 0) {  //Error checking
				printf(EXEC_ERR, proc->cmd);
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
		} else {
			if(job->bg)	{
				//structure stuff
				


				continue;
			}
            // As the parent, wait for the foreground job to finish
			wait_result = waitpid(pid, &exit_status, 0);
			if (wait_result < 0) {
				printf(WAIT_ERR);
				exit(EXIT_FAILURE);
			}
		}

		free_job(job);  // if a foreground job, we no longer need the data
		free(line);
	}
	return 0;
}
