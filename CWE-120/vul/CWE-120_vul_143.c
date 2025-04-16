static void initCropMasks(struct crop_mask *cps)
{
    int i;

    cps->crop_mode = CROP_NONE;
    cps->res_unit = RESUNIT_NONE;
    cps->edge_ref = EDGE_TOP;
    cps->width = 0;
    cps->length = 0;
    for (i = 0; i < 4; i++)
        cps->margins[i] = 0.0;
    cps->bufftotal = (uint32_t)0;
    cps->combined_width = (uint32_t)0;
    cps->combined_length = (uint32_t)0;
    cps->rotation = (uint16_t)0;
    cps->photometric = INVERT_DATA_AND_TAG;
    cps->mirror = (uint16_t)0;
    cps->invert = (uint16_t)0;
    cps->zones = (uint32_t)0;
    cps->regions = (uint32_t)0;
    for (i = 0; i < MAX_REGIONS; i++)
    {
        cps->corners[i].X1 = 0.0;
        cps->corners[i].X2 = 0.0;
        cps->corners[i].Y1 = 0.0;
        cps->corners[i].Y2 = 0.0;
        cps->regionlist[i].x1 = 0;
        cps->regionlist[i].x2 = 0;
        cps->regionlist[i].y1 = 0;
        cps->regionlist[i].y2 = 0;
        cps->regionlist[i].width = 0;
        cps->regionlist[i].length = 0;
        cps->regionlist[i].buffsize = 0;
        cps->regionlist[i].buffptr = NULL;
        cps->zonelist[i].position = 0;
        cps->zonelist[i].total = 0;
    }
    cps->exp_mode = ONE_FILE_COMPOSITE;
    cps->img_mode = COMPOSITE_IMAGES;
}