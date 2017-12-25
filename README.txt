CS 485g Project 5
Christopher Will

Files being submitted:
csapp.h
csapp.c
simpled.cpp
ssSet.cpp
ssGet.cpp
ssRun.cpp
ssDigest.cpp
Makefile
README.txt

Documentation/algorithms used:
This project required no especially nuanced or complex algorithms. All 4 clients of course handled 
sending data to the server in the same wasy. Namely, I stored the bytes I need to send in either
a char or int array, and then used Rio_writen(serverfd, arrayName, numBytes); to send these bytes to
the server file descriptor. Each of the 4 client programs did differ in how the server handled them.
ssSet required me to simply add the variable name as a key to a map(hash table) and have the variables 
value be the thing to which the key pointed to. ssGet was defintiely the most involved request to handle.
I first checked whether the userVar is a key in my map, and if it is then I jsut send data back to the client
about this var and the value it maps to in the map. The next thing I do is the check whether userVar is an
environemt string. I can do this by using: system(printenv userVar > /tmp/out.txt) and then checking whether
the size of out.txt is different from 0. If it is then userVar is a valid environment string. But if userVar is
either the empty string OR a string containing only whitepace then out.txt will contain every environment variable
for the user. This is not the output I want. So I 1st check whether userVar has length of 0 OR is made of only whitspace.
If this is true then I set the return value to failure, else I do the aforementioned steps to see if userVar is an
environment string. Handling ssRun is perhaps easier than ssGet. I simply look at the program name given, making sure it
is one of the valid choices. Then in simpled I can set the command I will use in system() based on the program given,
run system(command) and send the output to some tmp file. I then open this tmp file and read in either the 1st 99 bytes OR
the entire file if it's size is less than 99. Lastly, handling ssDigest is much like handling ssRun. I get the value 
the user wishes to encode and call system() using the command given in the project spec. I send stdout to tmp and know
that the result is less than 100 bytes so I can read in the entire file to some buffer. 

Special festures/limitations:
My program should not contain any special features. And to my knowledge there are no bugs.
