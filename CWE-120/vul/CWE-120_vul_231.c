int dnn_execute_layer_math_binary(DnnOperand *operands, const int32_t *input_operand_indexes,
                                 int32_t output_operand_index, const void *parameters)
{
    const DnnOperand *input = &operands[input_operand_indexes[0]];
    DnnOperand *output = &operands[output_operand_index];
    const DnnLayerMathBinaryParams *params = (const DnnLayerMathBinaryParams *)parameters;
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

    switch (params->bin_op) {
    case DMBO_SUB:
        if (params->input0_broadcast) {
            for (int i = 0; i < dims_count; ++i) {
                dst[i] = params->v - src[i];
            }
        } else if (params->input1_broadcast) {
            for (int i = 0; i < dims_count; ++i) {
                dst[i] = src[i] - params->v;
            }
        } else {
            const DnnOperand *input1 = &operands[input_operand_indexes[1]];
            const float *src1 = input1->data;
            for (int i = 0; i < dims_count; ++i) {
                dst[i] = src[i] - src1[i];
            }
        }
        return 0;
    case DMBO_ADD:
        if (params->input0_broadcast || params->input1_broadcast) {
            for (int i = 0; i < dims_count; ++i) {
                dst[i] = params->v + src[i];
            }
        } else {
            const DnnOperand *input1 = &operands[input_operand_indexes[1]];
            const float *src1 = input1->data;
            for (int i = 0; i < dims_count; ++i) {
                dst[i] = src[i] + src1[i];
            }
        }
        return 0;
    case DMBO_MUL:
        if (params->input0_broadcast || params->input1_broadcast) {
            for (int i = 0; i < dims_count; ++i) {
                dst[i] = params->v * src[i];
            }
        } else {
            const DnnOperand *input1 = &operands[input_operand_indexes[1]];
            const float *src1 = input1->data;
            for (int i = 0; i < dims_count; ++i) {
                dst[i] = src[i] * src1[i];
            }
        }
        return 0;
    case DMBO_REALDIV:
        if (params->input0_broadcast) {
            for (int i = 0; i < dims_count; ++i) {
                dst[i] = params->v / src[i];
            }
        } else if (params->input1_broadcast) {
            for (int i = 0; i < dims_count; ++i) {
                dst[i] = src[i] / params->v;
            }
        } else {
            const DnnOperand *input1 = &operands[input_operand_indexes[1]];
            const float *src1 = input1->data;
            for (int i = 0; i < dims_count; ++i) {
                dst[i] = src[i] / src1[i];
            }
        }
        return 0;
    case DMBO_MINIMUM:
        if (params->input0_broadcast || params->input1_broadcast) {
            for (int i = 0; i < dims_count; ++i) {
                dst[i] = FFMIN(params->v, src[i]);
            }
        } else {
            const DnnOperand *input1 = &operands[input_operand_indexes[1]];
            const float *src1 = input1->data;
            for (int i = 0; i < dims_count; ++i) {
                dst[i] = FFMIN(src[i], src1[i]);
            }
        }
        return 0;
    default:
        return -1;
    }
}