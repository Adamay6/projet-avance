CC = gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror
CPPFLAGS += -I. -finput-charset=UTF-8 -fexec-charset=UTF-8
SHELL := sh

ifeq ($(OS),Windows_NT)
EXE = .exe
endif

OBJ = build/obj
BIN = build/bin
RSA = phase1_nada/phase1/rsa_tools.c phase1_nada/phase1/bezout.c \
      phase1_nada/phase1/keys_io.c phase1_nada/phase1/log.c
CHIFFREMENT = phase1_adam/chiffrement.c
BASE64 = phase1_loann/base64.c phase1_loann/other_base64.c
TESTS_MODULES = phase1_adam/tests_chiffrement.c phase1_loann/tests_base64.c
COMMUN = $(RSA) $(CHIFFREMENT) $(BASE64)
SOURCES = $(COMMUN) $(TESTS_MODULES) phase1_malek/phase1.c phase1_malek/main_phase1.c \
          phase1_nada/phase1/test_cles.c phase1_nada/phase1/test_maxprime.c \
          phase1_loann/main_test_loann.c tests/test_regressions.c tests/test_io_errors.c

all: $(BIN)/test_phase1$(EXE) $(BIN)/test_cles$(EXE) $(BIN)/test_maxprime$(EXE) \
     $(BIN)/test_base64$(EXE) $(BIN)/test_regressions$(EXE) $(BIN)/test_io_errors$(EXE)

$(BIN)/test_phase1$(EXE): $(patsubst %.c,$(OBJ)/%.o,$(COMMUN) $(TESTS_MODULES) phase1_malek/phase1.c phase1_malek/main_phase1.c)

$(BIN)/test_cles$(EXE): $(patsubst %.c,$(OBJ)/%.o,$(RSA) phase1_nada/phase1/test_cles.c)

$(BIN)/test_maxprime$(EXE): $(patsubst %.c,$(OBJ)/%.o,$(RSA) phase1_nada/phase1/test_maxprime.c)

$(BIN)/test_base64$(EXE): $(patsubst %.c,$(OBJ)/%.o,$(BASE64) phase1_loann/tests_base64.c phase1_loann/main_test_loann.c)

$(BIN)/test_regressions$(EXE): $(patsubst %.c,$(OBJ)/%.o,$(COMMUN) phase1_malek/phase1.c tests/test_regressions.c)

# Les sources de fichiers sont incluses dans ce test pour simuler les erreurs d'E/S.
$(BIN)/test_io_errors$(EXE): $(patsubst %.c,$(OBJ)/%.o,phase1_nada/phase1/rsa_tools.c phase1_nada/phase1/bezout.c phase1_nada/phase1/log.c phase1_loann/other_base64.c tests/test_io_errors.c)

$(BIN)/%$(EXE):
	@mkdir -p $(BIN)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ)/%.o: %.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

test: all test-reference
	@mkdir -p build/tests/commun build/tests/cles build/tests/regressions build/tests/io
	cd build/tests/cles && ../../bin/test_cles$(EXE) 10000 20261015
	cd build/tests/commun && ../../bin/test_phase1$(EXE)
	cd build/tests/regressions && ../../bin/test_regressions$(EXE)
	cd build/tests/io && ../../bin/test_io_errors$(EXE)

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
