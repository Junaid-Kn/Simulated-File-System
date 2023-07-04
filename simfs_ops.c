/* This file contains functions that are not part of the visible interface.
 * So they are essentially helper functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simfs.h"

/* Internal helper functions first.
 */

FILE *
openfs(char *filename, char *mode)
{
    FILE *fp;
    if ((fp = fopen(filename, mode)) == NULL)
    {
        perror("openfs");
        exit(1);
    }
    return fp;
}

void closefs(FILE *fp)
{
    if (fclose(fp) != 0)
    {
        perror("closefs");
        exit(1);
    }
}

/* File system operations: creating, deleting, reading from, and writing to files.
 */

// Signatures omitted; design as you wish.
void createfile(char *sim_file, char *arg)
{
    // read the data from the file
    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];

    int i;

    FILE *fp = openfs(arg, "r");

    if ((fread(files, sizeof(fentry), MAXFILES, fp)) == 0)
    {
        fprintf(stderr, "Error: could not read file entries\n");
        closefs(fp);
        exit(1);
    }
    if ((fread(fnodes, sizeof(fnode), MAXBLOCKS, fp)) == 0)
    {
        fprintf(stderr, "Error: could not read fnodes\n");
        closefs(fp);
        exit(1);
    }
    int bytes_used = sizeof(fentry) * MAXFILES + sizeof(fnode) * MAXBLOCKS;
    int bytes_to_read = (BLOCKSIZE - (bytes_used % BLOCKSIZE)) % BLOCKSIZE;
    if (bytes_to_read != 0 && fseek(fp, bytes_to_read, SEEK_CUR) != 0)
    {
        fprintf(stderr, "Error: seek failed during print\n");
        closefs(fp);
        exit(1);
    }
    closefs(fp);
    // write the data to the file
    for (int k = 0; k < MAXFILES; k++)
    {
        if (strcmp(files[k].name, sim_file) == 0)
        {
            fprintf(stderr, "file name already exists\n");
            exit(1);
        }
    }

    int m = 0;
    while (m < MAXFILES)
    {
        if (files[m].name[0] != '\0')
        {
            m++;
        }
        else
        {
            break;
        }
    }
    if (m == MAXFILES)
    {
        fprintf(stderr, "Error: no room for name\n");

        exit(1);
    }

    for (i = 0; i < MAXFILES; i++)
    {

        if (files[i].name[0] == '\0')
        {
            if (strlen(sim_file) > 11)
            {
                fprintf(stderr, "Error: could not insert name\n");

                exit(1);
            }

            else
            {
                strncpy(files[i].name, sim_file, strlen(sim_file));
                files[i].name[strlen(sim_file)] = 0;
                break;
            }
        }
    }
    // int no = get_no_free_blocks(fnodes);
    // printf("%d\n", no);

    /* Write the metadata to the file. */
    fp = openfs(arg, "r+");

    if (fwrite(files, sizeof(fentry), MAXFILES, fp) < MAXFILES)
    {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }

    if (fwrite(fnodes, sizeof(fnode), MAXBLOCKS, fp) < MAXBLOCKS)
    {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }
    closefs(fp);
}

