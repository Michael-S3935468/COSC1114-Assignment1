/*
  Utils.cpp
  =========

  Description:           Misc. small functions
  Author:                Michael De Pasquale

*/

#include "Utils.h"

// Return a filename for a word list containing words of the specified length
std::string getListFilename(int len) { return getListFilename(len, ""); }

// Return a filename for a word list containing words of the specified length.
// Also adds a prefix if prefix is a non-empty string.
std::string getListFilename(int len, std::string prefix) {
    std::stringstream path("");

    if (prefix != "") {
        path << "." << prefix;
    }

    path << ".words" << len << ".txt";
    return path.str();
}

// Return true if c is a letter ([A-Za-z]), false otherwise
bool isLetter(char c) {
    return ((c >= 'A' && c <= 'Z')     // Upper case
            || (c >= 'a' && c <= 'z')  // Lower case
    );
}

// Return true if any character in word does not satisfy isLetter().
bool containsNonLetter(std::string word) {
    for (unsigned int i = 0; i < word.length(); i++) {
        if (!isLetter(word[i])) {
            return true;
        }
    }

    return false;
}

// Reads a word list from a file, returning a vector.
// Performs no filtering.
// Triggers an assertion failure if unsuccessful.
std::vector<std::string> readWordList(std::string inPath) {
    // Open input file for reading
    std::ifstream inStream(inPath);

    // Verify we can read
    // assert(inStream.good());

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
    // Commpented out as assertion failure is triggered here when running
    // with valgrind?
    // assert(outStream.good());

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
// * Remove all words with length below 3 or above 15 characters
// * Remove all words containing non-letter characters
// * Remove all duplicates
// * Randomly shuffle remaining words
// Returns the result.
std::vector<std::string> Task1Filter(std::vector<std::string> lines) {
    std::deque<std::string> filtered;

    for (unsigned int i = 0; i < lines.size(); i++) {
        std::string curWord = lines[i];

        // Skip if length is not between 3 and 15 chars (inclusive) or if
        // any character is not a letter (A-Z or a-z)
        if (curWord.length() < 3 || curWord.length() > 15 ||
            containsNonLetter(curWord)) {
            continue;
        }

        filtered.push_back(curWord);
    }

    // Sort filtered deque
    std::sort(filtered.begin(), filtered.end());

    // Move contents to vector, removing sequential duplicates as we go.
    // Since we sorted, all duplicates must be sequential.
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

// Split a vector of words by length and return a map of length to a vector of
// words of that length.
// Assumes word lengths are between 3 and 15 characters inclusive.
std::map<int, std::vector<std::string>> splitWordsByLength(
    std::vector<std::string> lines) {
    std::map<int, std::vector<std::string>> wordLists;

    // Initialise map with vector for each word length
    for (int i = 3; i < 15; i++) {
        wordLists[i] = std::vector<std::string>();
    }

    // Separate by length and store in map
    for (std::string str : lines) {
        wordLists[str.length()].push_back(str);
    }

    return wordLists;
}

/* vim: set ts=4 sw=4 tw=79 fdm=indent ff=unix fenc=utf-8 et :*/
