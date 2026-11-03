#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../phase1_malek/phase1.h"
#include "../console_utf8.h"

#ifdef _WIN32
#include <windows.h>
#endif

static unsigned int checks = 0, errors = 0;

#define CHECK(condition) do { \
    checks++; \
    if (!(condition)) { \
        fprintf(stderr, "Echec ligne %d : %s\n", __LINE__, #condition); \
        errors++; \
    } \
} while (0)

static keyPair_t known_keys = {{17, 3233}, {2753, 3233}};

static int writeFile(const char *path, const void *data, size_t size) {
    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    int ok = fwrite(data, 1, size, f) == size;
    if (fclose(f) != 0) ok = 0;
    return ok;
}

static int fileEquals(const char *path, const void *data, size_t size) {
    FILE *f = fopen(path, "rb");
    const unsigned char *expected = data;
    if (!f) return 0;
    int ok = 1;
    for (size_t i = 0; i < size; i++) {
        if (fgetc(f) != expected[i]) { ok = 0; break; }
    }
    if (fgetc(f) != EOF || ferror(f)) ok = 0;
    if (fclose(f) != 0) ok = 0;
    return ok;
}

static long fileSize(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    long size = fseek(f, 0, SEEK_END) == 0 ? ftell(f) : -1;
    if (fclose(f) != 0) size = -1;
    return size;
}

static int sameKeys(const keyPair_t *a, const keyPair_t *b) {
    return a->pubKey.E == b->pubKey.E && a->pubKey.N == b->pubKey.N &&
           a->privKey.E == b->privKey.E && a->privKey.N == b->privKey.N;
}

