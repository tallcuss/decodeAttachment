#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"

//
//  main.c
//  decodeAttachment
//
//  Created by Ron Heald on 7/9/20.
//  Copyright © 2020 U3A. All rights reserved.
//

int main(int argc, const char * argv[]) {

    // input file containing email with attachments
    // for Xcode set working directory in Product->Scheme->Edit Scheme->Options
    char* filename = "m202008292129_testAttachments";
    FILE* pinFile;
    FILE* ptmpFile;
    FILE* poutFile;
    char line[1092];

    // open file for reading
    if ((pinFile = fopen(filename, "r")) == NULL)
    {
        perror("fopen_failed");
        return(EXIT_FAILURE);
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
        perror("tmpfile_failed");
        return(EXIT_FAILURE);
    }

    // write tmp file with encoded line
    if (fwrite(line, 1, strlen(line), ptmpFile) != strlen(line))
    {
        perror("fwrite_failed");
        return(EXIT_FAILURE);
    }

    // reset tmp file pointer
    if (fseek(ptmpFile, 0, SEEK_SET) != 0)
    {
        perror("fseek_failed");
        return(EXIT_FAILURE);
    }

    // create output file
    if ((poutFile = fopen("out.txt", "w")) == NULL)
    {
        perror("fopen_failed");
        return(EXIT_FAILURE);
    }

    // decode tmp file into output file
    decode64(ptmpFile, poutFile);

    // close all
    fclose(pinFile);
    fclose(ptmpFile);
    fclose(poutFile);

    return 0;
}

