/* handler.c: HTTP Request Handlers */

#include "spidey.h"

#include <errno.h>
#include <limits.h>
#include <string.h>

#include <dirent.h>
#include <unistd.h>

/* Internal Declarations */
http_status handle_browse_request(struct request *request);
http_status handle_file_request(struct request *request);
http_status handle_cgi_request(struct request *request);
http_status handle_error(struct request *request, http_status status);

/**
 * Handle HTTP Request
 *
 * This parses a request, determines the request path, determines the request
 * type, and then dispatches to the appropriate handler type.
 *
 * On error, handle_error should be used with an appropriate HTTP status code.
 **/
http_status
handle_request(struct request *r)
{
    debug("IN HANDLE REQUEST");
    
    http_status result;

    /* Parse request */
    if (parse_request(r) < 0)
    {
        fprintf(stderr, "Unable to parse request: %s", strerror(errno));
        result = HTTP_STATUS_BAD_REQUEST;
        handle_error(r, result);
        return result;
    }

    debug("OUT OF PARSE REQUEST");

    /* Determine request path */
    r->path = determine_request_path(r->uri);
    debug("HTTP REQUEST PATH: %s", r->path);
    
    /* Dispatch to appropriate request handler type */
    request_type rt = determine_request_type(r->path);
    
    switch (rt)
    {
        case REQUEST_BROWSE:
            printf("BROWSE\n");
            result = handle_browse_request(r);
            break;
        case REQUEST_FILE:
            printf("FILE\n");
            result = handle_file_request(r);
            break;
        case REQUEST_CGI:
            printf("CGI\n");
            result = handle_cgi_request(r);
            break;
        default:
            printf("ERROR\n");
            result = HTTP_STATUS_NOT_FOUND;
            handle_error(r, result);
            break;
    }

    log("HTTP REQUEST STATUS: %s", http_status_string(result));
    return result;
}

/**
 * Handle browse request
 *
 * This lists the contents of a directory in HTML.
 *
 * If the path cannot be opened or scanned as a directory, then handle error
 * with HTTP_STATUS_NOT_FOUND.
 **/
http_status
handle_browse_request(struct request *r)
{
    struct dirent **entries;
    int n;

    /* Open a directory for reading or scanning */
    debug("r->path: %s", r->path);

    n = scandir(r->path, &entries, NULL, alphasort);
    if (n < 0)
    {
        fprintf(stderr, "Unable to scan directory: %s", strerror(errno));
        return HTTP_STATUS_NOT_FOUND;
    }
    /* Write HTTP Header with OK Status and text/html Content-Type */
    fprintf(r->file, "HTTP/1.0 200 OK\n");
    fprintf(r->file, "Context-Type: text/html\n");
    fprintf(r->file, "\r\n");
    fprintf(r->file, "<html>\n");
    fprintf(r->file, "<ul>\n");

    /* For each entry in directory, emit HTML list item */
    int i = 0;
    while (i < n)
    {
        if (!streq(entries[i]->d_name, "."))
        {
            char buf[BUFSIZ];
            if (strlen(r->uri) > 1)
            {
                sprintf(buf, "%s/%s", r->uri, entries[i]->d_name); 
            }
            else
            {
                sprintf(buf, "%s", entries[i]->d_name); 
            }
            //char * help = determine_request_path(buf);
            //printf("REQUEST_PATH: %s\n", help);
            printf("NEW URI: %s\n", buf);
            fprintf(r->file, "<li><a href=\"%s\">%s</a></li>\n", buf, entries[i]->d_name);

        }
        free(entries[i]);
        i++;
    }
    fprintf(r->file, "</ul>\n");
    fprintf(r->file, "</html>\n");
    /* Flush socket, return OK */
    if (fflush(r->file) != 0)
    {
        fprintf(stderr, "Unable to flush socket: %s\n", strerror(errno));
        printf("Browse Request FAILED\n");
        return HTTP_STATUS_NOT_FOUND;
    }
    
    free(entries);

    return HTTP_STATUS_OK;
}

/**
 * Handle file request
 *
 * This opens and streams the contents of the specified file to the socket.
 *
 * If the path cannot be opened for reading, then handle error with
 * HTTP_STATUS_NOT_FOUND.
 **/
