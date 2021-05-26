#include <tee_internal_api.h>
#include <trx/trx.h>

#include <trx_benchmark_ta.h>
#include "trx_benchmark_ta_private.h"

TEE_Result trx_benchmark_write(void *sess_ctx, uint32_t param_types, TEE_Param params[4])
{
    TEE_Result res;
    uint32_t exp_param_types, *report, *report_size, exp_report_size, report_index;
    unsigned long min, max, step, buffer_size;
    uint8_t *buffer = NULL, *tmp_buffer = NULL;
    trx_handle handle;
    TEE_Time start, end;

    (void)&sess_ctx;

    DMSG("has been called");

    exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT, TEE_PARAM_TYPE_VALUE_INPUT,
                                      TEE_PARAM_TYPE_MEMREF_OUTPUT, TEE_PARAM_TYPE_NONE);
    if (param_types != exp_param_types)
    {
        EMSG("failed checking parameter types");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    min = (unsigned long)params[0].value.a;
    max = (unsigned long)params[0].value.b;
    step = (unsigned long)params[1].value.a;
    report = params[2].memref.buffer;
    report_size = &(params[2].memref.size);

    if (min <= 0 || max < min || step <= 0)
    {
        EMSG("failed checking parameter values: min = %lu, max = %lu, step = %lu", min, max, step);
        return TEE_ERROR_BAD_PARAMETERS;
    }

    exp_report_size = (((max - min) / step) + 1) * sizeof(uint32_t) * 2;
    if (*report_size < exp_report_size)
    {
        EMSG("failed checking report buffer size: %" PRIu32 ". Expected size: %" PRIu32, *report_size, exp_report_size);
        *report_size = exp_report_size;
        return TEE_ERROR_SHORT_BUFFER;
    }

    res = trx_handle_init(&handle);
    if (res != TEE_SUCCESS)
    {
        EMSG("trx_handle_init failed with code 0x%x", res);
        return res;
    }

    for (buffer_size = min, report_index = 0; buffer_size <= max; buffer_size += step, report_index += 2)
    {
        if (!buffer)
        {
            buffer = TEE_Malloc(buffer_size, TEE_MALLOC_FILL_ZERO);
            if (!buffer)
            {
                trx_handle_clear(handle);
                EMSG("failed allocating buffer of size: %lu", buffer_size);
                return TEE_ERROR_OUT_OF_MEMORY;
            }
        }
        else
        {
            tmp_buffer = TEE_Realloc(buffer, buffer_size);
            if (!tmp_buffer)
            {
                trx_handle_clear(handle);
                TEE_Free(buffer);
                EMSG("failed allocating buffer of size: %lu", buffer_size);
                return TEE_ERROR_OUT_OF_MEMORY;
            }
            else
            {
                buffer = tmp_buffer;
            }
        }

        TEE_GetSystemTime(&start);
        res = trx_write(handle, default_poid, default_poid_size, buffer, buffer_size);
        TEE_GetSystemTime(&end);
        if (res != TEE_SUCCESS)
        {
            trx_handle_clear(handle);
            TEE_Free(buffer);
            DMSG("trx_write failed with code 0x%x", res);
            return TEE_ERROR_GENERIC;
        }

        report[report_index] = buffer_size;
        report[report_index + 1] = execution_time(start, end);
    }

    trx_handle_clear(handle);
    TEE_Free(buffer);

    return TEE_SUCCESS;
}

uint32_t execution_time(TEE_Time start, TEE_Time end)
{
    TEE_Time rtd_start;
    TEE_Time rtd_end;
    uint32_t rtd;

    TEE_GetSystemTime(&rtd_start);
    TEE_GetSystemTime(&rtd_end);
    rtd = (rtd_end.seconds - rtd_start.seconds)*1000 + rtd_end.millis - rtd_start.millis;

    return (end.seconds - start.seconds)*1000 + end.millis - start.millis - rtd;
}