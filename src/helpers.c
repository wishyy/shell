// Your helper functions need to be here.
#include "helpers.h"


void farewells(List_t *bglist)  {
    int n = 0;
    pid_t wait_result;
    int exit_status;
    node_t *cur = bglist->head;
    while(cur)  {
        bgentry_t *temp = cur->value;
        wait_result = waitpid(temp->pid, &exit_status, WNOHANG);
        if (wait_result < 0) {
				printf(WAIT_ERR);
				exit(EXIT_FAILURE);
		}
        if(wait_result > 0) {
            printf(BG_TERM, temp->pid, temp->job->line);
            cur = cur->next;
            removeByIndex(bglist, n);
            free(temp->job->line);
            free(temp->job);
            free(temp);
            continue;
        }
        cur = cur->next;
        n++;
    }
}