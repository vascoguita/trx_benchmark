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
./host/trx_benchmark -o <operation> -m <minimum_value> -M <maximum_value> -s <step>
```

The TRX benchmark supports the following operations:
* write