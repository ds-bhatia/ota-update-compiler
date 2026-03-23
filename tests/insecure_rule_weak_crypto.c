#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int version;
    char source_url[128];
    uint8_t payload[1024];
} FirmwarePackage;

int current_version = 20;

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
        if ((rand() & 1) == 0) {
            install(pkg);
            return 0;
        }
    }

    return -1;
}

int main(void) {
    FirmwarePackage pkg = {
        .version = 21,
        .source_url = "https://github.com/prod/fw-v21.bin"
    };
    return updateFirmware(&pkg);
}
