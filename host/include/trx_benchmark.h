#ifndef TRX_BENCHMARK_H
#define TRX_BENCHMARK_H

#include <stdio.h>
#include <tee_client_api.h>

void prepare_tee_session(TEEC_Context *ctx, TEEC_Session *sess);
void terminate_tee_session(TEEC_Context *ctx, TEEC_Session *sess);
void terminate_tee_session(TEEC_Context *ctx, TEEC_Session *sess);

int trx_benchmark_print(uint32_t *report, uint32_t report_size, unsigned long rounds);

void trx_benchmark_write(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds);
void trx_benchmark_read(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds);
void trx_benchmark_gp_write(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds);
void trx_benchmark_gp_read(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds);
void trx_benchmark_share(unsigned long rounds);
void trx_benchmark_mount(unsigned long rounds);

int trx_benchmark_pop_print(uint32_t *report, uint32_t report_size, unsigned long rounds);

void trx_benchmark_pop_write(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds);
void trx_benchmark_pop_read(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds);
void trx_benchmark_pop_write_best(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds);
void trx_benchmark_pop_read_best(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds);
void trx_benchmark_pop_write_worst(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds);
void trx_benchmark_pop_read_worst(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds);
void trx_benchmark_gp_pop_write(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds);
void trx_benchmark_gp_pop_read(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds);


static const char usage[] = "usage: %s -o operation -m min -M max -s step -r rounds\n";
static const char write_operation[] = "write";
static const char read_operation[] = "read";
static const char gp_write_operation[] = "gp_write";
static const char gp_read_operation[] = "gp_read";
static const char share_operation[] = "share";
static const char mount_operation[] = "mount";

static const char pop_write_operation[] = "pop_write";
static const char pop_read_operation[] = "pop_read";
static const char pop_write_best_operation[] = "pop_write_best";
static const char pop_read_best_operation[] = "pop_read_best";
static const char pop_write_worst_operation[] = "pop_write_worst";
static const char pop_read_worst_operation[] = "pop_read_worst";
static const char gp_pop_write_operation[] = "gp_pop_write";
static const char gp_pop_read_operation[] = "gp_pop_read";

#endif