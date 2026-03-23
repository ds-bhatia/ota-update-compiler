#include <stdint.h>
#include <string.h>

typedef enum {
    OTA_OK = 0,
    OTA_ERR
} OtaStatus;

typedef struct {
    int version;
    char source_url[128];
    uint8_t image[1024];
    uint32_t image_size;
} FirmwarePackage;

typedef struct {
    int active_version;
} DeviceState;

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

OtaStatus updateFirmware(DeviceState *dev, FirmwarePackage *pkg) {
    if (!verifySignature(pkg)) {
        return OTA_ERR;
    }

    if (!sourceTrusted(pkg)) {
        return OTA_ERR;
    }

    if (pkg->version > current_version) {
        install(pkg);
        dev->active_version = pkg->version;
        return OTA_OK;
    }

    return OTA_ERR;
}

int main(void) {
    DeviceState dev = { .active_version = 5 };
    FirmwarePackage pkg = {
        .version = 7,
        .source_url = "https://github.com/release/fw-v7.bin",
        .image_size = 1024
    };
    return updateFirmware(&dev, &pkg);
}
