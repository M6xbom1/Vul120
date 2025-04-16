static int extractCompositeRegions(struct image_data *image,
                                   struct crop_mask *crop,
                                   unsigned char *read_buff,
                                   unsigned char *crop_buff)
{
    int shift_width, bytes_per_sample, bytes_per_pixel;
    uint32_t i, trailing_bits, prev_trailing_bits;
    uint32_t row, first_row, last_row, first_col, last_col;
    uint32_t src_rowsize, dst_rowsize, src_offset, dst_offset;
    uint32_t crop_width, crop_length, img_width /*, img_length */;
    uint32_t prev_length, prev_width, composite_width;
    uint16_t bps, spp;
    uint8_t *src, *dst;
    tsample_t count, sample = 0; /* Update to extract one or more samples */

    img_width = image->width;
    /* img_length = image->length; */
    bps = image->bps;
    spp = image->spp;
    count = spp;

    bytes_per_sample = (bps + 7) / 8;
    bytes_per_pixel = ((bps * spp) + 7) / 8;
    if ((bps % 8) == 0)
        shift_width = 0;
    else
    {
        if (bytes_per_pixel < (bytes_per_sample + 1))
            shift_width = bytes_per_pixel;
        else
            shift_width = bytes_per_sample + 1;
    }
    src = read_buff;
    dst = crop_buff;

    /* These are setup for adding additional sections */
    prev_width = prev_length = 0;
    prev_trailing_bits = trailing_bits = 0;
    composite_width = crop->combined_width;
    crop->combined_width = 0;
    crop->combined_length = 0;

    /* If there is more than one region, check beforehand whether all the width
     * and length values of the regions are the same, respectively. */
    switch (crop->edge_ref)
    {
        default:
        case EDGE_TOP:
        case EDGE_BOTTOM:
            for (i = 1; i < crop->selections; i++)
            {
                uint32_t crop_width0 =
                    crop->regionlist[i - 1].x2 - crop->regionlist[i - 1].x1 + 1;
                uint32_t crop_width1 =
                    crop->regionlist[i].x2 - crop->regionlist[i].x1 + 1;
                if (crop_width0 != crop_width1)
                {
                    TIFFError("extractCompositeRegions",
                              "Only equal width regions can be combined for -E "
                              "top or bottom");
                    return (1);
                }
            }
            break;
        case EDGE_LEFT:
        case EDGE_RIGHT:
            for (i = 1; i < crop->selections; i++)
            {
                uint32_t crop_length0 =
                    crop->regionlist[i - 1].y2 - crop->regionlist[i - 1].y1 + 1;
                uint32_t crop_length1 =
                    crop->regionlist[i].y2 - crop->regionlist[i].y1 + 1;
                if (crop_length0 != crop_length1)
                {
                    TIFFError("extractCompositeRegions",
                              "Only equal length regions can be combined for "
                              "-E left or right");
                    return (1);
                }
            }
    }

    for (i = 0; i < crop->selections; i++)
    {
        /* rows, columns, width, length are expressed in pixels */
        first_row = crop->regionlist[i].y1;
        last_row = crop->regionlist[i].y2;
        first_col = crop->regionlist[i].x1;
        last_col = crop->regionlist[i].x2;

        crop_width = last_col - first_col + 1;
        crop_length = last_row - first_row + 1;

        /* These should not be needed for composite images */
        crop->regionlist[i].width = crop_width;
        crop->regionlist[i].length = crop_length;
        crop->regionlist[i].buffptr = crop_buff;

        src_rowsize = ((img_width * bps * spp) + 7) / 8;
        dst_rowsize = (((crop_width * bps * count) + 7) / 8);

        switch (crop->edge_ref)
        {
            default:
            case EDGE_TOP:
            case EDGE_BOTTOM:
                if ((crop->selections > i + 1) &&
                    (crop_width != crop->regionlist[i + 1].width))
                {
                    TIFFError("extractCompositeRegions",
                              "Only equal width regions can be combined for -E "
                              "top or bottom");
                    return (1);
                }

                crop->combined_width = crop_width;
                crop->combined_length += crop_length;

                for (row = first_row; row <= last_row; row++)
                {
                    src_offset = row * src_rowsize;
                    dst_offset = (row - first_row) * dst_rowsize;
                    src = read_buff + src_offset;
                    dst = crop_buff + dst_offset + (prev_length * dst_rowsize);
                    switch (shift_width)
                    {
                        case 0:
                            if (extractContigSamplesBytes(
                                    src, dst, img_width, sample, spp, bps,
                                    count, first_col, last_col + 1))
                            {
                                TIFFError("extractCompositeRegions",
                                          "Unable to extract row %" PRIu32,
                                          row);
                                return (1);
                            }
                            break;
                        case 1:
                            if (bps == 1)
                            {
                                if (extractContigSamplesShifted8bits(
                                        src, dst, img_width, sample, spp, bps,
                                        count, first_col, last_col + 1,
                                        prev_trailing_bits))
                                {
                                    TIFFError("extractCompositeRegions",
                                              "Unable to extract row %" PRIu32,
                                              row);
                                    return (1);
                                }
                                break;
                            }
                            else if (extractContigSamplesShifted16bits(
                                         src, dst, img_width, sample, spp, bps,
                                         count, first_col, last_col + 1,
                                         prev_trailing_bits))
                            {
                                TIFFError("extractCompositeRegions",
                                          "Unable to extract row %" PRIu32,
                                          row);
                                return (1);
                            }
                            break;
                        case 2:
                            if (extractContigSamplesShifted24bits(
                                    src, dst, img_width, sample, spp, bps,
                                    count, first_col, last_col + 1,
                                    prev_trailing_bits))
                            {
                                TIFFError("extractCompositeRegions",
                                          "Unable to extract row %" PRIu32,
                                          row);
                                return (1);
                            }
                            break;
                        case 3:
                        case 4:
                        case 5:
                            if (extractContigSamplesShifted32bits(
                                    src, dst, img_width, sample, spp, bps,
                                    count, first_col, last_col + 1,
                                    prev_trailing_bits))
                            {
                                TIFFError("extractCompositeRegions",
                                          "Unable to extract row %" PRIu32,
                                          row);
                                return (1);
                            }
                            break;
                        default:
                            TIFFError("extractCompositeRegions",
                                      "Unsupported bit depth %" PRIu16, bps);
                            return (1);
                    }
                }
                prev_length += crop_length;
                break;
            case EDGE_LEFT: /* splice the pieces of each row together, side by
                               side */
            case EDGE_RIGHT:
                if ((crop->selections > i + 1) &&
                    (crop_length != crop->regionlist[i + 1].length))
                {
                    TIFFError("extractCompositeRegions",
                              "Only equal length regions can be combined for "
                              "-E left or right");
                    return (1);
                }
                crop->combined_width += crop_width;
                crop->combined_length = crop_length;
                dst_rowsize = (((composite_width * bps * count) + 7) / 8);
                trailing_bits = (crop_width * bps * count) % 8;
                for (row = first_row; row <= last_row; row++)
                {
                    src_offset = row * src_rowsize;
                    dst_offset = (row - first_row) * dst_rowsize;
                    src = read_buff + src_offset;
                    dst = crop_buff + dst_offset + prev_width;

                    switch (shift_width)
                    {
                        case 0:
                            if (extractContigSamplesBytes(
                                    src, dst, img_width, sample, spp, bps,
                                    count, first_col, last_col + 1))
                            {
                                TIFFError("extractCompositeRegions",
                                          "Unable to extract row %" PRIu32,
                                          row);
                                return (1);
                            }
                            break;
                        case 1:
                            if (bps == 1)
                            {
                                if (extractContigSamplesShifted8bits(
                                        src, dst, img_width, sample, spp, bps,
                                        count, first_col, last_col + 1,
                                        prev_trailing_bits))
                                {
                                    TIFFError("extractCompositeRegions",
                                              "Unable to extract row %" PRIu32,
                                              row);
                                    return (1);
                                }
                                break;
                            }
                            else if (extractContigSamplesShifted16bits(
                                         src, dst, img_width, sample, spp, bps,
                                         count, first_col, last_col + 1,
                                         prev_trailing_bits))
                            {
                                TIFFError("extractCompositeRegions",
                                          "Unable to extract row %" PRIu32,
                                          row);
                                return (1);
                            }
                            break;
                        case 2:
                            if (extractContigSamplesShifted24bits(
                                    src, dst, img_width, sample, spp, bps,
                                    count, first_col, last_col + 1,
                                    prev_trailing_bits))
                            {
                                TIFFError("extractCompositeRegions",
                                          "Unable to extract row %" PRIu32,
                                          row);
                                return (1);
                            }
                            break;
                        case 3:
                        case 4:
                        case 5:
                            if (extractContigSamplesShifted32bits(
                                    src, dst, img_width, sample, spp, bps,
                                    count, first_col, last_col + 1,
                                    prev_trailing_bits))
                            {
                                TIFFError("extractCompositeRegions",
                                          "Unable to extract row %" PRIu32,
                                          row);
                                return (1);
                            }
                            break;
                        default:
                            TIFFError("extractCompositeRegions",
                                      "Unsupported bit depth %" PRIu16, bps);
                            return (1);
                    }
                }
                prev_width += (crop_width * bps * count) / 8;
                prev_trailing_bits += trailing_bits;
                if (prev_trailing_bits > 7)
                    prev_trailing_bits -= 8;
                break;
        }
    }
    if (crop->combined_width != composite_width)
        TIFFError("combineSeparateRegions",
                  "Combined width does not match composite width");

    return (0);
}