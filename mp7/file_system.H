/*
    File: file_system.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 21/11/28

    Description: Simple File System.


*/

#ifndef _FILE_SYSTEM_H_ // include file only once
#define _FILE_SYSTEM_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "simple_disk.H"
#include "file.H"

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

class FileSystem;
class File;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/
#define BLOCKSIZE 512

class Inode
{
   friend class FileSystem; // The inode is in an uncomfortable position between
   friend class File;       // File System and File. We give both full access
                            // to the Inode.

private:
   long id; // File "name"

   /* You will need additional information in the inode, such as allocation
      information. */

   /* You may need a few additional functions to help read and store the
      inodes from and to disk. */
   File *file;
   Inode *next;
};

/*--------------------------------------------------------------------------*/
/* FORWARD DECLARATIONS */
/*--------------------------------------------------------------------------*/

/* (none) */

/*--------------------------------------------------------------------------*/
/* F i l e S y s t e m  */
/*--------------------------------------------------------------------------*/

class FileSystem
{

   friend class Inode;

private:
   /* -- DEFINE YOUR FILE SYSTEM DATA STRUCTURES HERE. */
   static unsigned int size;

   static Inode *fileHead;
   static Inode *fileTail;

   static unsigned int file_count;


public:

   SimpleDisk * disk;
   static unsigned int freeBlock;

   FileSystem();
   /* Just initializes local data structures. Does not connect to disk yet. */

   ~FileSystem();
   /* Unmount file system if it has been mounted. */

   bool Mount(SimpleDisk *_disk);
   /* Associates this file system with a disk. Limit to at most one file system per disk.
      Returns true if operation successful (i.e. there is indeed a file system on the disk.) */

   static bool Format(SimpleDisk *_disk, unsigned int _size);
   /* Wipes any file system from the disk and installs an empty file system of given size. */

   Inode *LookupFile(int _file_id);
   /* Find file with given id in file system. If found, return its inode.
        Otherwise, return null. */

   bool CreateFile(int _file_id);
   /* Create file with given id in the file system. If file exists already,
      abort and return false. Otherwise, return true. */

   bool DeleteFile(int _file_id);
   /* Delete file with given id in the file system; free any disk block occupied by the file. */

   void AddToInode(File *newFile, unsigned int newfileID);
   /* Adds the new File Entry in the File List*/

   void RemoveFromInode(unsigned int fileID);

   File *getFile(int fileID);
};
#endif
