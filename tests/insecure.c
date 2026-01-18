#include <stdio.h>

typedef struct {
    int version;
} UpdatePkg;

int current_version = 5;

int verifySignature(UpdatePkg *pkg) {
    return 1;
}

int sourceTrusted(UpdatePkg *pkg) {
    return 1;
}

void install(UpdatePkg *pkg) {
    printf("Installing firmware version: %d\n", pkg->version);
}

void updateFirmware(UpdatePkg *pkg) {
    install(pkg);
}

int main() {
    UpdatePkg pkg = { .version = 6 };
    updateFirmware(&pkg);
    return 0;
}




