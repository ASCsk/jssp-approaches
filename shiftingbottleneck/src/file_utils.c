#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include "file_utils.h"

/**
 * Extracts the subdirectory from the filename.
 * The subdirectory is determined by the characters before the first digit in the filename.
 * @param filename The filename to extract the subdirectory from.
 * @param subdir The buffer to store the extracted subdirectory.
 * @param maxlen The maximum length of the subdir buffer.
 */
static void extract_subdir_from_filename(const char* filename, char* subdir, size_t maxlen) {
    size_t i = 0;
    while (filename[i] && !isdigit(filename[i]) && i < maxlen - 1) {
        subdir[i] = filename[i];
        i++;
    }
    subdir[i] = '\0';
}

/**
 * Loads a JSSP matrix from a file.
 * The function reads the number of jobs and machines from the first line of the file,
 * and then reads the matrix data into a dynamically allocated 2D array.
 * @param filename The name of the file to load the matrix from.
 * @param num_jobs Pointer to store the number of jobs.
 * @param num_machines Pointer to store the number of machines.
 * @return A pointer to the dynamically allocated 2D array representing the matrix, or NULL on failure.
 */
int** load_jssp_matrix(const char* filename, int* num_jobs, int* num_machines, int* optimum_makespan) {
    char subdir[32];
    extract_subdir_from_filename(filename, subdir, sizeof(subdir));

    char fullpath[PATH_LEN];
    snprintf(fullpath, sizeof(fullpath), "%s%s/%s", JSSP_ROOT, subdir, filename);

    printf("Looking for file at path: %s\n", fullpath);

    FILE* file = fopen(fullpath, "r");
    if (!file) {
        fprintf(stderr, "File not found at path: %s\n", fullpath);
        return NULL;
    }

    // Lookup optimum value if requested
    if (optimum_makespan != NULL) {
        char optimum_path[256];
        snprintf(optimum_path, sizeof(optimum_path), "%s%s/optimum/optimum.csv", JSSP_ROOT, subdir);
        int val = read_optimum_file(optimum_path, filename, optimum_makespan);
        if (val != 0) {
            fprintf(stderr, "Warning: Optimum value not found for %s in %s\n", filename, optimum_path);
            *optimum_makespan = -1;
        }
        else {
            printf("Found optimum makespan: %d\n", *optimum_makespan);
        }
    }

    char line[PATH_LEN];
    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') continue;
    
        if (sscanf(line, "%d %d", num_jobs, num_machines) == 2) {
            // printf("Reading line: %s", line);
            printf("Found matrix dimensions: %d, %d \n", *num_jobs, *num_machines);
            break;
        }
    }

    int rows = *num_jobs;
    int cols = *num_machines * 2;

    int** matrix = malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        matrix[i] = malloc(cols * sizeof(int));
        for (int j = 0; j < cols; j++) {
            if (fscanf(file, "%d", &matrix[i][j]) != 1) {
                fprintf(stderr, "Error reading value at [%d][%d] from file.\n", i, j);
                exit(EXIT_FAILURE);  
            }
        }
    }

    fclose(file);
    return matrix;
}

void print_matrix(int** matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j += 2) {
            printf("M%d T%d ", matrix[i][j], matrix[i][j + 1]);
        }
        printf("\n");
    }
}

void free_matrix(int** matrix, int rows) {
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void print_current_working_directory() {
    char cwd[PATH_LEN];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("\nCurrent working dir: %s\n", cwd);
    }
    else {
        perror("getcwd() error");
    }
}

int read_optimum_file(const char* path, const char* target_filename, int* optimum_out) {
    FILE* file = fopen(path, "r");
    if (!file) {
        perror("Could not open optimum.csv");
        return -1;
    }

    char line[128];
    // Skip header
    if (!fgets(line, sizeof(line), file)) {
        fclose(file);
        return -1;
    }

    while (fgets(line, sizeof(line), file)) {
        char fname[64];
        int makespan;
        if (sscanf(line, "%63[^,],%d", fname, &makespan) == 2) {
            if (strcmp(fname, target_filename) == 0) {
                *optimum_out = makespan;
                fclose(file);
                return 0;
            }
        }
    }

    fclose(file);
    return -1;  // Not found
}

