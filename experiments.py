#!/usr/bin/env python2.7

import commands
import os
import sys

requests = 10
processes = [ 1, 2, 4]
urls = [ "http://student00.cse.nd.edu:9777", "http://student00.cse.nd.edu:9777/text/hackers.txt", "http://student00.cse.nd.edu:9777/scripts/cowsay.sh" ]
samplefiles = [ "http://student00.cse.nd.edu:9777/samples/output1.dat", "http://student00.cse.nd.edu:9777/samples/output2.dat", "http://student00.cse.nd.edu:9777/samples/output3.dat" ]

print('| METHOD | PROCESSES |	  1 KB     |      1 MB    |      1 GB    |')
print('|--------|-----------|--------------|--------------|--------------|')
for num in processes:
  print "| SINGLE |     {}     |".format(num),
  for url in samplefiles:
    output = os.popen("./thor.py -r {} -p {} {}".format(requests, num, url))
    for line in output:
      if "AVERAGE ELAPSED TIME" in line:
	print "    {}     |".format(line.split(':')[1].strip()),
  print

#print type(output)
