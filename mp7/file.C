/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
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
#include "file.H"
#include "file_system.H"

extern FileSystem *FILE_SYSTEM;

File::File(FileSystem *_fs, int _id)
{
    /* We will need some arguments for the constructor, maybe pointer to disk
     block with file management and allocation data. */
    Console::puts("In file constructor.\n");

    File *f = _fs->getFile(_id);

    fileSize = 512;
    currentPosition = 0;
    startInfoBlock = f->startInfoBlock;
    startDataBlock = f->startDataBlock;
    currentBlock = f->currentBlock;
    fileEndBlock = f->fileEndBlock;
    dataBlockCount = f->dataBlockCount;
}

File::~File()
{
    Console::puts("closing the file.\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf)
{
    Console::puts("reading from file\n");

    unsigned char *readBuff = new unsigned char[512];
    unsigned int read_position = 0;

    while (read_position < _n)
    {
        FILE_SYSTEM->disk->read(currentBlock, readBuff);

        unsigned int readBytesRemaining = _n - read_position;
        unsigned int blockBytesRemaining = 508 - currentPosition;

        if (readBytesRemaining <= blockBytesRemaining)
        {
            memcpy(_buf + read_position, readBuff + currentPosition, readBytesRemaining);
            read_position = read_position + readBytesRemaining;
            currentPosition = currentPosition + readBytesRemaining;

            if (read_position == _n)
                return _n;
        }
        else
        {
            memcpy(_buf + read_position, readBuff + currentPosition, blockBytesRemaining);
            read_position = read_position + blockBytesRemaining;
            currentPosition = 0;
            memcpy(&currentBlock, readBuff + 508, 4);
        }
    }

    return _n;
}

int File::Write(unsigned int _n, const char *_buf)
{
    Console::puts("writing to file\n");

    unsigned char *writeBuff = new unsigned char[512];
    unsigned int writePosition = 0;

    while (writePosition < _n)
    {
        FILE_SYSTEM->disk->read(currentBlock, writeBuff);

        unsigned int writeBytesRemaining = _n - writePosition;
        unsigned int blockBytesRemaining = 508 - currentPosition;

        if (writeBytesRemaining < blockBytesRemaining)
        {
            memcpy(writeBuff + currentPosition, _buf + writePosition, writeBytesRemaining);
            FILE_SYSTEM->disk->write(currentBlock, writeBuff);

            writePosition += writeBytesRemaining;
            currentPosition += writeBytesRemaining;

            if (currentPosition == _n)
                Console::puts("write complete \n");
        }
        else
        {

            memcpy(writeBuff + currentPosition, _buf + writePosition, blockBytesRemaining);
            FILE_SYSTEM->disk->write(currentBlock, writeBuff);

            writePosition += blockBytesRemaining;
            currentPosition = 0;

            currentBlock = FILE_SYSTEM->freeBlock;
            FILE_SYSTEM->disk->read(FILE_SYSTEM->freeBlock, writeBuff);
            memcpy(&(FILE_SYSTEM->freeBlock), writeBuff + 508, 4);
            dataBlockCount += 1;
        }
    }

    if (((dataBlockCount - 1) * 512 + currentPosition) > fileSize)
    {
        fileSize = dataBlockCount * 512 + currentPosition;
        fileEndBlock = currentBlock;
    }

    return _n;
}

void File::Reset()
{
    Console::puts("reset current position in file\n");

    currentPosition = 0;
    currentBlock = startDataBlock;
}

bool File::EoF()
{
    Console::puts("testing end-of-file condition\n");
    return (dataBlockCount * 512 + currentPosition) == fileSize;
}

void File::Rewrite()
{
    Console::puts("erase content of file\n");

    unsigned char *writeBuff = new unsigned char[512];
    unsigned char *readBuff = new unsigned char[512];

    unsigned int temp_curr_block = startDataBlock;
    int temp_next_block = 0;

    for (unsigned int i = 0; i < dataBlockCount; i++)
    {
        if (temp_next_block == -1)
            break;

        FILE_SYSTEM->disk->read(temp_curr_block, readBuff);
        memcpy(&temp_next_block, readBuff + 508, 4);

        memcpy(writeBuff + 508, &(FILE_SYSTEM->freeBlock), 4);
        FILE_SYSTEM->disk->write(temp_curr_block, writeBuff);

        if (startDataBlock != temp_curr_block)
            FILE_SYSTEM->freeBlock = temp_curr_block;

        temp_curr_block = temp_next_block;
    }

    fileSize = 512;

    currentPosition = 0;
    currentBlock = startDataBlock;

    fileEndBlock = startDataBlock;
    dataBlockCount = 1;
}

unsigned int File::getInfoBlock() {
    return startInfoBlock;
}

unsigned int File::getDataBlockCount() {
    return dataBlockCount;
}

void File::setData(unsigned int *_startInfoBlock, unsigned int *_startDataBlock)
{
    startInfoBlock = *_startInfoBlock;
    startDataBlock = *_startDataBlock;
    currentBlock = startDataBlock;
    fileEndBlock = startDataBlock; // To keep track if file size exceeds 512
    dataBlockCount = 1;
}
