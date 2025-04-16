static int extractSeparateRegion(struct image_data *image,
                                 struct crop_mask *crop,
                                 unsigned char *read_buff,
                                 unsigned char *crop_buff, int region)
{
    int shift_width, prev_trailing_bits = 0;
    uint32_t bytes_per_sample, bytes_per_pixel;
    uint32_t src_rowsize, dst_rowsize;
    uint32_t row, first_row, last_row, first_col, last_col;
    uint32_t src_offset, dst_offset;
    uint32_t crop_width, crop_length, img_width /*, img_length */;
    uint16_t bps, spp;
    uint8_t *src, *dst;
    tsample_t count, sample = 0; /* Update to extract more or more samples */

    img_width = image->width;
    /* img_length = image->length; */
    bps = image->bps;
    spp = image->spp;
    count = spp;

    bytes_per_sample = (bps + 7) / 8;
    bytes_per_pixel = ((bps * spp) + 7) / 8;
    if ((bps % 8) == 0)
        shift_width = 0; /* Byte aligned data only */
    else
    {
        if (bytes_per_pixel < (bytes_per_sample + 1))
            shift_width = bytes_per_pixel;
        else
            shift_width = bytes_per_sample + 1;
    }

    /* rows, columns, width, length are expressed in pixels */
    first_row = crop->regionlist[region].y1;
    last_row = crop->regionlist[region].y2;
    first_col = crop->regionlist[region].x1;
    last_col = crop->regionlist[region].x2;

    crop_width = last_col - first_col + 1;
    crop_length = last_row - first_row + 1;

    crop->regionlist[region].width = crop_width;
    crop->regionlist[region].length = crop_length;
    crop->regionlist[region].buffptr = crop_buff;

    src = read_buff;
    dst = crop_buff;
    src_rowsize = ((img_width * bps * spp) + 7) / 8;
    dst_rowsize = (((crop_width * bps * spp) + 7) / 8);

    for (row = first_row; row <= last_row; row++)
    {
        src_offset = row * src_rowsize;
        dst_offset = (row - first_row) * dst_rowsize;
        src = read_buff + src_offset;
        dst = crop_buff + dst_offset;

        switch (shift_width)
        {
            case 0:
                if (extractContigSamplesBytes(src, dst, img_width, sample, spp,
                                              bps, count, first_col,
                                              last_col + 1))
                {
                    TIFFError("extractSeparateRegion",
                              "Unable to extract row %" PRIu32, row);
                    return (1);
                }
                break;
            case 1:
                if (bps == 1)
                {
                    if (extractContigSamplesShifted8bits(
                            src, dst, img_width, sample, spp, bps, count,
                            first_col, last_col + 1, prev_trailing_bits))
                    {
                        TIFFError("extractSeparateRegion",
                                  "Unable to extract row %" PRIu32, row);
                        return (1);
                    }
                    break;
                }
                else if (extractContigSamplesShifted16bits(
                             src, dst, img_width, sample, spp, bps, count,
                             first_col, last_col + 1, prev_trailing_bits))
                {
                    TIFFError("extractSeparateRegion",
                              "Unable to extract row %" PRIu32, row);
                    return (1);
                }
                break;
            case 2:
                if (extractContigSamplesShifted24bits(
                        src, dst, img_width, sample, spp, bps, count, first_col,
                        last_col + 1, prev_trailing_bits))
                {
                    TIFFError("extractSeparateRegion",
                              "Unable to extract row %" PRIu32, row);
                    return (1);
                }
                break;
            case 3:
            case 4:
            case 5:
                if (extractContigSamplesShifted32bits(
                        src, dst, img_width, sample, spp, bps, count, first_col,
                        last_col + 1, prev_trailing_bits))
                {
                    TIFFError("extractSeparateRegion",
                              "Unable to extract row %" PRIu32, row);
                    return (1);
                }
                break;
            default:
                TIFFError("extractSeparateRegion",
                          "Unsupported bit depth %" PRIu16, bps);
                return (1);
        }
    }

    return (0);
}