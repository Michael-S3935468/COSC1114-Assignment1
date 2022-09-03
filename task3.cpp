/*
  task3.cpp
  =========

  Description:           Task 3 solution
  Author:                Michael De Pasquale

*/

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "Utils.h"

static std::vector<std::string> Global;

// Arguments for map3 threads
struct MapThreadArgs {
    unsigned int wordLength;
    unsigned int wordCount;
    unsigned int* indices;
};

// Function declarations
void map3(unsigned int count);
void* map3Thread(void* argPtr);

int main(int argc, char** argv) {
    // Check argument count
    if (argc != 3) {
        printf("Error: Expected 2 arguments, got %i\n", argc - 1);
        printf("\nUsage: task3 {input_path} {output_path}\n");
        return 1;
    }

    // Read input word list
    std::vector<std::string> lines = readWordList(argv[1]);

    // Perform filtering/deduplication/shuffling
    lines = Task1Filter(lines);

    // Initialise global array of words
    Global = lines;  // new std::string[lines.size()];

    map3(lines.size());
}

void map3(unsigned int count) {
    pthread_t threads[13];
    MapThreadArgs threadArgs[13];
    printf("map3() count=%u\n", count);

    // Create index arrays and threads
    for (unsigned int i = 3; i < 15; i++) {
        MapThreadArgs* args = &threadArgs[i - 3];
        args->wordLength = i;
        args->wordCount = 0;

        // Count words with length i
        for (unsigned int j = 0; j < count; j++) {
            if (Global[j].length() == args->wordLength) {
                args->wordCount++;
            }
        }

        printf("Counted %u words with length %u\n", args->wordCount,
               args->wordLength);

        // Create index array and store indices of all words with length i
        args->indices = new unsigned int[args->wordCount];
        int curIndexPos = 0;

        for (unsigned int j = 0; j < count; j++) {
            if (Global[j].length() == args->wordLength) {
                // Store current index and advance position
                args->indices[curIndexPos] = j;
                curIndexPos++;
            }
        }

        // Create thread
        printf("Creating thread %i\n", i - 3);
        pthread_create(&threads[i - 3], NULL, map3Thread, args);
    }
}

int map3ThreadCompare(const void* a, const void* b) {
    // TODO
    return 0;
}

void* map3Thread(void* argPtr) {
    MapThreadArgs* args = (MapThreadArgs*)argPtr;

    printf("Thread len=%u %u words\n", args->wordLength, args->wordCount);

    // Sort on third letter of each word
    // qsort(
    //     args->indices,

    // )

    return 0;
}

void reduce3() {}
