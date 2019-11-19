#include "helpers.h"
#include "icssh.h"
#include <readline/readline.h>

int flag = 0;

//sigchild_handler
void sigchld_handler()  {
    flag = 1;
}

int my_compare(void* a, void* b)   {
    return ((bgentry_t*)a)->seconds - ((bgentry_t*)b)->seconds;
}

int main(int argc, char* argv[]) {
	int exec_result;
	int exit_status;
	pid_t pid;
	pid_t wait_result;
	char* line;
	//bglist
	List_t bglist;
	bglist.comparator = my_compare;
	bglist.head = NULL;
	bglist.length = 0;

	// Setup segmentation fault handler
	if (signal(SIGSEGV, sigsegv_handler) == SIG_ERR) {
		perror("Failed to set signal handler");
		exit(EXIT_FAILURE);
	}
	// Setup CHLD fault handler
	if(signal(SIGCHLD, sigchld_handler) == SIG_ERR)	{
		perror("Failed to set signal handler");
		exit(EXIT_FAILURE);
	}

    // print the prompt & wait for the user to enter commands string
	while ((line = readline(SHELL_PROMPT)) != NULL) {
		if(flag)	{
			farewells(&bglist);
			flag = 0;
		}
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
			free(job);
			free(line); 
			continue;
		}

		//built-in: estatus
		if(strcmp(job->procs->cmd, "estatus") == 0)	{
			printf("%d\n", exit_status);
			free(job);
			free(line);
			continue;
		}

		//built-in: bglist
		if(strcmp(job->procs->cmd, "bglist") == 0)	{
			node_t *cur = bglist.head;
			while(cur)	{
				print_bgentry(cur->value);
				cur = cur->next;
			}
			free(job);
			free(line);
			continue;
		}

		// example of good error handling!
		if ((pid = fork()) < 0) {
			perror("fork error");
			exit(EXIT_FAILURE);
		}
		//get time right after fork
		time_t bg_time = time(NULL);
		if (pid == 0) {  
			//If zero, then it's the child process
            //get the first command in the job list
		    proc_info* proc = job->procs;
			//pipe
			if(job->nproc == 2)	{
				int p[2], r;
				//create pipe
				if(pipe(p) < 0)
				{
					printf("syscall pipe() failed\n");
					exit(0);
				}
				proc_info *left = proc;
				proc_info *right = proc->next_proc;

				if(fork() == 0){
					close(1);
					dup(p[1]);
					close(p[0]);
					close(p[1]);
					exec_result = execvp(left->cmd, left->argv);
					if (exec_result < 0) {  //Error checking
						printf(EXEC_ERR, left->cmd);
						exit(EXIT_FAILURE);
					}
				}

				if(fork() == 0){
					close(0);
					dup(p[0]);
					close(p[0]);
					close(p[1]);
					exec_result = execvp(right->cmd, right->argv);
					if (exec_result < 0) {  //Error checking
						printf(EXEC_ERR, right->cmd);
						exit(EXIT_FAILURE);
					}
				}

				close(p[0]);
				close(p[1]);
				wait(&r);
				wait(&r);
				exit(EXIT_SUCCESS);
			}
			//double pipe
			if(job->nproc == 3)	{

			}
			//redirection
			if(proc->in_file || proc->out_file || proc->err_file)	{
				if(proc->in_file)	{
					int fd = open(proc->in_file, O_RDONLY);
					if(fd < 0)	{
						perror(RD_ERR);
					}
					dup2(fd, 0); 
					close(fd);
				}
				if(proc->out_file)	{
					int fd = open(proc->in_file, O_RDWR | O_CREAT);
					if(fd < 0)	{
						perror(RD_ERR);
					}
					dup2(fd, 1); 
					close(fd);
					return 0;
				}
				if(proc->err_file)	{
					int fd = open(proc->in_file, O_RDWR | O_CREAT);
					if(fd < 0)	{
						perror(RD_ERR);
					}
					dup2(fd, 2); 
					close(fd);
				}
			}
			exec_result = execvp(proc->cmd, proc->argv);
			if (exec_result < 0) {  //Error checking
				printf(EXEC_ERR, proc->cmd);
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
		} else {
			if(job->bg)	{
				//structure stuff
				bgentry_t *new_bg = malloc(sizeof(bgentry_t));
				new_bg->job = job;
				new_bg->pid = pid;
				new_bg->seconds = bg_time;
				insertInOrder(&bglist, new_bg);
				//dont free, continue
				continue;
			}
			else {
            // As the parent, wait for the foreground job to finish
			wait_result = waitpid(pid, &exit_status, 0);
			if (wait_result < 0) {
				printf(WAIT_ERR);
				exit(EXIT_FAILURE);
				}
			}
		}
		free_job(job);  // if a foreground job, we no longer need the data
		free(line);
	}
	return 0;
}
