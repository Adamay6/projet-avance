CC = gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror
CPPFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
ifneq ($(GMP_PREFIX),)
GMP_CFLAGS ?= -I"$(GMP_PREFIX)/include"
GMP_LIBS ?= -L"$(GMP_PREFIX)/lib" -lgmp
else
GMP_CFLAGS ?= $(shell pkg-config --static --cflags gmp)
GMP_LIBS ?= $(shell pkg-config --static --libs gmp)
endif
BUILD = ../build/adam_phase2
SHELL := sh
ifeq ($(OS),Windows_NT)
EXE = .exe
endif
GMP_RSA = ../phase2_nada/gmp_rsa.c ../phase2_nada/convert_bytes.c
GMP_HEADERS = ../phase2_nada/gmp_rsa.h ../phase2_nada/convert_bytes.h
SOURCES = chiffrement_blocs.c fichiers_gmp.c $(GMP_RSA) tests/test_fichiers_blocs.c
HEADERS = chiffrement_blocs.h ../phase1_adam/chiffrement.h ../phase1_nada/rsa_common_header.h ../console_utf8.h
all: $(BUILD)/test_fichiers_blocs$(EXE) $(BUILD)/test_blocs_io$(EXE)
$(BUILD)/test_fichiers_blocs$(EXE): $(SOURCES) $(HEADERS) $(GMP_HEADERS) Makefile
	@mkdir -p $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(GMP_CFLAGS) $(SOURCES) $(LDFLAGS) $(GMP_LIBS) -o $@
$(BUILD)/test_blocs_io$(EXE): tests/test_blocs_io.c chiffrement_blocs.c $(HEADERS) Makefile
	@mkdir -p $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_blocs_io.c $(LDFLAGS) -o $@
test: all
	@mkdir -p $(BUILD)/tests
	cd $(BUILD)/tests && ../test_fichiers_blocs$(EXE)
	cd $(BUILD)/tests && ../test_blocs_io$(EXE)
.PHONY: all test