void writefile(char *sim_file, char *start, char *length, char *arg)
{
    // user input
    int len = atoi(length);
    int beginning = atoi(start);

    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];

    int in;

    FILE *fp = openfs(arg, "r");

    if ((fread(files, sizeof(fentry), MAXFILES, fp)) == 0)
    {
        fprintf(stderr, "Error: could not read file entries\n");
        closefs(fp);
        exit(1);
    }
    if ((fread(fnodes, sizeof(fnode), MAXBLOCKS, fp)) == 0)
    {
        fprintf(stderr, "Error: could not read fnodes\n");
        closefs(fp);
        exit(1);
    }

    // get number of blocks required for the file

    for (in = 0; in < MAXBLOCKS; in++)
    {
        if (fnodes[in].blockindex < 0)
        {
            in = abs(fnodes[in].blockindex);
            break;
        }
    }
    // int first_fseek = i;
    int write_file_index;

    for (write_file_index = 0; write_file_index < MAXFILES; write_file_index++)
    {
        if (strcmp(files[write_file_index].name, sim_file) == 0)
        {
            break;
        }
    }
    if (write_file_index == MAXFILES)
    {
        fprintf(stderr, "file doesn't exist");
        exit(1);
    }

    // make the firstblock connected to the fnode
    // also first iteration vs other iteration implementation
    if (files[write_file_index].firstblock == -1)
    {
        files[write_file_index].firstblock = in;
        for (int m = 0; m < MAXBLOCKS; m++)
        {
            if (fnodes[m].blockindex < 0)
            {
                fnodes[m].blockindex = in;
                break;
            }
        }
        if ((len) > (get_no_free_blocks(fnodes) * BLOCKSIZE))
        {
            fprintf(stderr, "Error: memory exceeded\n");
            closefs(fp);
            exit(1);
        }
        files[write_file_index].size = len;
    }
    else
    {
        if (beginning > files[write_file_index].size)
        {
            fprintf(stderr, "start greater than file size\n");
            exit(1);
        }
        if ((beginning + len) > (get_no_free_blocks(fnodes) * BLOCKSIZE))
        {
            fprintf(stderr, "Error: memory exceeded\n");
            closefs(fp);
            exit(1);
        }
        if (beginning == files[write_file_index].size || ((beginning + len) > files[write_file_index].size))
        {
            files[write_file_index].size = beginning + len;
        }
    }

    int num_of_blocks;
    // files[write_file_index].size = beginning +  len;
    if (files[write_file_index].size % BLOCKSIZE != 0)
    {
        num_of_blocks = (files[write_file_index].size / BLOCKSIZE) + 1;
    }
    else
    {
        num_of_blocks = files[write_file_index].size / BLOCKSIZE;
    }

    closefs(fp);

    char buf[len + 1];
    char character;
    // char eof;
    int letter_count = 0;
    while (fread(&character, 1, 1, stdin) > 0)
    {

        buf[letter_count] = character;
        letter_count++;
        if (letter_count < len)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    buf[letter_count] = 0;

    if (letter_count < len)
    {
        fprintf(stderr, "not enough characters\n");
        exit(1);
    }

    int new = abs(files[write_file_index].firstblock);
    int origin[MAXBLOCKS];
    int lol = 0;
    while (fnodes[new].nextblock != -1)
    {
        origin[lol] = new;
        new = fnodes[new].nextblock;
        lol++;
    }
    // origin[lol] = new;

    // printf("%d\n", lol);
    // printf("%d\n", origin[lol]);
    // printf("%d\n", new);
    // long pos1;
    // pos1 = ftell(fp);
    // printf("pos1 is %ld bytes\n", pos1);
    // while(position < num_of_blocks ){
    //     if(position == num_of_blocks -1){
    //         break;
    //     }
    //     // printf("%d\n", new);
    //     new = abs(fnodes[new].nextblock);
    //     position ++;
    // }
    // printf("%d\n", new);
    int num_blocks_cpy = num_of_blocks;
    int prev;
    int next = new;
    // fix this loop
    // printf("%d\n", origin[1]);
    for (int i = 0; i < lol; i++)
    {
        fnodes[origin[lol]].blockindex = origin[lol];
        num_blocks_cpy--;
    }

    // for(int i = 0; i < num_blocks_cpy; i++){
    //     for(int j = 0; j < MAXBLOCKS; j++){
    //         if(fnodes[new].nextblock == -1){

    //         }
    //     }
    // }
    // printf("%d\n", num_blocks_cpy);
    if (num_of_blocks > 1)
    {
        while (num_blocks_cpy != 1)
        {
            for (int i = 0; i < MAXBLOCKS; i++)
            {
                if (fnodes[i].blockindex < 0)
                {
                    prev = next;
                    next = abs(fnodes[i].blockindex);
                    fnodes[i].blockindex = abs(fnodes[i].blockindex);
                    for (int j = 0; j < MAXBLOCKS; j++)
                    {
                        if (fnodes[j].blockindex == prev)
                        {
                            fnodes[j].nextblock = next;
                            break;
                        }
                    }
                    break;
                }
            }
            num_blocks_cpy -= 1;
        }
    }

    // long pos2;
    // pos2 = ftell(fp);
    // printf("pos2 is %ld bytes\n", pos2);

    // take the size and check if the correct amount of size is left

    fp = openfs(arg, "r+");

    if (fwrite(files, sizeof(fentry), MAXFILES, fp) < MAXFILES)
    {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }

    if (fwrite(fnodes, sizeof(fnode), MAXBLOCKS, fp) < MAXBLOCKS)
    {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }

    // fseek(fp, first_fseek * BLOCKSIZE, SEEK_SET);
    // get all the indicies of the fnodes that are associated with write_file_index

    int aids[MAXBLOCKS];
    int first_block = files[write_file_index].firstblock;
    fnode first_block_fnode = fnodes[first_block];
    int counter = 0;
    while (first_block_fnode.nextblock != -1)
    {
        aids[counter] = first_block_fnode.blockindex;
        first_block_fnode = fnodes[first_block_fnode.nextblock];
        counter++;
        // printf("%d\n", counter);
    }
    aids[counter] = first_block_fnode.blockindex;

    // for(int i= 0; i <= counter; i ++){
    //     printf("%d\n", aids[i]);
    // }
    // actually write the data into the blocks allocated for space.

    int last_block;
    if ((beginning + len) % BLOCKSIZE != 0)
    {
        last_block = ((beginning + len) / BLOCKSIZE) + 1;
    }
    else
    {
        last_block = (beginning + len) / BLOCKSIZE;
    }

    int block_transitioner = beginning % BLOCKSIZE;
    int block_transition = beginning;
    int tracker;
    // int buf_counter = 0;
    if (beginning == 0)
    {
        tracker = 0;
    }
    else
    {
        if ((beginning) % BLOCKSIZE != 0 && beginning != 0)
        {
            tracker = ((beginning) / BLOCKSIZE);
        }
        else
        {
            tracker = (beginning) / BLOCKSIZE;
        }
    }

    //   int stop_the_aids =0;

    // char c;
    // fseek(fp, beginning, SEEK_CUR);
    // long pos4;
    // pos4 = ftell(fp);
    // printf("pos4 is %ld bytes\n", pos4);
    // printf("tracker %d \n", tracker);
    while (tracker < last_block)
    {

        // write the data to the file using fwrite
        fseek(fp, aids[tracker] * BLOCKSIZE + block_transitioner, SEEK_SET);
        // long posl;
        // posl = ftell(fp);
        // printf("posl is %ld bytes\n", posl);
        // printf("aids is %d bytes\n", aids[tracker]);

        if (block_transition < beginning + len)
        {
            if (block_transitioner % BLOCKSIZE == 0 && beginning != 0 && block_transitioner != 0)
            {
                block_transitioner = 0;
                tracker++;
                continue;
            }
            if (fwrite(buf, 1, 1, fp) < 1)
            {
                fprintf(stderr, "Not working");
            };

            for (int i = 0; i < len; i++)
            {
                //  printf("%c", buf[i]);
                buf[i] = buf[i + 1];
            }
            //  fseek(fp, aids[tracker] * BLOCKSIZE, SEEK_SET);
            //  char block[BLOCKSIZE + 1];
            // while (fread(block, 1, BLOCKSIZE, fp) == BLOCKSIZE) {
            //     fwrite(block, BLOCKSIZE, 1, stdout);
            //     printf("\n");
            // }

            block_transition++;
            block_transitioner++;
        }
        else
        {
            // fwrite(buf, 1, 1, fp);
            break;
        }
    }
    // fwrite(buf, 1, 1, fp);

    // long pos3;
    // pos3 = ftell(fp);
    // printf("pos3 is %ld bytes\n", pos3);
    int new_bytes_to_write = (BLOCKSIZE - (files[write_file_index].size % BLOCKSIZE)) % BLOCKSIZE;
    fseek(fp, next * BLOCKSIZE + (files[write_file_index].size % BLOCKSIZE), SEEK_SET);
    // int bytes_used = sizeof(fentry) * MAXFILES + sizeof(fnode) * MAXBLOCKS;

    // int bytes_to_write = (BLOCKSIZE - (bytes_used % BLOCKSIZE)) % BLOCKSIZE;
    char zerobuf[BLOCKSIZE] = {0};
    if (new_bytes_to_write != 0 && fwrite(zerobuf, new_bytes_to_write, 1, fp) < 1)
    {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }

    closefs(fp);
}

