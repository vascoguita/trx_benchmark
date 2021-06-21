#include <tee_internal_api.h>
#include <trx_benchmark_ta.h>

#include "trx_benchmark_ta_private.h"

TEE_Result TA_CreateEntryPoint(void)
{
    DMSG("has been called");
    return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
    DMSG("has been called");
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types, TEE_Param params[4], void **sess_ctx)
{
    uint32_t exp_param_types;

    (void)&params;
    (void)&sess_ctx;

    DMSG("has been called");

    exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE,
                                      TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
    if (param_types != exp_param_types)
    {
        return TEE_ERROR_BAD_PARAMETERS;
    }

    return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void *sess_ctx)
{
    (void)&sess_ctx;

    DMSG("has been called");
}

TEE_Result TA_InvokeCommandEntryPoint(void *sess_ctx, uint32_t cmd, uint32_t param_types, TEE_Param params[4])
{
    switch (cmd)
    {
    case TA_TRX_BENCHMARK_CMD_WRITE:
        return trx_benchmark_write(sess_ctx, param_types, params);
    case TA_TRX_BENCHMARK_CMD_READ:
        return trx_benchmark_read(sess_ctx, param_types, params);
    case TA_TRX_BENCHMARK_CMD_GP_WRITE:
        return trx_benchmark_gp_write(sess_ctx, param_types, params);
    case TA_TRX_BENCHMARK_CMD_GP_READ:
        return trx_benchmark_gp_read(sess_ctx, param_types, params);
    case TA_TRX_BENCHMARK_CMD_SHARE:
        return trx_benchmark_share(sess_ctx, param_types, params);
    case TA_TRX_BENCHMARK_CMD_MOUNT:
        return trx_benchmark_mount(sess_ctx, param_types, params);
    case TA_TRX_BENCHMARK_CMD_POP_WRITE:
        return trx_benchmark_pop_write(sess_ctx, param_types, params);
    case TA_TRX_BENCHMARK_CMD_POP_READ:
        return trx_benchmark_pop_read(sess_ctx, param_types, params);
    case TA_TRX_BENCHMARK_CMD_POP_WRITE_BEST:
        return trx_benchmark_pop_write_best(sess_ctx, param_types, params);
    case TA_TRX_BENCHMARK_CMD_POP_READ_BEST:
        return trx_benchmark_pop_read_best(sess_ctx, param_types, params);
    case TA_TRX_BENCHMARK_CMD_POP_WRITE_WORST:
        return trx_benchmark_pop_write_worst(sess_ctx, param_types, params);
    case TA_TRX_BENCHMARK_CMD_POP_READ_WORST:
        return trx_benchmark_pop_read_worst(sess_ctx, param_types, params);
    case TA_TRX_BENCHMARK_CMD_GP_POP_WRITE:
        return trx_benchmark_gp_pop_write(sess_ctx, param_types, params);
    case TA_TRX_BENCHMARK_CMD_GP_POP_READ:
        return trx_benchmark_gp_pop_read(sess_ctx, param_types, params);
    default:
        return TEE_ERROR_NOT_SUPPORTED;
    }
}