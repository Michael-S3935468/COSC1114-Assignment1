/*
  task4.cpp
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

// Arguments for map4 threads
struct MapThreadArgs {
    unsigned int wordLength;
    unsigned int wordCount;
    unsigned int* indices;
};

// Function declarations
void map4(unsigned int count, pthread_t* threads, MapThreadArgs* threadArgs);
void* map4Thread(void* argPtr);
void reduce4(std::string outPath, pthread_t* threads,
             MapThreadArgs* threadArgs);

int main(int argc, char** argv) {
    // Check argument count
    if (argc != 3) {
        printf("Error: Expected 2 arguments, got %i\n", argc - 1);
        printf("\nUsage: task4 {input_path} {output_path}\n");
        return 1;
    }

    // Read input word list
    std::vector<std::string> lines = readWordList(argv[1]);
    printf("Loaded %li words after filtering\n", lines.size());

    // Perform filtering/deduplication/shuffling
    lines = Task1Filter(lines);

    // Initialise global array of words
    Global = lines;  // new std::string[lines.size()];

    // Create threads for each word count
    pthread_t threads[13];
    MapThreadArgs threadArgs[13];
    map4(lines.size(), threads, threadArgs);

    // Perform reduce/merge step
    reduce4(argv[2], threads, threadArgs);
}

void map4(unsigned int count, pthread_t* threads, MapThreadArgs* threadArgs) {
    printf("map4() count=%u\n", count);

    // Create index arrays and named pipes
    // Count words
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
        assert(mkfifo(getListFilename(args->wordLength, "task4").c_str(),
                      S_IRWXU /* Read/write/execute */) == 0);
    }

    // Determine thread priorities
    // Sort by word count and then prioritise in order of word count
    std::vector<std::tuple<int, int>> idxWordCounts;  // (index, wordCount)
    int priorities[13];

    for (int i = 0; i < 13; i++) {
        idxWordCounts.push_back(
            std::tuple<int, int>(i, threadArgs[i].wordCount));
    }

    auto cmpWordCounts = [](const std::tuple<int, int> a,
                            const std::tuple<int, int> b) {
        // Compare word counts
        return std::get<1>(a) < std::get<1>(b);
    };
    std::sort(idxWordCounts.begin(), idxWordCounts.end(), cmpWordCounts);

    for (unsigned int i = 0; i < idxWordCounts.size(); i++) {
        int threadNo = std::get<0>(idxWordCounts[i]);
        priorities[threadNo] = i + 1; /* 1 to 13 */
        printf("Priority for thread %i (wordLength=%i) = %u\n", threadNo,
               threadNo + 3, i);
    }

    // Create threads and set priorities
    for (unsigned int i = 0; i < 13; i++) {
        printf("Creating thread %i\n", i);
        pthread_create(&threads[i], NULL, map4Thread, &threadArgs[i]);

        // Task 4 - set scheduling policy
        sched_param prioParam;
        prioParam.sched_priority = priorities[i];
        int schedErrno =
            pthread_setschedparam(threads[i], SCHED_RR, &prioParam);

        // Check for errors
        if (schedErrno == EPERM) {
            printf(
                "We don't have permission to set thread priority! re-run with "
                "sudo\n");
            assert(false);
        } else if (schedErrno != 0) {
            printf("pthread_setschedparam failed with error %i", schedErrno);
            assert(false);
        }
    }
}

int map4ThreadCompare(const void* a, const void* b) {
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

void* map4Thread(void* argPtr) {
    MapThreadArgs* args = (MapThreadArgs*)argPtr;

    printf("Thread len=%u %u words\n", args->wordLength, args->wordCount);

    // Sort indices on third letter of each word
    qsort(args->indices, args->wordCount, sizeof(unsigned int),
          map4ThreadCompare);

    printf("Thread len=%u sort complete\n", args->wordLength);

    // Open named pipe for writing.
    // This will block until the corresponding open() call in reduce4().
    int outFile = open(getListFilename(args->wordLength, "task4").c_str(),
                       O_WRONLY /* Write */);

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
struct reduce4WordList {
    // Index of the pipe for this word list.
    int index;

    // Current number of words, including nextWord.
    int wordCount;

    // Word at the front of the word list.
    char nextWord[16];
};

int reduce4Compare(const void* a, const void* b) {
    // Read structs
    reduce4WordList* aList = (reduce4WordList*)a;
    reduce4WordList* bList = (reduce4WordList*)b;

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

void reduce4(std::string outPath, pthread_t* threads,
             MapThreadArgs* threadArgs) {
    printf("reduce4()\n");

    // Open named pipes for reading. Note each map4() thread will block until
    // this point.
    int pipes[13];

    for (int i = 0; i < 13; i++) {
        printf("Opening pipe for reading %i\n", i);
        pipes[i] =
            open(getListFilename(i + 3, "task4").c_str(), O_RDONLY /* Read */);
    }

    // Open outut file stream
    std::ofstream outStream(outPath);
    assert(outStream.good());

    // Initialise word counts and read first word from each list
    reduce4WordList curWords[13];

    for (int i = 0; i < 13; i++) {
        // wordCount is not modified in the map4 threads, so we can safely read
        // it here.
        curWords[i].wordCount = threadArgs[i].wordCount;
        printf("curWords[%i].wordCount = %i\n", i, threadArgs[i].wordCount);
        curWords[i].index = i;
        readNextWord(&curWords[i].nextWord[0], pipes[i], i + 3);
    }

    // Perform reduction step
    qsort(curWords, 13, sizeof(reduce4WordList), reduce4Compare);

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
        qsort(curWords, 13, sizeof(reduce4WordList), reduce4Compare);
    }

    // Wait for map4 threads to finish
    printf("Waiting for map4 threads...\n");

    for (int i = 0; i < 13; i++) {
        void* retVal;
        assert(pthread_join(threads[i], &retVal) == 0);
        assert(retVal == 0);
    }

    printf("map4 threads finished\n");

    // Free memory allocated for indices
    for (int i = 0; i < 13; i++) {
        delete[] threadArgs[i].indices;
    }

    // Close
    for (int i = 0; i < 13; i++) {
        printf("Closing pipe for reading %i\n", i);
        close(pipes[i]);
    }

    printf("reduce4() finished\n");
}
