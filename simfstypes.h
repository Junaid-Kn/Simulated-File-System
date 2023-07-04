typedef struct file_entry
{
    char name[12]; // An empty name means the fentry is not in use.
    unsigned short size;
    short firstblock; // A -1 indicates that no file blocks have been allocated.
} fentry;

typedef struct file_node
{
    short blockindex; // Negative value means this block is not in use.
    short nextblock;  // A -1 indicates there is no next block of data.
} fnode;

#define MAXFILES 8
#define MAXBLOCKS 32
#define BLOCKSIZE 128
