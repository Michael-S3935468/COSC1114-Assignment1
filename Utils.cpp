/*
  Utils.cpp
  =========

  Description:           Misc. small functions
  Author:                Michael De Pasquale

*/

#include "Utils.h"

// Return a filename for a word list containing words of the specified length
std::string getListFilename(int len) {
    std::stringstream path("");
    path << ".words" << len << ".txt";
    return path.str();
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

// Reads a word list from a file, returning a vector.
// Performs no filtering.
// Triggers an assertion failure if unsuccessful.
std::vector<std::string> readWordList(std::string inPath) {
    // Open input file for reading
    std::ifstream inStream(inPath);

    // Verify we can read
    assert(inStream.good());

    // Create vector to store contents of each line
    std::vector<std::string> lines;

    // Read first line
    std::string curWord;
    std::getline(inStream, curWord);

    // Read each line and store in vector
    while (inStream.good()) {
        lines.push_back(curWord);

        // Read next line
        std::getline(inStream, curWord);
    }

    return lines;
}

// Writes a sorted vector of words to the given file path, removing
// duplicates.
// MUST pass a sorted list as only sequential duplicates are removed.
// Triggers an assertion failure if unsuccessful.
void writeWordList(std::string path, std::vector<std::string> lines) {
    // Open output file
    std::ofstream outStream(path);

    // Verify we can write
    assert(outStream.good());

    // Write output file
    // We omit duplicates by taking advantage of sorting
    if (lines.size() > 0) {
        outStream << lines[0];
    }

    for (unsigned int i = 1; i < lines.size(); i++) {
        outStream << "\n" << lines[i];
    }

    outStream << "\n";
}

// Performs the following sequence of operations on a given list of words:
// * Remove all non-letter characters (see stripNonLetters)
// * Remove all words with length below 3 or above 15 characters
// * Remove all duplicates
// * Randomly shuffle remaining words
// Returns the result.
std::vector<std::string> Task1Filter(std::vector<std::string> lines) {
    std::deque<std::string> filtered;

    // Remove non-letters, then store in deque if 3 <= length <= 15
    for (unsigned int i = 0; i < lines.size(); i++) {
        std::string curWord = stripNonLetters(lines[i]);

        if (curWord.length() >= 3 && curWord.length() <= 15) {
            filtered.push_back(curWord);
        }
    }

    // Sort filtered deque
    std::sort(filtered.begin(), filtered.end());

    // Move contents to vector, removing sequential duplicates as we go.
    // We exploit sorting here as all duplicates must be sequential.
    std::vector<std::string> dedup;

    if (!filtered.empty()) {
        std::string prev = filtered[0];
        std::string next = filtered[0];

        // Add first element
        dedup.push_back(next);
        filtered.pop_front();

        // Add remaining element, skipping sequential duplicates
        while (!filtered.empty()) {
            prev = next;
            next = filtered[0];

            if (prev != next) {
                dedup.push_back(next);
            }

            filtered.pop_front();
        }
    }

    // Initialise RNG, then shuffle randomly
    std::mt19937 rng(time(NULL));
    std::shuffle(dedup.begin(), dedup.end(), rng);

    return dedup;
}

/* vim: set ts=4 sw=4 tw=79 fdm=indent ff=unix fenc=utf-8 et :*/
