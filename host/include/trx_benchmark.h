#ifndef TRX_BENCHMARK_H
#define TRX_BENCHMARK_H

#include <stdio.h>
#include <tee_client_api.h>

void prepare_tee_session(TEEC_Context *ctx, TEEC_Session *sess);
void terminate_tee_session(TEEC_Context *ctx, TEEC_Session *sess);
void terminate_tee_session(TEEC_Context *ctx, TEEC_Session *sess);
void trx_benchmark_write(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds);
void trx_benchmark_read(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds);
void trx_benchmark_gp_write(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds);
void trx_benchmark_gp_read(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds);
void trx_benchmark_share(unsigned long rounds);

static const char usage[] = "usage: %s -o operation -m min -M max -s step -r rounds\n";
static const char write_operation[] = "write";
static const char read_operation[] = "read";
static const char gp_write_operation[] = "gp_write";
static const char gp_read_operation[] = "gp_read";
static const char share_operation[] = "share";
static const char mount_operation[] = "mount";

#endif