/*
 * secure.c — Secure OTA Firmware Update Routine
 *
 * This file demonstrates a properly secured firmware update path.
 * All required security invariants are satisfied:
 *   1. Cryptographic signature verification before install
 *   2. Version rollback prevention
 *   3. Source/server trust validation
 *   4. install() only reachable through conditional guards
 *
 * Compile to LLVM IR:
 *   clang -S -emit-llvm -O0 tests/secure.c -o tests/secure.ll
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/*  Data Structures                                                    */
/* ------------------------------------------------------------------ */

/* SHA-256 digest length */
#define HASH_LEN 32

/* Maximum firmware image size: 16 MB */
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
    uint32_t current_version;       /* Currently running FW version     */
    uint32_t last_update_time;      /* Timestamp of last successful upd */
    uint8_t  public_key[64];        /* ECDSA public key for verification*/
    char     trusted_servers[4][256]; /* Allow-listed update servers    */
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
/*  Security Primitives                                                */
/* ------------------------------------------------------------------ */

/*
 * verifySignature — Verify the ECDSA-P256 signature of the firmware
 * package against the device's stored public key and the package hash.
 *
 * Returns 1 if the signature is valid, 0 otherwise.
 */
int verifySignature(OTAPackage *pkg) {
    if (pkg == NULL || pkg->payload == NULL) {
        printf("[SEC] Signature check failed: null package\n");
        return 0;
    }

    /* Verify payload size is within bounds */
    if (pkg->size == 0 || pkg->size > MAX_FIRMWARE_SIZE) {
        printf("[SEC] Signature check failed: invalid size %u\n", pkg->size);
        return 0;
    }

    /*
     * In production this would call into a crypto library:
     *   return ecdsa_verify(device_config.public_key,
     *                       pkg->hash, HASH_LEN,
     *                       pkg->signature, 64);
     *
     * For static analysis purposes we simulate a valid check.
     */
    int sig_valid = 1;  /* Simulated crypto result */
    if (!sig_valid) {
        printf("[SEC] Signature verification FAILED\n");
        return 0;
    }

    printf("[SEC] Signature verification passed\n");
    return 1;
}

/*
 * sourceTrusted — Validate that the update originates from an
 * allow-listed server URL.
 *
 * Returns 1 if the source is trusted, 0 otherwise.
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
 * install — Write the verified firmware image to the device's
 * flash partition and update the device configuration.
 *
 * SECURITY INVARIANT: This function must NEVER be called without
 * prior verification of signature, version, and source trust.
 */
void install(OTAPackage *pkg) {
    printf("[OTA] Writing %u bytes of firmware v%u to flash...\n",
           pkg->size, pkg->version);

    /*
     * Simulated flash write sequence:
     *   1. Erase target partition
     *   2. Write payload in 4KB blocks
     *   3. Verify written data via CRC
     *   4. Update boot configuration to point to new image
     */
    printf("[OTA] Flash write complete\n");

    /* Update device state */
    device_config.current_version  = pkg->version;
    device_config.last_update_time = pkg->timestamp;

    printf("[OTA] Device now running firmware v%u\n", pkg->version);
}

/* ------------------------------------------------------------------ */
/*  Secure Update Entrypoint                                           */
/* ------------------------------------------------------------------ */

/*
 * updateFirmware — The main OTA update handler.
 *
 * This function enforces the full security policy:
 *   1. Validate cryptographic signature  (verifySignature)
 *   2. Prevent version rollback          (version comparison)
 *   3. Check source trustworthiness      (sourceTrusted)
 *   4. Only then proceed with install    (install)
 */
void updateFirmware(OTAPackage *pkg) {
    printf("[OTA] Received update package: v%u (%u bytes)\n",
           pkg->version, pkg->size);

    /* --- Gate 1: Cryptographic signature verification --- */
    if (!verifySignature(pkg)) {
        printf("[REJECT] Update rejected: invalid signature\n");
        return;
    }

    /* --- Gate 2: Anti-rollback check --- */
    if (pkg->version <= device_config.current_version) {
        printf("[REJECT] Update rejected: rollback attempt "
               "(v%u <= v%u)\n", pkg->version,
               device_config.current_version);
        return;
    }

    /* --- Gate 3: Source trust validation --- */
    if (!sourceTrusted(pkg)) {
        printf("[REJECT] Update rejected: untrusted source\n");
        return;
    }

    /* --- All checks passed — safe to install --- */
    printf("[OTA] All security checks passed. Proceeding with install.\n");
    install(pkg);
}

/* ------------------------------------------------------------------ */
/*  Simulation Driver                                                  */
/* ------------------------------------------------------------------ */

int main() {
    /* Simulate a firmware image */
    uint8_t fake_payload[1024];
    memset(fake_payload, 0xAB, sizeof(fake_payload));

    OTAPackage pkg = {
        .version   = 6,
        .size      = sizeof(fake_payload),
        .signature = {0},
        .hash      = {0},
        .payload   = fake_payload,
        .server_url = "https://updates.firmware.example.com",
        .timestamp = 1700100000,
    };

    updateFirmware(&pkg);

    return 0;
}

