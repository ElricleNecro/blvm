ifndef config
	config=debug
endif

ifeq ($(config),debug)
	config=Debug
	CC=../build/Debug/bin/blasm
	OUT=../build/Debug/examples
else ifeq ($(config),release)
	config=Release
	CC=../build/Release/bin/blasm
	OUT=../build/Release/examples
else
	$(error "invalid configuration $(config)")
endif

EXAMPLES!= find . -name \*.blasm | sed -e "s/\.blasm/\.bl/" -e "s:^\.:${OUT}:"

.SUFFIXES: .blasm .bl

$(OUT)/%.bl: %.blasm
	@$(CC) -I ../lib $< -o $@

.PHONY: all

all: ${OUT} ${EXAMPLES}

${OUT}:
	@mkdir -p $(OUT)
