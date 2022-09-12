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
#include <chrono>

#include "Utils.h"

static std::vector<std::string> Global;

// Arguments for map3 threads
struct MapThreadArgs {
    unsigned int wordLength;
    unsigned int wordCount;
    unsigned int* indices;
};

// Function declarations
void map3(unsigned int count, pthread_t* threads, MapThreadArgs* threadArgs);
void* map3Thread(void* argPtr);
void reduce3(std::string outPath, pthread_t* threads,
             MapThreadArgs* threadArgs);

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
    printf("Loaded %li words after filtering\n", lines.size());

    // Initialise global array of words
    Global = lines;  // new std::string[lines.size()];

    // Create threads for each word count
    pthread_t threads[13];
    MapThreadArgs threadArgs[13];
    map3(lines.size(), threads, threadArgs);

    // Perform reduce/merge step
    reduce3(argv[2], threads, threadArgs);
}

void map3(unsigned int count, pthread_t* threads, MapThreadArgs* threadArgs) {
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
        assert(mkfifo(getListFilename(args->wordLength, "task3").c_str(),
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

    std::string aSub = Global[aIdx].substr(2, std::string::npos);
    std::string bSub = Global[bIdx].substr(2, std::string::npos);

    if (aSub < bSub) {
        // a first
        return -1;
    } else {
        // strings are equal or b should be first.
        return 1;
    }
}

void* map3Thread(void* argPtr) {
    // Record start time
    auto threadStart = executionTimingStart();

    MapThreadArgs* args = (MapThreadArgs*)argPtr;

    printf("Thread len=%u %u words\n", args->wordLength, args->wordCount);

    // Sort indices on third letter of each word
    qsort(args->indices, args->wordCount, sizeof(unsigned int),
          map3ThreadCompare);

    printf("Thread len=%u sort complete\n", args->wordLength);

    // Open named pipe for writing.
    // This will block until the corresponding open() call in reduce3().
    int outFile = open(getListFilename(args->wordLength, "task3").c_str(),
                       O_WRONLY /* Write */);

    // Raise named pipe buffer size to reduce the chance of blocking on
    // write.
    fcntl(outFile, F_SETPIPE_SZ, 1048576 /* /proc/sys/fs/pipe-max-size */);

    printf("Thread len=%u opened pipe\n", args->wordLength);

    // Write each word to pipe
    for (unsigned int i = 0; i < args->wordCount; i++) {
        // Write word
        int nWritten =
            write(outFile, Global[args->indices[i]].c_str(), args->wordLength);

        // Make sure we actually wrote all data
        // FIXME: block instead?
        if (nWritten != (int)args->wordLength) {
            printf("Thread len=%u incomplete write (%i < %u) i=%u\n",
                   args->wordLength, nWritten, args->wordLength, i);
            assert(false);
        }
    }

    printf("Thread len=%u writing complete\n", args->wordLength);

    // Close
    close(outFile);

    printf("Thread len=%u closed file\n", args->wordLength);

    // Print elapsed time
    printf("Thread len=%u wordCount=%u elapsed=%f\n", args->wordLength,
           args->wordCount, executionTimingEnd(threadStart));

    return 0;
}

// Read the next word from pipe.
// Assumes the largest word length is 15 characters.
std::string readNextWord(char* buf, int pipe, int wordLength) {
    int curLen = 0;

    // Read one character at a time until we reach NULL
    // NOTE: This may block if a word is not yet available
    while (curLen != wordLength) {
        ssize_t retVal = read(pipe, buf + curLen, wordLength - curLen);

        if (retVal < 0) {
            // Read error
            printf("Read error reading pipe for len=%i\n", wordLength);
        }

        curLen += retVal;
    }

    // Null terminate
    buf[curLen] = '\x00';

    // Convert to std::string implicitly
    return buf;
}

// Represents a word read from a pipe.
struct Reduce3WordList {
    // Index of the pipe for this word list.
    int index;

    // Current number of words, including nextWord.
    int wordCount;

    // Word at the front of the word list.
    char nextWord[16];
};

int reduce3Compare(const void* a, const void* b) {
    // Read structs
    Reduce3WordList* aList = (Reduce3WordList*)a;
    Reduce3WordList* bList = (Reduce3WordList*)b;

    // Handle case where word lists are empty
    if (aList->wordCount == 0) {
        // a empty or both empty. b should go first.
        return 1;
    } else if (bList->wordCount == 0) {
        // b empty only, a should go firstt.
        return -1;
    }

    // Compare from third letter
    std::string aSub =
        std::string(aList->nextWord).substr(2, std::string::npos);
    std::string bSub =
        std::string(bList->nextWord).substr(2, std::string::npos);

    if (aSub < bSub) {
        return -1;
    } else {
        return 1;
    }
}

void reduce3(std::string outPath, pthread_t* threads,
             MapThreadArgs* threadArgs) {
    // Record start time
    auto start = executionTimingStart();

    printf("reduce3()\n");

    // Open named pipes for reading. Note each map3() thread will block until
    // this point.
    int pipes[13];

    for (int i = 0; i < 13; i++) {
        printf("Opening pipe for reading %i\n", i);
        pipes[i] =
            open(getListFilename(i + 3, "task3").c_str(), O_RDONLY /* Read */);
    }

    // Open outut file stream
    std::ofstream outStream(outPath);
    assert(outStream.good());

    // Initialise word counts and read first word from each list
    Reduce3WordList curWords[13];

    for (int i = 0; i < 13; i++) {
        // wordCount is not modified in the map3 threads, so we can safely read
        // it here.
        curWords[i].wordCount = threadArgs[i].wordCount;
        printf("curWords[%i].wordCount = %i\n", i, threadArgs[i].wordCount);
        curWords[i].index = i;
        readNextWord(&curWords[i].nextWord[0], pipes[i], i + 3);
    }

    // Perform reduction step
    qsort(curWords, 13, sizeof(Reduce3WordList), reduce3Compare);

    while (curWords[0].wordCount > 0) {
        // Write word
        outStream << curWords[0].nextWord;
        outStream << "\n";

        // Update word count and read next word if one exists
        if (--curWords[0].wordCount > 0) {
            readNextWord(&curWords[0].nextWord[0], pipes[curWords[0].index],
                         curWords[0].index + 3);
        }

        // Determine next ordered word
        qsort(curWords, 13, sizeof(Reduce3WordList), reduce3Compare);
    }

    // Wait for map3 threads to finish
    printf("Waiting for map3 threads...\n");

    for (int i = 0; i < 13; i++) {
        void* retVal;
        assert(pthread_join(threads[i], &retVal) == 0);
        assert(retVal == 0);
    }

    printf("map3 threads finished\n");

    // Free memory allocated for indices
    for (int i = 0; i < 13; i++) {
        delete[] threadArgs[i].indices;
    }

    // Close
    for (int i = 0; i < 13; i++) {
        printf("Closing pipe for reading %i\n", i);
        close(pipes[i]);
    }

    printf("reduce3() finished, took %f sec\n", executionTimingEnd(start));
}
