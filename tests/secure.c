#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define URL_MAX 128
#define HASH_LEN 32

typedef enum {
    OTA_OK = 0,
    OTA_ERR_SIGNATURE,
    OTA_ERR_SOURCE,
    OTA_ERR_ROLLBACK,
    OTA_ERR_IMAGE
} OtaStatus;

typedef struct {
    int version;
    char channel[16];
    char source_url[URL_MAX];
    uint8_t image_hash[HASH_LEN];
    uint8_t signature[64];
    uint8_t image[512];
    uint32_t image_size;
} FirmwarePackage;

typedef struct {
    int current_version;
    int boot_slot;
    int pending_slot;
    char active_channel[16];
} DeviceState;

int current_version = 5;

static int startsWith(const char *s, const char *prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

int verifySignature(FirmwarePackage *pkg) {
    (void)pkg;
    return 1;
}

int sourceTrusted(FirmwarePackage *pkg) {
    return startsWith(pkg->source_url, "https://github.com/");
}

static int verifyImageHash(const FirmwarePackage *pkg) {
    (void)pkg;
    return 1;
}

void install(FirmwarePackage *pkg) {
    (void)pkg;
}

OtaStatus updateFirmware(DeviceState *dev, FirmwarePackage *pkg) {
    if (!verifySignature(pkg)) {
        return OTA_ERR_SIGNATURE;
    }

    if (!sourceTrusted(pkg)) {
        return OTA_ERR_SOURCE;
    }

    if (pkg->version <= current_version) {
        return OTA_ERR_ROLLBACK;
    }

    if (!verifyImageHash(pkg)) {
        return OTA_ERR_IMAGE;
    }

    install(pkg);
    dev->current_version = pkg->version;
    dev->pending_slot = dev->boot_slot ^ 1;
    return OTA_OK;
}

static void printResult(OtaStatus st) {
    if (st == OTA_OK) {
        printf("OTA completed successfully\n");
        return;
    }

    printf("OTA failed with status code: %d\n", st);
}

int main(void) {
    DeviceState device = {
        .current_version = 5,
        .boot_slot = 0,
        .pending_slot = 1,
        .active_channel = "stable"
    };

    FirmwarePackage pkg = {
        .version = 6,
        .channel = "stable",
        .source_url = "https://github.com/firmware/v6.bin",
        .image_size = 512
    };

    OtaStatus st = updateFirmware(&device, &pkg);
    printResult(st);
    return st == OTA_OK ? 0 : 1;
}