static void testBase64(void) {
    const char *clear[] = {"", "f", "fo", "foo", "foob", "fooba", "foobar"};
    const char *encoded[] = {"", "Zg==", "Zm8=", "Zm9v", "Zm9vYg==", "Zm9vYmE=", "Zm9vYmFy"};
    for (size_t i = 0; i < sizeof clear / sizeof clear[0]; i++) {
        size_t encoded_len = 999, decoded_len = 999;
        char *result = base64_encode((const unsigned char *)clear[i], strlen(clear[i]), &encoded_len);
        CHECK(result != NULL);
        if (result) {
            CHECK(encoded_len == strlen(encoded[i]));
            CHECK(result[encoded_len] == '\0');
            CHECK(strcmp(result, encoded[i]) == 0);
            unsigned char *back = base64_decode(result, encoded_len, &decoded_len);
            CHECK(back != NULL);
            if (back) {
                CHECK(decoded_len == strlen(clear[i]));
                CHECK(memcmp(back, clear[i], decoded_len) == 0);
            }
            free(back);
        }
        free(result);
    }

    const char *invalid[] = {"A", "AA", "!!!!", "====", "A===", "=AAA", "AA=A", "AA==AAAA", "Zh==", "Zm9=", "Zg==\n"};
    for (size_t i = 0; i < sizeof invalid / sizeof invalid[0]; i++) {
        size_t size = 999;
        unsigned char *data = base64_decode(invalid[i], strlen(invalid[i]), &size);
        CHECK(data == NULL);
        CHECK(size == 0);
        free(data);
    }
    const char non_ascii[] = {(char)0xff, 'A', 'A', 'A'};
    size_t size = 999;
    unsigned char *bad = base64_decode(non_ascii, sizeof non_ascii, &size);
    CHECK(bad == NULL && size == 0);
    free(bad);
    char *empty = base64_encode(NULL, 0, &size);
    CHECK(empty && size == 0 && empty[0] == '\0');
    free(empty);
    bad = base64_decode(NULL, 0, &size);
    CHECK(bad && size == 0);
    free(bad);
    empty = base64_encode(NULL, 1, &size);
    CHECK(empty == NULL && size == 0);
    empty = base64_encode((const unsigned char *)"A", SIZE_MAX, &size);
    CHECK(empty == NULL && size == 0);
    CHECK(base64_encode((const unsigned char *)"A", 1, NULL) == NULL);
    CHECK(base64_decode("AAAA", 4, NULL) == NULL);

    base64_cleanup();
    base64_cleanup();
    bad = base64_decode("Zg==", 4, &size);
    CHECK(bad && size == 1 && bad[0] == 'f');
    free(bad);

#ifdef _WIN32
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    size_t page = info.dwPageSize;
    unsigned char *memory = VirtualAlloc(NULL, 2 * page, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    CHECK(memory != NULL);
    if (memory) {
        DWORD old;
        int protected_page = VirtualProtect(memory, page, PAGE_NOACCESS, &old) != 0;
        CHECK(protected_page);
        if (protected_page) {
            memory[page] = '\0';
            bad = base64_decode((const char *)memory + page, 0, &size);
            CHECK(bad && size == 0);
            free(bad);
        }
        CHECK(VirtualFree(memory, 0, MEM_RELEASE) != 0);
    }
#endif
}

static void testKeys(void) {
    CHECK(keypair_valide(&known_keys));
    CHECK(save_keypair("valid.key", &known_keys) == 0);
    keyPair_t loaded = {{0, 0}, {0, 0}};
    CHECK(loadKeyPair("valid.key", &loaded) == 1);
    CHECK(sameKeys(&known_keys, &loaded));
    CHECK(saveKeyPair("other.key", &known_keys) == 1);
    CHECK(load_keypair("other.key", &loaded) == 0);
    CHECK(sameKeys(&known_keys, &loaded));

    const char *bad_files[] = {
        "", "DIYPG-KEYPAIR 2\nPUB 11 ca1\nPRIV ac1 ca1\n",
        "DIYPG-KEYPAIR 1\nPUB 11 ca1\n",
        "DIYPG-KEYPAIR 1\nPUB 0 ca1\nPRIV ac1 ca1\n",
        "DIYPG-KEYPAIR 1\nPUB 1 1\nPRIV 1 1\n",
        "DIYPG-KEYPAIR 1\nPUB ffff ca1\nPRIV ac1 ca1\n",
        "DIYPG-KEYPAIR 1\nPUB 11 100000001\nPRIV ac1 100000001\n",
        "DIYPG-KEYPAIR 1\nPUB 11 ca1\nPRIV 11 ca1\n",
        "DIYPG-KEYPAIR 1\nPUB 11 ca1\nPRIV ac1 ca1\nGARBAGE",
        "DIYPG-KEYPAIR 1\nPUB -11 ca1\nPRIV ac1 ca1\n",
        "DIYPG-KEYPAIR 1\nPUB 10000000000000000 ca1\nPRIV ac1 ca1\n"
    };
    for (size_t i = 0; i < sizeof bad_files / sizeof bad_files[0]; i++) {
        loaded = known_keys;
        CHECK(writeFile("bad.key", bad_files[i], strlen(bad_files[i])));
        CHECK(load_keypair("bad.key", &loaded) == -1);
        CHECK(sameKeys(&loaded, &known_keys));
    }
    keyPair_t invalid = {{1, 1}, {1, 1}};
    CHECK(writeFile("preserve.key", "CONSERVER", 9));
    CHECK(save_keypair("preserve.key", &invalid) == -1);
    CHECK(fileEquals("preserve.key", "CONSERVER", 9));
    CHECK(load_keypair(NULL, &loaded) == -1);
    CHECK(load_keypair("valid.key", NULL) == -1);
    CHECK(save_keypair("valid.key", NULL) == -1);

    const uint64_t limits[] = {0, 17, 18, 19, 65537, UINT64_MAX};
    for (size_t i = 0; i < sizeof limits / sizeof limits[0]; i++) {
        loaded = known_keys;
        CHECK(gen_keypair(&loaded, limits[i]) == -1);
        CHECK(sameKeys(&loaded, &known_keys));
    }
    CHECK(gen_keypair(NULL, 10000) == -1);
    for (unsigned int i = 0; i < 20; i++) {
        CHECK(gen_keypair(&loaded, 20) == 0);
        CHECK(keypair_valide(&loaded));
    }
    CHECK(gen_keypair(&loaded, 65536) == 0);
    CHECK(keypair_valide(&loaded));
}

static void testArrays(void) {
    uint8_t bytes[256];
    for (size_t i = 0; i < sizeof bytes; i++) bytes[i] = (uint8_t)i;
    size_t encoded_len = 0, decoded_len = 0;
    uint8_t *cipher = encryptByteArray(bytes, sizeof bytes, &known_keys.pubKey, &encoded_len);
    CHECK(cipher && encoded_len == 2048);
    if (cipher) {
        CHECK(cipher[65 * 8] == 0xe6 && cipher[65 * 8 + 1] == 0x0a);
        uint8_t *clear = decryptByteArray(cipher, encoded_len, &known_keys.privKey, &decoded_len);
        CHECK(clear && decoded_len == sizeof bytes && memcmp(clear, bytes, sizeof bytes) == 0);
        free(clear);
    }
    free(cipher);
    cipher = encryptByteArray(NULL, 0, &known_keys.pubKey, &encoded_len);
    CHECK(cipher && encoded_len == 0);
    free(cipher);
    cipher = encryptByteArray(bytes, SIZE_MAX, &known_keys.pubKey, &encoded_len);
    CHECK(cipher == NULL && encoded_len == 0);
    cipher = decryptByteArray(bytes, 7, &known_keys.privKey, &decoded_len);
    CHECK(cipher == NULL && decoded_len == 0);
    CHECK(encryptByteArray(bytes, 1, &known_keys.pubKey, NULL) == NULL);
}

static void testFiles(void) {
    const size_t sizes[] = {0, 1, 2, 3, 4, 5, 255, 256, 4095, 4096, 4097, 100003};
    unsigned char *data = malloc(100003);
    CHECK(data != NULL);
    if (!data) return;
    for (size_t i = 0; i < 100003; i++) data[i] = (unsigned char)(i * 31 + 7);
    for (size_t i = 0; i < sizeof sizes / sizeof sizes[0]; i++) {
        size_t n = sizes[i];
        CHECK(writeFile("source.bin", data, n));
        CHECK(encryptFile("source.bin", "cipher.bin", &known_keys.pubKey) == 1);
        CHECK(fileSize("cipher.bin") == (long)(8 * n));
        CHECK(file_to_base64("cipher.bin", "cipher.b64") == 0);
        CHECK(fileSize("cipher.b64") == (long)(4 * ((8 * n + 2) / 3)));
        CHECK(base64_to_file("cipher.b64", "recup.bin") == 0);
        CHECK(decryptFile("recup.bin", "return.bin", &known_keys.privKey) == 1);
        CHECK(fileEquals("return.bin", data, n));
    }
    free(data);

    CHECK(writeFile("same.bin", "ABC", 3));
    CHECK(encryptFile("same.bin", "same.bin", &known_keys.pubKey) == 1);
    CHECK(decryptFile("same.bin", "same.bin", &known_keys.privKey) == 1);
    CHECK(fileEquals("same.bin", "ABC", 3));
    CHECK(writeFile("preserve.bin", "CONSERVER", 9));
    CHECK(encryptFile("absent/source.bin", "preserve.bin", &known_keys.pubKey) == 0);
    CHECK(fileEquals("preserve.bin", "CONSERVER", 9));
    CHECK(writeFile("bad.bin", "1234567", 7));
    CHECK(decryptFile("bad.bin", "preserve.bin", &known_keys.privKey) == 0);
    CHECK(fileEquals("preserve.bin", "CONSERVER", 9));
    CHECK(writeFile("bad.b64", "!!!!", 4));
    CHECK(base64_to_file("bad.b64", "preserve.bin") == -1);
    CHECK(fileEquals("preserve.bin", "CONSERVER", 9));
    const char wrapped[] = " Q U\r\nJ\tD \n";
    CHECK(writeFile("wrapped.b64", wrapped, sizeof wrapped - 1));
    CHECK(base64_to_file("wrapped.b64", "unwrapped.bin") == 0);
    CHECK(fileEquals("unwrapped.bin", "ABC", 3));
}

static void testMath(void) {
    CHECK(premier(0) == 0 && premier(1) == 0 && premier(2) == 1);
    CHECK(rabin(2, 1) == 0 && rabin(2, 2) == 1);
    uint64_t factors[8] = {99};
    CHECK(decompose(factors, 0) == 0 && factors[0] == 99);
    CHECK(pgcdFast(48, 18) == 6);
    errno = 0;
    CHECK(random_uint(17, 17) == 17 && errno == EINVAL);
    CHECK(puissance_mod_n(123, 0, 1) == 0);
    CHECK(puissance_mod_n(123, 0, 3233) == 1);
    CHECK(puissance_mod_n(UINT64_MAX, 17, 3233) < 3233);
    int high_value = 0;
    for (int i = 0; i < 1000; i++) {
        uint64_t n = random_uint(17, 5000000);
        CHECK(n >= 17 && n < 5000000);
        if (n > 65536) high_value = 1;
    }
    CHECK(high_value);

    // Au-dessus de 2^32, une paire peut encore reussir sans donner de garantie generale.
    for (unsigned int byte = 0; byte < 256; byte++) {
        uint64_t cipher = puissance_mod_n(byte, 5, UINT64_C(4295229083));
        CHECK(puissance_mod_n(cipher, UINT64_C(2577058805), UINT64_C(4295229083)) == byte);
    }
    // Exemple deterministe du debordement conserve pour l'etude MAX_PRIME.
    CHECK(puissance_mod_n(UINT64_C(4294967296), 2, UINT64_C(4294967297)) != 1);
}

int main(void) {
    initialiser_console_utf8();
    srand(20261015);
    init_log();
    testBase64();
    testKeys();
    testArrays();
    testFiles();
    testMath();
    base64_cleanup();
    close_log();
    printf("Regressions phase 1 : %u verifications, %u echecs.\n", checks, errors);
    return errors ? EXIT_FAILURE : EXIT_SUCCESS;
}
