#ifndef TA_TRX_BENCHMARK_PRIVATE_H
#define TA_TRX_BENCHMARK_PRIVATE_H

#include <tee_internal_api.h>

TEE_Result trx_benchmark_write(void *sess_ctx, uint32_t param_types, TEE_Param params[4]);
TEE_Result trx_benchmark_read(void *sess_ctx, uint32_t param_types, TEE_Param params[4]);
uint32_t execution_time(TEE_Time start, TEE_Time end);

static const char default_poid[] = "default_poid";
static const size_t default_poid_size = 13;

#endif //TA_TRX_BENCHMARK_PRIVATE_H