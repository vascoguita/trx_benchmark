#include <tee_internal_api.h>
#include <trx/trx.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <trx_benchmark_ta.h>
#include "trx_benchmark_ta_private.h"

uint32_t execution_time(TEE_Time start, TEE_Time end)
{
    TEE_Time rtd_start;
    TEE_Time rtd_end;
    uint32_t rtd;

    TEE_GetSystemTime(&rtd_start);
    TEE_GetSystemTime(&rtd_end);
    rtd = (rtd_end.seconds - rtd_start.seconds) * 1000 + rtd_end.millis - rtd_start.millis;

    return (end.seconds - start.seconds) * 1000 + end.millis - start.millis - rtd;
}

TEE_Result trx_benchmark_write(void *sess_ctx, uint32_t param_types, TEE_Param params[4])
{
    TEE_Result res;
    uint32_t exp_param_types, *report, *report_size, exp_report_size, report_index;
    unsigned long min, max, step, rounds, round, buffer_size;
    uint8_t *buffer = NULL;
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
    rounds = (unsigned long)params[1].value.b;
    report = params[2].memref.buffer;
    report_size = &(params[2].memref.size);

    if (min <= 0 || max < min || step <= 0 || rounds <= 0)
    {
        EMSG("failed checking parameter values: min = %lu, max = %lu, step = %lu, rounds = %lu", min, max, step, rounds);
        return TEE_ERROR_BAD_PARAMETERS;
    }

    exp_report_size = (((max - min) / step) + 1) * sizeof(uint32_t) * (1 + rounds);
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

    for (buffer_size = min, report_index = 0; buffer_size <= max; buffer_size += step, report_index += (1 + rounds))
    {
        if (buffer != NULL)
        {
            TEE_Free(buffer);
        }
        buffer = TEE_Malloc(buffer_size, TEE_MALLOC_FILL_ZERO);
        if (!buffer)
        {
            trx_handle_clear(handle);
            EMSG("failed allocating buffer of size: %lu", buffer_size);
            return TEE_ERROR_OUT_OF_MEMORY;
        }

        report[report_index] = buffer_size;
        for (round = 0; round < rounds; round++)
        {
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
            report[report_index + round + 1] = execution_time(start, end);
        }
    }

    trx_handle_clear(handle);
    TEE_Free(buffer);

    return TEE_SUCCESS;
}

TEE_Result trx_benchmark_read(void *sess_ctx, uint32_t param_types, TEE_Param params[4])
{
    TEE_Result res;
    uint32_t exp_param_types, *report, *report_size, exp_report_size, report_index;
    unsigned long min, max, step, rounds, round, buffer_size;
    uint8_t *buffer = NULL;
    size_t tmp_buffer_size;
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
    rounds = (unsigned long)params[1].value.b;
    report = params[2].memref.buffer;
    report_size = &(params[2].memref.size);

    if (min <= 0 || max < min || step <= 0 || rounds <= 0)
    {
        EMSG("failed checking parameter values: min = %lu, max = %lu, step = %lu, rounds = %lu", min, max, step, rounds);
        return TEE_ERROR_BAD_PARAMETERS;
    }

    exp_report_size = (((max - min) / step) + 1) * sizeof(uint32_t) * (rounds + 1);
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

    for (buffer_size = min, report_index = 0; buffer_size <= max; buffer_size += step, report_index += (rounds + 1))
    {
        if (buffer != NULL)
        {
            TEE_Free(buffer);
        }
        buffer = TEE_Malloc(buffer_size, TEE_MALLOC_FILL_ZERO);
        if (!buffer)
        {
            trx_handle_clear(handle);
            EMSG("failed allocating buffer of size: %lu", buffer_size);
            return TEE_ERROR_OUT_OF_MEMORY;
        }

        report[report_index] = buffer_size;
        for (round = 0; round < rounds; round++)
        {
            res = trx_write(handle, default_poid, default_poid_size, buffer, buffer_size);
            if (res != TEE_SUCCESS)
            {
                trx_handle_clear(handle);
                TEE_Free(buffer);
                DMSG("trx_write failed with code 0x%x", res);
                return TEE_ERROR_GENERIC;
            }

            tmp_buffer_size = buffer_size;
            TEE_GetSystemTime(&start);
            res = trx_read(handle, default_poid, default_poid_size, buffer, &tmp_buffer_size);
            TEE_GetSystemTime(&end);
            if ((res != TEE_SUCCESS) || (tmp_buffer_size != buffer_size))
            {
                trx_handle_clear(handle);
                TEE_Free(buffer);
                DMSG("trx_write failed with code 0x%x", res);
                return TEE_ERROR_GENERIC;
            }

            report[report_index + round + 1] = execution_time(start, end);
        }
    }

    trx_handle_clear(handle);
    TEE_Free(buffer);

    return TEE_SUCCESS;
}

