#ifndef DAYAN_COMPILER_ATTRIBUTES_H
#define DAYAN_COMPILER_ATTRIBUTES_H

/*
 * dycc force-includes this file while it builds the split AST.  The final
 * Host and Device compilations replace these macros with their code-generation
 * definitions, so the annotations never become part of the public ABI.
 */
#ifndef __host__
#define __host__ __attribute__((annotate("dayan.host")))
#endif

#ifndef __device__
#define __device__ __attribute__((annotate("dayan.device")))
#endif

#ifndef __kernel__
#define __kernel__ __attribute__((annotate("dayan.kernel")))
#endif

#endif
