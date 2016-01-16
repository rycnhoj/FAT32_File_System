# COP4610 - Project 3 - FAT32 File System
## Due Date: 12-04-2015 11:59:59pm

##### Group Members

* Evan Lee (El12b)
* John Cyr(Jrc11v)
* Abraheem Omari(Afo12)

##### Tar Archive / Files

fat.c - Handles terminal simulation. Allows user to input filesystem commands such as open, ls, etc.
functions.c - Handles implementations for all utility functions.

##### Server

Linprog, compiled with gcc. 

##### Makefile Commands

The 'make' command will compile all necessary components. The 'make clean' command will clean the directory of generated files. The 'make backup' command automatically pushes are changes to our Github repo.

Then execute it with ./fat.exe

##### Known Bugs / Incomplete

Large files/directories spanning multiple clusters aren't supported.
* Read only reads the first cluster
* Write only writes to the first cluster
* ls can only list up to 6 files in the directory
* You can only write to the first cluster of a file

Create utility doesn’t work properly.

Remove utility doesn’t work entirely. The portion that doesn’t work has been commented out to avoid damaging the image.

Mkdir and Rmdir utility check if the directory currently exists. Rmdir additionally checks if it is not a file and not a directory.
