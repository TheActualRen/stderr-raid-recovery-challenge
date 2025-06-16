#include "gf_raid6.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <libgen.h>

void recover_part1();
void recover_part2();
void extract_base64();
void convert_to_mp4();
void setup_drives();

int main() {
    setup_drives();
    
    init_gf_table();
    init_inverses();
    
    recover_part1();
    
    extract_base64();
    
    int status = system("base64 -d base64_1.txt | gunzip | tar -xvf -");
    if (status != 0) {
        fprintf(stderr, "Error extracting archive\n");
        exit(1);
    }
    
    mkdir("part_2", 0755);
    mkdir("part_2/sectors", 0755);
    
    recover_part2();
    
    convert_to_mp4();
    
    remove("recovered.txt");
    remove("base64_1.txt");
    system("rm -rf part_2");
    
    return 0;
}

void setup_drives() {
    if (access("../drives", F_OK) == 0) {
        return;
    }

    char original_cwd[PATH_MAX];
    if (getcwd(original_cwd, sizeof(original_cwd)) == NULL) {
        perror("getcwd failed");
        exit(1);
    }

    char exe_path[PATH_MAX];
    if (readlink("/proc/self/exe", exe_path, sizeof(exe_path)) == -1) {
        perror("Failed to get executable path");
        exit(1);
    }

    char build_dir[PATH_MAX];
    strncpy(build_dir, dirname(exe_path), sizeof(build_dir));
    
    if (chdir(build_dir) == -1) {
        perror("Failed to enter build directory");
        exit(1);
    }
    
    if (chdir("..") == -1) {
        perror("Failed to enter project root");
        exit(1);
    }

    if (access("part_1.tar.gz", F_OK) != 0) {
        fprintf(stderr, "Error: part_1.tar.gz not found in project root\n");
        exit(1);
    }

    printf("Extracting part_1.tar.gz...\n");
    
    int status = system("tar -xzf part_1.tar.gz");
    if (status != 0) {
        fprintf(stderr, "Error extracting part_1.tar.gz\n");
        exit(1);
    }

    if (rename("part_1", "drives") != 0) {
        perror("Error renaming part_1 to drives");
        exit(1);
    }

    if (chdir(original_cwd) == -1) {
        perror("Failed to return to original directory");
        exit(1);
    }
}


void extract_base64() {
    FILE *in = fopen("recovered.txt", "r");
    FILE *out = fopen("base64_1.txt", "w");
    
    if (!in || !out) {
        perror("File open error");
        exit(1);
    }
    
    char buffer[1024];
    int line_num = 1;
    while (fgets(buffer, sizeof(buffer), in)) {
        if (line_num >= 105 && line_num <= 46092) {
            fputs(buffer, out);
        }
        line_num++;
    }
    
    fclose(in);
    fclose(out);
}

void convert_to_mp4() {
    if (rename("base64.txt", "../video/ricky.mp4") != 0) {
        perror("Error renaming file");
        exit(1);
    }
    printf("Video recovered as video.mp4\n");
}
