#ifndef NPU_HOST_COMPILED_KERNEL_H
#define NPU_HOST_COMPILED_KERNEL_H

#include <stddef.h>
#include <stdint.h>

#include "npu_package.h"
#include "npu_runtime.h"

#ifndef DAYAN_HART_MAX_PROCS
#define DAYAN_HART_MAX_PROCS 16U
#endif
#ifndef DAYAN_KERNEL_MAX_CHILD_THREADS
#define DAYAN_KERNEL_MAX_CHILD_THREADS (DAYAN_HART_MAX_PROCS - 1U)
#endif
#ifndef DAYAN_URAM_ALLOCATION_GRANULE
#define DAYAN_URAM_ALLOCATION_GRANULE (32U * 1024U)
#endif
#ifndef DAYAN_TCSR_LOGICAL_GROUPS
#define DAYAN_TCSR_LOGICAL_GROUPS 8U
#endif
#ifndef DAYAN_KERNEL_RESOURCE_LOCKED_TASK
#define DAYAN_KERNEL_RESOURCE_LOCKED_TASK (1U << 0)
#endif
#define DAYAN_KERNEL_RESOURCE_FLAGS_ALL DAYAN_KERNEL_RESOURCE_LOCKED_TASK

#ifndef DAYAN_KERNEL_RESOURCE_DESC_DEFINED
#define DAYAN_KERNEL_RESOURCE_DESC_DEFINED
typedef struct dayan_kernel_resource_desc {
    uint16_t max_child_threads;
    uint16_t max_tasks_per_core;
    uint32_t flags;
    uint8_t lsu_tcsr_count;
    uint8_t mat_tcsr_count;
    uint8_t vec_tcsr_count;
    uint8_t macro_tcsr_count;
    uint32_t reserved;
    uint64_t atomic_cache_size;
    uint64_t uram_size;
} dayan_kernel_resource_desc_t;
#endif

/*
 * Each __kernel__ must have exactly one descriptor with this macro.  dycc
 * keeps the exact <kernel>__desc symbol in the Device object and folds the
 * same constant values into the Host package metadata.
 */
#if defined(__DAYAN_DEVICE__)
#ifdef __cplusplus
#define DAYAN_KERNEL_DESC_LINKAGE extern "C"
#define DAYAN_KERNEL_DESC_ASSERT(condition, message) static_assert(condition, message)
#else
#define DAYAN_KERNEL_DESC_LINKAGE extern
#define DAYAN_KERNEL_DESC_ASSERT(condition, message) _Static_assert(condition, message)
#endif
#define DAYAN_KERNEL_RESOURCE_DESC(kernel, tasks, child_threads,              \
                                   resource_flags, atomic_bytes, uram_bytes,  \
                                   lsu_tcsr, mat_tcsr, vec_tcsr, macro_tcsr)  \
    DAYAN_KERNEL_DESC_ASSERT(                                                 \
        (child_threads) <= DAYAN_KERNEL_MAX_CHILD_THREADS,                    \
        "kernel child Thread limit exceeds one Hart");                       \
    DAYAN_KERNEL_DESC_ASSERT((tasks) <= 4U,                                  \
                             "kernel tasks-per-Core limit exceeds four");     \
    DAYAN_KERNEL_DESC_ASSERT(                                                \
        ((resource_flags) & ~DAYAN_KERNEL_RESOURCE_FLAGS_ALL) == 0U,          \
        "kernel resource descriptor contains unknown flags");                \
    DAYAN_KERNEL_DESC_ASSERT(                                                \
        !((resource_flags) & DAYAN_KERNEL_RESOURCE_LOCKED_TASK) ||           \
            (tasks) <= 1U,                                                   \
        "locked kernel cannot declare task co-residency");                   \
    DAYAN_KERNEL_DESC_ASSERT(                                                \
        ((uram_bytes) & (DAYAN_URAM_ALLOCATION_GRANULE - 1U)) == 0U,         \
        "kernel URAM size must be 32 KiB aligned");                           \
    DAYAN_KERNEL_DESC_ASSERT((lsu_tcsr) <= DAYAN_TCSR_LOGICAL_GROUPS &&       \
                             (mat_tcsr) <= DAYAN_TCSR_LOGICAL_GROUPS &&       \
                             (vec_tcsr) <= DAYAN_TCSR_LOGICAL_GROUPS &&       \
                             (macro_tcsr) <= DAYAN_TCSR_LOGICAL_GROUPS,       \
                             "kernel TCSR count exceeds eight groups");      \
    DAYAN_KERNEL_DESC_LINKAGE __device__                                      \
    __attribute__((visibility("default"), section(".dayan.kernel_desc")))     \
    const dayan_kernel_resource_desc_t kernel##__desc = {                     \
        (child_threads), (tasks), (resource_flags),                           \
        (lsu_tcsr), (mat_tcsr), (vec_tcsr), (macro_tcsr), 0U,                \
        (atomic_bytes), (uram_bytes)                                           \
    }
#define DAYAN_KERNEL_DESC(kernel, child_threads, atomic_bytes, uram_bytes,     \
                          lsu_tcsr, mat_tcsr, vec_tcsr, macro_tcsr)            \
    DAYAN_KERNEL_RESOURCE_DESC(kernel, 0U, child_threads, 0U, atomic_bytes,    \
                               uram_bytes, lsu_tcsr, mat_tcsr, vec_tcsr,       \
                               macro_tcsr)
