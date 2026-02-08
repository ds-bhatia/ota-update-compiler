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
    if (!verifySignature(pkg)) {
        printf("[LOG] Update rejected: invalid signature\n");
        return;
    }
    if (pkg->version <= current_version) {
        printf("[LOG] Update rejected: rollback detected\n");
        return;
    }
    if (!sourceTrusted(pkg)) {
        printf("[LOG] Update rejected: untrusted source\n");
        return;
    }
    printf("[LOG] Update accepted\n");
    install(pkg);
}

int main() {
    UpdatePkg pkg = { .version = 6 };
    updateFirmware(&pkg);
    return 0;
}

// clang-14 -S -emit-llvm tests/secure.c -o tests/secure.ll

