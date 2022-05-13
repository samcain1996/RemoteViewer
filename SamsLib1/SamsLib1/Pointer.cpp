#include "Pointer.h"

/**
 * @brief Counts the number of words in a string
 *
 * @param words     string to count words of
 * @return int      Number of words in the string
 */
size_t countWords(const char* words, char delim) {
    size_t wordCount = 0;
    const char* prevChar = nullptr;

    for (size_t index = 0; index < strlen(words); index++) {
        if (index == 0) {
            prevChar = &words[0];
            wordCount = 1;
            continue;
        }

        if (*prevChar == delim) { wordCount++; }
        prevChar = &words[index];
    }

    return wordCount;
}

/**
 * @brief               Convert char* array to a char* with a
 *                      seperator between array elements
 *
 * @param argc          Number of element in array
 * @param args          Array to convert
 * @param seperator     Character separating array elements
 * @return char*        Character string to flattened array
 */
char* flatten(int argc, char** args, char seperator) {

    // Get length of all words with separator between them
    size_t argSize = 0;
    for (int i = 0; i < argc; i++) {
        argSize += strlen(args[i]) + 1;
    }

    // Convert char** to char*
    char* smashed = new char[argSize];
    size_t pos = 0;
    for (int i = 0; i < argc; i++) {
        // Copy word in word char* array to current position in char*
        memcpy(&smashed[pos], args[i], strlen(args[i]));
        pos += strlen(args[i]);

        // Free memory storing word
        delete[] args[i];

        // Concat a separator between words
        memcpy(&smashed[pos], &seperator, sizeof(char));
        pos++;
    }

    // Set null character
    smashed[pos - 1] = static_cast<char>(NULL);

    return smashed;
}

/**
 * @brief               Split char array into multiple char arrays,
 *                      based uppon a specified delimeter
 *
 * @param argc          Number of element in array
 * @param args          Array to convert
 * @param seperator     Character separating array elements
 * @return char*        Character string to flattened array
 */
char** split(const char* args, int* numberOfStrings, char delim)
{
    size_t temp = countWords(args);
    memcpy(numberOfStrings, &temp, sizeof(int));
    char** splitStr = new char* [*numberOfStrings];

    /*
    * curPos        : Position being examined
    * lastDelimPos  : Position of the previous deliminator
    * splitIdx      : Index of each deliminated string
    */

    for (size_t curPos = 0, lastDelimPos = 0, splitIdx = 0;; curPos++) {

        // The character at the current position is the deliminated character
        if (args[curPos] == delim && curPos != NULL) {
            char* str = new char[curPos - lastDelimPos +1 ];
            memcpy(str, &args[lastDelimPos], curPos - lastDelimPos);
            str[curPos - lastDelimPos] = NULL;
            curPos++;
            lastDelimPos = curPos;
            splitStr[splitIdx] = str;
            splitIdx++;

            if (curPos == NULL) { break; }
        }
    }

    return splitStr;
}
