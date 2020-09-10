#include <stdio.h>
#include <stdlib.h>
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
 * Each Base64 digit represents 6 bits of data. Three 8-bit octets (bytes),
 * 24 bits total, can therefore be represented by four 6-bit Base64 digits.
 * Wikipedia entries for MIME and Base64
 *
 * A useful website for encoding/decoding strings is www.base64decode.org.
 *
 *	07FEB2010	kds	Terminate each line with \n not \r\n
 *					to match encoding by Thunderbird
 *
 ******************************************************************************/

extern int errno;

// MIME's Base64 implementation uses the ASCII character set
static char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                       "abcdefghijklmnopqrstuvwxyz"
                       "0123456789+/";

/* forward definitions */
static void compile64(const unsigned char*,unsigned char*,const int);
#ifdef DIAG
static int hexcv(unsigned char*,unsigned char*);
#endif

/*******************************************************************************
 *
 * Encode binary input file writing result to output file.
 * Returns 0  (Should be void funtion!)
 *
 ******************************************************************************/
int encode64(
    FILE* infd,            // input file to encode
    FILE* outfd)           // encoded output file
{
    int charcount = 0;     // current number of chars in output line
    size_t count;
    unsigned char iset[4];
    unsigned char oset[5];

    while ((count = fread(iset, 1, 3, infd)) > 0)
        {
        if (count > 3)
            {
            int errnum = errno;
            fprintf(stderr, "Error reading file: %s\n", strerror(errnum));
            exit(errnum);
            }

        int i1 = (iset[0] & 0xfc) >> 2;
        oset[0] = base64[i1];

        int i2 = (iset[0] & 0x3) << 4;
        int i3 = 0;
        if (count > 1)
        {
            i2 |= (iset[1] & 0xf0) >> 4;
            i3 = (iset[1] & 0xf) << 2;
            oset[2] = base64[i3];
        }
        else
            oset[2] = '=';
        oset[1] = base64[i2];

        int i4 = 0;
        if (count > 2)
        {
            i3 |= (iset[2] & 0xc0) >> 6;
            i4 = (iset[2] & 0x3f);
            oset[2] = base64[i3];
            oset[3] = base64[i4];
            }
        else
            oset[3] = '=';

        oset[4] = 0;
        if (fwrite(oset, 1, strlen((const char*)oset), outfd)
                   != strlen((const char*)oset))
            {
            int errnum = errno;
            fprintf(stderr, "Error writing file: %s\n", strerror(errnum));
            exit(errnum);
            }

#ifdef DIAG
        unsigned char itxt[28];
        hexcv(iset, itxt);
        printf("%s<>%s<>%d %d %d %d\n", itxt, oset, i1, i2, i3, i4);
#endif

        // bump char count
        // if EOL output line terminator and reset char count
        charcount += strlen((const char*)oset);
        if (charcount > 71)
            {
            charcount = 0;
            if (fwrite("\n", 1, 1, outfd) != 1)
                {
                int errnum = errno;
                fprintf(stderr, "Error writing file: %s\n", strerror(errnum));
                exit(errnum);
                }
            }
	}

    // terminate last line
    if (charcount)
        if (fwrite("\n", 1, 1, outfd) != 1)
            {
            int errnum = errno;
            fprintf(stderr, "Error writing file: %s\n", strerror(errnum));
            exit(errnum);
            }
    return (0);
}

/*******************************************************************************
 *
 * Decode input base64 file writing results to output file.
 * Returns 0 normally, or bad char if found
 *
 ******************************************************************************/
int decode64(
    FILE* infd,          // input encoded file
    FILE* outfd)         // output decoded file
{
    int bcount = 3;      // number of bytes in binary
    int ccount = 0;      // number of bytes in base64
    unsigned char oset[4];
    unsigned char iset[5];
    unsigned char xset[5];
    unsigned char cchar[2];
    size_t freadcount;
    int i;

    cchar[1] = 0;

    // read next char from file
    while ((freadcount = fread(cchar, 1, 1, infd)) > 0)
	{
        if (freadcount > 1)
            {
            int errnum = errno;
            fprintf(stderr, "Error reading file: %s\n", strerror(errnum));
            exit(errnum);
            }

        // skip if line feed or return
        if (*cchar == '\r' || *cchar == '\n')
            continue;

        // skip and dec octet count if pad char
        if (*cchar == '=')
            {
            bcount--;
            continue;	/* pad found */
            }

        // search for char in table
        iset[ccount] = *cchar;
        for (i = 0; i < 64; i++)
            {
            if (base64[i] == (*cchar & 0xff))
                {
                xset[ccount] = i;
                break;
                }
            }

        // check search failed
        if (i == 64)
            return (*cchar);        // bad char

        ccount++;
        if (ccount > 3)
            {
            iset[4] = 0;
            compile64(xset, oset, bcount);
            if (fwrite(oset, 1, 3, outfd) != 3)
                {
                int errnum = errno;
                fprintf(stderr, "Error writing tmp file: %s\n",
                    strerror(errnum));
                exit(errnum);
                }

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
        compile64(xset, oset, bcount);
        if (fwrite(oset, 1, bcount, outfd) != bcount)
            {
            int errnum = errno;
            fprintf(stderr, "Error writing tmp file: %s\n", strerror(errnum));
            exit(errnum);
            }

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
 * Pack octets with data from sextets.
 *
 ******************************************************************************/
void compile64(
    const unsigned char* iset,	// input sextets
    unsigned char* oset,        // output octets
    const int bcount)		// number of octets
{
    oset[0] = iset[0] << 2 | (iset[1] & 0x30) >> 4;
    if (bcount == 1)
        return;

    oset[1] = (iset[1] & 0xf) << 4 | (iset[2]) >> 2;
    if (bcount == 2)
        return;
    
    oset[2] = iset[2] << 6 | iset[3];
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
