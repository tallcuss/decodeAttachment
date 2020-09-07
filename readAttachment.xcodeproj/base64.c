/*

            Encode/Decode to base 64

    07FEB2010    kds    Terminate each line with \n not \r\n
                    to match encoding by Thunderbird

*/

encode64(infd, oufd)
FILE    *infd, *oufd;
{
    int     c, charcount, pad;
    unsigned char    iset[4], oset[5], i1, i2, i3, i4;
    unsigned char    itxt[28];
static    char    base64[] =      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789+/";

    charcount = 0;
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
        charcount += strlen(oset);
        if (charcount > 71)
        {
            fprintf(oufd, "\n");                                /*09FEB2010*/
            charcount = 0;
        }
    }

    if (charcount)
        fprintf(oufd, "\n");                                /*09FEB2010*/
    return (0);
}

decode64(infd, oufd)
FILE    *infd, *oufd;
{
    int     c, i;
    int    bcount,        /* Number of bytes in binary */
        ccount;        /* Number of bytes in base64 */
    unsigned char    oset[4], iset[5], xset[5], cchar[2], itxt[26];
static    char    base64[] =      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789+/";

    ccount = 0;
    bcount = 3;
    cchar[1] = 0;
    while (fread(cchar, 1, 1, infd) > 0)
    {
        if (*cchar == '\r' || *cchar == '\n')
            continue;
        if (*cchar == '=')
        {
            bcount--;
            continue;    /* pad found */
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

compile64(iset, oset, bcount, oufd)
unsigned char    *iset, *oset;
int    bcount;
FILE    *oufd;
{

    int    i, c;
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
hexcv(iset, itxt)
unsigned char    *iset, *itxt;
{
    int    i, n1, n2;
    static char    hextb[17] = "0123456789abcdef";
    unsigned char    *sptr;

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

