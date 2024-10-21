#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

// Constants and Macros
#define MAXINODE 50
#define READ 1
#define WRITE 2
#define MAXFILESIZE 2048
#define REGULAR 1
#define SPECIAL 2
#define START 0
#define CURRENT 1
#define END 2

// Struct Definitions

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
    int FileType;  // 1 = Regular, 2 = Special
    char *Buffer;
    int LinkCount;
    int ReferenceCount;
    int permission;  // 1 = Read, 2 = Write, 3 = Read+Write
    struct inode *next;
} INODE, *PINODE, **PPINODE;

typedef struct filetable
{
    int readoffset;
    int writeoffset;
    int count;
    int mode;  // 1 = Read, 2 = Write, 3 = Read+Write
    PINODE ptrinode;
} FILETABLE, *PFILETABLE;

typedef struct ufdt
{
    PFILETABLE ptrfiletable;
} UFDT;

// Global Variables
UFDT UFDTArr[50];
SUPERBLOCK SUPERBLOCKobj;
PINODE head = NULL;

// Function Prototypes
void man(char *name);
void DisplayHelp();
int GetFDFromName(char *name);
PINODE Get_Inode(char *name);
void CreateDILB();
void InitialiseSuperBlock();
int CreateFile(char *name, int permission);
int rm_File(char *name);
int ReadFile(int fd, char *arr, int isize);
int WriteFile(int fd, char *arr, int isize);
int OpenFile(char *name, int mode);
int CloseFileByName(char *name);
void CloseAllFile();
int LseekFile(int fd, int size, int from);
void ls_file();
void fstat_file(int fd);
void stat_file(char *name);

// Helper Functions

void man(char *name)
{
    if (name == NULL) return;

    if (strcmp(name, "create") == 0)
    {
        printf("Description: Used to create a new file\nUsage: create File_name Permission\n");
    }
    else if (strcmp(name, "read") == 0)
    {
        printf("Description: Used to read data from file\nUsage: read File_name No_of_Bytes\n");
    }
    else if (strcmp(name, "write") == 0)
    {
        printf("Description: Used to write data into file\nUsage: write File_name\n");
    }
    else if (strcmp(name, "ls") == 0)
    {
        printf("Description: Used to list all files\nUsage: ls\n");
    }
    else if (strcmp(name, "stat") == 0)
    {
        printf("Description: Used to display information of file\nUsage: stat File_name\n");
    }
    else if (strcmp(name, "fstat") == 0)
    {
        printf("Description: Used to display information of file descriptor\nUsage: fstat File_Descriptor\n");
    }
    else if (strcmp(name, "truncate") == 0)
    {
        printf("Description: Used to remove data from file\nUsage: truncate File_name\n");
    }
    else if (strcmp(name, "open") == 0)
    {
        printf("Description: Used to open existing file\nUsage: open File_name mode\n");
    }
    else if (strcmp(name, "close") == 0)
    {
        printf("Description: Used to close opened file\nUsage: close File_name\n");
    }
    else if (strcmp(name, "lseek") == 0)
    {
        printf("Description: Used to change file offset\nUsage: lseek File_name ChangeInOffset StartPoint\n");
    }
    else
    {
        printf("ERROR: No manual entry available\n");
    }
}

void DisplayHelp()
{
    printf("ls: To list all files\n");
    printf("create: To create a new file\n");
    printf("read: To read data from a file\n");
    printf("write: To write data into a file\n");
    printf("open: To open a file\n");
    printf("close: To close an opened file\n");
    printf("stat: To display information of a file\n");
    printf("fstat: To display information of file descriptor\n");
    printf("truncate: To truncate file data\n");
    printf("rm: To delete a file\n");
}

int GetFDFromName(char *name)
{
    int i = 0;
    while (i < 50)
    {
        if (UFDTArr[i].ptrfiletable != NULL)
        {
            if (strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName), name) == 0)
                break;
        }
        i++;
    }
    if (i == 50) return -1;
    else return i;
}