int get_no_free_blocks(fnode *fnodes)
{
    int i;
    int count;
    for (i = 0; i < MAXBLOCKS; i++)
    {
        if (fnodes[i].blockindex < 0)
        {
            count++;
        }
    }
    return count;
}

void readfile(char *sim_file, char *start, char *length, char *arg)
{
    int len = atoi(length);
    int beginning = atoi(start);
    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];

    // int i;

    FILE *fp = openfs(arg, "r+");

    if ((fread(files, sizeof(fentry), MAXFILES, fp)) == 0)
    {
        fprintf(stderr, "Error: could not read file entries\n");
        closefs(fp);
        exit(1);
    }
    if ((fread(fnodes, sizeof(fnode), MAXBLOCKS, fp)) == 0)
    {
        fprintf(stderr, "Error: could not read fnodes\n");
        closefs(fp);
        exit(1);
    }

    int write_file_index;

    for (write_file_index = 0; write_file_index < MAXFILES; write_file_index++)
    {
        if (strcmp(files[write_file_index].name, sim_file) == 0)
        {
            break;
        }
    }

    if (write_file_index == MAXFILES)
    {
        fprintf(stderr, "file doesn't exist\n");
        exit(1);
    }
    if (beginning > files[write_file_index].size || beginning + len > files[write_file_index].size)
    {
        fprintf(stderr, "Error: could not readfile cuz input too large\n");
        closefs(fp);
        exit(1);
    }

    int aids[MAXBLOCKS];
    int first_block = files[write_file_index].firstblock;
    fnode first_block_fnode = fnodes[first_block];
    int counter = 0;
    while (first_block_fnode.nextblock != -1)
    {
        aids[counter] = first_block_fnode.blockindex;
        first_block_fnode = fnodes[first_block_fnode.nextblock];
        counter++;
        // printf("%d\n", counter);
    }
    aids[counter] = first_block_fnode.blockindex;

    // for(int i= 0; i <= counter; i ++){
    //     printf("%d\n", aids[i]);
    // }
    // actually write the data into the blocks allocated for space.

    int last_block;
    if ((beginning + len) % BLOCKSIZE != 0)
    {
        last_block = ((beginning + len) / BLOCKSIZE) + 1;
    }
    else
    {
        last_block = (beginning + len) / BLOCKSIZE;
    }

    int block_transitioner = beginning % BLOCKSIZE;
    int block_transition = beginning % BLOCKSIZE;
    int tracker;
    // int buf_counter = 0;
    if (beginning == 0)
    {
        tracker = 0;
    }
    else
    {
        if ((beginning) % BLOCKSIZE != 0 && beginning != 0)
        {
            tracker = ((beginning) / BLOCKSIZE);
        }
        else
        {
            tracker = (beginning) / BLOCKSIZE;
        }
    }

    char buf[MAXBLOCKS * BLOCKSIZE];

    // char c;
    // fseek(fp, beginning, SEEK_CUR);
    // long pos4;
    // pos4 = ftell(fp);
    // printf("pos4 is %ld bytes\n", pos4);
    // printf("tracker %d \n", tracker);
    while (tracker < last_block)
    {
        // write the data to the file using fwrite
        fseek(fp, aids[tracker] * BLOCKSIZE + block_transitioner, SEEK_SET);
        // long posl;
        // posl = ftell(fp);
        // printf("posl is %ld bytes\n", posl);
        // printf("aids is %d bytes\n", aids[tracker]);

        if (block_transition < beginning + len)
        {
            if (block_transitioner % BLOCKSIZE == 0 && beginning != 0 && block_transitioner != 0)
            {
                block_transitioner = 0;
                tracker++;
                continue;
            }
            if (fread(buf, 1, 1, fp) < 1)
            {
                fprintf(stderr, "Not working");
                closefs(fp);
                exit(1);
            };
            if (fwrite(buf, 1, 1, stdout) < 1)
            {
                fprintf(stderr, "Not  working");
                closefs(fp);
                exit(1);
            };
            for (int i = 0; i < len; i++)
            {
                buf[i] = buf[i + 1];
            }
            block_transition++;
            block_transitioner++;
        }
        else
        {
            break;
        }
    }
    closefs(fp);
}

