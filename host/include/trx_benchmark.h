#ifndef TRX_BENCHMARK_H
#define TRX_BENCHMARK_H

#include <stdio.h>
#include <tee_client_api.h>

void prepare_tee_session(TEEC_Context *ctx, TEEC_Session *sess);
void terminate_tee_session(TEEC_Context *ctx, TEEC_Session *sess);
void trx_benchmark_write(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds);
int trx_benchmark_print(uint32_t *report, uint32_t report_size);

static const char usage[] = "usage: %s -o operation -m min -M max -s step -r rounds\n";
static const char write_operation[] = "write";

#endif