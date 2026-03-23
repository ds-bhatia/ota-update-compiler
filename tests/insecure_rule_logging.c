#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int version;
    char source_url[128];
    uint8_t payload[512];
} FirmwarePackage;

int current_version = 1;

int verifySignature(FirmwarePackage *pkg) {
    (void)pkg;
    return 1;
}

int sourceTrusted(FirmwarePackage *pkg) {
    return strncmp(pkg->source_url, "https://github.com/", strlen("https://github.com/")) == 0;
}

void install(FirmwarePackage *pkg) {
    (void)pkg;
}

int updateFirmware(FirmwarePackage *pkg) {
    if (!verifySignature(pkg)) {
        return -1;
    }

    if (!sourceTrusted(pkg)) {
        return -1;
    }

    if (pkg->version > current_version) {
        printf("[debug] proceeding with update %d\n", pkg->version);
        install(pkg);
        return 0;
    }

    return -1;
}

int main(void) {
    FirmwarePackage pkg = {
        .version = 2,
        .source_url = "https://github.com/edge/fw-v2.bin"
    };
    return updateFirmware(&pkg);
}
