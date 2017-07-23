/* request.c: HTTP Request Functions */

#include "spidey.h"

#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <stdio.h>

int parse_request_method(struct request *);
int parse_request_headers(struct request *);

/**
 * Accept request from server socket.
 *
 * This function does the following:
 *
 *  1. Allocates a request struct initialized to 0.
 *  2. Initializes the headers list in the request struct.
 *  3. Accepts a client connection from the server socket.
 *  4. Looks up the client information and stores it in the request struct.
 *  5. Opens the client socket stream for the request struct.
 *  6. Returns the request struct.
 *
 * The returned request struct must be deallocated using free_request.
 **/
struct request *
accept_request(int sfd)
{
    struct sockaddr raddr;
    socklen_t rlen = sizeof(struct sockaddr);

    /* Allocate request struct (zeroed) */
    struct request *r = calloc(1, sizeof(struct request));
    if (r == NULL)
    {
        printf("Error: cannot allocate memory for map.\n");
        goto fail;
    }

    r->headers = (struct header *) calloc(1, sizeof(struct header)); // sketchy
    //r->headers = calloc(1, sizeof(struct header)); // sketchy
    
    /* Accept a client */
    int client_fd;
    if ((client_fd = accept(sfd, &raddr, &rlen)) < 0)
    {
        perror("accept failed");
        goto fail;
    }
    r->fd = client_fd;

    /* Lookup client information */
    int error;
    if ((error = getnameinfo(&raddr, sizeof(raddr), r->host, NI_MAXHOST, r->port, NI_MAXSERV, 0)) != 0)
    {
        perror("getnameinfo failed");
        goto fail;
    }
    printf("HOSTTTTTT: %s", r->host);
    /* Open socket stream */
    FILE *stream;
    if ((stream = fdopen(r->fd, "w+")) == NULL) // readable or writable? w+
    {
        perror("fdopen failed");
        goto fail;
    }
    
    r->file = stream;
    //fclose(stream);
    log("Accepted request from %s:%s", r->host, r->port);
    return r;

fail:
    free_request(r);
    return NULL;
}

/**
 * Deallocate request struct.
 *
 * This function does the following:
 *
 *  1. Closes the request socket stream or file descriptor.
 *  2. Frees all allocated strings in request struct.
 *  3. Frees all of the headers (including any allocated fields).
 *  4. Frees request struct.
 **/
