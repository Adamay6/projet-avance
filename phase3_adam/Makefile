CC = gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror
CPPFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
BUILD = ../build/adam_contacts
SHELL := sh
ifeq ($(OS),Windows_NT)
EXE = .exe
endif
COMMUN = contacts.c contacts_io.c contacts_commandes.c
HEADERS = contacts.h contacts_interne.h ../phase1_nada/rsa_common_header.h ../console_utf8.h
all: $(BUILD)/test_contacts$(EXE) $(BUILD)/test_contacts_memoire$(EXE) $(BUILD)/rejouer_contacts$(EXE)
$(BUILD)/test_contacts$(EXE): $(COMMUN) contacts_interpreteur.c tests/test_contacts.c $(HEADERS) \
                            contacts_interpreteur.h ../phase3_nada/interpreteur.h \
                            ../phase1_adam/chiffrement.h Makefile
	@mkdir -p $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(COMMUN) contacts_interpreteur.c tests/test_contacts.c $(LDFLAGS) -o $@
$(BUILD)/test_contacts_memoire$(EXE): contacts.c contacts_io.c tests/test_contacts_memoire.c $(HEADERS) Makefile
	@mkdir -p $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_contacts_memoire.c $(LDFLAGS) -o $@
$(BUILD)/rejouer_contacts$(EXE): $(COMMUN) tests/rejouer_contacts.c $(HEADERS) Makefile
	@mkdir -p $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(COMMUN) tests/rejouer_contacts.c $(LDFLAGS) -o $@
test: all
	@mkdir -p $(BUILD)/tests
	cd $(BUILD)/tests && ../test_contacts$(EXE)
	cd $(BUILD)/tests && ../test_contacts_memoire$(EXE)
	cd $(BUILD)/tests && ../rejouer_contacts$(EXE) "$(CURDIR)/tests/scenario_contacts.txt"
.PHONY: all test
