#ifndef TA_TRX_BENCHMARK_PRIVATE_H
#define TA_TRX_BENCHMARK_PRIVATE_H

#include <tee_internal_api.h>

uint32_t execution_time(TEE_Time start, TEE_Time end);
TEE_Result trx_benchmark_write(void *sess_ctx, uint32_t param_types, TEE_Param params[4]);
TEE_Result trx_benchmark_read(void *sess_ctx, uint32_t param_types, TEE_Param params[4]);
TEE_Result trx_benchmark_gp_write(void *sess_ctx, uint32_t param_types, TEE_Param params[4]);
TEE_Result trx_benchmark_gp_read(void *sess_ctx, uint32_t param_types, TEE_Param params[4]);
TEE_Result trx_benchmark_share(void *sess_ctx, uint32_t param_types, TEE_Param params[4]);
TEE_Result trx_benchmark_mount(void *sess_ctx, uint32_t param_types, TEE_Param params[4]);

TEE_Result trx_benchmark_pop_write(void *sess_ctx, uint32_t param_types, TEE_Param params[4]);
TEE_Result trx_benchmark_pop_read(void *sess_ctx, uint32_t param_types, TEE_Param params[4]);
TEE_Result trx_benchmark_gp_pop_write(void *sess_ctx, uint32_t param_types, TEE_Param params[4]);
TEE_Result trx_benchmark_gp_pop_read(void *sess_ctx, uint32_t param_types, TEE_Param params[4]);

static const char default_poid[] = "default_poid";
static const size_t default_poid_size = 13;
static const uint8_t default_buffer[1];
static const size_t default_buffer_size = 1;
static const unsigned char default_udid[] = "a";
static const size_t default_udid_size = 2;
static const char default_mount_point[] = ".";
static const size_t default_mount_point_size = 2;
static const char default_label[] = "a";
static const size_t default_label_size = 2;
uint8_t default_dst[1500];
static const size_t default_dst_size = 1500;

#endif //TA_TRX_BENCHMARK_PRIVATE_H