void
free_request(struct request *r)
{
    //struct header *header; BUI added

    if (r == NULL) {
    	return;
    }

    /* Close socket or fd */
    if (r->file != NULL)
    {
        if (fclose(r->file) != 0)
        {
            fprintf(stderr, "Unable to close file: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        if (close(r->fd) != 0)
        {
            fprintf(stderr, "Unable to close socket: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    /* Free allocated strings */
    free(r->method);
    free(r->uri);
    free(r->path);
    //printf("STRING LEN: %zu\n", strlen(r->query));
    if (r->query && strlen(r->query) > 0)
    {
        free(r->query);
    }

    /* Free headers */
    struct header *temp = r->headers;
    struct header *curr = r->headers;
    while (curr != NULL)
    {
        temp = curr;
        curr = curr->next;
        free(temp->name);
        free(temp->value);
        free(temp);   
    }
    //free(r->headers);

    /* Free request */
    free(r);
}

/**
 * Parse HTTP Request.
 *
 * This function first parses the request method, any query, and then the
 * headers, returning 0 on success, and -1 on error.
 **/
int
parse_request(struct request *r)
{
    /* Parse HTTP Request Method */
    //debug("IN PARSE REQUEST");
    if (parse_request_method(r) != 0)
    {
        fprintf(stderr, "parse_request_method failed: %s\n", strerror(errno));
        return -1;
    }
    //debug("OUT OF PARSE METHOD");

    /* Parse HTTP Requet Headers*/
    if (parse_request_headers(r) != 0)
    {
        fprintf(stderr, "parse_request_headers failed: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

/**
 * Parse HTTP Request Method and URI
 *
 * HTTP Requests come in the form
 *
 *  <METHOD> <URI>[QUERY] HTTP/<VERSION>
 *
 * Examples:
 *
 *  GET / HTTP/1.1
 *  GET /cgi.script?q=foo HTTP/1.0
 *
 * This function extracts the method, uri, and query (if it exists).
 **/
int
parse_request_method(struct request *r) // WORKS!!!
{
    /* Read line from socket */
    char buffer[BUFSIZ]; // not declared by Bui
    debug("FGETS START");
    if (fgets(buffer, BUFSIZ, r->file) == NULL) // this?
    {
        printf("ERROR\n");
        return EXIT_FAILURE;
    }
    debug("FGETS DONE: %s", buffer);

    //printf("INFO: %s\n", buffer);

    /* Parse method and uri */
    char* method;

    if ((method = strtok(buffer, WHITESPACE)) == NULL)
    {
        goto fail;
    }
    char* uri; 
    char* query;

    char * uriQuery = strtok(NULL, WHITESPACE);

    if (!uriQuery)
    {
        goto fail;
    }
    
    /* Parse query from uri */
    if ((uri = strtok(uriQuery, "?")) != NULL)
        query = strtok(NULL, WHITESPACE);
    else 
    {
        char * temp = skip_nonwhitespace(buffer);

        if (temp != NULL)
        {
            uriQuery = skip_whitespace(temp);
        }
        else
        {
            goto fail;
        }

        uri = strtok(uriQuery, WHITESPACE);
        query = NULL;
    }

    /* Record method, uri, and query in request struct */
    r->method = strdup(method);
    r->uri = strdup(uri);
    if (query)
    {
        r->query = strdup(query);
    }
    else
    {
        r->query = "";
    }

    debug("HTTP METHOD: %s", r->method);
    debug("HTTP URI:    %s", r->uri);
    debug("HTTP QUERY:  %s", r->query);

    return 0;

fail:
    return -1;
}

/**
 * Parse HTTP Request Headers
 *
 * HTTP Headers come in the form:
 *
 *  <NAME>: <VALUE>
 *
 * Example:
 *
 *  Host: localhost:8888
 *  User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:29.0) Gecko/20100101 Firefox/29.0
 *  Accept: text/html,application/xhtml+xml
 *  Accept-Language: en-US,en;q=0.5
 *  Accept-Encoding: gzip, deflate
 *  Connection: keep-alive
 *
 * This function parses the stream from the request socket using the following
 * pseudo-code:
 *
 *  while (buffer = read_from_socket() and buffer is not empty):
 *      name, value = buffer.split(':')
 *      header      = new Header(name, value)
 *      headers.append(header)
 **/
int
parse_request_headers(struct request *r)
{
    struct header *curr = NULL;
    char buffer[BUFSIZ];
    char *name;
    char *value;

    /* Parse headers from socket */
    while (fgets(buffer, BUFSIZ, r->file))
    {
        name = strtok(buffer, ":");
        value = strtok(NULL, WHITESPACE);
        if (value)
        {
            chomp(value);
        }
        else
        {
            break;
        }
        printf("|%s|", name); 
        /*if (streq(name, "Referer"))
        {
            continue;
        }*/
        if (!r->headers->name)
        {
            r->headers->name = strdup(name);
            r->headers->value = strdup(value);
            r->headers->next = NULL;
        }
        else
        {
            curr = calloc(1, sizeof(struct header));
            if (!curr)
            {
                fprintf(stderr, "Unable to allocate memory: %s\n", strerror(errno));
                goto fail;
            }
            curr->name = strdup(name);
            curr->value = strdup(value);
            curr->next = r->headers;
            r->headers = curr;
        }
    }

#ifndef NDEBUG
    for (struct header *header = r->headers; header != NULL; header = header->next) {
    	debug("HTTP HEADER %s = %s", header->name, header->value);
    }
#endif
    return 0;

fail:
    return -1;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