TEE_Result trx_benchmark_gp_write(void *sess_ctx, uint32_t param_types, TEE_Param params[4])
{
    TEE_Result res;
    uint32_t exp_param_types, *report, *report_size, exp_report_size, report_index, flags;
    unsigned long min, max, step, rounds, round, buffer_size, po_exists;
    uint8_t *buffer = NULL, *id = NULL;
    TEE_Time start, end;
    TEE_ObjectHandle obj = TEE_HANDLE_NULL;

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
    rounds = (unsigned long)params[1].value.b;
    report = params[2].memref.buffer;
    report_size = &(params[2].memref.size);

    if (min <= 0 || max < min || step <= 0 || rounds <= 0)
    {
        EMSG("failed checking parameter values: min = %lu, max = %lu, step = %lu, rounds = %lu", min, max, step, rounds);
        return TEE_ERROR_BAD_PARAMETERS;
    }

    exp_report_size = (((max - min) / step) + 1) * sizeof(uint32_t) * (1 + rounds);
    if (*report_size < exp_report_size)
    {
        EMSG("failed checking report buffer size: %" PRIu32 ". Expected size: %" PRIu32, *report_size, exp_report_size);
        *report_size = exp_report_size;
        return TEE_ERROR_SHORT_BUFFER;
    }

    if (!(id = TEE_Malloc(default_poid_size, 0)))
    {
        EMSG("failed calling function \'TEE_Malloc\'");
        return TEE_ERROR_GENERIC;
    }
    TEE_MemMove(id, default_poid, default_poid_size);
    flags = TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE |
            TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_OVERWRITE;

    for (buffer_size = min, report_index = 0, po_exists = 0; buffer_size <= max; buffer_size += step, report_index += (1 + round))
    {
        if (buffer != NULL)
        {
            TEE_Free(buffer);
        }
        buffer = TEE_Malloc(buffer_size, TEE_MALLOC_FILL_ZERO);
        if (!buffer)
        {
            TEE_Free(id);
            EMSG("failed allocating buffer of size: %lu", buffer_size);
            return TEE_ERROR_OUT_OF_MEMORY;
        }

        report[report_index] = buffer_size;
        for (round = 0; round < rounds; round++)
        {
            if (po_exists)
            {
                TEE_GetSystemTime(&start);
                res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, id, default_poid_size, flags, &obj);
                if (res != TEE_SUCCESS)
                {
                    TEE_Free(id);
                    TEE_Free(buffer);
                    EMSG("failed calling function \'TEE_OpenPersistentObject\'");
                    return TEE_ERROR_GENERIC;
                }
                res = TEE_WriteObjectData(obj, buffer, buffer_size);
                if (res != TEE_SUCCESS)
                {
                    TEE_CloseObject(obj);
                    TEE_Free(id);
                    TEE_Free(buffer);
                    EMSG("failed calling function \'TEE_WriteObjectData\'");
                    return TEE_ERROR_GENERIC;
                }
                TEE_CloseObject(obj);
                TEE_GetSystemTime(&end);
            }
            else
            {
                TEE_GetSystemTime(&start);
                res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE, id, default_poid_size, flags,
                                                 TEE_HANDLE_NULL, buffer, buffer_size, &obj);
                if (res != TEE_SUCCESS)
                {
                    TEE_Free(id);
                    TEE_Free(buffer);
                    EMSG("failed calling function \'TEE_CreatePersistentObject\'");
                    return TEE_ERROR_GENERIC;
                }
                TEE_CloseObject(obj);
                TEE_GetSystemTime(&end);
                po_exists = 1;
            }

            report[report_index + round + 1] = execution_time(start, end);
        }
    }

    TEE_Free(id);
    TEE_Free(buffer);

    return TEE_SUCCESS;
}

