static int vorbis_parse_setup_hdr_residues(vorbis_context *vc)
{
    GetBitContext *gb = &vc->gb;
    unsigned i, j, k;

    vc->residue_count = get_bits(gb, 6)+1;
    vc->residues      = av_mallocz(vc->residue_count * sizeof(*vc->residues));

    av_dlog(NULL, " There are %d residues. \n", vc->residue_count);

    for (i = 0; i < vc->residue_count; ++i) {
        vorbis_residue *res_setup = &vc->residues[i];
        uint8_t cascade[64];
        unsigned high_bits, low_bits;

        res_setup->type = get_bits(gb, 16);

        av_dlog(NULL, " %u. residue type %d\n", i, res_setup->type);

        res_setup->begin          = get_bits(gb, 24);
        res_setup->end            = get_bits(gb, 24);
        res_setup->partition_size = get_bits(gb, 24) + 1;
        /* Validations to prevent a buffer overflow later. */
        if (res_setup->begin>res_setup->end ||
            res_setup->end > (res_setup->type == 2 ? vc->audio_channels : 1) * vc->blocksize[1] / 2 ||
            (res_setup->end-res_setup->begin) / res_setup->partition_size > V_MAX_PARTITIONS) {
            av_log(vc->avccontext, AV_LOG_ERROR,
                   "partition out of bounds: type, begin, end, size, blocksize: %"PRIu16", %"PRIu32", %"PRIu32", %u, %"PRIu32"\n",
                   res_setup->type, res_setup->begin, res_setup->end,
                   res_setup->partition_size, vc->blocksize[1] / 2);
            return AVERROR_INVALIDDATA;
        }

        res_setup->classifications = get_bits(gb, 6) + 1;
        GET_VALIDATED_INDEX(res_setup->classbook, 8, vc->codebook_count)

        res_setup->ptns_to_read =
            (res_setup->end - res_setup->begin) / res_setup->partition_size;
        res_setup->classifs = av_malloc(res_setup->ptns_to_read *
                                        vc->audio_channels *
                                        sizeof(*res_setup->classifs));
        if (!res_setup->classifs)
            return AVERROR(ENOMEM);

        av_dlog(NULL, "    begin %d end %d part.size %d classif.s %d classbook %d \n",
                res_setup->begin, res_setup->end, res_setup->partition_size,
                res_setup->classifications, res_setup->classbook);

        for (j = 0; j < res_setup->classifications; ++j) {
            high_bits = 0;
            low_bits  = get_bits(gb, 3);
            if (get_bits1(gb))
                high_bits = get_bits(gb, 5);
            cascade[j] = (high_bits << 3) + low_bits;

            av_dlog(NULL, "     %u class cascade depth: %d\n", j, ilog(cascade[j]));
        }

        res_setup->maxpass = 0;
        for (j = 0; j < res_setup->classifications; ++j) {
            for (k = 0; k < 8; ++k) {
                if (cascade[j]&(1 << k)) {
                    GET_VALIDATED_INDEX(res_setup->books[j][k], 8, vc->codebook_count)

                    av_dlog(NULL, "     %u class cascade depth %u book: %d\n",
                            j, k, res_setup->books[j][k]);

                    if (k>res_setup->maxpass)
                        res_setup->maxpass = k;
                } else {
                    res_setup->books[j][k] = -1;
                }
            }
        }
    }
    return 0;
}