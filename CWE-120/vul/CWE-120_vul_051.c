int ossl_punycode_decode(const char *pEncoded, const size_t enc_len,
                         unsigned int *pDecoded, unsigned int *pout_length)
{
    unsigned int n = initial_n;
    unsigned int i = 0;
    unsigned int bias = initial_bias;
    size_t processed_in = 0, written_out = 0;
    unsigned int max_out = *pout_length;

    unsigned int basic_count = 0;
    unsigned int loop;

    for (loop = 0; loop < enc_len; loop++) {
        if (pEncoded[loop] == delimiter)
            basic_count = loop;
    }

    if (basic_count > 0) {
        if (basic_count > max_out)
            return 0;

        for (loop = 0; loop < basic_count; loop++) {
            if (is_basic(pEncoded[loop]) == 0)
                return 0;

            pDecoded[loop] = pEncoded[loop];
            written_out++;
        }
        processed_in = basic_count + 1;
    }

    for (loop = processed_in; loop < enc_len;) {
        unsigned int oldi = i;
        unsigned int w = 1;
        unsigned int k, t;
        int digit;

        for (k = base;; k += base) {
            if (loop >= enc_len)
                return 0;

            digit = digit_decoded(pEncoded[loop]);
            loop++;

            if (digit < 0)
                return 0;
            if ((unsigned int)digit > (maxint - i) / w)
                return 0;

            i = i + digit * w;
            t = (k <= bias) ? tmin : (k >= bias + tmax) ? tmax : k - bias;

            if ((unsigned int)digit < t)
                break;

            if (w > maxint / (base - t))
                return 0;
            w = w * (base - t);
        }

        bias = adapt(i - oldi, written_out + 1, (oldi == 0));
        if (i / (written_out + 1) > maxint - n)
            return 0;
        n = n + i / (written_out + 1);
        i %= (written_out + 1);

        if (written_out >= max_out)
            return 0;

        memmove(pDecoded + i + 1, pDecoded + i,
                (written_out - i) * sizeof *pDecoded);
        pDecoded[i] = n;
        i++;
        written_out++;
    }

    *pout_length = written_out;
    return 1;
}