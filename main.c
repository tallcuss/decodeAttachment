#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
//  main.c
//  readAttachment
//
//  Created by Ron Heald on 7/9/20.
//  Copyright © 2020 U3A. All rights reserved.
//

extern int decode64(FILE*,FILE*);
extern int errno;

int main(int argc, const char * argv[]) {

    // set working directory in Product->Scheme->Edit Scheme->Options
    char* filename = "m202008292129_testAttachments";
    FILE* pinFile;
    FILE* ptmpFile;
    FILE* poutFile;
    char line[1092];

    // open file for reading
    if ((pinFile = fopen(filename, "r")) == NULL)
    {
        int errnum = errno;
        fprintf(stderr, "Error opening file: %s\n", strerror(errnum));
        return errnum;
    }

    // search file for our encoded message
    while(fgets(line, sizeof(line), pinFile) != NULL)
        if (strstr(line, ".csv") != NULL)
            break;

    // jump six lines ahead to start of message
    for (int i = 0; i < 6; i++)
        fgets(line, sizeof(line), pinFile);

    // create tmp file for input to decode64()
    if ((ptmpFile = tmpfile()) == NULL)
    {
        int errnum = errno;
        fprintf(stderr, "Error opening tmp file: %s\n", strerror(errnum));
        return errnum;
    }

    // write tmp file with encoded line
    if (fwrite(line, 1, strlen(line), ptmpFile) != strlen(line))
    {
        int errnum = errno;
        fprintf(stderr, "Error writing tmp file: %s\n", strerror(errnum));
        return errnum;
    }

    // reset tmp file pointer
    if (fseek(ptmpFile, 0, SEEK_SET) != 0)
    {
        int errnum = errno;
        fprintf(stderr, "Error opening tmp file: %s\n", strerror(errnum));
        return errnum;
    }

    // create output file
    if ((poutFile = fopen("out.txt", "w")) == NULL)
    {
        int errnum = errno;
        fprintf(stderr, "Error opening output file: %s\n", strerror(errnum));
        return errnum;
    }

    // decode tmp file into output file
    decode64(ptmpFile, poutFile);

    // close all
    fclose(pinFile);
    fclose(ptmpFile);
    fclose(poutFile);

    return 0;
}

