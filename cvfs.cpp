///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                                   //
// Project Name : Customized Virtual Dynamic File System                                                                             //
//                                                                                                                                   //
// Description : - This project provides all functionality to the user which is same as linux file system.                           //
//               - It proveides necessary commands, system calls implementations of the file systems through customised shell.       //
//               - In this project we implement all necessary data structures of the file systems like Incore Inode Table,           //
//               - File table, UAREA, User File Descriptr table.                                                                     //
//               - Using this project we can use every system level functionality of linux os on any other os platform.              //
//                                                                                                                                   //
// Author : Bhavesh Patil                                                                                                     //
//                                                                                              //
// Date : 01-01-2025                                                                                                                //
//                                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Header file inclusion
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <io.h> // for windows

// Userdefined Macros
#define MAXINODE 50
#define READ 1
#define WRITE 2
#define MAXFILESNO 50
#define MAXFILESIZE 2048
#define REGULAR 1
#define SPECIAL 2
#define START 0
#define CURRENT 1
#define END 2

/*
    // Structures :

    - superblock : Information about Total and free Inodes.
    - inode :      Information about file.
    - filetable :  Metadata about file, points and work with inode.
    - ufdt :       user file descriptor table - contains the address of filetable.
                    used to create the array of ufdt which holds the address of filetable
                    where index of it treat as fd.
*/

/*
    // Typedef :

    - SUPERBLOCK : struct superblock
    - PSUPERBLOCK : struct superblock *
    - INODE : struct inode
    - PINODE : struct inode *
    - PPINODE : struct inode **
    - FILETABLE : struct filetable
    - PFILETABLE : struct filetable *
*/

typedef struct superblock
{
    int TotalInodes;
    int FreeInode;
} SUPERBLOCK, *PSUPERBLOCK;

typedef struct inode
{
    char FileName[50];
    int InodeNumber;
    int FileSize;
    int FileActualSize;
    int FileType;
    char *Buffer;
    int LinkCount;
    int ReferenceCount;
    int permission;
    struct inode *next;
} INODE, *PINODE, **PPINODE;

typedef struct filetable
{
    int readoffset;
    int writeoffset;
    int count;
    int mode;
    PINODE ptrinode;
} FILETABLE, *PFILETABLE;

typedef struct ufdt
{
    PFILETABLE ptrfiletable;
} UFDT;

// Global Data
UFDT UFDTArr[50]; // Array of file discriptor inside it we will sotre the address of filetable.
SUPERBLOCK SUPERBLOCKobj;
PINODE head = NULL; // To maintain header of INODE linkedlist

// Function definations
void man(char *name);
void DisplayHelp();
int GetFDFromName(char *name);
void InitialiseSuperBlcok();
PINODE Get_Inode(char *name);
void CreateDILB();
void InitialiseSuperBlcok();
int CreateFile(char *name, int permission);
int rm_File(char *name);
int ReadFile(int fd, char *arr, int isize);
int WriteFile(int fd, char *arr, int isize);
int OpenFile(char *name, int mode);
void CloseFileByName(int fd);
void CloseAllFile();
int LseekFile(int fd, int size, int from);
void ls_file();
int fstat_file(int fd);
int fstat_file(char *name);
int truncate_File(char *name);

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     man
//  Input :             char * name
//  Output :            void
//  Description :       Display the information about given command
//                      (Description and syntax)
//  TimeComplexity :    O(1)
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        Match the input name with command string.
        If match found display the information
        ELSE 'no inforamtion found'
    STOP
