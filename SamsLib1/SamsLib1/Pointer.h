#include <string>
#include <vector>

size_t countWords(const char* words, char delim = ' ');
char* flatten(int argc, char** args, char seperator = ' ');
char** split(const char* args, int* numberOfStrings, char delim = ' ');
std::string* split(const std::string& src, int* numberOfWords, char delim = ' ');
std::vector<std::string> split(const std::string&);

template <typename T>
size_t ptrlen(T* ptr, T delim) {
	if (ptr == nullptr) { return 0; }
	size_t len = 0;
	while (*(ptr + len++) != delim) {}
	return len;
}
