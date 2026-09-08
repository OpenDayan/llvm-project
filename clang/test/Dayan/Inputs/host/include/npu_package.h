#ifndef NPU_HOST_PACKAGE_H
#define NPU_HOST_PACKAGE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NPU_PACKAGE_MAGIC             "DAYANPKG"
#define NPU_PACKAGE_VERSION           5U
#define NPU_PACKAGE_HEADER_SIZE       128U
#define NPU_PACKAGE_KERNEL_VERSION    5U
#define NPU_PACKAGE_MAX_KERNELS       4096U
#define NPU_PACKAGE_MAX_ARGS          8U
#define NPU_PACKAGE_HART_MAX_PROCS    16U
#define NPU_PACKAGE_MAX_CHILD_THREADS (NPU_PACKAGE_HART_MAX_PROCS - 1U)
#define NPU_PACKAGE_PROC_MIN_STACK_SIZE 1024U
#define NPU_PACKAGE_PROC_STACK_SIZE   0x10000U
#define NPU_PACKAGE_PROC_PRINTF_SIZE  0x1000U
#define NPU_PACKAGE_URAM_GRANULE      (32U * 1024U)
#define NPU_PACKAGE_TCSR_MAX_COUNT    8U
#define NPU_PACKAGE_ISA_RV64_DAYAN    "rv64imaf_zicsr_zifencei"

#define NPU_PACKAGE_IMMUTABLE_MODULE  (1U << 0)

#define NPU_PACKAGE_TASK_MODE_SINGLE_CORE (1U << 0)
#define NPU_PACKAGE_TASK_MODE_FOUR_CORE   (1U << 1)
#define NPU_PACKAGE_TASK_MODE_ALL \
    (NPU_PACKAGE_TASK_MODE_SINGLE_CORE | NPU_PACKAGE_TASK_MODE_FOUR_CORE)

#define NPU_PACKAGE_KERNEL_LOCKED_TASK (1U << 0)
#define NPU_PACKAGE_KERNEL_FLAGS_ALL   NPU_PACKAGE_KERNEL_LOCKED_TASK

typedef enum npu_package_arg_kind {
    NPU_PACKAGE_ARG_VALUE = 1,
    NPU_PACKAGE_ARG_BUFFER = 2,
} npu_package_arg_kind_t;

#define NPU_PACKAGE_ARG_READ  (1U << 0)
#define NPU_PACKAGE_ARG_WRITE (1U << 1)

typedef enum npu_package_sync_type {
    NPU_PACKAGE_SYNC_SHARED_RESOURCE = 1,
} npu_package_sync_type_t;

/* All offsets are from the beginning of the package and use little endian. */
typedef struct npu_package_header {
    unsigned char magic[8];
    uint32_t version;
    uint32_t header_size;
    uint64_t total_size;
    uint32_t flags;
    uint32_t target_abi;
    uint64_t object_offset;
    uint64_t object_size;
    uint64_t kernel_offset;
    uint32_t kernel_count;
    uint32_t reserved0;
    uint64_t argument_offset;
    uint32_t argument_count;
    uint32_t reserved1;
    uint64_t sync_offset;
    uint32_t sync_count;
    uint32_t reserved2;
    uint64_t string_offset;
    uint64_t string_size;
    uint32_t isa_string_offset;
    uint32_t isa_string_size;
    uint64_t reserved[1];
} npu_package_header_t;

typedef struct npu_package_kernel {
    uint32_t version;
    uint32_t name_offset;
    uint32_t entry_offset;
    uint32_t supported_task_modes;
    uint32_t first_argument;
    uint32_t argument_count;
    uint32_t first_sync;
    uint32_t sync_count;
    uint32_t max_tasks_per_core;
    uint32_t max_child_threads;
    uint32_t flags;
    uint32_t reserved0;
    uint64_t stack_size;
    uint64_t atomic_cache_size;
    uint64_t printf_buffer_size;
    uint64_t resident_memory;
    uint64_t uram_size;
    uint8_t lsu_tcsr_count;
    uint8_t mat_tcsr_count;
    uint8_t vec_tcsr_count;
    uint8_t macro_tcsr_count;
    uint32_t reserved1;
} npu_package_kernel_t;

typedef struct npu_package_argument {
    uint32_t kind;
    uint32_t size;
    uint32_t alignment;
    uint32_t access;
} npu_package_argument_t;

typedef struct npu_package_sync {
    uint32_t type;
    uint32_t count;
    uint32_t producer_count;
    uint32_t consumer_count;
    uint64_t flags;
    uint64_t reserved;
} npu_package_sync_t;

typedef struct npu_package_kernel_desc {
    const char *name;
    const char *entry_symbol;
    uint32_t supported_task_modes;
    uint32_t max_tasks_per_core;
    uint32_t max_child_threads;
    uint32_t flags;
    uint8_t lsu_tcsr_count;
    uint8_t mat_tcsr_count;
    uint8_t vec_tcsr_count;
    uint8_t macro_tcsr_count;
    uint64_t stack_size;
    uint64_t atomic_cache_size;
    uint64_t printf_buffer_size;
    uint64_t resident_memory;
    uint64_t uram_size;
    const npu_package_argument_t *arguments;
    size_t argument_count;
    const npu_package_sync_t *sync_resources;
    size_t sync_count;
} npu_package_kernel_desc_t;

#ifdef __cplusplus
static_assert(sizeof(npu_package_header_t) == NPU_PACKAGE_HEADER_SIZE,
              "Dayan package header ABI");
static_assert(sizeof(npu_package_kernel_t) == 96,
              "Dayan package kernel ABI");
static_assert(sizeof(npu_package_argument_t) == 16,
              "Dayan package argument ABI");
static_assert(sizeof(npu_package_sync_t) == 32,
              "Dayan package sync ABI");
#else
_Static_assert(sizeof(npu_package_header_t) == NPU_PACKAGE_HEADER_SIZE,
               "Dayan package header ABI");
_Static_assert(sizeof(npu_package_kernel_t) == 96,
               "Dayan package kernel ABI");
_Static_assert(sizeof(npu_package_argument_t) == 16,
               "Dayan package argument ABI");
_Static_assert(sizeof(npu_package_sync_t) == 32,
               "Dayan package sync ABI");
#endif

/* Utility used by dycc wrappers and standalone package producers. */
int npu_package_build(const void *riscv_object, size_t object_size,
                      const npu_package_kernel_desc_t *kernels,
                      size_t kernel_count, uint32_t flags,
                      void **package_out, size_t *package_size_out);
void npu_package_free(void *package_image);

#ifdef __cplusplus
}
#endif

#endif
