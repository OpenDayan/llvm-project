#ifndef NPU_HOST_RUNTIME_H
#define NPU_HOST_RUNTIME_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NPU_RUNTIME_OPTIONS_VERSION 5U
#define NPU_CONTEXT_DESC_VERSION    1U
#define NPU_BUFFER_DESC_VERSION     1U
#define NPU_STREAM_DESC_VERSION     1U
#define NPU_LAUNCH_DESC_VERSION     3U
#define NPU_TIMEOUT_INFINITE        UINT32_MAX
#define NPU_KERNEL_MAX_ARGS         8U
#define NPU_PROC_NAME_SIZE          16U
#define NPU_DEFAULT_SERVER_SOCKET   "/tmp/dayan-server.sock"
#define NPU_RUNTIME_OUTPUT_CLEAR    (1U << 0)
#define NPU_RUNTIME_OUTPUT_CONTINUED (1U << 1)
#define NPU_PROFILE_OPTIONS_VERSION 1U
#define NPU_PROFILE_DEFAULT_BUFFER_SIZE (16ULL * 1024ULL * 1024ULL)
#define NPU_PROFILE_BATCH_END       (1U << 0)
#define NPU_PROFILE_MAX_BATCH_BYTES (64U * 1024U)
#define NPU_PROFILE_RECORD_SIZE     8U
#define NPU_PROFILE_SLOT_DEFINE_SIZE 24U
#define NPU_PROFILE_COUNTER_MASK    0x0000ffffffffffffULL
#define NPU_PROFILE_SLOT_SHIFT      48U
#define NPU_PROFILE_STATE_SHIFT     56U
#define NPU_PROFILE_SLOT_SCHEDULER  0U
#define NPU_PROFILE_SLOT_USER_BASE  1U
#define NPU_PROFILE_SLOT_RPC_RECEIVE 17U
#define NPU_PROFILE_SLOT_RPC_SEND   18U
#define NPU_PROFILE_SLOT_FLUSH      19U
#define NPU_PROFILE_SLOT_COUNT      20U

typedef struct NPUTaskDim3 {
    uint32_t x;
    uint32_t y;
    uint32_t z;
} NPUTaskDim3;

#ifdef __cplusplus
enum class NPUTaskMode : uint32_t {
    kSingleCore = 1,
    kFourCore = 4,
};
#else
typedef uint32_t NPUTaskMode;
enum {
    NPU_TASK_MODE_SINGLE_CORE = 1,
    NPU_TASK_MODE_FOUR_CORE = 4,
};
#endif

typedef struct NPULaunchConfig {
    NPUTaskDim3 task_dim;
    NPUTaskMode task_mode;
} NPULaunchConfig;

typedef struct npu_runtime npu_runtime_t;
typedef struct npu_context npu_context_t;
typedef struct npu_buffer npu_buffer_t;
typedef struct npu_stream npu_stream_t;
typedef struct npu_module npu_module_t;
typedef struct npu_kernel npu_kernel_t;
typedef struct npu_event npu_event_t;
typedef struct npu_profile npu_profile_t;

typedef void (*npu_runtime_output_fn)(void *context, uint32_t cluster,
                                      uint32_t core, uint32_t proc,
                                      const void *data, size_t size,
                                      uint32_t flags);

typedef enum npu_buffer_flags {
    NPU_BUFFER_DEVICE_LOCAL = 1U << 0,
    NPU_BUFFER_COHERENT = 1U << 1,
    NPU_BUFFER_READ = 1U << 2,
    NPU_BUFFER_WRITE = 1U << 3,
} npu_buffer_flags_t;

typedef enum npu_kernel_arg_kind {
    NPU_KERNEL_ARG_VALUE = 1,
    NPU_KERNEL_ARG_BUFFER = 2,
} npu_kernel_arg_kind_t;

typedef enum npu_kernel_arg_access {
    NPU_KERNEL_ARG_READ = 1U << 0,
    NPU_KERNEL_ARG_WRITE = 1U << 1,
} npu_kernel_arg_access_t;

