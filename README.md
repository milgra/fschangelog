# fschangelog
Writes actual file system tree into a lightweight text file and compares these text files for changes. Works on POSIX systems with dirent.h.

How to use it :
Compile it with clang/gcc : 
> gcc main.c mtcstr.c mtmap.c mtmem.c mtvec.c -o fschangelog

Create a snapshot :
> ./fschangelog -s

It will create a snapshot.txt automatically in the working directory with the current date.

Compare snapshots :
> ./fschangelog -c log_old.txt log_nex.txt

Todo :
    - Compress the log files 