void deletefile(char *sim_file, char *arg)
{
    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];

    // int i;

    FILE *fp = openfs(arg, "r");

    if ((fread(files, sizeof(fentry), MAXFILES, fp)) == 0)
    {
        fprintf(stderr, "Error: could not read file entries\n");
        closefs(fp);
        exit(1);
    }
    if ((fread(fnodes, sizeof(fnode), MAXBLOCKS, fp)) == 0)
    {
        fprintf(stderr, "Error: could not read fnodes\n");
        closefs(fp);
        exit(1);
    }

    int write_file_index;

    for (write_file_index = 0; write_file_index < MAXFILES; write_file_index++)
    {
        if (strcmp(files[write_file_index].name, sim_file) == 0)
        {
            break;
        }
    }
    if (write_file_index == MAXFILES)
    {
        fprintf(stderr, "file does not exist\n");

        exit(1);
    }
    closefs(fp);

    int aids[MAXBLOCKS];
    int first_block = files[write_file_index].firstblock;
    fnode first_block_fnode = fnodes[first_block];
    int counter = 0;
    while (first_block_fnode.nextblock != -1)
    {
        aids[counter] = first_block_fnode.blockindex;
        first_block_fnode = fnodes[first_block_fnode.nextblock];
        counter++;
    }
    aids[counter] = first_block_fnode.blockindex;

    files[write_file_index].size -= files[write_file_index].size;
    files[write_file_index].firstblock = -1;
    files[write_file_index].name[0] = '\0';

    int k = -1;
    int foo = 0;
    while (k < counter)
    {
        fnodes[aids[foo]].blockindex = -(fnodes[aids[foo]].blockindex);
        fnodes[aids[foo]].nextblock = -1;
        k++;
        foo++;
    }
    fp = openfs(arg, "r+");
    if (fwrite(files, sizeof(fentry), MAXFILES, fp) < MAXFILES)
    {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }

    if (fwrite(fnodes, sizeof(fnode), MAXBLOCKS, fp) < MAXBLOCKS)
    {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }

    int tracker = 0;
    int second = -1;
    // int buf_counter = 0;
    // int new_bytes_to_write = files[write_file_index].size - (BLOCKSIZE - (files[write_file_index].size % BLOCKSIZE)) % BLOCKSIZE;
    char zerobuf[BLOCKSIZE] = {0};
    while (second < counter)
    {
        fseek(fp, aids[tracker] * BLOCKSIZE, SEEK_SET);
        if (fwrite(zerobuf, BLOCKSIZE, 1, fp) < 1)
        {
            fprintf(stderr, "Error: write failed on init\n");
            closefs(fp);
            exit(1);
        }

        tracker++;
        second++;
    }

    closefs(fp);
}