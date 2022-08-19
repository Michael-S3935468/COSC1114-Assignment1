/*
  main.cpp
  ========

  Description:           TODO
  Author:                Michael De Pasquale
  Creation Date:         2022-08-19
  Modification Date:     2022-08-19

*/

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

int Task1Filter(std::string inPath, std::string outPath);

int main(int argc, char** argv) {
    // Check argument count
    if (argc != 3) {
        printf("Error: Expected 2 arguments, got %i\n", argc - 1);
        printf("\nUsage: assign1 {input_path} {output_path}\n");
        return 1;
    }

    return Task1Filter(argv[1], argv[2]);
}

// Return true if c is a letter ([A-Za-z]), false otherwise
bool isLetter(char c) {
    return ((c >= 'A' && c <= 'Z')     // Upper case
            || (c >= 'a' && c <= 'z')  // Lower case
    );
}

// Return the input string with all non-letter characters removed.
std::string stripNonLetters(std::string str) {
    std::stringstream stream("");

    // Add characters to stringstream, ignoring non-letters
    for (unsigned int i = 0; i < str.length(); i++) {
        if (isLetter(str[i])) {
            stream << str[i];
        }
    }

    // Convert stringstream to string
    return stream.str();
}

int Task1Filter(std::string inPath, std::string outPath) {
    // Open input file for reading
    std::ifstream inStream(inPath);

    // Verify we can read
    if (!inStream.good()) {
        printf("Failed to open input file!");
        return 255;
    }

    // Create vector to store contents of each line
    std::vector<std::string> lines;

    // Read first line
    std::string curWord;
    std::getline(inStream, curWord);

    // Read each line, remove non-letters, store in vector if 3 <= length <= 15
    while (inStream.good()) {
        curWord = stripNonLetters(curWord);

        if (curWord.length() >= 3 && curWord.length() <= 15) {
            lines.push_back(curWord);
        }

        // Read next line
        std::getline(inStream, curWord);
    }

    // Sort vector
    std::sort(lines.begin(), lines.end());

    // Open output file
    std::ofstream outStream(outPath);

    // Verify we can write
    if (!outStream.good()) {
        printf("Failed to open output file!");
        return 255;
    }

    // Write output file
    // We omit duplicates by taking advantage of sorting
    if (lines.size() > 0) {
        outStream << lines[0];
    }

    for (unsigned int i = 1; i < lines.size(); i++) {
        if (lines[i] != lines[i - 1]) {
            outStream << "\n" << lines[i];
        }
    }

    outStream << "\n";

    return 0;
}

/* vim: set ts=4 sw=4 tw=79 fdm=indent ff=unix fenc=utf-8 et :*/
