#include <stdint.h>

typedef struct {
    int version;
    char source_url[128];
    uint8_t manifest[256];
} FirmwarePackage;

int current_version = 3;

int verifySignature(FirmwarePackage *pkg) {
    (void)pkg;
    return 1;
}

int sourceTrusted(FirmwarePackage *pkg) {
    (void)pkg;
    return 1;
}

void install(FirmwarePackage *pkg) {
    (void)pkg;
}

int updateFirmware(FirmwarePackage *pkg) {
    if (!verifySignature(pkg)) {
        return -1;
    }

    if (pkg->version > current_version) {
        install(pkg);
        return 0;
    }

    return -1;
}

int main(void) {
    FirmwarePackage pkg = {
        .version = 4,
        .source_url = "http://mirror.local/channel/stable.bin"
    };
    return updateFirmware(&pkg);
}
