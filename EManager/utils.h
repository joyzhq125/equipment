

#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include <sys/time.h>
#include <string>
#include <vector>

typedef std::vector<std::string> StringVector;

const std::string stringtf( const char *format, ... );
const std::string stringtf( const std::string &format, ... );

bool startsWith( const std::string &haystack, const std::string &needle );
StringVector split( const std::string &string, const std::string chars, int limit=0 );

const std::string base64Encode( const std::string &inString );

int split(const char* string, const char delim, std::vector<std::string>& items);
int pairsplit(const char* string, const char delim, std::string& name, std::string& value);

inline int max( int a, int b )
{
    return( a>=b?a:b );
}

inline int min( int a, int b )
{
    return( a<=b?a:b );
}

void ssedetect();
void* sse2_aligned_memcpy(void* dest, const void* src, size_t bytes);
void timespec_diff(struct timespec *start, struct timespec *end, struct timespec *diff);


void getVersion();
void getOnlineTime();

int create_path(char *path);
FILE * myfopen(char *path, const char *mode);
std::string& trim(std::string& s);

unsigned long get_file_size(const char *path);
std::string getLogFilename(const char* path,std::string& id);
extern unsigned int sseversion;

#endif // UTILS_H
