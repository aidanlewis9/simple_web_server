/* forking.c: Forking HTTP Server */

#include "spidey.h"

#include <errno.h>
#include <signal.h>
#include <string.h>

#include <unistd.h>

/**
 * Fork incoming HTTP requests to handle the concurrently.
 *
 * The parent should accept a request and then fork off and let the child
 * handle the request.
 **/
void
forking_server(int sfd)
{
    struct request *request;
    pid_t pid;

    /* Accept and handle HTTP request */
    while (true) {
    	/* Accept request */
        /*if ((request = accept_request(sfd)) == NULL)
        {
            fprintf(stderr, "Unable to close accept request: %s\n", strerror(errno));
            exit(EXIT_FAILURE);        
        }*/

        request = accept_request(sfd);

	/* Ignore children */
        signal(SIGCHLD, SIG_IGN);
	
        /* Fork off child process to handle request */
        pid = fork();
        //int pidInt = (int) pid;
        if (pid == 0) // child
        {
            /*debug("Child: %d | Parent: %d", getpid(), getppid());
            if (close(sfd) < 0)
            {
                fprintf(stderr, "Unable to close socket: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            //debug("here");
            int status;
            if ((status = handle_request(request)) != HTTP_STATUS_OK)
            {
                fprintf(stderr, "%d Error: %s\n", status, strerror(errno));
                exit(EXIT_FAILURE);
            }
            //debug("here");*/
            close(sfd);
            handle_request(request);

            exit(EXIT_SUCCESS);
        }
        else if (pid > 0) // parent
        {
            //debug("Parent: %d", getpid());
            //debug("here");
            free_request(request);
            //debug("here");
            //break;
        }
        else // error
        {
            /*debug("Error PID: %d", getpid());
            fprintf(stderr, "Unable to fork: %s\n", strerror(errno));*/
            exit(EXIT_FAILURE);
        }
    }

    /* Close server socket and exit*/
    close(sfd);
    exit(EXIT_SUCCESS);
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
