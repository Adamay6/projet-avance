#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "base64_v2.h"
#include "email_sender.h"
#include "../phase1_loann/base64.h"
#include "../console_utf8.h"

/* Le mot de passe d'application n'est jamais ecrit dans le code : il est lu dans
 * DIYPG_SMTP_PASSWORD. Sans cette variable, l'envoi est ignore (pas d'echec). */
int exemple_envoi(void) {
    const char *mot_de_passe = getenv("DIYPG_SMTP_PASSWORD");
    if (mot_de_passe == NULL || mot_de_passe[0] == '\0') {
        printf("ignoré (DIYPG_SMTP_PASSWORD non définie)\n");
        return 0;
    }
    email_config_t mail_cfg = {
        .smtp_server = "smtp.gmail.com",
        .smtp_port = "587",
        .username = "diypg.gproj@gmail.com",
        .password = mot_de_passe,
        .from = "diypg.gproj@gmail.com",
        .to = "mloann2802@gmail.com"
    };

    int status = send_email_curl_cmd(&mail_cfg, "Fichier chiffre DIYPG Phase 2", "test_cipher.b64");
    if (status == 0) {
        printf("OK\n");
    } else {
        printf("Echec de l'envoi de l'e-mail.\n");
    }
    return status;
}

int main(void) {
    initialiser_console_utf8();
    printf("==================================================\n");
    printf("   TESTS UNITAIRES PHASE 2 - LOANN (Base64 & Keys)\n");
    printf("==================================================\n\n");

    int success = 1;

    printf("[1/4] Export/Import Clé Publique Base64... ");
    rsaKey_t k1 = {.E = 65537, .N = 4294967291ULL};
    rsaKey_t k2 = {.E = 0, .N = 0};
    char type[PUBKEY_TYPE_TAILLE] = "";

    if (export_pubkey_b64("test_pub.b64", &k1, "crypt") == 0 &&
        import_pubkey_b64("test_pub.b64", &k2, type) == 0 &&
        k1.E == k2.E && k1.N == k2.N && strcmp(type, "crypt") == 0) {
        printf("OK\n");
    } else {
        printf("ÉCHEC\n");
        success = 0;
    }

    printf("[2/4] Conversion Base64 sur blocs binaires... ");
    FILE *f = fopen("test_blocks.bin", "wb");
    if (!f) {
        printf("ÉCHEC\n");
        success = 0;
    } else {
        uint32_t b[2] = {0x12345678, 0x9ABCDEF0};
        int write_ok = (fwrite(b, sizeof(uint32_t), 2, f) == 2);
        if (fclose(f) != 0) write_ok = 0;

        if (!write_ok) {
            printf("ÉCHEC\n");
            success = 0;
        } else if (file_to_base64_v2("test_blocks.bin", "test_blocks.b64") == 0 &&
                   base64_to_file_v2("test_blocks.b64", "test_out.bin") == 0) {
            printf("OK\n");
        } else {
            printf("ÉCHEC\n");
            success = 0;
        }
    }

    printf("[3/4] Chiffrement / Déchiffrement bloc + Base64... ");
    rsaKey_t pub = {.E = 65537, .N = 4295229443ULL};
    rsaKey_t priv = {.E = 65537, .N = 4295229443ULL};
    FILE *f_plain = fopen("test_plain.txt", "w");
    if (!f_plain) {
        printf("ÉCHEC\n");
        success = 0;
    } else {
        fputs("Message secret de test pour la phase 2 !", f_plain);
        fclose(f_plain);

        if (chiffrer_fichier_blocs_b64("test_plain.txt", "test_cipher.b64", &pub) == 0 &&
            dechiffrer_fichier_blocs_b64("test_cipher.b64", "test_restored.txt", &priv) == 0) {
            FILE *fr = fopen("test_restored.txt", "r");
            if (fr) {
                char buf[128] = {0};
                fgets(buf, sizeof(buf), fr);
                fclose(fr);
                if (strcmp(buf, "Message secret de test pour la phase 2 !") == 0) {
                    printf("OK\n");
                } else {
                    printf("ÉCHEC (contenu restauré incorrect)\n");
                    success = 0;
                }
            } else {
                printf("ÉCHEC (fichier restauré introuvable)\n");
                success = 0;
            }
        } else {
            printf("ÉCHEC (erreur lors du traitement de chiffrement/déchiffrement)\n");
            success = 0;
        }
    }

    printf("[4/4] Envoi par mail... ");

    if(exemple_envoi()) success = 0;

    base64_cleanup();
    printf("\nRésultat Phase 2 Loann : %s\n", success ? "REUSSITE" : "ECHEC");
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
