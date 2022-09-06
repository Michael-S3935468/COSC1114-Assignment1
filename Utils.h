/*
  Utils.h
  =======

  Description:           Misc. small functions
  Author:                Michael De Pasquale

*/

#ifndef __UTILS_H__
#define __UTILS_H__

#include <assert.h>
#include <time.h>
#include <algorithm>
#include <deque>
#include <fstream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

// Return a filename for a word list containing words of the specified length
std::string getListFilename(int len);

// Same as getListFilename, but adds a prefix.
std::string getListFilename(int len, std::string prefix);

// Return true if c is a letter ([A-Za-z]), false otherwise
bool isLetter(char c);

// Return true if any character in word does not satisfy isLetter().
bool containsNonLetter(std::string word);

// Return the input string with all non-letter characters removed.
std::string stripNonLetters(std::string str);

// Reads a word list from a file, returning a vector.
// Performs no filtering.
std::vector<std::string> readWordList(std::string inPath);

// Writes a sorted vector of words to the given file path, removing
// duplicates.
// MUST pass a sorted list as only sequential duplicates are removed.
void writeWordList(std::string path, std::vector<std::string> lines);

// Performs the following sequence of operations on a given list of words:
// * Remove all non-letter characters (see stripNonLetters)
// * Remove all words with length below 3 or above 15 characters
// * Remove all duplicates
// * Randomly shuffle remaining words
// Returns the result.
std::vector<std::string> Task1Filter(std::vector<std::string> lines);

// Split a vector of words by length and return a map of length to a vector of
// words of that length.
// Assumes word lengths are between 3 and 15 characters inclusive.
std::map<int, std::vector<std::string>> splitWordsByLength(
    std::vector<std::string> lines);

#endif /* __UTILS_H__ */
