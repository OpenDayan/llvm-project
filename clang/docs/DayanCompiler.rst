Dayan compiler driver
=====================

``dycc`` is the Dayan heterogeneous C++ compiler driver. It is built as part of
Clang and reuses Clang's AST and Tooling libraries to split a translation unit
into Host and RISC-V Device sources.

Architecture
------------

The implementation has two layers:

* ``clangDayan`` owns annotation interpretation, kernel validation, resource
  descriptor validation, wrapper generation, and Host/Device source rewriting.
  Its public entry point is ``clang/Dayan/Dayan.h``.
* ``dycc`` owns command-line parsing, SDK discovery, invocation of Host and
  Device Clang, relocatable Device linking, embedding the Device image, and
  optional Host linking.

Keeping the source transformation in a library allows tests and future Clang
frontends to reuse it without invoking a nested ``dycc`` process.

SDK layout
----------

``dycc`` still consumes the Dayan runtime SDK. The path passed to
``--dayan-root`` must contain::

  device/include/api/
  host/include/dayan_kernel.h
  host/include/npu_package.h
  host/include/npu_runtime.h
  host/dycc/include/dayan_compiler.h

An installed SDK may instead place the Device headers directly in
``include/api``. The driver also checks ``DAYAN_ROOT`` and the standard
``lib/dayan``, ``lib64/dayan``, and ``share/dayan`` locations relative to its
executable.

Examples
--------

Compile one Dayan source into a Host relocatable object containing the Device
object::

  dycc --dayan-root /opt/dayan -c operator.cpp -o operator.o

Compile and link multiple Dayan sources into a Host executable.  In this mode
``dycc`` compiles every split Device translation unit, performs a relocatable
RISC-V Device link, and embeds the single linked Device image into the final
Host link.  This allows a ``__kernel__`` in one source file to call
``__device__`` helpers defined in another source file, while all Host wrappers
share the same linked Device image::

  dycc --dayan-root /opt/dayan kernels.cpp helpers.cpp main.cpp -o app

Prebuilt ``.o`` and ``.a`` inputs passed during this Host link are currently
Host-only inputs.  Compile-only objects still embed their own per-translation-unit
Device object for compatibility; extracting and relinking Device payloads from
precompiled ``dycc -c`` objects is a follow-up feature.

Also emit a standalone Dayan package for the same translation unit.  The
recommended file suffix is ``.dypkg``.  The package contains the compiled
RISC-V Device object plus the kernel table, argument metadata, resource
metadata, and target ISA string consumed by the NPU module-loading runtime::

  dycc --dayan-root /opt/dayan -c operator.cpp -o operator.o \
       --emit-package operator.dypkg

With ``-c``, package output is currently limited to one input source.  During
executable linking, package output may cover multiple Dayan source inputs and
uses the linked Device image plus the merged kernel metadata.  Package metadata
comes from the existing
``DAYAN_KERNEL_DESC``/``DAYAN_KERNEL_TASK_DESC``/``DAYAN_KERNEL_LOCKED_TASK_DESC``
resource descriptor macros.  The package records the current resource descriptor
ABI, including child thread limits, task co-residency, TCSR group counts, atomic
cache size, and 32 KiB-aligned URAM requirements.

Generate and retain the two split sources without compiling them::

  dycc --dayan-root /opt/dayan --split-only --save-temps=split operator.cpp

Device inline assembly
----------------------

Device code may use Clang's GNU-style RISC-V inline assembly.  Dayan ``tx.*``
custom instruction mnemonics are accepted by the RISC-V assembler and
printer when the corresponding ``txbase``, ``txlsu``, ``txmat``, ``txvec``, or
``txmacro`` target feature is enabled.  ``dycc`` enables the current Dayan
feature set for Device compilation by default::

  __device__ int increment(int value) {
    asm volatile("addi %0, %0, 1" : "+r"(value));
    asm volatile("tx.lsu.tcsr.write %0, 14" : : "r"(value));
    return value;
  }

This is RISC-V/Dayan ISA assembly, not CUDA PTX inline assembly.  Normal Clang
operand constraints and clobbers apply.  Resource descriptors must still
reserve any TCSR groups and other NPU resources used by inline instructions.

Compatibility
-------------

The initial in-tree implementation intentionally preserves the command-line
and generated-wrapper ABI of the standalone ``npu-firmware/host/dycc`` tool.
The Dayan SDK remains a separate project; only compiler-specific logic and the
compiler executable live in ``llvm-project``.