TEE_Result trx_benchmark_gp_read(void *sess_ctx, uint32_t param_types, TEE_Param params[4])
{
    TEE_Result res;
    uint32_t exp_param_types, *report, *report_size, exp_report_size, report_index, flags, count;
    unsigned long min, max, step, rounds, round, buffer_size, po_exists;
    uint8_t *buffer = NULL, *id = NULL;
    TEE_Time start, end;
    TEE_ObjectHandle obj = TEE_HANDLE_NULL;

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
    rounds = (unsigned long)params[1].value.b;
    report = params[2].memref.buffer;
    report_size = &(params[2].memref.size);

    if (min <= 0 || max < min || step <= 0 || rounds <= 0)
    {
        EMSG("failed checking parameter values: min = %lu, max = %lu, step = %lu, rounds = %lu", min, max, step, rounds);
        return TEE_ERROR_BAD_PARAMETERS;
    }

    exp_report_size = (((max - min) / step) + 1) * sizeof(uint32_t) * (1 + rounds);
    if (*report_size < exp_report_size)
    {
        EMSG("failed checking report buffer size: %" PRIu32 ". Expected size: %" PRIu32, *report_size, exp_report_size);
        *report_size = exp_report_size;
        return TEE_ERROR_SHORT_BUFFER;
    }

    if (!(id = TEE_Malloc(default_poid_size, 0)))
    {
        EMSG("failed calling function \'TEE_Malloc\'");
        return TEE_ERROR_GENERIC;
    }
    TEE_MemMove(id, default_poid, default_poid_size);
    flags = TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE |
            TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_OVERWRITE;

    for (buffer_size = min, report_index = 0, po_exists = 0; buffer_size <= max; buffer_size += step, report_index += (1 + rounds))
    {
        if (buffer != NULL)
        {
            TEE_Free(buffer);
        }
        buffer = TEE_Malloc(buffer_size, TEE_MALLOC_FILL_ZERO);
        if (!buffer)
        {
            TEE_Free(id);
            EMSG("failed allocating buffer of size: %lu", buffer_size);
            return TEE_ERROR_OUT_OF_MEMORY;
        }

        report[report_index] = buffer_size;
        for (round = 0; round < rounds; round++)
        {
            if (po_exists)
            {
                res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, id, default_poid_size, flags, &obj);
                if (res != TEE_SUCCESS)
                {
                    TEE_Free(id);
                    TEE_Free(buffer);
                    EMSG("failed calling function \'TEE_OpenPersistentObject\'");
                    return TEE_ERROR_GENERIC;
                }
                res = TEE_WriteObjectData(obj, buffer, buffer_size);
                if (res != TEE_SUCCESS)
                {
                    TEE_CloseObject(obj);
                    TEE_Free(id);
                    TEE_Free(buffer);
                    EMSG("failed calling function \'TEE_WriteObjectData\'");
                    return TEE_ERROR_GENERIC;
                }
                TEE_CloseObject(obj);
            }
            else
            {
                res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE, id, default_poid_size, flags,
                                                 TEE_HANDLE_NULL, buffer, buffer_size, &obj);
                if (res != TEE_SUCCESS)
                {
                    TEE_Free(id);
                    TEE_Free(buffer);
                    EMSG("failed calling function \'TEE_CreatePersistentObject\'");
                    return TEE_ERROR_GENERIC;
                }
                TEE_CloseObject(obj);
                po_exists = 1;
            }
            TEE_GetSystemTime(&start);
            res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, id, default_poid_size, flags, &obj);
            if (res != TEE_SUCCESS)
            {
                TEE_Free(id);
                TEE_Free(buffer);
                EMSG("failed calling function \'TEE_OpenPersistentObject\'");
                return TEE_ERROR_GENERIC;
            }
            res = TEE_ReadObjectData(obj, buffer, buffer_size, &count);
            if ((res |= TEE_SUCCESS) || (count != buffer_size))
            {
                TEE_CloseObject(obj);
                TEE_Free(id);
                TEE_Free(buffer);
                EMSG("failed calling function \'TEE_ReadObjectData\'");
            }
            TEE_CloseObject(obj);
            TEE_GetSystemTime(&end);

            report[report_index + round + 1] = execution_time(start, end);
        }
    }

    TEE_Free(id);
    TEE_Free(buffer);

    return TEE_SUCCESS;
}

