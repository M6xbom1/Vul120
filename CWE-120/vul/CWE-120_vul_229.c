int dnn_execute_layer_math_unary(DnnOperand *operands, const int32_t *input_operand_indexes,
                                int32_t output_operand_index, const void *parameters)
{
    const DnnOperand *input = &operands[input_operand_indexes[0]];
    DnnOperand *output = &operands[output_operand_index];
    const DnnLayerMathUnaryParams *params = (const DnnLayerMathUnaryParams *)parameters;
    int dims_count;
    const float *src;
    float *dst;

    for (int i = 0; i < 4; ++i)
        output->dims[i] = input->dims[i];

    output->data_type = input->data_type;
    output->length = calculate_operand_data_length(output);
    output->data = av_realloc(output->data, output->length);
    if (!output->data)
        return DNN_ERROR;

    dims_count = calculate_operand_dims_count(output);
    src = input->data;
    dst = output->data;

    switch (params->un_op) {
    case DMUO_ABS:
        for (int i = 0; i < dims_count; ++i)
            dst[i] = FFABS(src[i]);
        return 0;
    case DMUO_SIN:
        for (int i = 0; i < dims_count; ++i)
            dst[i] = sin(src[i]);
        return 0;
    case DMUO_COS:
        for (int i = 0; i < dims_count; ++i)
            dst[i] = cos(src[i]);
        return 0;
    case DMUO_TAN:
        for (int i = 0; i < dims_count; ++i)
            dst[i] = tan(src[i]);
        return 0;
    case DMUO_ASIN:
        for (int i = 0; i < dims_count; ++i)
            dst[i] = asin(src[i]);
        return 0;
    case DMUO_ACOS:
        for (int i = 0; i < dims_count; ++i)
            dst[i] = acos(src[i]);
        return 0;
    case DMUO_ATAN:
        for (int i = 0; i < dims_count; ++i)
            dst[i] = atan(src[i]);
        return 0;
    case DMUO_SINH:
        for (int i = 0; i < dims_count; ++i)
            dst[i] = sinh(src[i]);
        return 0;
    case DMUO_COSH:
        for (int i = 0; i < dims_count; ++i)
            dst[i] = cosh(src[i]);
        return 0;
    case DMUO_TANH:
        for (int i = 0; i < dims_count; ++i)
            dst[i] = tanh(src[i]);
        return 0;
    case DMUO_ASINH:
        for (int i = 0; i < dims_count; ++i)
            dst[i] = asinh(src[i]);
        return 0;
    case DMUO_ACOSH:
        for (int i = 0; i < dims_count; ++i)
            dst[i] = acosh(src[i]);
        return 0;
    case DMUO_ATANH:
        for (int i = 0; i < dims_count; ++i)
            dst[i] = atanh(src[i]);
        return 0;
    default:
        return -1;
    }
}