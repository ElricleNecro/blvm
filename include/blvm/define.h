#ifndef DEFINE_H_76PABUD2
#define DEFINE_H_76PABUD2

#include <assert.h>
#include <stdint.h>

#ifndef BLISP_WORD_SIZE
#define BLISP_WORD_SIZE 8
#endif

#ifndef BLISP_STACK_CAPACITY
#define BLISP_STACK_CAPACITY 1024
#endif

#ifndef BLISP_PROGRAM_CAPACITY
#define BLISP_PROGRAM_CAPACITY 1024
#endif

#ifndef BLISP_NATIVES_CAPACITY
#define BLISP_NATIVES_CAPACITY 1024
#endif

#ifndef BLISP_STATIC_MEMORY_CAPACITY
#define BLISP_STATIC_MEMORY_CAPACITY (640 * 1024)
#endif

#define STATIC_ARRAY_SIZE(xs) (sizeof(xs) / sizeof((xs)[0]))

#define INFINITE_FIB_NUMBER { MAKE_INST_PUSH(0), MAKE_INST_PUSH(1), MAKE_INST_DUP(1), MAKE_INST_DUP(1), MAKE_INST_ADD, MAKE_INST_JMP(2), MAKE_INST_HALT, }

#ifndef _STRUCT_OPTIONS
#	if defined(__GNUC__) || defined(__clang__)
#		define _STRUCT_OPTIONS __attribute__((packed))
#	else
#		warning "Packed attributes for struct is not available/supported for this compiler."
#		define _STRUCT_OPTIONS
#	endif
#endif

#ifndef BL_MAGIC
#define BL_MAGIC 0x4C42
#endif

#ifndef BL_VERSION
#define BL_VERSION 1
#endif

typedef union word_u {
	uint64_t u64;
	int64_t i64;
	double f64;
	void *ptr;
} Word;

static_assert(sizeof(Word) == BLISP_WORD_SIZE, "Blvm is expected to be compiled on a 64bits system.");

#endif /* end of include guard: DEFINE_H_76PABUD2 */
