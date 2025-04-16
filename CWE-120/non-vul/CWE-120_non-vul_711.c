static int LogL16Encode(TIFF *tif, uint8_t *bp, tmsize_t cc, uint16_t s)
{
    static const char module[] = "LogL16Encode";
    LogLuvState *sp = EncoderState(tif);
    int shft;
    tmsize_t i;
    tmsize_t j;
    tmsize_t npixels;
    uint8_t *op;
    int16_t *tp;
    int16_t b;
    tmsize_t occ;
    int rc = 0, mask;
    tmsize_t beg;

    (void)s;
    assert(s == 0);
    assert(sp != NULL);
    npixels = cc / sp->pixel_size;

    if (sp->user_datafmt == SGILOGDATAFMT_16BIT)
        tp = (int16_t *)bp;
    else
    {
        tp = (int16_t *)sp->tbuf;
        if (sp->tbuflen < npixels)
        {
            TIFFErrorExtR(tif, module, "Translation buffer too short");
            return (0);
        }
        (*sp->tfunc)(sp, bp, npixels);
    }
    /* compress each byte string */
    op = tif->tif_rawcp;
    occ = tif->tif_rawdatasize - tif->tif_rawcc;
    for (shft = 8; shft >= 0; shft -= 8)
    {
        for (i = 0; i < npixels; i += rc)
        {
            if (occ < 4)
            {
                tif->tif_rawcp = op;
                tif->tif_rawcc = tif->tif_rawdatasize - occ;
                if (!TIFFFlushData1(tif))
                    return (0);
                op = tif->tif_rawcp;
                occ = tif->tif_rawdatasize - tif->tif_rawcc;
            }
            mask = 0xff << shft; /* find next run */
            for (beg = i; beg < npixels; beg += rc)
            {
                b = (int16_t)(tp[beg] & mask);
                rc = 1;
                while (rc < 127 + 2 && beg + rc < npixels &&
                       (tp[beg + rc] & mask) == b)
                    rc++;
                if (rc >= MINRUN)
                    break; /* long enough */
            }
            if (beg - i > 1 && beg - i < MINRUN)
            {
                b = (int16_t)(tp[i] & mask); /*check short run */
                j = i + 1;
                while ((tp[j++] & mask) == b)
                    if (j == beg)
                    {
                        *op++ = (uint8_t)(128 - 2 + j - i);
                        *op++ = (uint8_t)(b >> shft);
                        occ -= 2;
                        i = beg;
                        break;
                    }
            }
            while (i < beg)
            { /* write out non-run */
                if ((j = beg - i) > 127)
                    j = 127;
                if (occ < j + 3)
                {
                    tif->tif_rawcp = op;
                    tif->tif_rawcc = tif->tif_rawdatasize - occ;
                    if (!TIFFFlushData1(tif))
                        return (0);
                    op = tif->tif_rawcp;
                    occ = tif->tif_rawdatasize - tif->tif_rawcc;
                }
                *op++ = (uint8_t)j;
                occ--;
                while (j--)
                {
                    *op++ = (uint8_t)(tp[i++] >> shft & 0xff);
                    occ--;
                }
            }
            if (rc >= MINRUN)
            { /* write out run */
                *op++ = (uint8_t)(128 - 2 + rc);
                *op++ = (uint8_t)(tp[beg] >> shft & 0xff);
                occ -= 2;
            }
            else
                rc = 0;
        }
    }
    tif->tif_rawcp = op;
    tif->tif_rawcc = tif->tif_rawdatasize - occ;

    return (1);
}