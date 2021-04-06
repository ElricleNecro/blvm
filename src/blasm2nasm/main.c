#include <stdio.h>
#include <stdlib.h>

#include "Parser.h"

#include "blvm/blvm.h"
#include "blvm/define.h"
#include "blvm/records.h"
#include "blvm/stringview.h"
#include "blvm/translation.h"

IncludeList clist_to_include(CList include_paths_opt) {
	IncludeList include_paths = NULL;
	if( include_paths_opt != NULL ) {
		for(CList tmp = include_paths_opt; tmp != NULL; tmp = tmp->next) {
			if( include_paths == NULL ) {
				include_paths = malloc(sizeof(struct _include_list_t));
				include_paths->fname = tmp->opt;
				include_paths->next = NULL;
				include_paths->end = include_paths;
			} else {
				include_paths->end = malloc(sizeof(struct _include_list_t));
				include_paths->end->fname = tmp->opt;
				include_paths->end->next = NULL;
				for(IncludeList iter = include_paths->next; iter != NULL; iter = iter->next)
					iter->end = include_paths->end;
			}
		}
	}

	return include_paths;
}

void gen_print_i64(FILE *stream) {
	fprintf(stream, "\tprint_i64:\n");
	fprintf(stream, "\t\t; extracting from BLISP stack\n");
	fprintf(stream, "\t\tmov rsi, [stack_top]\n");
	fprintf(stream, "\t\tsub rsi, BLISP_WORD_SIZE\n");
	fprintf(stream, "\t\tmov rax, [rsi]\n");
	fprintf(stream, "\t\tmov [stack_top], rsi\n");
	fprintf(stream, "\t\t; rax has the value we need to print\n");
	fprintf(stream, "\t\tmov rdi, 0\t; rdi is the counter of chars\n");
	fprintf(stream, "\t\t; add new line\n");
	fprintf(stream, "\t\tdec rsp\n");
	fprintf(stream, "\t\tinc rdi\n");
	fprintf(stream, "\t\tmov BYTE [rsp], 10\n");
	fprintf(stream, "\t.loop:\n");
	fprintf(stream, "\t\txor rdx, rdx\n");
	fprintf(stream, "\t\tmov rbx, 10\n");
	fprintf(stream, "\t\tdiv rbx\n");
	fprintf(stream, "\t\tadd rdx, '0'\n");
	fprintf(stream, "\t\tdec rsp\n");
	fprintf(stream, "\t\tinc rdi\n");
	fprintf(stream, "\t\tmov [rsp], dl\n");
	fprintf(stream, "\t\tcmp rax, 0\n");
	fprintf(stream, "\t\tjne .loop\n");
	fprintf(stream, "\t\t; rsp is the beginning of the buffer\n");
	fprintf(stream, "\t\t; rdi contains the size of the buf\n");
	fprintf(stream, "\t\tmov rbx, rdi\n");
	fprintf(stream, "\t\t; write(STDOUT, buf, buf_size)\n");
	fprintf(stream, "\t\tmov rax, SYS_WRITE\n");
	fprintf(stream, "\t\tmov rdi, STDOUT\n");
	fprintf(stream, "\t\tmov rsi, rsp\n");
	fprintf(stream, "\t\tmov rdx, rbx\n");
	fprintf(stream, "\t\tsyscall\n");
	fprintf(stream, "\t\tadd rsp, rbx\n");
	fprintf(stream, "\t\tret\n");
}

