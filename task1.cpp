/*
  task1.cpp
  =========

  Description:           Task 1 solution
  Author:                Michael De Pasquale

*/

#include "Utils.h"

int main(int argc, char** argv) {
    // Check argument count
    if (argc != 3) {
        printf("Error: Expected 2 arguments, got %i\n", argc - 1);
        printf("\nUsage: task1 {input_path} {output_path}\n");
        return 1;
    }

    // Read input word list
    std::vector<std::string> lines = readWordList(argv[1]);

    // Record start time
    auto start = executionTimingStart();

    // Perform filtering/deduplication/shuffling
    lines = Task1Filter(lines);

    printf("Task1Filter took %f sec\n", executionTimingEnd(start));

    // Write to output file
    writeWordList(argv[2], lines);
}
