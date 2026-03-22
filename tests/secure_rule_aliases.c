#include <stdint.h>
#include <string.h>

typedef struct {
    int version;
    char source_url[128];
    uint8_t image[512];
} FirmwarePackage;

int current_version = 8;

int checkSignature(FirmwarePackage *pkg) {
    (void)pkg;
    return 1;
}

int validateSource(FirmwarePackage *pkg) {
    return strncmp(pkg->source_url, "https://updates.vendor.example/", 31) == 0;
}

void applyUpdate(FirmwarePackage *pkg) {
    (void)pkg;
}

int updateFirmware(FirmwarePackage *pkg) {
    if (!checkSignature(pkg)) {
        return -1;
    }

    if (!validateSource(pkg)) {
        return -1;
    }

    if (pkg->version <= current_version) {
        return -1;
    }

    applyUpdate(pkg);
    return 0;
}

int main(void) {
    FirmwarePackage pkg = {
        .version = 9,
        .source_url = "https://updates.vendor.example/alias/fw-v9.bin"
    };
    return updateFirmware(&pkg);
}
