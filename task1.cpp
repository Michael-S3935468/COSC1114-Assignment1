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
        printf("\nUsage: assign1 {input_path} {output_path}\n");
        return 1;
    }

    // Read input word list
    std::vector<std::string> lines = readWordList(argv[1]);

    // Perform filtering/deduplication/shuffling
    lines = Task1Filter(lines);

    // Write to output file
    writeWordList(argv[2], lines);
}
