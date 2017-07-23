#!/usr/bin/env python2.7

import multiprocessing
import os
import requests
import sys
import time

# Globals

ARGUMENTS = sys.argv[1:]
PROCESSES = 1
REQUESTS  = 1
VERBOSE   = False
URL       = None

# Functions

def usage(status=0):
    print '''Usage: {} [-p PROCESSES -r REQUESTS -v] URL
    -h              Display help message
    -v              Display verbose output

    -p  PROCESSES   Number of processes to utilize (1)
    -r  REQUESTS    Number of requests per process (1)
    '''.format(os.path.basename(sys.argv[0]))
    sys.exit(status)

def do_request(pid):
    totalTime = 0
    avgTime = 0;
    for req in range(0,REQUESTS):
        start = time.time()
        r = requests.get(URL)
        if r.status_code != 200:
            print "Error: could not get request", URL
            sys.exit(1)
        end = time.time()
        print "Process: {}, Request: {}, Elapsed Time: {}".format(pid, req, round(end - start, 2))
        totalTime = totalTime + (end - start)
        avgTime = totalTime / REQUESTS
        
    print "Process: {}, AVERAGE   , Elapsed Time: {}".format(pid, round(avgTime, 2))
    return avgTime
    pass

# Main execution

if __name__ == '__main__':
    # Parse command line arguments
    while len(ARGUMENTS) and ARGUMENTS[0].startswith('-') and len(ARGUMENTS[0]) > 1:
        arg = ARGUMENTS.pop(0);
        if arg == '-v':
            VERBOSE = True
        elif arg == '-p':
            PROCESSES = int(ARGUMENTS.pop(0))
        elif arg == '-r':
            REQUESTS = int(ARGUMENTS.pop(0))
        elif arg == '-h':
            usage(0)
        else:
            usage(1)
    
    if len(ARGUMENTS) > 0:
        URL = ARGUMENTS.pop(0)
    else:
        usage(1)

    if VERBOSE:
        r = requests.get(URL)
        sys.stdout.write(r.content)

    # Create pool of workers and perform requests
    totalAverage = 0;
    pool = multiprocessing.Pool(PROCESSES)
    time = pool.map(do_request, range(0,PROCESSES)) 
    for t in time:
        totalAverage += t
    totalAverage = (totalAverage/PROCESSES)
    print "TOTAL AVERAGE ELAPSED TIME: {}".format(round(totalAverage, 2))
    pass

# vim: set sts=4 sw=4 ts=8 expandtab ft=python:
