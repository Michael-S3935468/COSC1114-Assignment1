/*
  main.cpp
  ========

  Description:           TODO
  Author:                Michael De Pasquale
  Creation Date:         2022-08-19
  Modification Date:     2022-09-03

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
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

    // Use map2 to separate by word length
    std::vector<pid_t> childPIDs = map2(wordList);

    // Wait for child processes to terminate
    for (pid_t child : childPIDs) {
        waitpid(child, NULL, 0);
    }

    // Reduce
    reduce2(argv[2]);
}

// Separates word lists by word length, sorts each on the third letter and
// writes each to a file using one fork() call per word length in [3, 15]
// to perform these tasks.
// Returns a vector containing the child process PIDs.
std::vector<pid_t> map2(std::vector<std::string> list) {
    // Create map to store lists for each length
    std::map<int, std::vector<std::string>> wordLists;

    // Initialise map with vector for each word length
    for (int i = 3; i < 15; i++) {
        wordLists[i] = std::vector<std::string>();
    }

    // Separate by length and store in map
    for (std::string str : list) {
        wordLists[str.length()].push_back(str);
    }

    // Use fork() to sort each list on the third letter only and save to file
    std::vector<pid_t> childPIDs;
    int sortLen = 3;

    for (; sortLen < 15; sortLen++) {
        int pid = fork();

        // Break if child process, otherwise store child PID
        if (pid == 0) {
            break;
        } else {
            childPIDs.push_back(pid);
        }
    }

    // Define comparison function for sorting on third letter
    auto cmpLetter3 = [](const std::string &a, const std::string &b) {
        return a[2] < b[2];
    };

    // Sort words of specified length on third letter (?)
    std::sort(wordLists[sortLen].begin(), wordLists[sortLen].end(), cmpLetter3);

    // Write result to file
    writeWordList(getListFilename(sortLen), wordLists[sortLen]);

    // Return child PIDs so we can wait for them to terminate
    return childPIDs;
}

void reduce2(std::string outPath) {
    // 1. Open all lists from map2()
    // 2. Sort each list
    // Create list of word lists
    std::vector<std::vector<std::string>> wordLists;

    // Load each of the word lists
    for (int i = 3; i < 15; i++) {
        std::vector<std::string> curList = readWordList(getListFilename(i));
        std::sort(curList.begin(), curList.end());
        wordLists.push_back(curList);
    }

    // 3. While words remain in any list:
    //    - write the first sort ordered word out of the first words of each
    //       list
    //    - advance one position in the list from which the word was written

    // Define comparison function for sorting word lists based on first word
    // in each.
    auto cmpFirstWords = [](const std::vector<std::string> &a,
                            const std::vector<std::string> &b) {
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
        return a[0] < b[0];
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
            break;
        }

        // Write first ordered word and remove it from the corresponding list
        outStream << wordLists[0][0] << "\n";
        wordLists[0].erase(wordLists[0].begin());
    }
}

/* vim: set ts=4 sw=4 tw=79 fdm=indent ff=unix fenc=utf-8 et :*/
