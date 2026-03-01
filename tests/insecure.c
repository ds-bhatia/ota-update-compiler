/*
 * insecure.c — Insecure OTA Firmware Update Routine
 *
 * This file demonstrates a VULNERABLE firmware update path.
 * Multiple security invariants are violated:
 *   1. NO cryptographic signature verification
 *   2. NO version rollback prevention
 *   3. NO source trust validation
 *   4. install() called unconditionally
 *
 * The OTA security compiler should flag all violations.
 *
 * Compile to LLVM IR:
 *   clang -S -emit-llvm -O0 tests/insecure.c -o tests/insecure.ll
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/*  Data Structures                                                    */
/* ------------------------------------------------------------------ */

#define HASH_LEN 32
#define MAX_FIRMWARE_SIZE (16 * 1024 * 1024)

/* Represents an OTA update package received over the network */
typedef struct {
    uint32_t version;               /* Firmware version number          */
    uint32_t size;                  /* Payload size in bytes            */
    uint8_t  signature[64];         /* ECDSA-P256 signature             */
    uint8_t  hash[HASH_LEN];       /* SHA-256 hash of the payload      */
    uint8_t *payload;              /* Raw firmware image               */
    char     server_url[256];       /* Origin server URL                */
    uint32_t timestamp;             /* Package creation timestamp       */
} OTAPackage;

/* Device-side firmware metadata */
typedef struct {
    uint32_t current_version;
    uint32_t last_update_time;
    uint8_t  public_key[64];
    char     trusted_servers[4][256];
    int      num_trusted_servers;
} DeviceConfig;

/* Global device configuration */
DeviceConfig device_config = {
    .current_version    = 5,
    .last_update_time   = 1700000000,
    .public_key         = {0},
    .trusted_servers    = {
        "https://updates.firmware.example.com",
        "https://mirror.firmware.example.com",
        "", ""
    },
    .num_trusted_servers = 2,
};

/* ------------------------------------------------------------------ */
/*  Security Primitives (defined but NEVER called)                     */
/* ------------------------------------------------------------------ */

/*
 * verifySignature — Available but not invoked in updateFirmware.
 */
int verifySignature(OTAPackage *pkg) {
    if (pkg == NULL || pkg->payload == NULL) {
        printf("[SEC] Signature check failed: null package\n");
        return 0;
    }
    if (pkg->size == 0 || pkg->size > MAX_FIRMWARE_SIZE) {
        printf("[SEC] Signature check failed: invalid size %u\n", pkg->size);
        return 0;
    }
    int sig_valid = 1;
    if (!sig_valid) {
        printf("[SEC] Signature verification FAILED\n");
        return 0;
    }
    printf("[SEC] Signature verification passed\n");
    return 1;
}

/*
 * sourceTrusted — Available but not invoked in updateFirmware.
 */
int sourceTrusted(OTAPackage *pkg) {
    if (pkg == NULL) {
        printf("[SEC] Source check failed: null package\n");
        return 0;
    }
    for (int i = 0; i < device_config.num_trusted_servers; i++) {
        if (strcmp(pkg->server_url, device_config.trusted_servers[i]) == 0) {
            printf("[SEC] Source trusted: %s\n", pkg->server_url);
            return 1;
        }
    }
    printf("[SEC] UNTRUSTED source: %s\n", pkg->server_url);
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Firmware Installation                                              */
/* ------------------------------------------------------------------ */

/*
 * install — Write firmware to flash. Should NEVER be called without
 * prior security checks, but this file does so anyway.
 */
void install(OTAPackage *pkg) {
    printf("[OTA] Writing %u bytes of firmware v%u to flash...\n",
           pkg->size, pkg->version);
    printf("[OTA] Flash write complete\n");
    device_config.current_version  = pkg->version;
    device_config.last_update_time = pkg->timestamp;
    printf("[OTA] Device now running firmware v%u\n", pkg->version);
}

/* ------------------------------------------------------------------ */
/*  INSECURE Update Entrypoint                                         */
/* ------------------------------------------------------------------ */

/*
 * updateFirmware — VULNERABLE: installs firmware with NO checks.
 *
 * Violations:
 *   - verifySignature() is never called
 *   - No version/rollback comparison
 *   - sourceTrusted() is never called
 *   - install() is reached unconditionally
 */
void updateFirmware(OTAPackage *pkg) {
    printf("[OTA] Received update package: v%u (%u bytes)\n",
           pkg->version, pkg->size);

    /* BUG: No signature check */
    /* BUG: No rollback prevention */
    /* BUG: No source trust validation */

    printf("[OTA] Skipping all security checks... installing directly.\n");
    install(pkg);
}

/* ------------------------------------------------------------------ */
/*  Simulation Driver                                                  */
/* ------------------------------------------------------------------ */

int main() {
    uint8_t fake_payload[1024];
    memset(fake_payload, 0xAB, sizeof(fake_payload));

    /* This could be a malicious downgrade from an untrusted server */
    OTAPackage pkg = {
        .version   = 3,                             /* Rollback! */
        .size      = sizeof(fake_payload),
        .signature = {0},
        .hash      = {0},
        .payload   = fake_payload,
        .server_url = "http://evil.attacker.com",   /* Untrusted! */
        .timestamp = 1700100000,
    };

    updateFirmware(&pkg);

    return 0;
}