PINODE Get_Inode(char *name)
{
    PINODE temp = head;
    while (temp != NULL)
    {
        if (strcmp(name, temp->FileName) == 0)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

void CreateDILB()
{
    int i = 1;
    PINODE newn = NULL;
    PINODE temp = head;

    while (i <= MAXINODE)
    {
        newn = (PINODE)malloc(sizeof(INODE));

        newn->InodeNumber = i;
        newn->FileSize = MAXFILESIZE;
        newn->FileActualSize = 0;
        newn->FileType = 0; // Not assigned
        newn->Buffer = NULL;
        newn->LinkCount = 0;
        newn->ReferenceCount = 0;
        newn->permission = 0;
        newn->next = NULL;

        if (head == NULL)
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

    printf("DILB created successfully\n");
}

void InitialiseSuperBlock()
{
    int i = 0;
    while (i < MAXINODE)
    {
        UFDTArr[i].ptrfiletable = NULL;
        i++;
    }
    SUPERBLOCKobj.TotalInodes = MAXINODE;
    SUPERBLOCKobj.FreeInode = MAXINODE;
}

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
        return -3;

    while (temp != NULL)
    {
        if (temp->FileType == 0)
            break;
        temp = temp->next;
    }

    while (i < 50)
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

int rm_File(char *name)
{
    int fd = GetFDFromName(name);
    if (fd == -1)
        return -1;

    (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

    if (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
    {
        UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;
        free(UFDTArr[fd].ptrfiletable->ptrinode->Buffer);
        free(UFDTArr[fd].ptrfiletable);
    }

    UFDTArr[fd].ptrfiletable = NULL;
    (SUPERBLOCKobj.FreeInode)++;

    return 0;
}

int ReadFile(int fd, char *arr, int isize)
{
    if (UFDTArr[fd].ptrfiletable == NULL)
        return -1;
    if (UFDTArr[fd].ptrfiletable->mode != READ && UFDTArr[fd].ptrfiletable->mode != READ + WRITE)
        return -2;
    if (UFDTArr[fd].ptrfiletable->ptrinode->permission != READ && UFDTArr[fd].ptrfiletable->ptrinode->permission != READ + WRITE)
        return -3;

    if (UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
        return -4;

    if (UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
        return -5;

    int read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFDTArr[fd].ptrfiletable->readoffset);
    if (read_size < isize)
    {
        strncpy(arr, (UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset), read_size);
        UFDTArr[fd].ptrfiletable->readoffset += read_size;
    }
    else
    {
        strncpy(arr, (UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset), isize);
        UFDTArr[fd].ptrfiletable->readoffset += isize;
    }

    return isize;
}

int WriteFile(int fd, char *arr, int isize)
{
    if (UFDTArr[fd].ptrfiletable->mode != WRITE && UFDTArr[fd].ptrfiletable->mode != READ + WRITE)
        return -1;
    if (UFDTArr[fd].ptrfiletable->ptrinode->permission != WRITE && UFDTArr[fd].ptrfiletable->ptrinode->permission != READ + WRITE)
        return -2;
    if (UFDTArr[fd].ptrfiletable->writeoffset == MAXFILESIZE)
        return -3;
    if (UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
        return -4;

    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->writeoffset), arr, isize);

    UFDTArr[fd].ptrfiletable->writeoffset += isize;
    UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize += isize;

    return isize;
}

int OpenFile(char *name, int mode)
{
    int i = 0;
    PINODE temp = NULL;

    if (name == NULL || mode <= 0)
        return -1;

    temp = Get_Inode(name);
    if (temp == NULL)
        return -2;

    if (temp->permission < mode)
        return -3;

    while (i < 50)
    {
        if (UFDTArr[i].ptrfiletable == NULL)
            break;
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

    if (UFDTArr[i].ptrfiletable == NULL)
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

int CloseFileByName(char *name)
{
    int i = GetFDFromName(name);
    if (i == -1)
        return -1;

    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;

    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;

    return 0;
}

void CloseAllFile()
{
    int i = 0;
    while (i < 50)
    {
        if (UFDTArr[i].ptrfiletable != NULL)
        {
            UFDTArr[i].ptrfiletable->readoffset = 0;
            UFDTArr[i].ptrfiletable->writeoffset = 0;
            (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
        }
        i++;
    }
}

int LseekFile(int fd, int size, int from)
{
    if (fd < 0 || from > 2)
        return -1;
    if (UFDTArr[fd].ptrfiletable == NULL)
        return -1;
    if (UFDTArr[fd].ptrfiletable->mode == READ || UFDTArr[fd].ptrfiletable->mode == READ + WRITE)
    {
        if (from == START)
        {
            if (size > UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize || size < 0)
                return -1;
            UFDTArr[fd].ptrfiletable->readoffset = size;
        }
        else if (from == CURRENT)
        {
            if ((UFDTArr[fd].ptrfiletable->readoffset + size) > UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize || (UFDTArr[fd].ptrfiletable->readoffset + size) < 0)
                return -1;
            UFDTArr[fd].ptrfiletable->readoffset += size;
        }
        else if (from == END)
        {
            if ((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize + size) > UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize || (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize + size) < 0)
                return -1;
            UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize + size;
        }
    }
    else if (UFDTArr[fd].ptrfiletable->mode == WRITE)
    {
        if (from == START)
        {
            if (size > MAXFILESIZE || size < 0)
                return -1;
            UFDTArr[fd].ptrfiletable->writeoffset = size;
        }
        else if (from == CURRENT)
        {
            if ((UFDTArr[fd].ptrfiletable->writeoffset + size) > MAXFILESIZE || (UFDTArr[fd].ptrfiletable->writeoffset + size) < 0)
                return -1;
            UFDTArr[fd].ptrfiletable->writeoffset += size;
        }
        else if (from == END)
        {
            if ((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize + size) > MAXFILESIZE || (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize + size) < 0)
                return -1;
            UFDTArr[fd].ptrfiletable->writeoffset = UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize + size;
        }
    }
    return 0;
}

void ls_file()
{
    PINODE temp = head;

    if (SUPERBLOCKobj.FreeInode == MAXINODE)
    {
        printf("Error: There are no files\n");
        return;
    }

    printf("\nFile Name\tInode Number\tFile Size\tLink Count\n");
    printf("--------------------------------------------------------\n");

    while (temp != NULL)
    {
        if (temp->FileType != 0)
        {
            printf("%s\t\t%d\t\t%d\t\t%d\n", temp->FileName, temp->InodeNumber, temp->FileActualSize, temp->LinkCount);
        }
        temp = temp->next;
    }
}

void fstat_file(int fd)
{
    if (fd < 0 || UFDTArr[fd].ptrfiletable == NULL)
    {
        printf("Error: Invalid file descriptor\n");
        return;
    }

    PINODE temp = UFDTArr[fd].ptrfiletable->ptrinode;

    printf("\nFile Name: %s\n", temp->FileName);
    printf("Inode Number: %d\n", temp->InodeNumber);
    printf("File Size: %d\n", temp->FileSize);
    printf("Actual File Size: %d\n", temp->FileActualSize);
    printf("Link Count: %d\n", temp->LinkCount);
    printf("Reference Count: %d\n", temp->ReferenceCount);
    printf("File Permission: %d\n", temp->permission);
}

void stat_file(char *name)
{
    PINODE temp = Get_Inode(name);

    if (temp == NULL)
    {
        printf("Error: File not found\n");
        return;
    }

    printf("\nFile Name: %s\n", temp->FileName);
    printf("Inode Number: %d\n", temp->InodeNumber);
    printf("File Size: %d\n", temp->FileSize);
    printf("Actual File Size: %d\n", temp->FileActualSize);
    printf("Link Count: %d\n", temp->LinkCount);
    printf("Reference Count: %d\n", temp->ReferenceCount);
    printf("File Permission: %d\n", temp->permission);
}

// Main Function
int main()
{
    char command[4][80], str[80], arr[MAXFILESIZE];
    int ret = 0, fd = 0, count = 0;

    InitialiseSuperBlock();
    CreateDILB();

    while (1)
    {
        fflush(stdin);
        strcpy(str, "");

        printf("\nVFS: > ");
        fgets(str, 80, stdin);

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
            }
            else if (strcmp(command[0], "clear") == 0)
            {
                system("clear");  // Use "cls" if on Windows
            }
            else if (strcmp(command[0], "help") == 0)
            {
                DisplayHelp();
            }
            else if (strcmp(command[0], "exit") == 0)
            {
                printf("Terminating the Virtual File System\n");
                break;
            }
            else
            {
                printf("ERROR: Command not found!\n");
            }
        }
        else if (count == 2)
        {
            if (strcmp(command[0], "stat") == 0)
            {
                stat_file(command[1]);
            }
            else if (strcmp(command[0], "fstat") == 0)
            {
                fd = atoi(command[1]);
                fstat_file(fd);
            }
            else if (strcmp(command[0], "close") == 0)
            {
                ret = CloseFileByName(command[1]);
                if (ret == 0)
                    printf("File closed successfully\n");
                else
                    printf("Error: Unable to close file\n");
            }
            else if (strcmp(command[0], "rm") == 0)
            {
                ret = rm_File(command[1]);
                if (ret == 0)
                    printf("File deleted successfully\n");
                else
                    printf("Error: Unable to delete file\n");
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
                    printf("Error: Incorrect parameter\n");
                    continue;
                }
                printf("Enter the data: \n");
                fgets(arr, MAXFILESIZE, stdin);

                ret = WriteFile(fd, arr, strlen(arr) - 1);
                if (ret == -1)
                    printf("Permission denied\n");
                if (ret == -2)
                    printf("Error: There is no sufficient memory to write\n");
                if (ret == -3)
                    printf("Error: It is not a regular file\n");
            }
            else if (strcmp(command[0], "truncate") == 0)
            {
                ret = rm_File(command[1]);
                if (ret == 0)
                    printf("File truncated successfully\n");
            }
            else
            {
                printf("ERROR: Command not found!\n");
            }
        }
        else if (count == 3)
        {
            if (strcmp(command[0], "create") == 0)
            {
                ret = CreateFile(command[1], atoi(command[2]));
                if (ret >= 0)
                    printf("File created successfully\n");
                if (ret == -1)
                    printf("ERROR: Incorrect parameters\n");
                if (ret == -2)
                    printf("ERROR: There are no inodes\n");
                if (ret == -3)
                    printf("ERROR: File already exists\n");
                if (ret == -4)
                    printf("ERROR: Memory allocation failure\n");
                continue;
            }
            else if (strcmp(command[0], "open") == 0)
            {
                ret = OpenFile(command[1], atoi(command[2]));
                if (ret >= 0)
                    printf("File opened successfully with file descriptor: %d\n", ret);
                if (ret == -1)
                    printf("Error: Incorrect parameters\n");
                if (ret == -2)
                    printf("Error: File not present\n");
                if (ret == -3)
                    printf("Error: Permission denied\n");
            }
            else if (strcmp(command[0], "read") == 0)
            {
                fd = GetFDFromName(command[1]);
                if (fd == -1)
                {
                    printf("Error: File not found\n");
                    continue;
                }
                ret = ReadFile(fd, arr, atoi(command[2]));
                if (ret == -1)
                    printf("Error: File not existing\n");
                if (ret == -2)
                    printf("Error: Permission denied\n");
                if (ret == -3)
                    printf("Error: Reached at end of file\n");
                if (ret == -4)
                    printf("Error: It is not a regular file\n");
                if (ret > 0)
                {
                    write(2, arr, ret);
                }
            }
            else
            {
                printf("ERROR: Command not found!\n");
            }
        }
        else if (count == 4)
        {
            if (strcmp(command[0], "lseek") == 0)
            {
                fd = GetFDFromName(command[1]);
                if (fd == -1)
                {
                    printf("Error: Incorrect parameters\n");
                    continue;
                }
                ret = LseekFile(fd, atoi(command[2]), atoi(command[3]));
                if (ret == -1)
                    printf("Error: Unable to perform lseek\n");
            }
            else
            {
                printf("ERROR: Command not found!\n");
            }
        }
        else
        {
            printf("ERROR: Command not found!\n");
        }
    }

    return 0;
}
