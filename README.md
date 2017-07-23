Project 02 - README
===================

Members
-------

- Jessica Hardey (jhardey@nd.edu)
- Aidan Lewis (alewis9@nd.edu)

Summary
-------

In this project we successfully created a very simple web server. Certain parts of this process worked nicely and on the first few tries, while much of what we did did not work, requiring extensive debugging. We successfully implemented, without too much trouble, thor.py, socket.c, utils.c, the Makefile, and spidey.c. These files inlcluded minor errors that we were able to debug with a few external testing files. We originally thought that single.c and forking.c were not working properly, however we eventually found that all issues associated with these files were actually problems in request.c and handler.c. Thus we correctly implemented single and forking mode. Nearly all of the major issues in our program had to do with request.c and handler.c, in getting, parsing, and handling HTTP requests. We struggled to properly allocate and free memory for the headers inside the request structure. We came across issues in using various functions to open and read files, as well as correctly checking for their failure. It tool multiple tries to properly grab the URI and handle the query. In handler.c it took a whle to correctly implement error handling in handler_request. Getting the correct HTML list items in handle_browse_request from the entries proved difficult. We struggled to correctly use fread and fwrite, and forgot to flush out error messages to the screen a few times, causing other problems. Handling the CGI environment variables took a number of edits and handling the files in handle_cgi_request was a little confusing. Eventually we fixed all these errors and got the program to work in single mode. However, we could not figure out for the longest time why forking would not work. It turned out it was a very simple line of code handling the freeing of memory in query, which did not check a specific condition and thus would sometimes free memory when it shouldn't. We eventually found this error using Valgrind and the whole program worked in forking mode. 

Latency
-------
| METHOD | PROCESSES |  DIRECTORIES | STATIC FILES |    SCRIPTS   |
|--------|-----------|--------------|--------------|--------------|
| SINGLE |     1     |     0.01     |     0.01     |     0.02     |
| SINGLE |     2     |     0.01     |     0.01     |     0.03     |
| SINGLE |     4     |     0.02     |     0.02     |     0.05     |
| FORKING|     1     |     0.01     |     0.01     |     0.02     |
| FORKING|     2     |     0.01     |     0.01     |     0.02     |
| FORKING|     4     |     0.04     |     0.04     |     0.04     |

We measured the average latency by running thor.py on three differend types of files (directories, static files, and scripts) in single and forking mode, using 1, 2, and 4 processes. Here we're trying to measure the delay in processing this information over the network. From the table we can see that the scripts have the highest latency of all the file types. The directories and static files are comparable in their measurements. There is really no noticeable difference in single mode versus forking in directories and static files except for when using 4 processes. It seems that using too many more processes actually reduces the helpful effector of forking. With the scripts, we can see that forking actually reduces latency by a little. It makes sense that latency would increase as the number of processes increases, which the table shows. Thus forking is ideal for a small number of processes for files like scripts, where as single mode is preferable for more processes and more static files.

Throughput (bytes/s)
----------
| METHOD | PROCESSES |     1 KB       |     1 MB     |     1 GB     |
|--------|-----------|----------------|--------------|--------------|
| SINGLE |     1     |     100000     |     1E+8     |    1.59E+8   |
| SINGLE |     2     |     100000     |     1E+8     |    1.27E+8   |
| SINGLE |     4     |     200000     |     3E+8     |    6.44E+7   |
| FORKING|     1     |     100000     |     1E+8     |    2.11E+8   |
| FORKING|     2     |     100000     |     1E+8     |    2.08E+8   |
| FORKING|     4     |     200000     |     2E+8     |    1.59E+7   |

We measured the average throughput by running thor.py on files of different sizes on both single and forking methods with a range of processes. We did this in order to determine how handling high size loads affected the average throughput of a file. The most obvious takeaway from the table is that as the size of the file increases, the average throughput increases, regardless of the number of processes or the method. Intuitively, the average throughput should decreases when having to carry greater loads. Also, as the number of processes increases, the average throughput also decreases. The most interesting part is the comparison between the two methods. With a fewer number of processes, the forking method tends to have a higher average throughput. However, when the number of processes is high (four, in this case), the effectiveness of forking decreases incredibly, and the single method has a much higher average throughput. 

Analysis
--------
The results of our experiments show clearly that the number of processes increases the  average latency and decreases the average throughput. This makes sense a greater number of processes will require more resources to process, which leads to a slower latency and throughput. The preferable method, meanwhile, depends on the needs of a program. Both tests make clear that the forking method is unable to adequately handle a large number of processes. However, given a small number of processes, forking is preferable for handling large amounts of data (throughput test). In addition, with a small number of processes, the forking method is comparable to single in handling directories and static files, and superior in handling scripts. Therefore, the advantages of the forking model are handling large amount of data with a small number of processes, and its main disadvantage is its inability to adequately function with a large number of processes. This all makes sense because latency effects throughput, in that if there are delays in processing data then bottlenecks can occur, or at least this will limit throughput, or how much data can be moved in a given amount of time.

Conclusion
----------
In this project we learned the number of processes used, the method used to process them, and the types and sized of files tested on, all effect the latency and throughput of a network. Depending on what kind of data you're transporting, and what you would like to do with it, you would want to change these parameters in a different way. In creating the web server itself, we learned that getting all the parts between the client and server to "talk to each other" correctly, is quite difficult. The combination of opening a socket, sending and receiving requests, knowing what to do with and how to interpret these requests, properly setting errors, is all very complicated. We learned the importance of memory management, especially since our final and most frustrating issue was a miniscule memory problem and because valgrind proved very useful in debugging throughout the entire process. We learned a lot about how to debug in this project and about the way machines communicate with each other and what that process looks like, especially since this is something that has always seemed like magic in the past.

Contributions
-------------

Jess: thor.py, Makefile, socket.c, request.c, README.md
Aidan: request.c, handler.c, single.c, forking.c, spidey.c, utils.c, README.md

Debugging: group effort

Note: Jess came down with the flu near the start of the project and was bedridden for ~4 days. So Aidan did a lot of the initial writing of the heftier files, hence the imbalance of distribution above. When Jess was healthier, we debugged and worked on all the files together. 