http_status
handle_file_request(struct request *r)
{
    FILE *fs;
    char buffer[BUFSIZ];
    char *mimetype = NULL;
    size_t nread;

    /* Open file for reading */
    if ((fs = fopen(r->path, "r")) == NULL)
    {
        fprintf(stderr, "Unable to open file: %s\n", strerror(errno));
        return HTTP_STATUS_NOT_FOUND;
    }

    printf("PATH: |%s|\n", r->path);

    /*fgets(buffer, strlen(buffer), fs);
    printf("BUFFER: %s\n", buffer);*/

    /* Determine mimetype */
    mimetype = determine_mimetype(r->path);
    /* Write HTTP Headers with OK status and determined Content-Type */
    fprintf(r->file, "HTTP/1.0 200 OK\n");
    fprintf(r->file, "Context-Type: text/html\n");
    fprintf(r->file, "\r\n");

    /* Read from file and write to socket in chunks */
    while ((nread = fread(buffer, 1, BUFSIZ, fs)) > 0) // HELP
    {
        //printf("BUFFER WATCH: %s\n", buffer);
        fwrite(buffer, 1, nread, r->file); // HELP
    }

    /* Close file, flush socket, deallocate mimetype, return OK */
    if (fclose(fs) != 0)
    {
        fprintf(stderr, "Unable to close file: %s\n", strerror(errno));     
        return HTTP_STATUS_NOT_FOUND;
    }
    
    if (fflush(r->file) != 0)
    {
        fprintf(stderr, "Unable to flush socket: %s\n", strerror(errno));
        //printf("File Request FAILED\n");
        return HTTP_STATUS_NOT_FOUND;
    }
    //printf("FLUSHED\n");
    
    free(mimetype);

    return HTTP_STATUS_OK;
}

/**
 * Handle file request
 *
 * This popens and streams the results of the specified executables to the
 * socket.
 *
 *
 * If the path cannot be popened, then handle error with
 * HTTP_STATUS_INTERNAL_SERVER_ERROR.
 **/
http_status
handle_cgi_request(struct request *r)
{
    FILE *pfs;
    char buffer[BUFSIZ];

    /* Export CGI environment variables from request:
    * http://en.wikipedia.org/wiki/Common_Gateway_Interface */
    setenv("REQUEST_METHOD", r->method, 1);
    setenv("REQUEST_URI", r->uri, 1);
    setenv("SCRIPT_FILENAME", r->path, 1);
    setenv("QUERY_STRING", r->query, 1);
    setenv("REMOTE_ADDR", r->host, 1);
    setenv("REMOTE_PORT", r->port, 1); // or SERVER_PORT
    setenv("SERVER_PORT", Port, 1);
    setenv("DOCUMENT_ROOT", RootPath, 1);

    
    /* Export CGI environment variables from request headers */
    for (struct header *h = r->headers; h != NULL; h = h->next)
    {
        if (streq(h->name, "Host"))
            setenv("HTTP_HOST", h->value, 1);
        else if (streq(h->name, "Accept"))
            setenv("HTTP_ACCEPT", h->value, 1);
        else if (streq(h->name, "Accept-Language"))
            setenv("HTTP_ACCEPT_LANGUAGE", h->value, 1);
        else if (streq(h->name, "Accept-Encoding"))
            setenv("HTTP_ACCEPT_ENCODING", h->value, 1);
        else if (streq(h->name, "Connection"))
            setenv("HTTP_CONNECTION", h->value, 1);
        else if (streq(h->name, "User-Agent"))
            setenv("HTTP_USER_AGENT", h->value, 1);
    }
    /* POpen CGI Script */
    printf("CGI PATH: %s\n", r->path);
    if ((pfs = popen(r->path, "r")) == NULL)
    {
        fprintf(stderr, "Unable to open CGI Script: |%s|", strerror(errno));
        return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

    /*fprintf(r->file, "HTTP/1.0 200 OK\n");
    fprintf(r->file, "Context-Type: text/html\n");
    fprintf(r->file, "\r\n");
    //fprintf(r->file, "<html>\n");*/

    /* Copy data from popen to socket */
    while (fgets(buffer, BUFSIZ, pfs))
    {
        fputs(buffer, r->file);
    }

    //fprintf(r->file, "</html>\n");

    /* Close popen, flush socket, return OK */
    if (pclose(pfs) < 0)
    {
        fprintf(stderr, "Unable to close file: %s\n", strerror(errno));
        return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

    if (fflush(r->file) != 0)
    {
        fprintf(stderr, "Unable to flush socket: %s\n", strerror(errno));
        printf("CGI Request FAILED\n");
        return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

    return HTTP_STATUS_OK;
}

/**
 * Handle displaying error page
 *
 * This writes an HTTP status error code and then generates an HTML message to
 * notify the user of the error.
 **/
http_status
handle_error(struct request *r, http_status status)
{
    const char *status_string = http_status_string(status);

    printf("Status: %d\n", status);
    
    /* Write HTTP Header */
    fprintf(r->file, "HTTP/1.0 200 OK\n");//, status_string);
    fprintf(r->file, "Context-Type: text/html\n");
    fprintf(r->file, "\r\n");

    printf("String: %s\n\n", status_string);

    /* Write HTML Description of Error*/
    fprintf(r->file, "<html>\n");
    fprintf(r->file, "<h1>\n");
    fprintf(r->file, "%s\n", status_string);
    fprintf(r->file, "</h1>\n");
    fprintf(r->file, "<h2>\n");
    fprintf(r->file, "Wut u doin bruh?\n"); // ??? 
    fprintf(r->file, "</h2>\n");
    fprintf(r->file, "</html>\n");
    
    if (fflush(r->file) != 0)
    {
        fprintf(stderr, "Unable to flush socket: %s\n", strerror(errno));
        return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

    /* Return specified status */
    return status;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
