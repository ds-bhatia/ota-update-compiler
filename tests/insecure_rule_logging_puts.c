#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int version;
    char source_url[128];
} FirmwarePackage;

int current_version = 2;

int verifySignature(FirmwarePackage *pkg) {
    (void)pkg;
    return 1;
}

int sourceTrusted(FirmwarePackage *pkg) {
    return strncmp(pkg->source_url, "https://updates.vendor.example/", 31) == 0;
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
        puts("debug: preparing to flash image");
        install(pkg);
        return 0;
    }

    return -1;
}

int main(void) {
    FirmwarePackage pkg = {
        .version = 3,
        .source_url = "https://updates.vendor.example/logging/fw-v3.bin"
    };
    return updateFirmware(&pkg);
}
