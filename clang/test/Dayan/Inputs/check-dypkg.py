import struct
import sys

path = sys.argv[1]
data = open(path, "rb").read()
header = struct.unpack_from("<8sIIQIIQQQIIQIIQIIQQIIQ", data, 0)
(
    magic,
    version,
    header_size,
    total_size,
    flags,
    target_abi,
    object_offset,
    object_size,
    kernel_offset,
    kernel_count,
    _reserved0,
    argument_offset,
    argument_count,
    _reserved1,
    sync_offset,
    sync_count,
    _reserved2,
    string_offset,
    string_size,
    isa_string_offset,
    isa_string_size,
    _reserved3,
) = header

assert magic == b"DAYANPKG", magic
assert version == 5, version
assert header_size == 128, header_size
assert total_size == len(data), (total_size, len(data))
assert flags == 0, flags
assert target_abi == 0, target_abi
assert object_offset == 128, object_offset
assert object_size > 0, object_size
assert kernel_count == 1, kernel_count
assert argument_count == 2, argument_count
assert sync_count == 0, sync_count
assert sync_offset == string_offset, (sync_offset, string_offset)
assert string_offset + string_size == total_size, (string_offset, string_size, total_size)
assert data[isa_string_offset:isa_string_offset + isa_string_size] == b"rv64imaf_zicsr_zifencei\0"

kernel = struct.unpack_from("<IIIIIIIIIIIIQQQQQBBBBI", data, kernel_offset)
(
    kernel_version,
    name_offset,
    entry_offset,
    supported_task_modes,
    first_argument,
    kernel_argument_count,
    first_sync,
    kernel_sync_count,
    max_tasks_per_core,
    max_child_threads,
    kernel_flags,
    _kernel_reserved0,
    stack_size,
    atomic_cache_size,
    printf_buffer_size,
    resident_memory,
    uram_size,
    lsu_tcsr_count,
    mat_tcsr_count,
    vec_tcsr_count,
    macro_tcsr_count,
    _kernel_reserved1,
) = kernel
assert kernel_version == 5, kernel_version
assert data[name_offset:data.index(b"\0", name_offset)] == b"add"
assert data[entry_offset:data.index(b"\0", entry_offset)] == b"_Z3addPii"
assert supported_task_modes == 3, supported_task_modes
assert first_argument == 0, first_argument
assert kernel_argument_count == 2, kernel_argument_count
assert first_sync == 0, first_sync
assert kernel_sync_count == 0, kernel_sync_count
assert max_tasks_per_core == 0, max_tasks_per_core
assert max_child_threads == 1, max_child_threads
assert kernel_flags == 0, kernel_flags
assert stack_size == 0x10000, stack_size
assert atomic_cache_size == 64, atomic_cache_size
assert printf_buffer_size == 0x1000, printf_buffer_size
assert resident_memory == 0, resident_memory
assert uram_size == 32768, uram_size
assert lsu_tcsr_count == 1, lsu_tcsr_count
assert mat_tcsr_count == 2, mat_tcsr_count
assert vec_tcsr_count == 3, vec_tcsr_count
assert macro_tcsr_count == 4, macro_tcsr_count

arg0 = struct.unpack_from("<IIII", data, argument_offset)
arg1 = struct.unpack_from("<IIII", data, argument_offset + 16)
assert arg0 == (2, 8, 4, 3), arg0
assert arg1 == (1, 4, 0, 0), arg1
