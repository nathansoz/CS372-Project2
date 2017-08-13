To start, build the native code using build.sh. This will drop a fserve executable in the root of the project.

Now you can start fserve:

-bash-4.2$ ./fserve 6655
Welcome to fserver!
User specified port is: 6655
Starting server...

At this point, you can cd to the client directory and use fclient.py to interact with the server. You must pass the arguments --server --port --dataport to the python script. Any text after these arguments is interpreted as your command. Example:

-bash-4.2$ ./fclient.py --server localhost --port 6655 --dataport 6656 -l
Connecting to localhost on port 6655
.gitignore
fserve
.README.txt.swp
build.sh

Cool! We were able to get a listing of the workspace of our project. We can now use this to "transfer" build.sh into the client directory.

-bash-4.2$ ./fclient.py --server localhost --port 6655 --dataport 6656 -g build.sh
Connecting to localhost on port 6655
Got file build.sh
-bash-4.2$ ls
build.sh  fclient.py


The files are copied as binary, so they should be exactly the same:

-bash-4.2$ md5sum build.sh
332c77f0a5a7eb9447eb25c20a2041d6  build.sh
-bash-4.2$ md5sum ../build.sh
332c77f0a5a7eb9447eb25c20a2041d6  ../build.sh


This should work with pretty large files as well:

-bash-4.2$ dd if=/dev/urandom of=../mytestfile
^C103303+0 records in
103302+0 records out
52890624 bytes (53 MB) copied, 4.42909 s, 11.9 MB/s

-bash-4.2$ ./fclient.py --server localhost --port 6655 --dataport 6656 -l
Connecting to localhost on port 6655
README.txt
.gitignore
fserve
.README.txt.swp
build.sh
mytestfile

-bash-4.2$ ./fclient.py --server localhost --port 6655 --dataport 6656 -g mytestfile
Connecting to localhost on port 6655
Got file mytestfile
-bash-4.2$ md5sum mytestfile
2e89abca731f86944acf9f1920dcc8d6  mytestfile
-bash-4.2$ md5sum ../mytestfile
2e89abca731f86944acf9f1920dcc8d6  ../mytestfile
-bash-4.2$


FOR EXTRA CREDIT, binary files work. For example, I can copy the server and run it after the copy:

-bash-4.2$ ./fclient.py --server localhost --port 6655 --dataport 6656 -g fserve
Connecting to localhost on port 6655
Got file fserve
-bash-4.2$ ls
fclient.py  fserve  mytestfile
-bash-4.2$ chmod +x fserve
-bash-4.2$ ./fserve
Error! Invalid usage.

Usage:
        fserve <port>
-bash-4.2$ ./fserve 6655
Welcome to fserver!
User specified port is: 6655
Starting server...
server: bind: Address already in use
server: bind: Address already in use
server: failed to bind

