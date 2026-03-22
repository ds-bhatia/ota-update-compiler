#include <stdint.h>
#include <string.h>

typedef struct {
    int version;
    char source_url[128];
    uint8_t payload[2048];
} FirmwarePackage;

int current_version = 10;

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

    install(pkg);
    return 0;
}

int main(void) {
    FirmwarePackage pkg = {
        .version = 7,
        .source_url = "https://updates.vendor.example/major/fw-v7.bin"
    };
    return updateFirmware(&pkg);
}
