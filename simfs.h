#include <stdio.h>
#include "simfstypes.h"

/* File system operations */
void printfs(char *);
void initfs(char *);
void createfile(char *, char *);
void writefile(char *, char *, char *, char *);
int get_no_free_blocks(fnode *);
void readfile(char *, char *, char *, char *);
void deletefile(char *, char *);

/* Internal functions */
FILE *openfs(char *filename, char *mode);
void closefs(FILE *fp);
