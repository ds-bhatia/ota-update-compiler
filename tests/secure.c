#include <stdio.h>

typedef struct {
    int version;
} UpdatePkg;

int current_version = 5;

int verify_signature(UpdatePkg *pkg) {
    return 1;
}

int source_is_trusted(UpdatePkg *pkg) {
    return 1;
}

void install(UpdatePkg *pkg) {
    printf("Installing firmware version: %d\n", pkg->version);
}

void updateFirmware(UpdatePkg *pkg) {
    if (verify_signature(pkg) &&
        pkg->version > current_version &&
        source_is_trusted(pkg)) {
        install(pkg);
    }
}

int main() {
    UpdatePkg pkg = { .version = 6 };
    updateFirmware(&pkg);
    return 0;
}