#define DAYAN_KERNEL_TASK_DESC(kernel, tasks, child_threads, atomic_bytes,     \
                               uram_bytes, lsu_tcsr, mat_tcsr, vec_tcsr,       \
                               macro_tcsr)                                    \
    DAYAN_KERNEL_DESC_ASSERT((tasks) >= 1U,                                   \
                             "kernel tasks-per-Core limit is at least one");  \
    DAYAN_KERNEL_RESOURCE_DESC(kernel, tasks, child_threads, 0U, atomic_bytes, \
                               uram_bytes, lsu_tcsr, mat_tcsr, vec_tcsr,       \
                               macro_tcsr)
#define DAYAN_KERNEL_LOCKED_TASK_DESC(kernel, child_threads, atomic_bytes,    \
                                      uram_bytes, lsu_tcsr, mat_tcsr,          \
                                      vec_tcsr, macro_tcsr)                   \
    DAYAN_KERNEL_RESOURCE_DESC(kernel, 1U, child_threads,                     \
                               DAYAN_KERNEL_RESOURCE_LOCKED_TASK,             \
                               atomic_bytes, uram_bytes, lsu_tcsr, mat_tcsr,  \
                               vec_tcsr, macro_tcsr)
#else
#define DAYAN_KERNEL_RESOURCE_DESC(kernel, tasks, child_threads, resource_flags, atomic_bytes, uram_bytes, lsu_tcsr, mat_tcsr, vec_tcsr, macro_tcsr)
#define DAYAN_KERNEL_DESC(kernel, child_threads, atomic_bytes, uram_bytes, lsu_tcsr, mat_tcsr, vec_tcsr, macro_tcsr)
#define DAYAN_KERNEL_TASK_DESC(kernel, tasks, child_threads, atomic_bytes, uram_bytes, lsu_tcsr, mat_tcsr, vec_tcsr, macro_tcsr)
#define DAYAN_KERNEL_LOCKED_TASK_DESC(kernel, child_threads, atomic_bytes, uram_bytes, lsu_tcsr, mat_tcsr, vec_tcsr, macro_tcsr)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define NPU_KERNEL_LAUNCH_ATTRS_VERSION 3U
#define NPU_KERNEL_LAUNCH_ASYNC         (1U << 0)

typedef struct npu_kernel_descriptor npu_kernel_descriptor_t;

typedef struct npu_kernel_launch_attrs {
    uint32_t version;
    uint32_t flags;
    npu_context_t *context;
    npu_stream_t *stream;
    NPULaunchConfig config;
    uint32_t timeout_ms;
    uint32_t reserved;
    uint64_t cookie;
    char task_name[NPU_PROC_NAME_SIZE];
    npu_event_t **event_out;
} npu_kernel_launch_attrs_t;

void npu_kernel_launch_attrs_init(npu_kernel_launch_attrs_t *attrs,
                                  npu_context_t *context,
                                  npu_stream_t *stream,
                                  const char *task_name);

/* Entry points used by dycc-generated Host wrappers. */
int npu_kernel_descriptor_create(
    const void *object, size_t object_size, const char *entry_symbol,
    const npu_package_argument_t *arguments, size_t argument_count,
    const dayan_kernel_resource_desc_t *resources,
    npu_kernel_descriptor_t **descriptor_out);
void npu_kernel_descriptor_destroy(npu_kernel_descriptor_t *descriptor);
int npu_kernel_descriptor_launch(npu_kernel_descriptor_t *descriptor,
                                 const npu_kernel_arg_t *args, size_t argc,
                                 const npu_kernel_launch_attrs_t *attrs);

static inline uint64_t npu_kernel_pack_f32(float value)
{
    uint32_t bits;

    __builtin_memcpy(&bits, &value, sizeof(bits));
    return (uint64_t)bits;
}

static inline uint64_t npu_kernel_pack_f64(double value)
{
    uint64_t bits;

    __builtin_memcpy(&bits, &value, sizeof(bits));
    return bits;
}

#ifdef __cplusplus
static_assert(sizeof(dayan_kernel_resource_desc_t) == 32,
              "Dayan kernel resource descriptor ABI");
static_assert(offsetof(dayan_kernel_resource_desc_t, flags) == 4,
              "Dayan kernel resource flags offset");
static_assert(offsetof(dayan_kernel_resource_desc_t, max_tasks_per_core) == 2,
              "Dayan kernel task residency offset");
static_assert(sizeof(npu_kernel_launch_attrs_t) == 80,
              "Dayan kernel launch attributes ABI");
static_assert(offsetof(npu_kernel_launch_attrs_t, task_name) == 56,
              "Dayan kernel launch task name offset");
static_assert(offsetof(npu_kernel_launch_attrs_t, event_out) == 72,
              "Dayan kernel launch event offset");
}
#else
_Static_assert(sizeof(dayan_kernel_resource_desc_t) == 32,
               "Dayan kernel resource descriptor ABI");
_Static_assert(offsetof(dayan_kernel_resource_desc_t, flags) == 4,
               "Dayan kernel resource flags offset");
_Static_assert(offsetof(dayan_kernel_resource_desc_t, max_tasks_per_core) == 2,
               "Dayan kernel task residency offset");
_Static_assert(sizeof(npu_kernel_launch_attrs_t) == 80,
               "Dayan kernel launch attributes ABI");
_Static_assert(offsetof(npu_kernel_launch_attrs_t, task_name) == 56,
               "Dayan kernel launch task name offset");
_Static_assert(offsetof(npu_kernel_launch_attrs_t, event_out) == 72,
               "Dayan kernel launch event offset");
#endif

#endif
