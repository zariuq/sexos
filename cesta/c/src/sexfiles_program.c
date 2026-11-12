#ifndef CESTA_OWNERSHIP_ABI_PROFILE
#define CESTA_OWNERSHIP_ABI_PROFILE 1
#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");
#endif
#include "sexfiles.h"
#include "kernel_current.h"
#include "sexos_memory_regions.h"
#include "sexfiles_program.h"

#define PROGRAM_DIRECTORY_LBA 2048u
#define PROGRAM_SECTOR_BYTES 512u
#define PROGRAM_DIRECTORY_SECTORS_MAX 64u
#define PROGRAM_DIRECTORY_BYTES (PROGRAM_SECTOR_BYTES * PROGRAM_DIRECTORY_SECTORS_MAX)
#define PROGRAM_IMAGE_BYTES 33554432u
#define PROGRAM_IMAGE_SLOT 100u
#define PROGRAM_IMAGE_REGION_ID 0x4000u
#define PROGRAM_REPLY_TAG UINT64_C(0x50)

enum {
    PROGRAM_NOT_FOUND = 1,
    PROGRAM_TOO_LARGE = 2,
    PROGRAM_IO = 3,
    PROGRAM_CORRUPT = 4
};

static uint8_t directory[PROGRAM_DIRECTORY_BYTES];
static uint8_t sector[PROGRAM_SECTOR_BYTES];

static uint32_t le32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static uint64_t le64(const uint8_t *p) {
    return (uint64_t)le32(p) | ((uint64_t)le32(p + 4) << 32);
}

static uint64_t reply_failed(uint32_t error) {
    return (PROGRAM_REPLY_TAG << 56) | error;
}

static uint64_t reply_loaded(uint32_t len) {
    return (PROGRAM_REPLY_TAG << 56) | (UINT64_C(1) << 48) | len;
}

static uint8_t *program_image_region(void) {
    MemoryRegionReply view = sexos_memory_region_view(PROGRAM_IMAGE_SLOT, 0, PROGRAM_IMAGE_BYTES,
                                                       REGION_QUERY | REGION_READ | REGION_WRITE);
    if (view.status || !view.value0 || view.value0 == UINT64_MAX ||
        view.value1 != PROGRAM_IMAGE_BYTES || view.value3 != PROGRAM_IMAGE_REGION_ID)
        return NULL;
    return (uint8_t *)(uintptr_t)view.value0;
}

static uint32_t program_read_sector(uint64_t lba, uint8_t *out) {
    uint64_t bridge = diskfs_bridge_get_buf_va();
    if (!bridge || bridge == UINT64_MAX) return PROGRAM_IO;
    if (diskfs_block_read(lba * PROGRAM_SECTOR_BYTES, PROGRAM_SECTOR_BYTES,
                          SEXOS_SLOT_buf_lend) != 0)
        return PROGRAM_IO;
    memcpy(out, (const void *)(uintptr_t)bridge, PROGRAM_SECTOR_BYTES);
    return 0;
}

static uint64_t fnv(const uint8_t *bytes, size_t len) {
    uint64_t hash = UINT64_C(0xcbf29ce484222325);
    for (size_t i = 0; i < len; ++i) {
        hash ^= bytes[i];
        hash *= UINT64_C(0x100000001b3);
    }
    return hash;
}

static uint64_t program_try_load(uint64_t name0, uint64_t name1) {
    uint8_t *image = program_image_region();
    if (!image) return reply_failed(PROGRAM_IO);
    uint32_t status = program_read_sector(PROGRAM_DIRECTORY_LBA, directory);
    if (status) return reply_failed(status);
    static const uint8_t magic[8] = {'S','E','X','P','R','O','G','1'};
    if (memcmp(directory, magic, 8) != 0) return reply_failed(PROGRAM_NOT_FOUND);
    uint32_t sectors = le32(directory + 8);
    if (!sectors) sectors = 1;
    if (sectors > PROGRAM_DIRECTORY_SECTORS_MAX) return reply_failed(PROGRAM_NOT_FOUND);
    for (uint32_t i = 1; i < sectors; ++i) {
        status = program_read_sector(PROGRAM_DIRECTORY_LBA + i, directory + i * PROGRAM_SECTOR_BYTES);
        if (status) return reply_failed(status);
    }
    uint8_t wanted[16];
    memcpy(wanted, &name0, 8);
    memcpy(wanted + 8, &name1, 8);
    uint64_t lba = 0, expected_hash = 0;
    uint32_t len = 0;
    bool found = false;
    for (uint32_t s = 0; s < sectors && !found; ++s) {
        const uint8_t *entries = directory + s * PROGRAM_SECTOR_BYTES + 16;
        for (size_t i = 0; i < 15; ++i) {
            const uint8_t *entry = entries + i * 32;
            if (memcmp(entry, wanted, 16) != 0) continue;
            lba = le32(entry + 16);
            len = le32(entry + 20);
            expected_hash = le64(entry + 24);
            found = true;
            break;
        }
    }
    if (!found) return reply_failed(PROGRAM_NOT_FOUND);
    if (!len || len > PROGRAM_IMAGE_BYTES) return reply_failed(PROGRAM_TOO_LARGE);
    if (lba < PROGRAM_DIRECTORY_LBA + sectors) return reply_failed(PROGRAM_CORRUPT);
    size_t done = 0;
    while (done < len) {
        status = program_read_sector(lba, sector);
        if (status) return reply_failed(status);
        size_t used = len - done < PROGRAM_SECTOR_BYTES ? len - done : PROGRAM_SECTOR_BYTES;
        memcpy(image + done, sector, used);
        done += used;
        ++lba;
    }
    if (fnv(image, len) != expected_hash) {
        sexos_line("[files.program.corrupt] bytes=%u", (unsigned)len);
        return reply_failed(PROGRAM_CORRUPT);
    }
    sexos_line("[files.program.read] bytes=%u lba=%llu", (unsigned)len,
               (unsigned long long)(lba - 1));
    return reply_loaded(len);
}

uint64_t sexfiles_program_load(uint64_t name0, uint64_t name1) {
    return program_try_load(name0, name1);
}
