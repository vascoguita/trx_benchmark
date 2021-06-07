#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <tee_client_api.h>

#include <trx_benchmark_ta.h>
#include <trx_benchmark.h>

int main(int argc, char *argv[])
{
    extern char *optarg;
    int opt;
    char *operation = NULL;
    char *min_str = NULL;
    char *max_str = NULL;
    char *step_str = NULL;
    char *rounds_str = NULL;
    unsigned long min, max, step, rounds;
    char *p;

    while ((opt = getopt(argc, argv, "o:m:M:s:r:")) != -1)
    {
        switch (opt)
        {
        case 'o':
            operation = optarg;
            break;
        case 'm':
            min_str = optarg;
            break;
        case 'M':
            max_str = optarg;
            break;
        case 's':
            step_str = optarg;
            break;
        case 'r':
            rounds_str = optarg;
            break;
        case ':':
            errx(1, usage, argv[0]);
            break;
        case '?':
            errx(1, usage, argv[0]);
            break;
        }
    }

    if (!operation || !min_str || !max_str || !step_str || !rounds_str)
    {
        errx(1, usage, argv[0]);
    }

    min = strtoul(min_str, &p, 10);
    if (errno != 0 || *p != '\0')
    {
        errx(1, "failed to read min value. %s", strerror(errno));
    }

    max = strtoul(max_str, &p, 10);
    if (errno != 0 || *p != '\0')
    {
        errx(1, "failed to read max value. %s", strerror(errno));
    }

    step = strtoul(step_str, &p, 10);
    if (errno != 0 || *p != '\0')
    {
        errx(1, "failed to read step value. %s", strerror(errno));
    }

    rounds = strtoul(rounds_str, &p, 10);
    if (errno != 0 || *p != '\0')
    {
        errx(1, "failed to read rounds value. %s", strerror(errno));
    }

    if (!strcmp(operation, write_operation))
    {
        trx_benchmark_write(min, max, step, rounds);
    }
    else
    {
        errx(1, "invalid operation");
    }

    return 0;
}

void prepare_tee_session(TEEC_Context *ctx, TEEC_Session *sess)
{
    TEEC_UUID uuid = TA_TRX_BENCHMARK_UUID;
    uint32_t origin;
    TEEC_Result res;

    res = TEEC_InitializeContext(NULL, ctx);
    if (res != TEEC_SUCCESS)
    {
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);
    }

    res = TEEC_OpenSession(ctx, sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
    if (res != TEEC_SUCCESS)
    {
        errx(1, "TEEC_OpenSession failed with code 0x%x origin 0x%x", res, origin);
    }
}

void terminate_tee_session(TEEC_Context *ctx, TEEC_Session *sess)
{
    TEEC_CloseSession(sess);
    TEEC_FinalizeContext(ctx);
}

void trx_benchmark_write(unsigned long min, unsigned long max, unsigned long step, unsigned long rounds)
{
    TEEC_Result res;
    TEEC_Operation op;
    uint32_t err_origin;
    TEEC_Context ctx;
    TEEC_Session sess;
    uint32_t *report = NULL;
    uint32_t report_size = 0;
    int print_res;

    prepare_tee_session(&ctx, &sess);

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT,
                                     TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE);
    op.params[0].value.a = min;
    op.params[0].value.b = max;
    op.params[1].value.a = step;
    op.params[1].value.b = rounds;
    op.params[2].tmpref.buffer = report;
    op.params[2].tmpref.size = report_size;

    res = TEEC_InvokeCommand(&sess, TA_TRX_BENCHMARK_CMD_WRITE, &op, &err_origin);
    if(res != TEEC_ERROR_SHORT_BUFFER)
    {
        errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x", res, err_origin);
    }

    report_size = op.params[2].tmpref.size;
    report = malloc(report_size);
    if(!report)
    {
        terminate_tee_session(&ctx, &sess);
        errx(1, "failed to allocating report buffer");
    }
    op.params[2].tmpref.buffer = report;
    op.params[2].tmpref.size = report_size;

    res = TEEC_InvokeCommand(&sess, TA_TRX_BENCHMARK_CMD_WRITE, &op, &err_origin);
    if(res != TEEC_SUCCESS)
    {
        free(report);
        errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x", res, err_origin);
    }

    print_res = trx_benchmark_print(report, report_size);
    if(print_res)
    {
        free(report);
        terminate_tee_session(&ctx, &sess);
        errx(1, "failed to print report");
    }

    free(report);
    terminate_tee_session(&ctx, &sess);
}

int trx_benchmark_print(uint32_t *report, uint32_t report_size)
{
    uint32_t report_len, report_index;

    report_len = report_size / sizeof(uint32_t);

    if(report_len % 2)
    {
        return 1;
    }

    printf("size (B), execution time (ms)\n");

    for(report_index = 0; report_index < report_len; report_index += 2)
    {
        printf("%" PRIu32 ",%" PRIu32 "\n", report[report_index], report[report_index + 1]);
    }

    return 0;
}