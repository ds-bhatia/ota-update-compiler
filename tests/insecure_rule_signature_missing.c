#include <stdint.h>
#include <string.h>

typedef struct {
    int version;
    char source_url[128];
    uint8_t image[1024];
    uint32_t image_size;
} FirmwarePackage;

int current_version = 5;

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
    if (!sourceTrusted(pkg)) {
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
        .version = 7,
        .source_url = "https://github.com/release/fw-v7.bin",
        .image_size = 1024
    };
    return updateFirmware(&pkg);
}
