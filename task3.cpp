/*
  task3.cpp
  =========

  Description:           Task 3 solution
  Author:                Michael De Pasquale

*/

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "Utils.h"

static std::vector<std::string> Global;

// Arguments for map3 threads
struct MapThreadArgs {
    unsigned int wordLength;
    unsigned int wordCount;
    unsigned int* indices;
};

// Function declarations
void map3(unsigned int count, pthread_t threads[13],
          MapThreadArgs threadArgs[13]);
void* map3Thread(void* argPtr);
void reduce3(std::string outPath, pthread_t threads[13],
             MapThreadArgs threadArgs[13]);

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

    // Create threads for each word count
    pthread_t threads[13];
    MapThreadArgs threadArgs[13];
    map3(lines.size(), threads, threadArgs);

    // Perform reduce/merge step
    reduce3(argv[2], threads, threadArgs);
}

void map3(unsigned int count, pthread_t threads[13],
          MapThreadArgs threadArgs[13]) {
    printf("map3() count=%u\n", count);

    // Create index arrays, threads and named pipes
    for (unsigned int i = 3; i < 16; i++) {
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

        // Create named pipe for writing.
        assert(mkfifo(getListFilename(args->wordLength).c_str(),
                      S_IRWXU /* Read/write/execute */) == 0);

        // Create thread
        printf("Creating thread %i\n", i - 3);
        pthread_create(&threads[i - 3], NULL, map3Thread, args);
    }
}

int map3ThreadCompare(const void* a, const void* b) {
    // Read indices from array
    int aIdx = *(int*)a;
    int bIdx = *(int*)b;

    // Get third letter of each word
    char aLetter = Global[aIdx][2];
    char bLetter = Global[bIdx][2];

    // Return integer indicating sort order
    return (int)aLetter - (int)bLetter;
}

void* map3Thread(void* argPtr) {
    MapThreadArgs* args = (MapThreadArgs*)argPtr;

    printf("Thread len=%u %u words\n", args->wordLength, args->wordCount);

    // Sort indices on third letter of each word
    qsort(args->indices, args->wordCount, sizeof(unsigned int),
          map3ThreadCompare);

    printf("Thread len=%u sort complete\n", args->wordLength);

    // Open named pipe for writing.
    // This will block until the corresponding open() call in reduce3().
    int outFile =
        open(getListFilename(args->wordLength).c_str(), O_WRONLY /* Write */);

    printf("Thread len=%u opened pipe\n", args->wordLength);

    // Write each word to pipe
    for (unsigned int i = 0; i < args->wordCount; i++) {
        // Write word (and null terminator)
        int nWritten = write(outFile, Global[args->indices[i]].c_str(),
                             args->wordLength + 1);

        // Make sure we actually wrote all data
        // FIXME: block instead?
        if (nWritten != (int)(args->wordLength + 1)) {
            printf("Thread len=%u incomplete write (%i < %u) i=%u\n",
                   args->wordLength, nWritten, args->wordLength + 1, i);
            assert(false);
        }
    }

    printf("Thread len=%u writing complete\n", args->wordLength);

    // Close
    close(outFile);

    printf("Thread len=%u closed file\n", args->wordLength);

    return 0;
}

void reduce3(std::string outPath, pthread_t threads[13],
             MapThreadArgs threadArgs[13]) {
    printf("reduce3()\n");

    // Open named pipes for reading. Note each map3() thread will block until
    // this point.
    int pipes[13];

    for (int i = 0; i < 13; i++) {
        printf("Opening pipe for reading %i\n", i);
        pipes[i] = open(getListFilename(i + 3).c_str(), O_RDONLY /* Read */);
    }

    // Open outut file stream
    std::ofstream outStream(outPath);
    assert(outStream.good());

    // Perform reduction step
    // std::string curWords[13];
    // int indices[13];

    // Wait for map3 threads to finish
    printf("Waiting for map3 threads...\n");

    for (int i = 0; i < 13; i++) {
        void* retVal;
        assert(pthread_join(threads[i], &retVal) == 0);
        assert(retVal == 0);
    }

    printf("map3 threads finished\n");

    // Close
    for (int i = 0; i < 13; i++) {
        printf("Closing pipe for reading %i\n", i);
        close(pipes[i]);
    }

    printf("reduce3() finished\n");
}