TEE_Result trx_benchmark_share(void *sess_ctx, uint32_t param_types, TEE_Param params[4])
{
    TEE_Result res;
    uint32_t exp_param_types, *report, *report_size, exp_report_size;
    unsigned long rounds, round;
    trx_handle handle;
    size_t tmp_dst_size;
    TEE_Time start, end;

    (void)&sess_ctx;

    DMSG("has been called");

    exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT, TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                      TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
    if (param_types != exp_param_types)
    {
        EMSG("failed checking parameter types");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    rounds = (unsigned long)params[0].value.a;
    report = params[1].memref.buffer;
    report_size = &(params[1].memref.size);

    if (rounds <= 0)
    {
        EMSG("failed checking parameter values: rounds = %lu", rounds);
        return TEE_ERROR_BAD_PARAMETERS;
    }

    exp_report_size = sizeof(uint32_t) * rounds;
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

    res = trx_write(handle, default_poid, default_poid_size, default_buffer, default_buffer_size);
    if (res != TEE_SUCCESS)
    {
        trx_handle_clear(handle);
        DMSG("trx_write failed with code 0x%x", res);
        return TEE_ERROR_GENERIC;
    }

    for (round = 0; round < rounds; round++)
    {
        tmp_dst_size = default_dst_size;
        TEE_GetSystemTime(&start);
        res = trx_share(handle, default_udid, default_udid_size, default_mount_point, default_mount_point_size,
                        default_label, default_label_size, default_dst, &tmp_dst_size);
        TEE_GetSystemTime(&end);
        if (res != TEE_SUCCESS)
        {
            trx_handle_clear(handle);
            DMSG("trx_share failed with code 0x%x", res);
            return TEE_ERROR_GENERIC;
        }

        report[round] = execution_time(start, end);
    }

    trx_handle_clear(handle);

    return TEE_SUCCESS;
}

TEE_Result trx_benchmark_mount(void *sess_ctx, uint32_t param_types, TEE_Param params[4])
{
    TEE_Result res;
    uint32_t exp_param_types, *report, *report_size, exp_report_size;
    unsigned long rounds, round;
    trx_handle handle;
    size_t tmp_dst_size, src_size;
    char *src = NULL;
    char *dir = NULL;
    size_t dir_size;
    TEE_Time start, end;

    (void)&sess_ctx;

    DMSG("has been called");

    exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT, TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                      TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
    if (param_types != exp_param_types)
    {
        EMSG("failed checking parameter types");
        return TEE_ERROR_BAD_PARAMETERS;
    }

    rounds = (unsigned long)params[0].value.a;
    report = params[1].memref.buffer;
    report_size = &(params[1].memref.size);

    if (rounds <= 0)
    {
        EMSG("failed checking parameter values: rounds = %lu", rounds);
        return TEE_ERROR_BAD_PARAMETERS;
    }

    exp_report_size = sizeof(uint32_t) * rounds;
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

    res = trx_write(handle, default_poid, default_poid_size, default_buffer, default_buffer_size);
    if (res != TEE_SUCCESS)
    {
        trx_handle_clear(handle);
        DMSG("trx_write failed with code 0x%x", res);
        return TEE_ERROR_GENERIC;
    }

    tmp_dst_size = default_dst_size;
    res = trx_share(handle, default_udid, default_udid_size, default_mount_point, default_mount_point_size,
                    default_label, default_label_size, default_dst, &tmp_dst_size);
    if (res != TEE_SUCCESS)
    {
        trx_handle_clear(handle);
        DMSG("trx_share failed with code 0x%x", res);
        return TEE_ERROR_GENERIC;
    }

    memcpy(&src_size, default_dst, sizeof(size_t));

    src = malloc(src_size);
    if (!src)
    {
        trx_handle_clear(handle);
        EMSG("failed allocating \"src\" buffer of size: %lu", src_size);
        return TEE_ERROR_OUT_OF_MEMORY;
    }

    memcpy(src, default_dst + sizeof(size_t), src_size);

    memcpy(&dir_size, default_dst + sizeof(size_t) + src_size, sizeof(size_t));
    if (!(dir = malloc(dir_size)))
    {
        free(src);
        trx_handle_clear(handle);
        EMSG("failed allocating \"dir\" buffer of size: %lu", dir_size);
        return TEE_ERROR_OUT_OF_MEMORY;
    }
    memcpy(dir, default_dst + sizeof(size_t) + src_size + sizeof(size_t), dir_size);


    for (round = 0; round < rounds; round++)
    {
        TEE_GetSystemTime(&start);
        res = trx_mount(handle, default_udid, default_udid_size, dir, dir_size,
                        default_mount_point, default_mount_point_size, src, src_size);
        TEE_GetSystemTime(&end);
        if (res != TEE_SUCCESS)
        {
            free(dir);
            free(src);
            trx_handle_clear(handle);
            DMSG("trx_mount failed with code 0x%x", res);
            return TEE_ERROR_GENERIC;
        }

        report[round] = execution_time(start, end);
    }

    free(dir);
    free(src);
    trx_handle_clear(handle);

    return TEE_SUCCESS;
}

TEE_Result trx_benchmark_pop_write_best(void *sess_ctx, uint32_t param_types, TEE_Param params[4])
{
    TEE_Result res;
    uint32_t exp_param_types, *report, *report_size, exp_report_size, report_index;
    unsigned long min, max, step, rounds, round, pop, current_pop;
    trx_handle handle;
    TEE_Time start, end;
    char poid[10];
    size_t poid_size = 10;
    int tmp_size;

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
    rounds = (unsigned long)params[1].value.b;
    report = params[2].memref.buffer;
    report_size = &(params[2].memref.size);

    if (max < min || step <= 0 || rounds <= 0)
    {
        EMSG("failed checking parameter values: min = %lu, max = %lu, step = %lu, rounds = %lu", min, max, step, rounds);
        return TEE_ERROR_BAD_PARAMETERS;
    }

    exp_report_size = (((max - min) / step) + 1) * sizeof(uint32_t) * (1 + rounds);
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

    for (pop = min, report_index = 0, current_pop = 0; pop <= max; pop += step, report_index += (1 + rounds))
    {
        for(; current_pop < pop; current_pop++)
        {
            if(!(tmp_size = snprintf(poid, poid_size, "%lu", current_pop)))
            {
                trx_handle_clear(handle);
                DMSG("snprintf failed");
                return TEE_ERROR_GENERIC;
            }

            res = trx_write(handle, poid, tmp_size, default_buffer, default_buffer_size);
            if (res != TEE_SUCCESS)
            {
                trx_handle_clear(handle);
                DMSG("trx_write failed with code 0x%x", res);
                return TEE_ERROR_GENERIC;
            }
        }

        report[report_index] = pop;
        for (round = 0; round < rounds; round++)
        {
            TEE_GetSystemTime(&start);
            res = trx_write(handle, default_poid, default_poid_size, default_buffer, default_buffer_size);
            TEE_GetSystemTime(&end);
            if (res != TEE_SUCCESS)
            {
                trx_handle_clear(handle);
                DMSG("trx_write failed with code 0x%x", res);
                return TEE_ERROR_GENERIC;
            }
            report[report_index + round + 1] = execution_time(start, end);
        }
    }

    trx_handle_clear(handle);

    return TEE_SUCCESS;
}


TEE_Result trx_benchmark_pop_read_best(void *sess_ctx, uint32_t param_types, TEE_Param params[4])
{
    TEE_Result res;
    uint32_t exp_param_types, *report, *report_size, exp_report_size, report_index;
    unsigned long min, max, step, rounds, round, pop, current_pop;
    size_t tmp_buffer_size;
    trx_handle handle;
    TEE_Time start, end;
    char poid[10];
    size_t poid_size = 10;
    int tmp_size;

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
    rounds = (unsigned long)params[1].value.b;
    report = params[2].memref.buffer;
    report_size = &(params[2].memref.size);

    if (max < min || step <= 0 || rounds <= 0)
    {
        EMSG("failed checking parameter values: min = %lu, max = %lu, step = %lu, rounds = %lu", min, max, step, rounds);
        return TEE_ERROR_BAD_PARAMETERS;
    }

    exp_report_size = (((max - min) / step) + 1) * sizeof(uint32_t) * (rounds + 1);
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

    for (pop = min, report_index = 0, current_pop = 0; pop <= max; pop += step, report_index += (rounds + 1))
    {
        for(; current_pop < pop; current_pop++)
        {
            if(!(tmp_size = snprintf(poid, poid_size, "%lu", current_pop)))
            {
                trx_handle_clear(handle);
                DMSG("snprintf failed");
                return TEE_ERROR_GENERIC;
            }

            res = trx_write(handle, poid, tmp_size, default_buffer, default_buffer_size);
            if (res != TEE_SUCCESS)
            {
                trx_handle_clear(handle);
                DMSG("trx_write failed with code 0x%x", res);
                return TEE_ERROR_GENERIC;
            }
        }
        report[report_index] = pop;
        for (round = 0; round < rounds; round++)
        {
            res = trx_write(handle, default_poid, default_poid_size, default_buffer, default_buffer_size);
            if (res != TEE_SUCCESS)
            {
                trx_handle_clear(handle);
                DMSG("trx_write failed with code 0x%x", res);
                return TEE_ERROR_GENERIC;
            }

            tmp_buffer_size = default_buffer_size;
            TEE_GetSystemTime(&start);
            res = trx_read(handle, default_poid, default_poid_size, default_buffer, &tmp_buffer_size);
            TEE_GetSystemTime(&end);
            if ((res != TEE_SUCCESS) || (tmp_buffer_size != default_buffer_size))
            {
                trx_handle_clear(handle);
                DMSG("trx_write failed with code 0x%x", res);
                return TEE_ERROR_GENERIC;
            }

            report[report_index + round + 1] = execution_time(start, end);
        }
    }

    trx_handle_clear(handle);
    return TEE_SUCCESS;
}

TEE_Result trx_benchmark_pop_write_worst(void *sess_ctx, uint32_t param_types, TEE_Param params[4])
{
    TEE_Result res;
    uint32_t exp_param_types, *report, *report_size, exp_report_size, report_index;
    unsigned long min, max, step, rounds, round, pop, current_pop;
    trx_handle handle;
    TEE_Time start, end;
    char poid[10];
    size_t poid_size = 10;
    int tmp_size;

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
    rounds = (unsigned long)params[1].value.b;
    report = params[2].memref.buffer;
    report_size = &(params[2].memref.size);

    if (max < min || step <= 0 || rounds <= 0)
    {
        EMSG("failed checking parameter values: min = %lu, max = %lu, step = %lu, rounds = %lu", min, max, step, rounds);
        return TEE_ERROR_BAD_PARAMETERS;
    }

    exp_report_size = (((max - min) / step) + 1) * sizeof(uint32_t) * (1 + rounds);
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

    for (pop = min, report_index = 0, current_pop = 0; pop <= max; pop += step, report_index += (1 + rounds))
    {
        for(; current_pop < pop; current_pop++)
        {
            if(!(tmp_size = snprintf(poid, poid_size, "%lu", current_pop)))
            {
                trx_handle_clear(handle);
                DMSG("snprintf failed");
                return TEE_ERROR_GENERIC;
            }

            res = trx_write(handle, poid, tmp_size, default_buffer, default_buffer_size);
            if (res != TEE_SUCCESS)
            {
                trx_handle_clear(handle);
                DMSG("trx_write failed with code 0x%x", res);
                return TEE_ERROR_GENERIC;
            }
        }

        if(!(tmp_size = snprintf(poid, poid_size, "%lu", pop)))
        {
            trx_handle_clear(handle);
            DMSG("snprintf failed");
            return TEE_ERROR_GENERIC;
        }

        report[report_index] = pop;
        for (round = 0; round < rounds; round++)
        {
            
            TEE_GetSystemTime(&start);
            res = trx_write(handle, poid, tmp_size, default_buffer, default_buffer_size);
            TEE_GetSystemTime(&end);
            if (res != TEE_SUCCESS)
            {
                trx_handle_clear(handle);
                DMSG("trx_write failed with code 0x%x", res);
                return TEE_ERROR_GENERIC;
            }
            report[report_index + round + 1] = execution_time(start, end);
        }
    }

    trx_handle_clear(handle);

    return TEE_SUCCESS;
}


TEE_Result trx_benchmark_pop_read_worst(void *sess_ctx, uint32_t param_types, TEE_Param params[4])
{
    TEE_Result res;
    uint32_t exp_param_types, *report, *report_size, exp_report_size, report_index;
    unsigned long min, max, step, rounds, round, pop, current_pop;
    size_t tmp_buffer_size;
    trx_handle handle;
    TEE_Time start, end;
    char poid[10];
    size_t poid_size = 10;
    int tmp_size;

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
    rounds = (unsigned long)params[1].value.b;
    report = params[2].memref.buffer;
    report_size = &(params[2].memref.size);

    if (max < min || step <= 0 || rounds <= 0)
    {
        EMSG("failed checking parameter values: min = %lu, max = %lu, step = %lu, rounds = %lu", min, max, step, rounds);
        return TEE_ERROR_BAD_PARAMETERS;
    }

    exp_report_size = (((max - min) / step) + 1) * sizeof(uint32_t) * (rounds + 1);
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

    for (pop = min, report_index = 0, current_pop = 0; pop <= max; pop += step, report_index += (rounds + 1))
    {
        for(; current_pop < pop; current_pop++)
        {
            if(!(tmp_size = snprintf(poid, poid_size, "%lu", current_pop)))
            {
                trx_handle_clear(handle);
                DMSG("snprintf failed");
                return TEE_ERROR_GENERIC;
            }

            res = trx_write(handle, poid, tmp_size, default_buffer, default_buffer_size);
            if (res != TEE_SUCCESS)
            {
                trx_handle_clear(handle);
                DMSG("trx_write failed with code 0x%x", res);
                return TEE_ERROR_GENERIC;
            }
        }

        if(!(tmp_size = snprintf(poid, poid_size, "%lu", current_pop)))
        {
            trx_handle_clear(handle);
            DMSG("snprintf failed");
            return TEE_ERROR_GENERIC;
        }

        report[report_index] = pop;
        for (round = 0; round < rounds; round++)
        {
            res = trx_write(handle, poid, tmp_size, default_buffer, default_buffer_size);
            if (res != TEE_SUCCESS)
            {
                trx_handle_clear(handle);
                DMSG("trx_write failed with code 0x%x", res);
                return TEE_ERROR_GENERIC;
            }

            tmp_buffer_size = default_buffer_size;
            TEE_GetSystemTime(&start);
            res = trx_read(handle, poid, tmp_size, default_buffer, &tmp_buffer_size);
            TEE_GetSystemTime(&end);
            if ((res != TEE_SUCCESS) || (tmp_buffer_size != default_buffer_size))
            {
                trx_handle_clear(handle);
                DMSG("trx_read failed with code 0x%x", res);
                return TEE_ERROR_GENERIC;
            }

            report[report_index + round + 1] = execution_time(start, end);
        }
    }

    trx_handle_clear(handle);
    return TEE_SUCCESS;
}

TEE_Result trx_benchmark_gp_pop_write(void *sess_ctx, uint32_t param_types, TEE_Param params[4])
{
    TEE_Result res;
    uint32_t exp_param_types, *report, *report_size, exp_report_size, report_index, flags;
    unsigned long min, max, step, rounds, round, po_exists, pop, current_pop;
    uint8_t *id = NULL;
    TEE_Time start, end;
    TEE_ObjectHandle obj = TEE_HANDLE_NULL;
    char *poid;
    size_t poid_size = 10;
    int tmp_size;

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
    rounds = (unsigned long)params[1].value.b;
    report = params[2].memref.buffer;
    report_size = &(params[2].memref.size);

    if (max < min || step <= 0 || rounds <= 0)
    {
        EMSG("failed checking parameter values: min = %lu, max = %lu, step = %lu, rounds = %lu", min, max, step, rounds);
        return TEE_ERROR_BAD_PARAMETERS;
    }

    exp_report_size = (((max - min) / step) + 1) * sizeof(uint32_t) * (1 + rounds);
    if (*report_size < exp_report_size)
    {
        EMSG("failed checking report buffer size: %" PRIu32 ". Expected size: %" PRIu32, *report_size, exp_report_size);
        *report_size = exp_report_size;
        return TEE_ERROR_SHORT_BUFFER;
    }

    if (!(poid = TEE_Malloc(poid_size, 0)))
    {
        EMSG("failed calling function \'TEE_Malloc\'");
        return TEE_ERROR_GENERIC;
    }

    if (!(id = TEE_Malloc(default_poid_size, 0)))
    {
        TEE_Free(poid);
        EMSG("failed calling function \'TEE_Malloc\'");
        return TEE_ERROR_GENERIC;
    }

    TEE_MemMove(id, default_poid, default_poid_size);
    flags = TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE |
            TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_OVERWRITE;

    for (pop = min, report_index = 0, po_exists = 0, current_pop = 0; pop <= max; pop += step, report_index += (1 + round))
    {
        for(; current_pop < pop; current_pop++)
        {
            if(!(tmp_size = snprintf(poid, poid_size, "%lu", current_pop)))
            {
                TEE_Free(poid);
                TEE_Free(id);
                DMSG("snprintf failed");
                return TEE_ERROR_GENERIC;
            }

            res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE, poid, poid_size, flags,
                                            TEE_HANDLE_NULL, default_buffer, default_buffer_size, &obj);
            if (res != TEE_SUCCESS)
            {
                TEE_Free(poid);
                TEE_Free(id);
                EMSG("failed calling function \'TEE_CreatePersistentObject\'");
                return TEE_ERROR_GENERIC;
            }
            TEE_CloseObject(obj);
        }

        report[report_index] = pop;
        for (round = 0; round < rounds; round++)
        {
            if (po_exists)
            {
                TEE_GetSystemTime(&start);
                res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, id, default_poid_size, flags, &obj);
                if (res != TEE_SUCCESS)
                {
                    TEE_Free(poid);
                    TEE_Free(id);
                    EMSG("failed calling function \'TEE_OpenPersistentObject\'");
                    return TEE_ERROR_GENERIC;
                }
                res = TEE_WriteObjectData(obj, default_buffer, default_buffer_size);
                if (res != TEE_SUCCESS)
                {
                    TEE_CloseObject(obj);
                    TEE_Free(poid);
                    TEE_Free(id);
                    EMSG("failed calling function \'TEE_WriteObjectData\'");
                    return TEE_ERROR_GENERIC;
                }
                TEE_CloseObject(obj);
                TEE_GetSystemTime(&end);
            }
            else
            {
                TEE_GetSystemTime(&start);
                res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE, id, default_poid_size, flags,
                                                 TEE_HANDLE_NULL, default_buffer, default_buffer_size, &obj);
                if (res != TEE_SUCCESS)
                {
                    TEE_Free(poid);
                    TEE_Free(id);
                    EMSG("failed calling function \'TEE_CreatePersistentObject\'");
                    return TEE_ERROR_GENERIC;
                }
                TEE_CloseObject(obj);
                TEE_GetSystemTime(&end);
                po_exists = 1;
            }

            report[report_index + round + 1] = execution_time(start, end);
        }
    }

    TEE_Free(poid);
    TEE_Free(id);

    return TEE_SUCCESS;
}


