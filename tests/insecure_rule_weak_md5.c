#include <stdint.h>
#include <string.h>

typedef struct {
    int version;
    char source_url[128];
    uint8_t payload[2048];
    uint32_t size;
} FirmwarePackage;

int current_version = 30;

int verifySignature(FirmwarePackage *pkg) {
    (void)pkg;
    return 1;
}

int sourceTrusted(FirmwarePackage *pkg) {
    return strncmp(pkg->source_url, "https://github.com/", strlen("https://github.com/")) == 0;
}

int MD5(const uint8_t *data, uint32_t size) {
    (void)data;
    (void)size;
    return 1;
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

    if (!MD5(pkg->payload, pkg->size)) {
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
        .version = 31,
        .source_url = "https://github.com/prod/fw-v31.bin",
        .size = 2048
    };
    return updateFirmware(&pkg);
}
