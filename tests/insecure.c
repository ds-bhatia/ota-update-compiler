#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define URL_MAX 128

typedef struct {
    int version;
    char source_url[URL_MAX];
    uint8_t signature[64];
    uint8_t image[512];
    uint32_t image_size;
} FirmwarePackage;

int current_version = 5;

int verifySignature(FirmwarePackage *pkg) {
    (void)pkg;
    return 1;
}

int sourceTrusted(FirmwarePackage *pkg) {
    return strncmp(pkg->source_url, "https://updates.vendor.example/", 31) == 0;
}

int SHA1(const uint8_t *buffer, uint32_t len) {
    (void)buffer;
    (void)len;
    return 1;
}

void install(FirmwarePackage *pkg) {
    printf("[device] flashing firmware v%d (%u bytes)\n", pkg->version, pkg->image_size);
}

int updateFirmware(FirmwarePackage *pkg) {
    printf("[debug] received OTA from %s\n", pkg->source_url);

    if (rand() % 2 == 0) {
        printf("[debug] random throttle applied\n");
    }

    if (!SHA1(pkg->image, pkg->image_size)) {
        return -1;
    }

    install(pkg);

    if (!sourceTrusted(pkg)) {
        printf("[warn] source was not trusted but image already installed\n");
    }

    if (!verifySignature(pkg)) {
        printf("[warn] signature failed after installation\n");
    }

    if (pkg->version <= current_version) {
        printf("[warn] rollback detected after installation\n");
    }

    return 0;
}

int main(void) {
    FirmwarePackage pkg = {
        .version = 4,
        .source_url = "http://mirror.local/firmware/latest.bin",
        .image_size = 512
    };

    return updateFirmware(&pkg);
}




