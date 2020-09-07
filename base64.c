#include <stdio.h>
#include <string.h>
#include <time.h>

/*******************************************************************************
 *
 *			Encode/Decode to Base64
 *
 * Base64 is a group of binary-to-text encoding schemes that represent binary
 * data in an ASCII string format by translating it into a radix-64
 * representation. The term Base64 originates from a specific Multipurpose
 * Internet Mail Entensions (MIME) content transfer encoding.
 *
 * MIME is an Internet standard that extends the format of email messages to
 * support text in character sets other than ASCII, as well as attachments of
 * audio, video, images, and application programs.
 *
 * Each Base64 digit represents exactly 6 bits of data. Three 8-bit bytes 
 * (24 bits total) can therefore be represented by four 6-bit Base64 digits.
 * Wikipedia entries for MIME and Base64
 *
 *	07FEB2010	kds	Terminate each line with \n not \r\n
 *					to match encoding by Thunderbird
 *
 ******************************************************************************/

// MIME's Base64 implementation uses the ASCII character set
static char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                       "abcdefghijklmnopqrstuvwxyz"
                       "0123456789+/";

/* forward definitions */
static int compile64(unsigned char*,unsigned char*,int,FILE*);
#ifdef DIAG
static int hexcv(unsigned char*,unsigned char*);
#endif

/*******************************************************************************
 *
 * ???
 * Returns ???
 *
 ******************************************************************************/
int encode64(
    FILE* infd,			// input file to encode
    FILE* oufd)			// encoded output file
{
    int c;
    int charcount = 0;
    unsigned char iset[4];
    unsigned char oset[5];
    unsigned char i1;
    unsigned char i2;
    unsigned char i3;
    unsigned char i4;

    while ((c = fread(iset, 1, 3, infd)) > 0)
	{
        i1 = (iset[0] & 0xfc) >> 2;
        oset[0] = base64[i1];
        i2 = (iset[0] & 0x3) << 4;
        if (c > 1)
            {
            i2 |= (iset[1] & 0xf0) >> 4;
            i3  = (iset[1] & 0xf) << 2;
            oset[2] = base64[i3];
            }
        else oset[2] = '=';
        oset[1] = base64[i2];
        if (c > 2)
            {
            i3 |= (iset[2] & 0xc0) >> 6;
            i4 = (iset[2] & 0x3f);
            oset[2] = base64[i3];
            oset[3] = base64[i4];
            }
        else oset[3] = '=';
        oset[4] = 0;
        fprintf(oufd, "%s", oset);

#ifdef DIAG
        hexcv(iset, itxt);
        printf("%s<>%s<>%d %d %d %d\n", itxt, oset, i1, i2, i3, i4);
#endif

        // bump char count
        // if EOL output line terminator and reset char count
        charcount += strlen((char*)oset);
        if (charcount > 71)
            {
            fprintf(oufd, "\n");								/*09FEB2010*/
            charcount = 0;
            }
	}
    
    if (charcount)
        fprintf(oufd, "\n");								/*09FEB2010*/
    return (0);
}

/*******************************************************************************
 *
 * ???
 * Returns ???
 *
 ******************************************************************************/
int decode64(
    FILE* infd,			// input encoded file
    FILE* oufd)			// output decoded file
{
    int i;
    int bcount = 3;		// number of bytes in binary
    int ccount = 0;		// number of bytes in base64
    unsigned char oset[4];
    unsigned char iset[5];
    unsigned char xset[5];
    unsigned char cchar[2];

    cchar[1] = 0;

    // read next char from file
    while (fread(cchar, 1, 1, infd) > 0)
	{
        // skip if line feed or return
        if (*cchar == '\r' || *cchar == '\n')
            continue;
        
        if (*cchar == '=')
            {
            bcount--;
            continue;	/* pad found */
            }
        
        iset[ccount] = *cchar;
        for (i = 0; i < 64; i++)
            {
            if (base64[i] == (*cchar & 0xff))
                {
                xset[ccount] = i;
                break;
                }
            }

        if (i == 64)
            return (*cchar);

        ccount++;
        if (ccount > 3)
            {
            iset[4] = 0;
            compile64(xset, oset, bcount, oufd);
            
#ifdef DIAG
            hexcv(oset, itxt);
            printf("%s<>%s<>", itxt, iset);
            for (i = 0; i < 4; i++)
                printf("%d ", xset[i]);
            printf("\n");
#endif
            
            ccount = 0;
            }
	}

    if (ccount)
	{
        iset[ccount] = 0;
        compile64(xset, oset, bcount, oufd);
        
#ifdef DIAG
        hexcv(oset, itxt);
        printf("%s<>%s<>", itxt, iset);
        for (i = 0; i < ccount; i++)
            printf("%d ", xset[i]);
        printf("\n");
#endif
	}
    return (0);
}

/*******************************************************************************
 *
 * ???
 * Returns ???
 *
 ******************************************************************************/
int compile64(
    unsigned char* iset,
    unsigned char* oset,
    int bcount,
    FILE* oufd)
{
    *(oset+0) = *(iset+0) << 2;
    *(oset+0) |= (*(iset+1) & 0x30) >> 4;
    fwrite(oset+0, 1, 1, oufd);
    if (bcount == 1)
        return (0);

    *(oset+1) = (*(iset+1) & 0xf) << 4;
    *(oset+1) |= (*(iset+2)) >> 2;
    fwrite(oset+1, 1, 1, oufd);
    if (bcount == 2)
        return (0);
    
    *(oset+2) = (*(iset+2)) << 6;
    *(oset+2) |= *(iset+3);
    fwrite(oset+2, 1, 1, oufd);
    return (0);
}

#ifdef DIAG
/*******************************************************************************
 *
 * ???
 * Returns ???
 *
 ******************************************************************************/
int hexcv(
    unsigned char* iset,
    unsigned char* itxt)
{
    int i;
    int n1;
    int n2;
    unsigned char* sptr;
    static char	hextb[17] = "0123456789abcdef";

    sptr = itxt;
    for (i = 0; i < 3; i++)
	{
        n1 = *(iset+i) >> 4;
        n2 = *(iset+i) & 0xf;
        *(sptr++) = hextb[n1];
        *(sptr++) = hextb[n2];
        *(sptr++) = ' ';
	}
    *sptr = 0;
    return (0);
}
#endif