typedef enum npu_event_state {
    NPU_EVENT_QUEUED = 0,
    NPU_EVENT_RUNNING = 1,
    NPU_EVENT_COMPLETE = 2,
    NPU_EVENT_FAILED = 3,
    NPU_EVENT_CANCELLED = 4,
} npu_event_state_t;

/*
 * Device stdout is delivered when Runtime observes command progress through
 * Event query/wait or Stream synchronization.  Long waits poll output at a
 * bounded interval, so Device fflush does not wait for kernel completion.
 * The callback runs in the calling application thread and may be null to
 * discard Device output.
 */
typedef struct npu_runtime_options {
    uint32_t version;
    uint32_t flags;
    const char *server_socket;
    uint32_t connect_timeout_ms;
    uint32_t default_timeout_ms;
    npu_runtime_output_fn output;
    void *output_context;
} npu_runtime_options_t;

typedef struct npu_context_desc {
    uint32_t version;
    uint32_t flags;
    uint64_t memory_quota;
    uint32_t max_pending_commands;
    uint32_t reserved;
} npu_context_desc_t;

typedef struct npu_buffer_desc {
    uint32_t version;
    uint32_t flags;
    uint64_t size;
    uint64_t alignment;
} npu_buffer_desc_t;

typedef struct npu_stream_desc {
    uint32_t version;
    uint32_t flags;
    int32_t priority;
    uint32_t reserved;
} npu_stream_desc_t;

typedef struct npu_kernel_arg {
    uint32_t kind;
    uint32_t access;
    uint32_t size;
    uint32_t reserved;
    union {
        uint64_t value;
        struct {
            npu_buffer_t *buffer;
            uint64_t offset;
        } buffer;
    } data;
} npu_kernel_arg_t;

typedef struct npu_launch_desc {
    uint32_t version;
    uint32_t flags;
    uint32_t arg_count;
    uint32_t reserved;
    NPULaunchConfig config;
    uint64_t cookie;
    char task_name[NPU_PROC_NAME_SIZE];
    npu_kernel_arg_t args[NPU_KERNEL_MAX_ARGS];
} npu_launch_desc_t;

typedef struct npu_event_info {
    uint32_t state;
    uint32_t command_type;
    int64_t status;
    uint64_t sequence;
    uint64_t cookie;
} npu_event_info_t;

typedef struct npu_kernel_info {
    uint32_t supported_task_modes;
    uint32_t max_tasks_per_core;
    uint32_t max_child_threads;
    uint32_t arg_count;
    uint32_t sync_resource_count;
    uint32_t flags;
    uint8_t lsu_tcsr_count;
    uint8_t mat_tcsr_count;
    uint8_t vec_tcsr_count;
    uint8_t macro_tcsr_count;
    uint32_t reserved;
    uint64_t stack_size;
    uint64_t atomic_cache_size;
    uint64_t resident_memory;
    uint64_t uram_size;
} npu_kernel_info_t;

#define NPU_KERNEL_LOCKED_TASK (1U << 0)

typedef struct npu_profile_options {
    uint32_t version;
    uint32_t flags;
    uint64_t buffer_size;
} npu_profile_options_t;

typedef struct npu_profile_record {
    /* [47:0] counter, [55:48] slot, [63:56] state or record tag. */
    uint64_t value;
} npu_profile_record_t;

typedef struct npu_profile_slot_define {
    npu_profile_record_t header;
    char name[NPU_PROC_NAME_SIZE];
} npu_profile_slot_define_t;

typedef enum npu_profile_proc_state {
    NPU_PROFILE_PROC_RUNNING = 0,
    NPU_PROFILE_PROC_READY = 1,
    NPU_PROFILE_PROC_WAIT_POLL = 2,
    NPU_PROFILE_PROC_WAIT_EVENT = 3,
    NPU_PROFILE_PROC_WAIT_START = 4,
    NPU_PROFILE_PROC_WAIT_EXIT = 5,
    NPU_PROFILE_PROC_WAIT_PRODUCE = 6,
    NPU_PROFILE_PROC_KILL_WAIT_PRODUCE = 7,
} npu_profile_proc_state_t;