int main(int argc, const char **argv) {
	Args *args = Args_New();
	Args_Error err;

	const char *output = "blasm.m";
	CList include_paths_opt = NULL;

	Args_Add(args, "-o", "--output", T_CHAR, &output, "Compiled program.");
	Args_Add(args, "-I", "--include", T_LIST, &include_paths_opt, "List of include directory to search in.");

	err = Args_Parse(args, argc, argv);
	if( err == TREAT_ERROR ) {
		Args_Free(args);
		return EXIT_FAILURE;
	} else if( err == HELP ) {
		Args_Free(args);
		return EXIT_SUCCESS;
	} else if( args->rest == NULL ) {
		fprintf(stderr, "Missing arguments.");
		return EXIT_FAILURE;
	}

	BlProg bl = {0};
	Records records = {0};

	const char *input = args->rest->opt;
	IncludeList include_paths = clist_to_include(include_paths_opt);

	clist_free(include_paths_opt);
	Args_Free(args);

	bl.mem.memory_capacity = BLISP_STATIC_MEMORY_CAPACITY;

	StringView src = load_file(input);
	if( ! translate_source(&bl, include_paths, input, src, &records) )
		goto error;

	printf("BITS 64\n");
	printf("%%define BLISP_STACK_CAPACITY %lu\n", bl.mem.memory_capacity);
	printf("%%define BLISP_WORD_SIZE %d\n", BLISP_WORD_SIZE);
	printf("%%define SYS_EXIT 60\n");
	printf("%%define STDOUT 1\n");
	printf("%%define SYS_WRITE 1\n");
	printf("segment .text\n");
	printf("global _start\n");
	gen_print_i64(stdout);
	printf("_start:\n");

	for(size_t idx = 0; idx < bl.prog.program_size; idx++) {
		Inst instruction = bl.prog.program[idx];

		switch(instruction.type) {
			case INST_NOP:
				assert(false && "INST_NOP compilation is not yet implemented.");
				break;


			case INST_PUSH:
				printf("\t;; push %lu\n", instruction.operand.u64);
				printf("\tmov rsi,[stack_top]\n");
				printf("\tmov QWORD [rsi],%lu\n", instruction.operand.u64);
				printf("\tadd QWORD [stack_top], BLISP_WORD_SIZE\n");
				break;

			case INST_POP:
				printf("\t;; pop\n");
				printf("\tsub QWORD [stack_top], BLISP_WORD_SIZE\n");
				break;

			case INST_SWAP:
				printf("\t;; swap %lu\n", instruction.operand.u64);
				printf("\tmov rsi, [stack_top]\n");
				printf("\tsub rsi, BLISP_WORD_SIZE\n");
				printf("\tmov rdi, rsi\n");
				printf("\tsub rdi, BLISP_WORD_SIZE * %%%lu\n", instruction.operand.u64);
				printf("\t;; rsi and rdi contains the addresses to swap around\n");
				printf("\tmov rax, [rsi]\n");
				printf("\tmov rbx, [rdi]\n");
				printf("\tmov [rdi], rax\n");
				printf("\tmov [rsi], rbx\n");
				break;

			case INST_DUP:
				printf("\t;; dup %lu\n", instruction.operand.u64);
				printf("\tmov rsi, [stack_top]\n");
				printf("\tmov rdi, rsi\n");
				printf("\tsub rdi, BLISP_WORD_SIZE * (%lu + 1)\n", instruction.operand.u64);
				printf("\tmov rax, [rdi]\n");
				printf("\tmov [rsi], rax\n");
				printf("\tadd rsi, BLISP_WORD_SIZE\n");
				printf("\tmov [stack_top], rsi\n");
				break;


			case INST_ADD:
				printf("\t;; add\n");
				printf("\tmov rsi,[stack_top]\n");
				printf("\tsub rsi,BLISP_WORD_SIZE\n");
				printf("\tmov rbx,[rsi]\n");
				printf("\tsub rsi,BLISP_WORD_SIZE\n");
				printf("\tmov rax,[rsi]\n");
				printf("\tadd rax,rbx\n");
				printf("\tmov [rsi],rax\n");
				printf("\tadd rsi,BLISP_WORD_SIZE\n");
				printf("\tmov [stack_top],rsi\n");
				break;

			case INST_SUB:
				printf("\t;; sub\n");
				printf("\tmov rsi,[stack_top]\n");
				printf("\tsub rsi,BLISP_WORD_SIZE\n");
				printf("\tmov rbx,[rsi]\n");
				printf("\tsub rsi,BLISP_WORD_SIZE\n");
				printf("\tmov rax,[rsi]\n");
				printf("\tsub rax,rbx\n");
				printf("\tmov [rsi],rax\n");
				printf("\tadd rsi,BLISP_WORD_SIZE\n");
				printf("\tmov [stack_top],rsi\n");
				break;

			case INST_MUL:
				printf("\t;; sub\n");
				printf("\tmov rsi,[stack_top]\n");
				printf("\tsub rsi,BLISP_WORD_SIZE\n");
				printf("\tmov rbx,[rsi]\n");
				printf("\tsub rsi,BLISP_WORD_SIZE\n");
				printf("\tmov rax,[rsi]\n");
				printf("\tmul rax,rbx\n");
				printf("\tmov [rsi],rax\n");
				printf("\tadd rsi,BLISP_WORD_SIZE\n");
				printf("\tmov [stack_top],rsi\n");
				break;

			case INST_DIV:
				assert(false && "INST_DIV compilation is not yet implemented.");
				break;

			case INST_MOD:
				assert(false && "INST_MOD compilation is not yet implemented.");
				break;


			case INST_ADDF:
				assert(false && "INST_ADDF compilation is not yet implemented.");
				break;

			case INST_SUBF:
				assert(false && "INST_SUBF compilation is not yet implemented.");
				break;

			case INST_MULF:
				assert(false && "INST_MULF compilation is not yet implemented.");
				break;

			case INST_DIVF:
				assert(false && "INST_DIVF compilation is not yet implemented.");
				break;


			case INST_JMP:
				assert(false && "INST_JMP compilation is not yet implemented.");
				break;

			case INST_JIF:
				assert(false && "INST_JIF compilation is not yet implemented.");
				break;


			case INST_CALL:
				assert(false && "INST_CALL compilation is not yet implemented.");
				break;

			case INST_RET:
				assert(false && "INST_RET compilation is not yet implemented.");
				break;

			case INST_NATIVE:
				printf("\t;; native %lu\n", instruction.operand.u64);
				switch(instruction.operand.u64) {
					case 3:
						printf("\t;; native print_i64\n");
						printf("\tcall print_i64\n");
						break;
					default:
						assert(false && "Unsupported native function");
				}
				break;


			case INST_EQ:
				assert(false && "INST_EQ compilation is not yet implemented.");
				break;

			case INST_GT:
				assert(false && "INST_GT compilation is not yet implemented.");
				break;

			case INST_GE:
				assert(false && "INST_GE compilation is not yet implemented.");
				break;

			case INST_LT:
				assert(false && "INST_LT compilation is not yet implemented.");
				break;

			case INST_LE:
				assert(false && "INST_LE compilation is not yet implemented.");
				break;

			case INST_NE:
				assert(false && "INST_NE compilation is not yet implemented.");
				break;

			case INST_GTF:
				assert(false && "INST_GTF compilation is not yet implemented.");
				break;

			case INST_GEF:
				assert(false && "INST_GEF compilation is not yet implemented.");
				break;

			case INST_LTF:
				assert(false && "INST_LTF compilation is not yet implemented.");
				break;

			case INST_LEF:
				assert(false && "INST_LEF compilation is not yet implemented.");
				break;

			case INST_NOT:
				assert(false && "INST_NOT compilation is not yet implemented.");
				break;


			case INST_AND:
				assert(false && "INST_AND compilation is not yet implemented.");
				break;

			case INST_OR:
				assert(false && "INST_OR compilation is not yet implemented.");
				break;

			case INST_XOR:
				assert(false && "INST_XOR compilation is not yet implemented.");
				break;

			case INST_SHR:
				assert(false && "INST_SHR compilation is not yet implemented.");
				break;

			case INST_SHL:
				assert(false && "INST_SHL compilation is not yet implemented.");
				break;

			case INST_BNOT:
				assert(false && "INST_BNOT compilation is not yet implemented.");
				break;


			case INST_READ8:
				assert(false && "INST_READ8 compilation is not yet implemented.");
				break;

			case INST_READ16:
				assert(false && "INST_READ16 compilation is not yet implemented.");
				break;

			case INST_READ32:
				assert(false && "INST_READ32 compilation is not yet implemented.");
				break;

			case INST_READ64:
				assert(false && "INST_READ64 compilation is not yet implemented.");
				break;


			case INST_WRITE8:
				assert(false && "INST_WRITE8 compilation is not yet implemented.");
				break;

			case INST_WRITE16:
				assert(false && "INST_WRITE16 compilation is not yet implemented.");
				break;

			case INST_WRITE32:
				assert(false && "INST_WRITE32 compilation is not yet implemented.");
				break;

			case INST_WRITE64:
				assert(false && "INST_WRITE64 compilation is not yet implemented.");
				break;


			case INST_I2F:
				assert(false && "INST_I2F compilation is not yet implemented.");
				break;

			case INST_U2F:
				assert(false && "INST_U2F compilation is not yet implemented.");
				break;

			case INST_F2I:
				assert(false && "INST_F2I compilation is not yet implemented.");
				break;

			case INST_F2U:
				assert(false && "INST_F2U compilation is not yet implemented.");
				break;


			case INST_HALT:
				printf("\t;; halt\n");
				printf("\tmov rax, SYS_EXIT\n");
				printf("\tmov rdi, 0\n");
				printf("\tsyscall\n");

				break;


			case INST_PRINT_DEBUG:
				assert(false && "INST_PRINT_DEBUG compilation is not yet implemented.");
				break;

			case NUMBER_OF_INSTS:
			default:
				assert(false && "Unknown instruction");
		}
	}

	printf("segment .data\n");
	printf("stack_top: dq stack\n");
	printf("segment .bss\n");
	printf("stack: resq BLISP_STACK_CAPACITY\n");

error:
	blprog_clean(&bl);
	records_free(&records);
	free(src.data);
	for(IncludeList tmp = include_paths; tmp != NULL; ) {
		IncludeList free_lst = tmp;
		tmp = tmp->next;

		free(free_lst);
	}

	return EXIT_SUCCESS;
}
