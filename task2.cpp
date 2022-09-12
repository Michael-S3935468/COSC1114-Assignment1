/*
  main.cpp
  ========

  Description:           TODO
  Author:                Michael De Pasquale

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

#include "Utils.h"

// Function definitions for map2/reduce2
std::vector<pid_t> map2(std::vector<std::string> list);
void reduce2(std::string outPath);

int main(int argc, char **argv) {
    // Check argument count
    if (argc != 3) {
        printf("Error: Expected 2 arguments, got %i\n", argc - 1);
        printf("\nUsage: task2 {input_path} {output_path}\n");
        return 1;
    }

    // Load input word list
    std::vector<std::string> wordList = readWordList(argv[1]);

    // Run Task1filter (filter/deduplicate/shuffle)
    wordList = Task1Filter(wordList);
    printf("Loaded %li words after filtering\n", wordList.size());

    // Use map2 to separate by word length
    std::vector<pid_t> childPIDs = map2(wordList);

    // Wait for child processes to terminate
    if (childPIDs.size() > 0) {
        // We are the parent
        printf("Waiting for child processes...\n");

        for (pid_t child : childPIDs) {
            printf("Waiting for child pid=%i\n", child);
            waitpid(child, NULL, 0);
        }

        // Reduce
        reduce2(argv[2]);
    } else {
        printf("Child exiting\n");
    }

    return 0;
}

// Separates word lists by word length, sorts each on the third letter and
// writes each to a file using one fork() call per word length in [3, 15]
// to perform these tasks.
// Returns a vector containing the child process PIDs.
std::vector<pid_t> map2(std::vector<std::string> list) {
    printf("map2()\n");

    // Open one file for each word length
    printf("opening temp files\n");
    std::vector<std::ofstream> tmpFiles;

    for (int i = 0; i < 13; i++) {
        tmpFiles.push_back(std::ofstream(getListFilename(i + 3, "tmp.task2")));
        assert(tmpFiles[i].good());
    }

    // Write words of each length to the corresponding file
    printf("Separating words by length\n");

    for (std::string word : list) {
        tmpFiles[word.length() - 3] << word << "\n";
    }

    // Close files
    printf("Closing temp files\n");

    for (int i = 0; i < 13; i++) {
        tmpFiles[i].close();
    }

    // Use fork() to sort each list on the third letter only and save to file
    printf("Forking\n");
    std::vector<pid_t> childPIDs;
    int sortLen = 3;

    for (; sortLen < 16; sortLen++) {
        pid_t pid = fork();

        // Break if child process, otherwise store child PID
        if (pid == 0) {
            // Clear child PID list
            childPIDs.clear();

            break;
        } else {
            childPIDs.push_back(pid);
        }
    }

    // Record start time
    auto childStart = executionTimingStart();

    if (childPIDs.size() == 0) {
        // Child process
        printf("Child sortLen=%i reading word list\n", sortLen);

        // Open corresponding file
        auto targetList = readWordList(getListFilename(sortLen, "tmp.task2"));
        printf("Child sortLen=%i read word list, %li words\n", sortLen,
               targetList.size());

        // Define comparison function for sorting on third letter onwards
        auto cmpLetter3 = [](const std::string &a, const std::string &b) {
            std::string aSub = a.substr(2, std::string::npos);
            std::string bSub = b.substr(2, std::string::npos);

            return aSub < bSub;
        };

        // Child process
        printf("Child sortLen=%i sorting words\n", sortLen);

        // Sort words of specified length on third letter
        std::sort(targetList.begin(), targetList.end(), cmpLetter3);

        // Write result to file
        printf("Child sortLen=%i writing word list\n", sortLen);
        writeWordList(getListFilename(sortLen, "task2"), targetList);

        printf("Child sortLen=%i finished writing word list\n", sortLen);
    }

    // Display time for child process
    printf("Child sortLen=%i took %f sec\n", sortLen,
           executionTimingEnd(childStart));

    // Return child PIDs so we can wait for them to terminate
    // This list will be empty for all child processes
    return childPIDs;
}

void reduce2(std::string outPath) {
    // Record start time
    auto start = executionTimingStart();

    printf("reduce2()\n");

    // 1. Open all lists from map2()
    // Create list of word lists
    std::vector<std::deque<std::string>> wordLists;

    // Load each of the word lists
    for (int i = 3; i < 16; i++) {
        // Use deque for better performance since we need to repeatedly
        // remove the first element from each word list later.
        std::deque<std::string> curList;
        std::vector<std::string> curListVec =
            readWordList(getListFilename(i, "task2"));
        printf("reduce2() read word list len=%i wordCount=%li\n", i,
               curListVec.size());
        std::move(begin(curListVec), end(curListVec), back_inserter(curList));

        wordLists.push_back(curList);
    }

    // 2. While words remain in any list:
    //    - write the first sort ordered word out of the first words of each
    //       list
    //    - advance one position in the list from which the word was written

    // Define comparison function for sorting word lists based on first word
    // in each.
    auto cmpFirstWords = [](const std::deque<std::string> &a,
                            const std::deque<std::string> &b) {
        // Handle case where either or both of the lists are empty. We ensure
        // empty lists are sorted last.
        if (a.size() == 0) {
            // a empty or both empty. In either case b should be first.
            return false;
        } else if (b.size() == 0) {
            // b empty only
            return true;
        }

        // Both lists non-empty
        // Sort on the third letter of each word onwards
        std::string aSub = a[0].substr(2, std::string::npos);
        std::string bSub = b[0].substr(2, std::string::npos);

        return aSub < bSub;
    };

    // Open output file
    std::ofstream outStream(outPath);
    assert(outStream.good());

    while (true) {
        // Sort lists by first word in each
        std::sort(wordLists.begin(), wordLists.end(), cmpFirstWords);

        // If first ordered list is empty, then all others must be empty and we
        // are finished.
        if (wordLists[0].size() == 0) {
            // Ensure we aren't breaking early
            for (auto dq : wordLists) {
                assert(dq.size() == 0);
            }

            break;
        }

        // Write first ordered word and remove it from the corresponding list
        outStream << wordLists[0][0] << "\n";
        wordLists[0].pop_front();
    }

    printf("reduce2() took %f sec\n", executionTimingEnd(start));
}

/* vim: set ts=4 sw=4 tw=79 fdm=indent ff=unix fenc=utf-8 et :*/