typedef enum npu_profile_record_tag {
    NPU_PROFILE_RECORD_SLOT_DEFINE = 0x80,
} npu_profile_record_tag_t;

static inline uint64_t npu_profile_record_counter(uint64_t value)
{
    return value & NPU_PROFILE_COUNTER_MASK;
}

static inline uint8_t npu_profile_record_slot(uint64_t value)
{
    return (uint8_t)(value >> NPU_PROFILE_SLOT_SHIFT);
}

static inline uint8_t npu_profile_record_state(uint64_t value)
{
    return (uint8_t)(value >> NPU_PROFILE_STATE_SHIFT);
}

static inline size_t npu_profile_record_size(uint64_t value)
{
    uint8_t state = npu_profile_record_state(value);

    if (state <= NPU_PROFILE_PROC_KILL_WAIT_PRODUCE)
        return NPU_PROFILE_RECORD_SIZE;
    if (state == NPU_PROFILE_RECORD_SLOT_DEFINE)
        return NPU_PROFILE_SLOT_DEFINE_SIZE;
    return 0;
}

typedef struct npu_profile_batch_info {
    uint32_t cluster;
    uint32_t hart;
    /* Number of raw trace bytes returned, not a record count. */
    uint32_t byte_count;
    uint32_t flags;
} npu_profile_batch_info_t;

void npu_runtime_options_init(npu_runtime_options_t *options);
void npu_context_desc_init(npu_context_desc_t *desc);
void npu_buffer_desc_init(npu_buffer_desc_t *desc, uint64_t size);
void npu_stream_desc_init(npu_stream_desc_t *desc);
void npu_launch_config_init(NPULaunchConfig *config);
/* task_name must be nonempty, NUL-terminated, and fit in 16 bytes. */
void npu_launch_desc_init(npu_launch_desc_t *desc, const char *task_name);
void npu_profile_options_init(npu_profile_options_t *options);
void npu_kernel_arg_value(npu_kernel_arg_t *arg, uint64_t value,
                          uint32_t size);
void npu_kernel_arg_buffer(npu_kernel_arg_t *arg, npu_buffer_t *buffer,
                           uint64_t offset, uint32_t access);

/* Runtime is a socket client.  It never opens register or interrupt devices. */
int npu_runtime_open(npu_runtime_t **runtime_out,
                     const npu_runtime_options_t *options);
int npu_runtime_close(npu_runtime_t *runtime);

/* Destroying a Context cancels its queued work and reclaims all child objects. */
int npu_context_create(npu_runtime_t *runtime,
                       const npu_context_desc_t *desc,
                       npu_context_t **context_out);
int npu_context_destroy(npu_context_t *context);

int npu_buffer_create(npu_context_t *context, const npu_buffer_desc_t *desc,
                      npu_buffer_t **buffer_out);
int npu_buffer_create_view(npu_buffer_t *buffer, uint64_t offset,
                           uint64_t size, npu_buffer_t **view_out);
int npu_buffer_destroy(npu_buffer_t *buffer);
int npu_buffer_get_size(const npu_buffer_t *buffer, uint64_t *size_out);

int npu_stream_create(npu_context_t *context, const npu_stream_desc_t *desc,
                      npu_stream_t **stream_out);
int npu_stream_destroy(npu_stream_t *stream);
int npu_stream_synchronize(npu_stream_t *stream, uint32_t timeout_ms);
int npu_stream_wait_event(npu_stream_t *stream, npu_event_t *event);

/*
 * H2D/D2H payloads do not travel over the socket.  Service returns a scoped
 * token; libDayanRuntime's data worker applies it directly through HAL.  The
 * Host pointer must remain valid until the returned Event completes.
 */
int npu_memcpy_h2d_async(npu_stream_t *stream, npu_buffer_t *destination,
                         uint64_t destination_offset, const void *source,
                         size_t size, npu_event_t **event_out);
int npu_memcpy_d2h_async(npu_stream_t *stream, void *destination,
                         npu_buffer_t *source, uint64_t source_offset,
                         size_t size, npu_event_t **event_out);
int npu_memcpy_d2d_async(npu_stream_t *stream, npu_buffer_t *destination,
                         uint64_t destination_offset, npu_buffer_t *source,
                         uint64_t source_offset, size_t size,
                         npu_event_t **event_out);

