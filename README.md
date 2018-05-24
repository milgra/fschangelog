# fschangelog
Writes actual file system tree into a lightweight text file and compares these text files for changes. Works on POSIX systems with dirent.h.

How to use it :
Compile it with clang/gcc : 
> gcc main.c mtcstr.c mtmap.c mtmem.c mtvec.c -o fschangelog

Create a snapshot :
> ./fschangelog

It will create a snapshot.txt automatically in the working directory with the current date.

Compare snapshots :
> ./fschangelog log_old.txt log_nex.txt

Todo :
    - Add a switch that enables comparing by date also
    - Add a switch that sets the top level path of the comparison
