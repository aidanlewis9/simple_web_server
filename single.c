/* single.c: Single User HTTP Server */

#include "spidey.h"

#include <errno.h>
#include <string.h>

#include <unistd.h>

/**
 * Handle one HTTP request at a time
 **/
void
single_server(int sfd)
{
    struct request *request;

    /* Accept and handle HTTP request */
    /* Accept request */
    while (true) 
    {
        if ((request = accept_request(sfd)) == NULL)
        {
            fprintf(stderr, "Unable to accept request: %s\n", strerror(errno));
            //goto fail;
        }

        /* Handle request */
        //http_status http_stat = handle_request(request);
        if (handle_request(request) != HTTP_STATUS_OK)
        {
            fprintf(stderr, "Unable to handle request: %s\n", strerror(errno));
            free_request(request);
            continue;
            //goto fail;
        }

        /* Free request */
        free_request(request);

    }

        /* Close socket and exit */
        close(sfd);
        exit(EXIT_SUCCESS);
    //}
    /*fail:
        printf("In fail\n");
        close(sfd);
        exit(EXIT_SUCCESS);*/
        
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