int npu_module_load(npu_context_t *context, const void *package_image,
                    size_t package_size, npu_module_t **module_out);
int npu_module_load_file(npu_context_t *context, const char *path,
                         npu_module_t **module_out);
int npu_module_unload(npu_module_t *module);
int npu_module_get_kernel(npu_module_t *module, const char *name,
                          npu_kernel_t **kernel_out);
int npu_kernel_release(npu_kernel_t *kernel);
int npu_kernel_get_info(npu_kernel_t *kernel, npu_kernel_info_t *info);
int npu_kernel_launch(npu_stream_t *stream, npu_kernel_t *kernel,
                      const npu_launch_desc_t *desc,
                      npu_event_t **event_out);

int npu_event_record(npu_stream_t *stream, npu_event_t **event_out);
int npu_event_query(npu_event_t *event, npu_event_info_t *info);
int npu_event_wait(npu_event_t *event, uint32_t timeout_ms,
                   npu_event_info_t *info);
int npu_event_destroy(npu_event_t *event);

/* Only one Device-wide proc profile session may be active at a time. */
int npu_profile_start(npu_runtime_t *runtime,
                      const npu_profile_options_t *options,
                      npu_profile_t **profile_out);
/* Raw records are tightly packed; one record may span consecutive reads. */
int npu_profile_read(npu_profile_t *profile, void *data,
                     uint32_t capacity_bytes, uint32_t timeout_ms,
                     npu_profile_batch_info_t *batch_out);
int npu_profile_stop(npu_profile_t *profile);
int npu_profile_destroy(npu_profile_t *profile);

#ifdef __cplusplus
static_assert(sizeof(NPUTaskDim3) == 12, "NPU task dimension ABI");
static_assert(sizeof(NPULaunchConfig) == 16, "NPU launch config ABI");
static_assert(sizeof(npu_launch_desc_t) == 312, "NPU launch descriptor ABI");
static_assert(offsetof(npu_launch_desc_t, task_name) == 40,
              "NPU launch task name offset");
static_assert(offsetof(npu_launch_desc_t, args) == 56,
              "NPU launch argument offset");
static_assert(sizeof(npu_kernel_info_t) == 64, "NPU kernel info ABI");
static_assert(sizeof(npu_profile_record_t) == NPU_PROFILE_RECORD_SIZE,
              "NPU profile record ABI");
static_assert(sizeof(npu_profile_slot_define_t) ==
                  NPU_PROFILE_SLOT_DEFINE_SIZE,
              "NPU profile slot definition ABI");
static_assert(offsetof(npu_profile_slot_define_t, name) == 8,
              "NPU profile slot definition name offset");
static_assert(sizeof(npu_profile_batch_info_t) == 16,
              "NPU profile batch ABI");
#else
_Static_assert(sizeof(NPUTaskDim3) == 12, "NPU task dimension ABI");
_Static_assert(sizeof(NPULaunchConfig) == 16, "NPU launch config ABI");
_Static_assert(sizeof(npu_launch_desc_t) == 312,
               "NPU launch descriptor ABI");
_Static_assert(offsetof(npu_launch_desc_t, task_name) == 40,
               "NPU launch task name offset");
_Static_assert(offsetof(npu_launch_desc_t, args) == 56,
               "NPU launch argument offset");
_Static_assert(sizeof(npu_kernel_info_t) == 64, "NPU kernel info ABI");
_Static_assert(sizeof(npu_profile_record_t) == NPU_PROFILE_RECORD_SIZE,
               "NPU profile record ABI");
_Static_assert(sizeof(npu_profile_slot_define_t) ==
                   NPU_PROFILE_SLOT_DEFINE_SIZE,
               "NPU profile slot definition ABI");
_Static_assert(offsetof(npu_profile_slot_define_t, name) == 8,
               "NPU profile slot definition name offset");
_Static_assert(sizeof(npu_profile_batch_info_t) == 16,
               "NPU profile batch ABI");
#endif

#ifdef __cplusplus
}
#endif

#endif