TEE_Result trx_benchmark_gp_pop_read(void *sess_ctx, uint32_t param_types, TEE_Param params[4])
{
    TEE_Result res;
    uint32_t exp_param_types, *report, *report_size, exp_report_size, report_index, flags, count;
    unsigned long min, max, step, rounds, round, po_exists, pop, current_pop;
    uint8_t *id = NULL;
    TEE_Time start, end;
    TEE_ObjectHandle obj = TEE_HANDLE_NULL;
    char *poid;
    size_t poid_size = 10;
    int tmp_size;

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
    rounds = (unsigned long)params[1].value.b;
    report = params[2].memref.buffer;
    report_size = &(params[2].memref.size);

    if (max < min || step <= 0 || rounds <= 0)
    {
        EMSG("failed checking parameter values: min = %lu, max = %lu, step = %lu, rounds = %lu", min, max, step, rounds);
        return TEE_ERROR_BAD_PARAMETERS;
    }

    exp_report_size = (((max - min) / step) + 1) * sizeof(uint32_t) * (1 + rounds);
    if (*report_size < exp_report_size)
    {
        EMSG("failed checking report buffer size: %" PRIu32 ". Expected size: %" PRIu32, *report_size, exp_report_size);
        *report_size = exp_report_size;
        return TEE_ERROR_SHORT_BUFFER;
    }

    if (!(poid = TEE_Malloc(poid_size, 0)))
    {
        EMSG("failed calling function \'TEE_Malloc\'");
        return TEE_ERROR_GENERIC;
    }

    if (!(id = TEE_Malloc(default_poid_size, 0)))
    {
        TEE_Free(poid);
        EMSG("failed calling function \'TEE_Malloc\'");
        return TEE_ERROR_GENERIC;
    }
    TEE_MemMove(id, default_poid, default_poid_size);
    flags = TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE |
            TEE_DATA_FLAG_ACCESS_WRITE_META | TEE_DATA_FLAG_OVERWRITE;

    for (pop = min, report_index = 0, po_exists = 0, current_pop = 0; pop <= max; pop += step, report_index += (1 + rounds))
    {
        for(; current_pop < pop; current_pop++)
        {
            if(!(tmp_size = snprintf(poid, poid_size, "%lu", current_pop)))
            {
                TEE_Free(poid);
                TEE_Free(id);
                DMSG("snprintf failed");
                return TEE_ERROR_GENERIC;
            }

            res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE, poid, poid_size, flags,
                                            TEE_HANDLE_NULL, default_buffer, default_buffer_size, &obj);
            if (res != TEE_SUCCESS)
            {
                TEE_Free(poid);
                TEE_Free(id);
                EMSG("failed calling function \'TEE_CreatePersistentObject\'");
                return TEE_ERROR_GENERIC;
            }
            TEE_CloseObject(obj);
        }
        
        report[report_index] = pop;
        for (round = 0; round < rounds; round++)
        {
            if (po_exists)
            {
                res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, id, default_poid_size, flags, &obj);
                if (res != TEE_SUCCESS)
                {
                    TEE_Free(poid);
                    TEE_Free(id);
                    EMSG("failed calling function \'TEE_OpenPersistentObject\'");
                    return TEE_ERROR_GENERIC;
                }
                res = TEE_WriteObjectData(obj, default_buffer, default_buffer_size);
                if (res != TEE_SUCCESS)
                {
                    TEE_CloseObject(obj);
                    TEE_Free(poid);
                    TEE_Free(id);
                    EMSG("failed calling function \'TEE_WriteObjectData\'");
                    return TEE_ERROR_GENERIC;
                }
                TEE_CloseObject(obj);
            }
            else
            {
                res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE, id, default_poid_size, flags,
                                                 TEE_HANDLE_NULL, default_buffer, default_buffer_size, &obj);
                if (res != TEE_SUCCESS)
                {
                    TEE_Free(poid);
                    TEE_Free(id);
                    EMSG("failed calling function \'TEE_CreatePersistentObject\'");
                    return TEE_ERROR_GENERIC;
                }
                TEE_CloseObject(obj);
                po_exists = 1;
            }
            TEE_GetSystemTime(&start);
            res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, id, default_poid_size, flags, &obj);
            if (res != TEE_SUCCESS)
            {
                TEE_Free(poid);
                TEE_Free(id);
                EMSG("failed calling function \'TEE_OpenPersistentObject\'");
                return TEE_ERROR_GENERIC;
            }
            res = TEE_ReadObjectData(obj, default_buffer, default_buffer_size, &count);
            if ((res |= TEE_SUCCESS) || (count != default_buffer_size))
            {
                TEE_CloseObject(obj);
                TEE_Free(poid);
                TEE_Free(id);
                EMSG("failed calling function \'TEE_ReadObjectData\'");
            }
            TEE_CloseObject(obj);
            TEE_GetSystemTime(&end);

            report[report_index + round + 1] = execution_time(start, end);
        }
    }

    TEE_Free(poid);
    TEE_Free(id);

    return TEE_SUCCESS;
}