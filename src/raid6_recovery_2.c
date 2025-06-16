#include "gf_raid6.h"

#define BLOCK_SIZE 512
#define SECTOR_COUNT 839

unsigned char* read_file(const char *path, size_t size) {
    FILE *file = fopen(path, "rb");
    if (!file) return NULL;
    
    unsigned char *data = (unsigned char*)malloc(size);
    size_t read = fread(data, 1, size, file);
    fclose(file);
    
    if (read < size) {
        free(data);
        return NULL;
    }
    return data;
}

void recover_part2() {
    FILE *out = fopen("base64.txt", "wb");
    if (!out) {
        perror("Failed to open output file");
        exit(1);
    }

    for (int count = 0; count < SECTOR_COUNT; count++) {
        char paths[8][256];
        for (int i = 0; i < 8; i++) {
            snprintf(paths[i], sizeof(paths[i]), 
                    "part_2/sectors/sd%c.sector%03d", 
                    'a' + i, count);
        }

        unsigned char *sd[8];
        for (int i = 0; i < 8; i++) {
            sd[i] = read_file(paths[i], BLOCK_SIZE);
        }

        int P_index = 7 - (count % 8);
        int Q_index = (P_index + 1) % 8;
        int data_start = (Q_index + 1) % 8;
        
        unsigned char *blocks[6];
        for (int i = 0; i < 6; i++) {
            blocks[i] = sd[(data_start + i) % 8];
        }

        if (missing_drives(blocks, 6) <= 2) {
            eight_drive_recovery(blocks, sd[P_index], sd[Q_index], BLOCK_SIZE);
        }

        for (int i = 0; i < 6; i++) {
            if (blocks[i]) {
                fwrite(blocks[i], 1, BLOCK_SIZE, out);
            }
        }

        for (int i = 0; i < 8; i++) {
            free(sd[i]);
        }
    }

    fclose(out);
}
