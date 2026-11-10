CC = gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror
CPPFLAGS += -I. -I../phase1_nada -I../phase1_loann -I../phase2_adam -I../phase2_nada -finput-charset=UTF-8 -fexec-charset=UTF-8
LDFLAGS += -lgmp

EXEC = test_phase2_loann

SRCS = main_test_loann.c \
       base64_v2.c \
	   email_sender.c \
       ../phase1_loann/base64.c \
       ../phase1_loann/other_base64.c \
       ../phase2_adam/chiffrement_blocs.c \
       ../phase2_adam/fichiers_gmp.c \
       ../phase2_nada/gmp_rsa.c \
       ../phase2_nada/convert_bytes.c \
       ../phase1_adam/chiffrement.c \
       ../phase1_nada/phase1/rsa_tools.c \
       ../phase1_nada/phase1/bezout.c \
       ../phase1_nada/phase1/log.c

OBJ_DIR = build
OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(filter %.c,$(SRCS))))

vpath %.c . ../phase1_loann ../phase2_adam ../phase2_nada ../phase1_adam ../phase1_nada/phase1

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

test: $(EXEC)
	./$(EXEC)

clean:
	rm -rf $(OBJ_DIR) $(EXEC) *.b64 *.bin *.txt temp_*.bin

.PHONY: all test clean