# TRX Benchmark

This repository holds a trusted application and a REE client application for assessing the performance of TRX.

# Compile

Compile the TA application and the REE client application by running the following command on the project directory:

```shell
make CROSS_COMPILE=<compiler> PLATFORM=<platform> TA_DEV_KIT_DIR=<ta_dev_kit_dir> TEEC_EXPORT=<optee_client>/out/export/usr
```

# Install

Open the project directory on the destination platform and run the following command:

```shell
./scripts/setup
```

# Run

Open the project directory on the destination platform and run the following command:

```shell
./host/trx_benchmark -o <operation> -m <minimum_value> -M <maximum_value> -s <step> -r <rounds>
```

The TRX benchmark supports the following operations:
* write
* read
* gp_write
* gp_read
* share (no -m -M -s values required)
* mount (no -m -M -s values required)
* pop_write_best
* pop_write_worst
* pop_read_best
* pop_read_worst
* gp_pop_write
* gp_pop_read