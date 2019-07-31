#ifndef RAND_STRING_H_
#define RAND_STRING_H_

// Generates a random string of given size "size" and strores it in the given "str"
// Returns the length of the string
long int rand_string(char *str, size_t size);

// Allocates space for a string of given size "size",
// fills the allocated string with random characters
// and returns a pointer to this string
char* rand_string_alloc(size_t size);

// Generates and returns a random interger in the area of "min" to "max"
int rand_number(size_t min, size_t max);

#endif