*/
void man(      // Manual
    char *name // Name of command
)
{
    if (name == NULL)
        return;

    if (strcmp(name, "create") == 0)
    {
        printf("Description : Used to create new regular file\n");
        printf("Usage : create File_name Permission\n");
    }
    else if (strcmp(name, "read") == 0)
    {
        printf("Description : Used to read data form regular file\n");
        printf("Usage : read File_name No_of_Bytes_To_Read\n");
    }
    else if (strcmp(name, "write") == 0)
    {
        printf("Description : Used to write into regular file\n");
        printf("Usage : write File_name\n After this enter the daat that we want to write\n");
    }
    else if (strcmp(name, "ls") == 0)
    {
        printf("Description : Used to list all information of files\n");
        printf("Usage : ls\n");
    }
    else if (strcmp(name, "stat") == 0)
    {
        printf("Description : Used to display information of file\n");
        printf("Usage : stat File_name\n");
    }
    else if (strcmp(name, "fstat") == 0)
    {
        printf("Description : Used to display inforamtion of file\n");
        printf("Usage : fstat File_Descriptor_number\n");
    }
    else if (strcmp(name, "truncate") == 0)
    {
        printf("Description : Used to remove data form  file\n");
        printf("Usage : truncate File_name\n");
    }
    else if (strcmp(name, "open") == 0)
    {
        printf("Description : Used to open existing file\n");
        printf("Usage : open File_name mode\n");
    }
    else if (strcmp(name, "close") == 0)
    {
        printf("Description : Used to close opend file\n");
        printf("Usage : close File_name\n");
    }
    else if (strcmp(name, "closeall") == 0)
    {
        printf("Description : Used to close all opend file\n");
        printf("Usage : closeall\n");
    }
    else if (strcmp(name, "lseek") == 0)
    {
        printf("Description : Used to change file offset\n");
        printf("Usage : lseek File_name ChangeInOffset StartPoint\n");
    }
    else if (strcmp(name, "rm") == 0)
    {
        printf("Description : Used to delete the file\n");
        printf("Usage : rm File_Name\n");
    }
    else
    {
        printf("ERROR : No manual entry available. \n");
    }
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     Display Help
//  Input :             Void
//  Output :            Void
//  Description :       Display the information about all comands
//  TimeComplexity :    O(1)
//
//////////////////////////////////////////////////////////////////////////
void DisplayHelp()
{
    printf("\n----------------------------------------------------------------------\n");
    printf("| ls      | To List out all files\n");
    printf("| clear   | To clear console\n");
    printf("| open    | To open the file\n");
    printf("| closeall| To close all opened files\n");
    printf("| create  | To Create new regular file\n");
    printf("| read    | To Read the contents from file\n");
    printf("| write   | To write contents into file\n");
    printf("| exit    | To Terminate the file system\n");
    printf("| stat    | To Display information of file using name\n");
    printf("| fstat   | To Display information of file using file descriptor\n");
    printf("| truncate| To remove all data from file\n");
    printf("| rm      | To Delete the file\n");
    printf("----------------------------------------------------------------------\n");
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     GetFDFromName
//  Input :             char * name
//  Output :            int
//  Description :       Gives the filedescriptor by file name
//  TimeComplexity :    O(n)    n = no. of max. files
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        Intilize the counter with 0.
        Iterate through all present files with increment counter.
        If filetable contains the name of file break the loop and return the counter.
        If counter == MAXFILENO then return error code -1.
    STOP
*/
int GetFDFromName(
    char *name // Name of file
)
{
    int iCnt = 0;
    while (iCnt < MAXFILESNO)
    {
        if (UFDTArr[iCnt].ptrfiletable != NULL)
        {
            if (strcmp((UFDTArr[iCnt].ptrfiletable->ptrinode->FileName), name) == 0)
            {
                break;
            }
        }
        iCnt++;
    }
    if (iCnt == MAXFILESNO) // no file found
        return -1;
    else
        return iCnt; // return fd
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     Get_Inode
//  Input :             char * name
//  Output :            int
//  Description :       Gives the address of INODE of input file_name.
//  TimeComplexity :    O(n)    where n = no. of nodes in INODE linkedlist
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        Iterate through all INODE Linkedlist by inode pointer.
        If file_name found break the loop return the inode pointer.
    STOP
*/
PINODE Get_Inode(char *name)
{
    PINODE temp = head;
    int i = 0;

    if (name == NULL)
        return NULL;

    while (temp != NULL)
    {
        if (strcmp(name, temp->FileName) == 0)
            break;
        temp = temp->next;
    }
    return temp; // If no file temp will be null
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     CreateDILB
//  Input :             void
//  Output :            void
//  Description :       Create and intiate the MAXINODE length linkedlist
//                      Intilze the attributes of INODE linkedlist
//  TimeComplexity :    O(n)  where n = no. of nodes in INODE linkedlist
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        Iterate while loop for MAXIONDE times
        In each iteration allocate the memory for new node
        intilize attributes of inode for each new node of linkedlist
        Insert the new node into the linkedlist
    STOP
*/
void CreateDILB()
{
    int i = 1;
    PINODE newn = NULL;
    PINODE temp = head;

    while (i <= MAXINODE)
    {
        newn = (PINODE)malloc(sizeof(INODE));

        newn->LinkCount = 0;
        newn->ReferenceCount = 0;
        newn->FileType = 0;
        newn->FileSize = 0;

        newn->Buffer = NULL;
        newn->next = NULL;

        newn->InodeNumber = i;

        if (temp == NULL)
        {
            head = newn;
            temp = head;
        }
        else
        {
            temp->next = newn;
            temp = temp->next;
        }
        i++;
    }
    // printf("DILB created successfully\n");
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     IntialiseSuperBlock
//  Input :             void
//  Output :            void
//  Description :       set the ponter of filetable which is inside superblock
//                      to null. intlize the toatlinode and free inode
//  TimeComplexity :    O(n)    where n = no. of nodes in INODE linkedlist
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        Iterate while loop for MAXIONDE times
        Initlize ptrfiletable for each fd to null while iterating.
        Initilize Totalnodes and freenode of inode with MAXINODE.
    STOP
*/
void InitialiseSuperBlcok()
{
    int i = 0;
    while (i < MAXINODE) // Intilize all array with null
    {
        UFDTArr[i].ptrfiletable = NULL;
        i++;
    }

    // Intially intilize following data with MAXINODE(50)
    SUPERBLOCKobj.TotalInodes = MAXINODE;
    SUPERBLOCKobj.FreeInode = MAXINODE; // it will ++ or -- // when it will 50 then we can't create more files
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     CreateFile
//  Input :             char *name, int permission
//  Output :            int(fd)
//  Description :       Create new file and return fd by ufdt array index
//  TimeComplexity :    O(1)
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        Fileters for invalid permission and freeinode
        Decrement the Freeinode by 1
        Check for file name presence if file name already found return error code and incrment Freeinode again.
        Find out empty space in ufdt array for filetable ptr
        For filetable allocate memory layout intilize its address to fileptr of ufdtarrindex and intlize attributes of filetable.
        Return the index of ufdt in which we allocated memory for filetable as fd.
    STOP
*/
int CreateFile(char *name, int permission)
{
    int i = 0;
    PINODE temp = head;

    if ((name == NULL) || (permission == 0) || (permission > 3))
        return -1;

    if (SUPERBLOCKobj.FreeInode == 0)
        return -2;

    (SUPERBLOCKobj.FreeInode)--;

    if (Get_Inode(name) != NULL)
    {
        (SUPERBLOCKobj.FreeInode)++;
        return -3;
    }

    while (temp != NULL)
    {
        if (temp->FileType == 0)
            break;
        temp = temp->next;
    }

    while (i < MAXINODE)
    {
        if (UFDTArr[i].ptrfiletable == NULL)
            break;
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = permission;
    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;

    UFDTArr[i].ptrfiletable->ptrinode = temp;

    strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName, name);
    UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
    UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
    UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
    UFDTArr[i].ptrfiletable->ptrinode->permission = permission;
    UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char *)malloc(MAXFILESIZE);

    return i;
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     rm_File
//  Input :             char *name
//  Output :            int
//  Description :       Remove file
//  TimeComplexity :    O(1)
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        Get fd from file name.
        Decrement the LinkCount from inode.
        Set filetype to 0
        Free filetable memory
        Increment FreeInode count
    STOP
*/
int rm_File(char *name)
{
    int fd = 0;

    fd = GetFDFromName(name);
    if (fd == -1)
        return -1;

    (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

    if (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
    {
        UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;
        // free(UFDTArr[fd].ptrfiletable->ptrinode->Buffer);
        free(UFDTArr[fd].ptrfiletable);
    }

    UFDTArr[fd].ptrfiletable = NULL;
    (SUPERBLOCKobj.FreeInode)++;

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     ReadFile
//  Input :             int fd, char *arr, int isize
//  Output :            int
//  Description :       Read file by set offset and
//  TimeComplexity :    O(1)
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        read_size = the size of file from right of curosr to the end of file.
        If no. of characters we want read are more than filecontent form offset then copy all data in arr
        Else copy the isize data in arr
        Reset offset
        Return isize
    STOP
*/
int ReadFile(int fd, char *arr, int isize)
{
    int read_size = 0;

    // Filters with error code return-
    if (UFDTArr[fd].ptrfiletable == NULL)
        return -1;
    if (UFDTArr[fd].ptrfiletable->mode != READ && UFDTArr[fd].ptrfiletable->mode != READ + WRITE)
        return -2;
    if (UFDTArr[fd].ptrfiletable->ptrinode->permission != READ && UFDTArr[fd].ptrfiletable->ptrinode->permission != READ + WRITE)
        return -2;
    if (UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
        return -3;
    if (UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
        return -4;

    read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFDTArr[fd].ptrfiletable->readoffset);
    if (read_size < isize)
    {

        strncpy(arr,                                                                                   // Destination array
                (UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset), // Points to Dataaddress location from where we want to cpy
                read_size                                                                              // no. of bytes that want to cpy
        );

        // Reset offset
        UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset + read_size;
    }
    else
    {
        // copy the numbers of characters that we want to cpy
        strncpy(arr, (UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset), isize);

        (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + isize;
    }
    return isize;
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     WriteFile
//  Input :             int fd, char *arr, int isize
//  Output :            int
//  Description :       Write content into the file
//  TimeComplexity :    O(1)
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        Add arr content of isize bytes in Bufferlocation at offset.
        Reset offset
        Increase file size by isize bytes
    STOP
*/
int WriteFile(int fd, char *arr, int isize)
{
    if (((UFDTArr[fd].ptrfiletable->mode) != WRITE) && ((UFDTArr[fd].ptrfiletable->ptrinode->permission) != READ + WRITE))
        return -1;
    if ((UFDTArr[fd].ptrfiletable->writeoffset) == MAXFILESIZE)
        return -2;
    if ((UFDTArr[fd].ptrfiletable->ptrinode->FileType) != REGULAR)
        return -3;

    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->writeoffset), arr, isize);

    (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + isize;

    (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize;

    return isize; // actual wirte system calls return the sussessful no. of bytes return in file
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     OpenFile
//  Input :             char *name, int mode
//  Output :            int
//  Description :       Open file
//  TimeComplexity :    O(1)
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        Allocate the memory location for filetable
        Set Filetable attributes
    STOP
*/
int OpenFile(char *name, int mode)
{
    int i = 0;
    PINODE temp = NULL;

    // Filters
    if (name == NULL || mode <= 0)
        return -1;

    temp = Get_Inode(name);
    if (temp == NULL)
        return -2;

    if (temp->permission < mode)
        return -3;

    while (i < MAXINODE)
    {
        if (UFDTArr[i].ptrfiletable == NULL)
            break;
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
    if (UFDTArr[i].ptrfiletable == NULL) // If memory not allocated
        return -1;
    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = mode;
    if (mode == READ + WRITE)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }
    else if (mode == READ)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
    }
    else if (mode == WRITE)
    {
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }
    UFDTArr[i].ptrfiletable->ptrinode = temp;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)++;
    return i;
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     CloseFileByName
//  Input :             int fd
//  Output :            void
//  Description :       close opend file by taking file descriptor
//  TimeComplexity :    O(1)
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        Set readoffset and writeoffset to the zero
        Decrement the Referencecount
    STOP
*/
void CloseFileByName(int fd)
{
    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;
    (UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)--;
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     CloseFileByName
//  Input :             char *name
//  Output :            void
//  Description :       close opend file by taking file name
//  TimeComplexity :    O(1)
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        Set readoffset and writeoffset to the zero
        Decrement the Referencecount
    STOP
*/
int CloseFileByName(char *name)
{
    int i = 0;
    i = GetFDFromName(name);
    if (i == -1)
        return -1;

    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     CloseFileFiles
//  Input :             void
//  Output :            void
//  Description :       close all opened files
//  TimeComplexity :    (n)    where n = no. of nodes in INODE linkedlist
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        Iterate through all existings files
        Set readoffset and writeoffset
        Decrement referanceCount
    STOP
*/
void CloseAllFile()
{
    int i = 0;
    while (i < MAXINODE)
    {
        if (UFDTArr[i].ptrfiletable != NULL)
        {
            UFDTArr[i].ptrfiletable->readoffset = 0;
            UFDTArr[i].ptrfiletable->writeoffset = 0;
            (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
            break;
        }
        i++;
    }
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     LseekFile
//  Input :             int fd, int size, int from
//  Output :            int
//  Description :
//  TimeComplexity :    O(1)
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        If mode is for READ or READ + WRITE then set lseek according to starting point.
        If mode is for WRITE only set lseek according to statrting point
    STOP
*/
int LseekFile(int fd, int size, int from)
{
    if ((fd < 0) || (from > 2))
        return -1;
    if (UFDTArr[fd].ptrfiletable == NULL)
        return -1;

    if ((UFDTArr[fd].ptrfiletable->mode == READ) || (UFDTArr[fd].ptrfiletable->mode == READ + WRITE))
    {
        if (from == CURRENT)
        {
            if (((UFDTArr[fd].ptrfiletable->readoffset) + size) > UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
                return -1;
            if (((UFDTArr[fd].ptrfiletable->readoffset) + size) < 0)
                return -2;
            (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + size;
        }
        else if (from == START)
        {
            if (size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
                return -1;
            if (size < 0)
                return -1;
            (UFDTArr[fd].ptrfiletable->readoffset) = size;
        }

        else if (from == END)
        {
            if ((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)
                return -1;
            if (((UFDTArr[fd].ptrfiletable->readoffset) + size) < 0)
                return -1;
            (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;
        }
    }

    else if (UFDTArr[fd].ptrfiletable->mode == WRITE)
    {
        if (from == CURRENT)
        {
            if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) > MAXFILESIZE)
                return -1;
            if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) < 0)
                return -1;
            if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
                (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + size;
        }
        else if (from == START)
        {
            if (size > MAXFILESIZE)
                return -1;
            if (size < 0)
                return -1;
            if (size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
                (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = size;
            (UFDTArr[fd].ptrfiletable->writeoffset) = size;
        }
        else if (from == END)
        {
            if ((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)
                return -1;
            if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) < 0)
                return -1;
            (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;
        }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     ls_file
//  Input :             void
//  Output :            void
//  Description :       List out all existing files
//  TimeComplexity :    O(n)    where n = no. of nodes in INODE linkedlist
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        Traverse through all existing files
        Display the basic information about files
    STOP
*/
void ls_file()
{
    int i = 0;
    PINODE temp = head;

    if (SUPERBLOCKobj.FreeInode == MAXINODE)
    {
        printf("Error : There are no files\n");
        return;
    }

    printf("\nFile Name\t Inode number\tFile size\tLink count\n");
    printf("--------------------------------------------------\n");
    while (temp != NULL)
    {
        if (temp->FileType != 0)
        {
            printf("%s\t\t%d\t\t%d\t\t%d\n", temp->FileName, temp->InodeNumber, temp->FileActualSize, temp->LinkCount);
        }
        temp = temp->next;
    }
    printf("--------------------------------------------------\n");
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     fstat_file
//  Input :             int fd
//  Output :            int
//  Description :       Display information of file
//  TimeComplexity :    O(1)
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        Display the statstical information of file by filedescriptor
    STOP
*/
int fstat_file(int fd)
{
    PINODE temp = head;
    int i = 0;

    if (fd < 0)
        return -1;
    if (UFDTArr[fd].ptrfiletable == NULL)
        return -2;

    temp = UFDTArr[fd].ptrfiletable->ptrinode;

    printf("\n--------Statistical Information about file----------\n");
    printf("File name : %s\n", temp->FileName);
    printf("Inode Number %d\n", temp->InodeNumber);
    printf("File size : %d\n", temp->FileSize);
    printf("Actual File size : %d\n", temp->FileActualSize);
    printf("Link count : %d\n", temp->LinkCount);
    printf("Reference count : %d\n", temp->ReferenceCount);

    if (temp->permission == 1)
        printf("File Permission : Read only\n");
    else if (temp->permission == 2)
        printf("File permission : Write\n");
    else if (temp->permission == 3)
        printf("File permission : Read & write\n");
    printf("-------------------------------------------------\n\n");

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     stat_file
//  Input :             char *name
//  Output :            int
//  Description :       DDisplay information of file
//  TimeComplexity :    (n)    where n = no. of nodes in INODE linkedlist
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        Display the statstical information of file by name
    STOP
*/
int stat_file(char *name)
{
    PINODE temp = head;
    int i = 0;

    if (name == NULL)
        return -1;

    while (temp != NULL)
    {
        if (strcmp(name, temp->FileName) == 0)
            break;
        temp = temp->next;
    }

    if (temp == NULL)
        return -2;

    printf("\n----------Statistical Information about file-----------\n");
    printf("File name : %s\n", temp->FileName);
    printf("Inode Number : %d", temp->InodeNumber);
    printf("\nFile size : %d", temp->FileSize);
    printf("\nActual File size : %d", temp->FileActualSize);
    printf("\nLink Count : %d\n", temp->LinkCount);
    printf("Reference count : %d\n", temp->ReferenceCount);

    if (temp->permission == 1)
        printf("File permission : Read only\n");
    else if (temp->permission == 2)
        printf("File permssion : Write\n");
    else if (temp->permission == 3)
        printf("File permission : Read & Write\n");
    printf("-------------------------------------------------------------\n\n");

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//
//  Function name :     stat_file
//  Input :             char *name
//  Output :            int
//  Description :       DDisplay information of file
//  TimeComplexity :    O(1)
//
//////////////////////////////////////////////////////////////////////////
/*
    Algorithm
    START
        Truncate the file
    STOP
*/
int truncate_File(char *name)
{
    int fd = GetFDFromName(name);
    if (fd == -1)
        return -1;

    memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer, 0, 1024);
    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;
    UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;

    return 0;
}

// Entry point function
int main()
{
    // Local variable
    char *ptr = NULL;
    int ret = 0, fd = 0, count = 0;
    char command[4][80];
    char str[80], arr[1024];

    InitialiseSuperBlcok();
    CreateDILB();

    // Shell : Infinite listning loop
    while (1)
    {
        fflush(stdin);
        strcpy(str, "");
        printf("\nMarvellous VFS : > ");

        fgets(str, 80, stdin); // scanf("%[^'\n']s", str);

        count = sscanf(str, "%s %s %s %s", command[0], command[1], command[2], command[3]);

        if (count == 1)
        {
            if (strcmp(command[0], "ls") == 0)
            {
                ls_file();
            }
            else if (strcmp(command[0], "closeall") == 0)
            {
                CloseAllFile();
                printf("All files closed successfully\n");
                continue;
            }
            else if (strcmp(command[0], "clear") == 0)
            {
                system("cls");
                continue;
            }
            else if (strcmp(command[0], "help") == 0)
            {
                DisplayHelp();
                continue;
            }
            else if (strcmp(command[0], "exit") == 0)
            {
                printf("Terminating the Marvellous Virtual File System\n");
                break;
            }
            else
            {
                printf("\nERROR : Command not found !!!\n");
                continue;
            }
        }
        else if (count == 2)
        {
            if (strcmp(command[0], "stat") == 0)
            {
                ret = stat_file(command[1]);
                if (ret == -1)
                    printf("EROOR : Incorrect paramters\n");
                if (ret == -2)
                    printf("ERROR : There is no such file\n");
                continue;
            }
            else if (strcmp(command[0], "fstat") == 0)
            {
                ret = fstat_file(atoi(command[1]));
                if (ret == -1)
                    printf("ERROR : Incorrect paramters\n");
                if (ret == -2)
                    printf("ERROR : There is no such file\n");
                continue;
            }
            else if (strcmp(command[0], "close") == 0)
            {
                ret = CloseFileByName(command[1]);
                if (ret == -1)
                    printf("ERROR : There is no such file\n");
                continue;
            }
            else if (strcmp(command[0], "close") == 0)
            {
                ret = CloseFileByName(command[1]);
                if (ret == -1)
                    printf("ERROR : There is no such file\n");
                continue;
            }
            else if (strcmp(command[0], "rm") == 0)
            {
                ret = rm_File(command[1]);
                if (ret == -1)
                    printf("ERROR : There is no such file\n");
                continue;
            }
            else if (strcmp(command[0], "man") == 0)
            {
                man(command[1]);
            }
            else if (strcmp(command[0], "write") == 0)
            {
                fd = GetFDFromName(command[1]);
                if (fd == -1)
                {
                    printf("Error : Incorrect paramter\n");
                    continue;
                }
                printf("Enter the data : \n");
                scanf("%[^\n]", arr);

                ret = strlen(arr);
                if (ret == 0)
                {
                    printf("Error : Incorrect paramter\n");
                    continue;
                }
                ret = WriteFile(fd, arr, ret);
                if (ret == -1)
                    printf("ERROR : Permission denied\n");
                if (ret == -2)
                    printf("ERROR : There is no sufficient memory to write\n");
                if (ret == -3)
                    printf("ERROR : It is not a regular file\n");
            }
            else if (strcmp(command[0], "truncate") == 0)
            {
                ret = truncate_File(command[1]);
                if (ret == -1)
                    printf("Error : Incorrect parameter\n");
            }
            else
            {
                printf("\nERROR : Command not found !!!\n");
                continue;
            }
        }
        else if (count == 3)
        {
            if (strcmp(command[0], "create") == 0)
            {
                ret = CreateFile(command[1], atoi(command[2]));
                if (ret >= 0)
                    printf("File is successfully created with file descriptor : %d\n", ret);
                if (ret == -1)
                    printf("ERROR : Incorrect paramters\n");
                if (ret == -2)
                    printf("ERROR : There is no inodes\n");
                if (ret == -3)
                    printf("ERROR : File already exists\n");
                if (ret == -4)
                    printf("ERROR : Memory allocation falilure\n");
                continue;
            }
            else if (strcmp(command[0], "open") == 0)
            {
                ret = OpenFile(command[1], atoi(command[2]));
                if (ret >= 0)
                    printf("File is successfully opend with file descriptor : %d\n", ret);
                if (ret == -1)
                    printf("ERROR : Incorrect parameters\n");
                if (ret == -2)
                    printf("ERROR : File not present\n");
                if (ret == -3)
                    printf("ERROR : Permission denied\n");
                continue;
            }
            else if (strcmp(command[0], "read") == 0)
            {
                fd = GetFDFromName(command[1]);
                if (fd == -1)
                {
                    printf("Error : Incorrect paramter\n");
                    continue;
                }
                ptr = (char *)malloc(sizeof(atoi(command[2])) + 1);
                if (ptr == NULL)
                {
                    printf("Error : Memory allocation failure\n");
                    continue;
                }
                ret = ReadFile(fd, ptr, atoi(command[2]));
                if (ret == -1)
                    printf("ERROR : File not existing\n");
                if (ret == -2)
                    printf("ERROR : Permission denied\n");
                if (ret == -3)
                    printf("ERROR : Reached at end of file\n");
                if (ret == -4)
                    printf("ERROR : It is not regular file\n");
                if (ret == 0)
                    printf("ERROR : File empty\n");
                if (ret > 0)
                {
                    write(2, ptr, ret);
                }
                continue;
            }
            else
            {
                printf("\nERROR : Command not found !!!\n");
                continue;
            }
        }
        else if (count == 4)
        {
            if (strcmp(command[0], "lseek") == 0)
            {
                fd = GetFDFromName(command[1]);
                if (fd == -1)
                {
                    printf("Error : Incorrect paramter\n");
                    continue;
                }
                ret = LseekFile(fd, atoi(command[2]), atoi(command[3]));
                if (ret == -1)
                {
                    printf("ERROR : Unable to perform lseek\n");
                }
            }
            else
            {
                printf("\nERROR : command not found !!!\n");
                continue;
            }
        }
        else
        {
            printf("\nERROR : Command not found !!!\n");
            continue;
        }
    }

    return 0;
}
