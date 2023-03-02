/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store
   inodes from and to disk. */

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/
Inode *FileSystem::fileHead;
Inode *FileSystem::fileTail;
unsigned int FileSystem::freeBlock = 0;
unsigned int FileSystem::file_count = 0;
unsigned int FileSystem::size = 0;

FileSystem::FileSystem()
{
    Console::puts("In file system constructor.\n");

    fileHead = new Inode();
    fileTail = fileHead;

    file_count = 0;
}

FileSystem::~FileSystem()
{
    Console::puts("unmounting file system\n");
    if(disk != NULL)
        disk = NULL;
}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

bool FileSystem::Mount(SimpleDisk *_disk)
{
    Console::puts("mounting file system from disk\n");
    disk = _disk;
    unsigned char *buf = new unsigned char[512];

    disk->read(0, buf);
    memcpy(&size, buf, 4);
    memcpy(&freeBlock, buf + 4, 4);
    memcpy(&file_count, buf + 8, 4);

    return true;
}

bool FileSystem::Format(SimpleDisk *_disk, unsigned int _size)
{ // static!
    Console::puts("formatting disk\n");

    unsigned int numBlocks = _size / BLOCKSIZE;
    unsigned char *buf = new unsigned char[512];


    for (unsigned int block = 1; block < 3; block++)
        _disk->write(block, buf);

    unsigned int block;

    for (block = 3; block < numBlocks - 1; block++)
    {
        unsigned int nextBlock = block + 1;
        memcpy(buf + 508, &nextBlock, 4);
        _disk->write(block, buf);
    }

    int lastBlockLimit = -1;
    memcpy(buf + 508, &lastBlockLimit, 4);
    _disk->write(block, buf);

    unsigned int temp = _size;
    memcpy(buf + 0, &temp, 4);

    temp = 3;
    memcpy(buf + 4, &temp, 4);

    temp = 0;
    memcpy(buf + 8, &temp, 4);
    memcpy(buf + 12, 0, 500);
    _disk->write(0, buf);

    return true;
}

Inode *FileSystem::LookupFile(int _file_id)
{
    Console::puts("looking up file\n");

    Inode *newNode = fileHead;

    if (newNode == NULL)
    {
        Console::puts("\n NO files in LIST \n");
        return NULL;
    }
    else
    {
        for (unsigned int i = 0; i < file_count; i++)
        {
            if (newNode->id == _file_id)
                return newNode;
            newNode = newNode->next;
        }
    }
    return NULL;
}

File* FileSystem::getFile(int fileID) {
    Inode *node = LookupFile(fileID);
    return node->file;
}


void FileSystem::AddToInode(File *newFile, unsigned int newfileID)
{
    Inode *node = new Inode();
    node->id = newfileID;
    node->file = newFile;
    node->next = NULL;

    if (file_count == 0)
    {
        fileHead = node;
        fileTail = node;
    }
    else
    {
        fileTail->next = node;
        fileTail = node;
    }
}

bool FileSystem::CreateFile(int _file_id)
{
    unsigned char *buf = new unsigned char[512];
    unsigned int startInfoBlock = freeBlock;
    unsigned int startDataBlock;

    disk->read(freeBlock, buf);
    memcpy(&startDataBlock, buf + 508, 4);

    disk->read(startDataBlock, buf);
    memcpy(&freeBlock, buf + 508, 4);

    File *newCreatedFile = new File(this, _file_id);
    newCreatedFile->setData(&startInfoBlock, &startDataBlock);

    AddToInode(newCreatedFile, _file_id);
    file_count += 1;

    Console::puts("creating file\n");
    return true;
}

void FileSystem::RemoveFromInode(unsigned int id)
{
    Inode *prev = NULL; 
    Inode *curr = fileHead;

    if (curr == NULL)
    {
        Console::puts("file list empty \n");
        return;
    }

    for (unsigned int i = 0; i < file_count; i++)
    {

        if (curr->id == id)
        {
            unsigned int fileDataBlock = 0;

            unsigned char *read_buf = new unsigned char[512];
            unsigned char *write_buf = new unsigned char[512];

            unsigned int fileInfoBlock = curr->file->getInfoBlock();

            disk->read(fileInfoBlock, read_buf);
            memcpy(&fileDataBlock, read_buf + 508, 4);

            memcpy(write_buf + 508, &freeBlock, 4);
            disk->write(fileInfoBlock, write_buf);

            freeBlock = fileInfoBlock;

            unsigned int next_data_block = 0;

            for (unsigned int j = 0; j < curr->file->getDataBlockCount(); j++)
            {
                disk->read(fileDataBlock, read_buf);
                memcpy(&next_data_block, read_buf + 508, 4);

                memcpy(write_buf + 508, &freeBlock, 4);
                disk->write(fileDataBlock, write_buf);

                freeBlock = fileDataBlock;

                fileDataBlock = next_data_block;
            }
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    if (curr->next == NULL)
    {
        if (prev != NULL)
            prev->next = NULL;
        fileTail = prev;
    }
    else
    {
        if (prev != NULL)
            prev->next = curr->next;
        else
            fileHead = curr->next;
    }
}

bool FileSystem::DeleteFile(int _file_id)
{
    Console::puts("deleting file\n");
    RemoveFromInode(_file_id);
    file_count -= 1;
    return true;
}

