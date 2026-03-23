#include <stdint.h>
#include <string.h>

typedef struct {
    int version;
    char source_url[128];
} FirmwarePackage;

int current_version = 15;

int verifySignature(FirmwarePackage *pkg) {
    (void)pkg;
    return 1;
}

int sourceTrusted(FirmwarePackage *pkg) {
    return strncmp(pkg->source_url, "https://github.com/", strlen("https://github.com/")) == 0;
}

int updateFirmware(FirmwarePackage *pkg) {
    if (!verifySignature(pkg)) {
        return -1;
    }

    if (!sourceTrusted(pkg)) {
        return -1;
    }

    if (pkg->version <= current_version) {
        return 1;
    }

    return 1;
}

int main(void) {
    FirmwarePackage pkg = {
        .version = 16,
        .source_url = "https://github.com/no-install/fw-v16.bin"
    };
    return updateFirmware(&pkg);
}
