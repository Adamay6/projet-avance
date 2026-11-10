CC = gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror
CPPFLAGS += -I. -finput-charset=UTF-8 -fexec-charset=UTF-8
SHELL := sh

ifeq ($(OS),Windows_NT)
EXE = .exe
endif

# Phase 2 : GMP. Si gmp.h n'est pas dans les chemins par defaut : make GMP_PREFIX=/chemin/vers/gmp
# Sans GMP_PREFIX, on essaie Homebrew (macOS) ; ailleurs la commande echoue sans bruit.
ifeq ($(origin GMP_PREFIX),undefined)
GMP_PREFIX := $(shell brew --prefix gmp 2>/dev/null)
endif
ifneq ($(GMP_PREFIX),)
GMP_CPPFLAGS = -I$(GMP_PREFIX)/include
GMP_LDFLAGS = -L$(GMP_PREFIX)/lib
endif

OBJ = build/obj
BIN = build/bin
RSA = phase1_nada/phase1/rsa_tools.c phase1_nada/phase1/bezout.c \
      phase1_nada/phase1/keys_io.c phase1_nada/phase1/log.c
CHIFFREMENT = phase1_adam/chiffrement.c
BASE64 = phase1_loann/base64.c phase1_loann/other_base64.c
TESTS_MODULES = phase1_adam/tests_chiffrement.c phase1_loann/tests_base64.c
GMP_RSA = phase2_nada/gmp_rsa.c phase2_nada/convert_bytes.c
FICHIERS_BLOCS = phase2_adam/chiffrement_blocs.c phase2_adam/fichiers_gmp.c
BASE64_V2 = phase2_loann/base64_v2.c
TESTS_GMP = $(BIN)/test_gmp_rsa$(EXE) $(BIN)/test_fichiers_blocs$(EXE) \
            $(BIN)/test_phase2_loann$(EXE)
COMMUN = $(RSA) $(CHIFFREMENT) $(BASE64)
SOURCES = $(COMMUN) $(TESTS_MODULES) phase1_malek/phase1.c phase1_malek/main_phase1.c \
          phase1_nada/phase1/test_cles.c phase1_nada/phase1/test_maxprime.c \
          phase1_loann/main_test_loann.c tests/test_regressions.c tests/test_io_errors.c \
          $(GMP_RSA) phase2_nada/tests_gmp_rsa.c $(FICHIERS_BLOCS) $(BASE64_V2) \
          phase2_adam/tests/test_fichiers_blocs.c phase2_adam/tests/test_blocs_io.c \
          tests/test_phase2_loann.c

all: $(BIN)/test_phase1$(EXE) $(BIN)/test_cles$(EXE) $(BIN)/test_maxprime$(EXE) \
     $(BIN)/test_base64$(EXE) $(BIN)/test_regressions$(EXE) $(BIN)/test_io_errors$(EXE) \
     $(TESTS_GMP) $(BIN)/test_blocs_io$(EXE)

$(BIN)/test_phase1$(EXE): $(patsubst %.c,$(OBJ)/%.o,$(COMMUN) $(TESTS_MODULES) phase1_malek/phase1.c phase1_malek/main_phase1.c)

$(BIN)/test_cles$(EXE): $(patsubst %.c,$(OBJ)/%.o,$(RSA) phase1_nada/phase1/test_cles.c)

$(BIN)/test_maxprime$(EXE): $(patsubst %.c,$(OBJ)/%.o,$(RSA) phase1_nada/phase1/test_maxprime.c)

$(BIN)/test_base64$(EXE): $(patsubst %.c,$(OBJ)/%.o,$(BASE64) phase1_loann/tests_base64.c phase1_loann/main_test_loann.c)

$(BIN)/test_regressions$(EXE): $(patsubst %.c,$(OBJ)/%.o,$(COMMUN) phase1_malek/phase1.c tests/test_regressions.c)

# Les sources de fichiers sont incluses dans ce test pour simuler les erreurs d'E/S.
$(BIN)/test_io_errors$(EXE): $(patsubst %.c,$(OBJ)/%.o,phase1_nada/phase1/rsa_tools.c phase1_nada/phase1/bezout.c phase1_nada/phase1/log.c phase1_loann/other_base64.c tests/test_io_errors.c)

$(BIN)/test_gmp_rsa$(EXE): $(patsubst %.c,$(OBJ)/%.o,$(RSA) $(GMP_RSA) phase2_nada/tests_gmp_rsa.c)

$(BIN)/test_fichiers_blocs$(EXE): $(patsubst %.c,$(OBJ)/%.o,$(GMP_RSA) $(FICHIERS_BLOCS) phase2_adam/tests/test_fichiers_blocs.c)

$(BIN)/test_blocs_io$(EXE): $(OBJ)/phase2_adam/tests/test_blocs_io.o

$(BIN)/test_phase2_loann$(EXE): $(patsubst %.c,$(OBJ)/%.o,$(BASE64) $(GMP_RSA) $(FICHIERS_BLOCS) $(BASE64_V2) tests/test_phase2_loann.c)

$(TESTS_GMP): LDFLAGS += $(GMP_LDFLAGS)
$(TESTS_GMP): LDLIBS += -lgmp
$(OBJ)/phase2_nada/%.o: CPPFLAGS += $(GMP_CPPFLAGS)
$(OBJ)/phase2_adam/fichiers_gmp.o $(OBJ)/phase2_adam/tests/test_fichiers_blocs.o \
$(OBJ)/tests/test_phase2_loann.o: CPPFLAGS += $(GMP_CPPFLAGS)

$(BIN)/%$(EXE):
	@mkdir -p $(BIN)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ)/%.o: %.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

test: all test-reference
	@mkdir -p build/tests/commun build/tests/cles build/tests/regressions build/tests/io build/tests/gmp
	@mkdir -p build/tests/fichiers_blocs build/tests/blocs_io build/tests/phase2_loann
	cd build/tests/cles && ../../bin/test_cles$(EXE) 10000 20261015
	cd build/tests/commun && ../../bin/test_phase1$(EXE)
	cd build/tests/regressions && ../../bin/test_regressions$(EXE)
	cd build/tests/io && ../../bin/test_io_errors$(EXE)
	cd build/tests/gmp && ../../bin/test_gmp_rsa$(EXE)
	cd build/tests/fichiers_blocs && ../../bin/test_fichiers_blocs$(EXE)
	cd build/tests/blocs_io && ../../bin/test_blocs_io$(EXE)
	cd build/tests/phase2_loann && ../../bin/test_phase2_loann$(EXE)

test-reference: $(BIN)/test_base64$(EXE)
	@mkdir -p build/tests/reference
	cd build/tests/reference && ../../bin/test_base64$(EXE)
	cd build/tests/reference && base64 < test_source.bin | tr -d '\r\n' > unix_ref.b64
	cd build/tests/reference && cmp test_out.b64 unix_ref.b64
	@echo "Base64 : comparaison avec la commande Unix reussie."

test-maxprime: $(BIN)/test_maxprime$(EXE)
	@mkdir -p build/tests/maxprime
	@echo "Graine 20261015, 2000 paires par borne" > build/resultats_maxprime.txt
	@for borne in 1000 10000 30000 60000 65536 70000 80000 100000 200000 1000000 5000000; do \
	  (cd build/tests/maxprime && ../../bin/test_maxprime$(EXE) $$borne 2000 20261015) >> build/resultats_maxprime.txt || exit 1; \
	done
	@cat build/resultats_maxprime.txt

clean:
	rm -rf build

-include $(patsubst %.c,$(OBJ)/%.d,$(SOURCES))

.PHONY: all test test-reference test-maxprime clean
