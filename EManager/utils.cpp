

//#include "zm_logger.h"

#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>  
#include <dirent.h>
#include <sys/types.h>
#include "em.h"

unsigned int sseversion = 0;

/* Array used by cmmn_byte2hex below. */
static const char hex_digits[16] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

/* Array of all ASCII characters, lower case only.
 * Used by the routines below that transform a string or character
 * into lower case. */
const unsigned char ascii_lc[256] = {
  0, 1, 2, 3, 4, 5, 6, 7,
  8, 9, 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23,
  24, 25, 26, 27, 28, 29, 30, 31,
  ' ', '!', '"', '#', '$', '%', '&', '\'',
  '(', ')', '*', '+', ',', '-', '.', '/',
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', ':', ';', '<', '=', '>', '?',
  '@', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
  'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
  'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
  'x', 'y', 'z', '[', '\\', ']', '^', '_',
  '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
  'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
  'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
  'x', 'y', 'z', '{', '|', '}', '~', 127,
  128, 129, 130, 131, 132, 133, 134, 135,
  136, 137, 138, 139, 140, 141, 142, 143,
  144, 145, 146, 147, 148, 149, 150, 151,
  152, 153, 154, 155, 156, 157, 158, 159,
  160, 161, 162, 163, 164, 165, 166, 167,
  168, 169, 170, 171, 172, 173, 174, 175,
  176, 177, 178, 179, 180, 181, 182, 183,
  184, 185, 186, 187, 188, 189, 190, 191,
  192, 193, 194, 195, 196, 197, 198, 199,
  200, 201, 202, 203, 204, 205, 206, 207,
  208, 209, 210, 211, 212, 213, 214, 215,
  216, 217, 218, 219, 220, 221, 222, 223,
  224, 225, 226, 227, 228, 229, 230, 231,
  232, 233, 234, 235, 236, 237, 238, 239,
  240, 241, 242, 243, 244, 245, 246, 247,
  248, 249, 250, 251, 252, 253, 254, 255};

const std::string stringtf( const char *format, ... )
{
    va_list ap;
    char tempBuffer[8192];
    std::string tempString;

    va_start(ap, format );
    vsnprintf( tempBuffer, sizeof(tempBuffer), format , ap );
    va_end(ap);

    tempString = tempBuffer;

    return( tempString );
}

const std::string stringtf( const std::string &format, ... )
{
    va_list ap;
    char tempBuffer[8192];
    std::string tempString;

    va_start(ap, format );
    vsnprintf( tempBuffer, sizeof(tempBuffer), format.c_str() , ap );
    va_end(ap);

    tempString = tempBuffer;

    return( tempString );
}

bool startsWith( const std::string &haystack, const std::string &needle )
{
    return( haystack.substr( 0, needle.length() ) == needle );
}

StringVector split( const std::string &string, const std::string chars, int limit )
{
    StringVector stringVector;
    std::string tempString = string;
    std::string::size_type startIndex = 0;
    std::string::size_type endIndex = 0;

    //Info( "Looking for '%s' in '%s', limit %d", chars.c_str(), string.c_str(), limit );
    do
    {
        // Find delimiters
        endIndex = string.find_first_of( chars, startIndex );
        //Info( "Got endIndex at %d", endIndex );
        if ( endIndex > 0 )
        {
            //Info( "Adding '%s'", string.substr( startIndex, endIndex-startIndex ).c_str() );
            stringVector.push_back( string.substr( startIndex, endIndex-startIndex ) );
        }
        if ( endIndex == std::string::npos )
            break;
        // Find non-delimiters
        startIndex = tempString.find_first_not_of( chars, endIndex );
        if ( limit && (stringVector.size() == (unsigned int)(limit-1)) )
        {
            stringVector.push_back( string.substr( startIndex ) );
            break;
        }
        //Info( "Got new startIndex at %d", startIndex );
    } while ( startIndex != std::string::npos );
    //Info( "Finished with %d strings", stringVector.size() );

    return( stringVector );
}

const std::string base64Encode( const std::string &inString )
{
	static char base64_table[64] = { '\0' };

	if ( !base64_table[0] )
	{
		int i = 0;
		for ( char c = 'A'; c <= 'Z'; c++ )
			base64_table[i++] = c;
		for ( char c = 'a'; c <= 'z'; c++ )
			base64_table[i++] = c;
		for ( char c = '0'; c <= '9'; c++ )
			base64_table[i++] = c;
		base64_table[i++] = '+';
		base64_table[i++] = '/';
	}

    std::string outString;
    outString.reserve( 2 * inString.size() );

	const char *inPtr = inString.c_str();
	while( *inPtr )
	{
		unsigned char selection = *inPtr >> 2;
		unsigned char remainder = (*inPtr++ & 0x03) << 4;
		outString += base64_table[selection];

		if ( *inPtr )
		{
			selection = remainder | (*inPtr >> 4);
			remainder = (*inPtr++ & 0x0f) << 2;
			outString += base64_table[selection];
		
			if ( *inPtr )
			{
				selection = remainder | (*inPtr >> 6);
				outString += base64_table[selection];
				selection = (*inPtr++ & 0x3f);
				outString += base64_table[selection];
			}
			else
			{
				outString += base64_table[remainder];
				outString += '=';
			}
		}
		else
		{
			outString += base64_table[remainder];
			outString += '=';
			outString += '=';
		}
	}
    return( outString );
}

int split(const char* string, const char delim, std::vector<std::string>& items) {
	if(string == NULL)
		return -1;

	if(string[0] == 0)
		return -2;

	std::string str(string);
	size_t pos;
	
	while(true) {
		pos = str.find(delim);
		items.push_back(str.substr(0, pos));
		str.erase(0, pos+1);

		if(pos == std::string::npos)
			break;
	}

	return items.size();
}

int pairsplit(const char* string, const char delim, std::string& name, std::string& value) {
	if(string == NULL)
		return -1;

	if(string[0] == 0)
		return -2;

	std::string str(string);
	size_t pos = str.find(delim);

	if(pos == std::string::npos || pos == 0 || pos >= str.length())
		return -3;

	name = str.substr(0, pos);
	value = str.substr(pos+1, std::string::npos);

	return 0;
}

/* Sets sse_version  */
void ssedetect() {
#if (defined(__i386__) || defined(__x86_64__))
	/* x86 or x86-64 processor */
	uint32_t r_edx, r_ecx;
	
	__asm__ __volatile__(
	"mov $0x1,%%eax\n\t"
	"cpuid\n\t"
	: "=d" (r_edx), "=c" (r_ecx)
	:
	: "%eax", "%ebx"
	);
	
	if (r_ecx & 0x00000200) {
		sseversion = 35; /* SSSE3 */
		Debug(1,"Detected a x86\\x86-64 processor with SSSE3");
	} else if (r_ecx & 0x00000001) {
		sseversion = 30; /* SSE3 */
		Debug(1,"Detected a x86\\x86-64 processor with SSE3");
	} else if (r_edx & 0x04000000) {
		sseversion = 20; /* SSE2 */
		Debug(1,"Detected a x86\\x86-64 processor with SSE2");
	} else if (r_edx & 0x02000000) {
		sseversion = 10; /* SSE */
		Debug(1,"Detected a x86\\x86-64 processor with SSE");
	} else {
		sseversion = 0;
		Debug(1,"Detected a x86\\x86-64 processor");
	}
	
#else
	/* Non x86 or x86-64 processor, SSE2 is not available */
	Debug(1,"Detected a non x86\\x86-64 processor");
	sseversion = 0;
#endif
}

/* SSE2 aligned memory copy. Useful for big copying of aligned memory like image buffers in ZM */
/* For platforms without SSE2 we will use standard x86 asm memcpy or glibc's memcpy() */
__attribute__((noinline,__target__("sse2"))) void* sse2_aligned_memcpy(void* dest, const void* src, size_t bytes) {
#if ((defined(__i386__) || defined(__x86_64__) || defined(ZM_KEEP_SSE)) && !defined(ZM_STRIP_SSE))
	if(bytes > 128) {
		unsigned int remainder = bytes % 128;
		const uint8_t* lastsrc = (uint8_t*)src + (bytes - remainder);

		__asm__ __volatile__(
		"sse2_copy_iter:\n\t"
		"movdqa (%0),%%xmm0\n\t"
		"movdqa 0x10(%0),%%xmm1\n\t"
		"movdqa 0x20(%0),%%xmm2\n\t"    
		"movdqa 0x30(%0),%%xmm3\n\t"
		"movdqa 0x40(%0),%%xmm4\n\t"
		"movdqa 0x50(%0),%%xmm5\n\t"
		"movdqa 0x60(%0),%%xmm6\n\t"
		"movdqa 0x70(%0),%%xmm7\n\t"
		"movntdq %%xmm0,(%1)\n\t"
		"movntdq %%xmm1,0x10(%1)\n\t"
		"movntdq %%xmm2,0x20(%1)\n\t"
		"movntdq %%xmm3,0x30(%1)\n\t"
		"movntdq %%xmm4,0x40(%1)\n\t"
		"movntdq %%xmm5,0x50(%1)\n\t"
		"movntdq %%xmm6,0x60(%1)\n\t"
		"movntdq %%xmm7,0x70(%1)\n\t"
		"add $0x80, %0\n\t"
		"add $0x80, %1\n\t"
		"cmp %2, %0\n\t"
		"jb sse2_copy_iter\n\t"
		"test %3, %3\n\t"
		"jz sse2_copy_finish\n\t"
		"cld\n\t"
		"rep movsb\n\t"
		"sse2_copy_finish:\n\t"
		:
		: "S" (src), "D" (dest), "r" (lastsrc), "c" (remainder)
		: "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7", "cc", "memory"
		);

	} else {
		/* Standard memcpy */
		__asm__ __volatile__("cld; rep movsb" :: "S"(src), "D"(dest), "c"(bytes) : "cc", "memory");
	}
#else
	/* Non x86\x86-64 platform, use memcpy */
	memcpy(dest,src,bytes);
#endif
	return dest;
}

void timespec_diff(struct timespec *start, struct timespec *end, struct timespec *diff) {
	if (((end->tv_nsec)-(start->tv_nsec))<0) {
		diff->tv_sec = end->tv_sec-start->tv_sec-1;
		diff->tv_nsec = 1000000000+end->tv_nsec-start->tv_nsec;
	} else {
		diff->tv_sec = end->tv_sec-start->tv_sec;
		diff->tv_nsec = end->tv_nsec-start->tv_nsec;
	}
}

void getVersion()
{
}
void getOnlineTime()
{
}



/**
 * create_path
 *
 *   This function creates a whole path, like mkdir -p. Example paths:
 *      this/is/an/example/
 *      /this/is/an/example/
 *   Warning: a path *must* end with a slash!
 *
 * Parameters:
 *
 *   cnt  - current thread's context structure (for logging)
 *   path - the path to create
 *
 * Returns: 0 on success, -1 on failure
 */
int create_path(char *path)
{
    char *start;
    mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

    if (path[0] == '/')
        start = strchr(path + 1, '/');
    else
        start = strchr(path, '/');

    while (start) {
        char *buffer = strdup(path);
        buffer[start-path] = 0x00;

        if (mkdir(buffer, mode) == -1 && errno != EEXIST) {
            //Fatal( "Problem creating directory %s", buffer);
            free(buffer);
            return 0;
        }

        free(buffer);
        start = strchr(start + 1, '/');
    }

    return 1;
}

/**
 * myfopen
 *
 *   This function opens a file, if that failed because of an ENOENT error
 *   (which is: path does not exist), the path is created and then things are
 *   tried again. This is faster then trying to create that path over and over
 *   again. If someone removes the path after it was created, myfopen will
 *   recreate the path automatically.
 *
 * Parameters:
 *
 *   path - path to the file to open
 *   mode - open mode
 *
 * Returns: the file stream object
 */
FILE * myfopen(char *path, const char *mode)
{
    /* first, just try to open the file */
    FILE *dummy = fopen(path, mode);

    /* could not open file... */
    if (!dummy) {
        /* path did not exist? */
        if (errno == ENOENT) {

            /* create path for file... */
            if (create_path(path) == -1)
                return NULL;

            /* and retry opening the file */
            dummy = fopen(path, mode);
            if (dummy)
                return dummy;
        }

        /* two possibilities
         * 1: there was an other error while trying to open the file for the first time
         * 2: could still not open the file after the path was created
         */
        //Fatal("Error opening file %s with mode %s", path, mode);

        return NULL;
    }

    return dummy;
}


 
unsigned long get_file_size(const char *path)  
{  
    unsigned long filesize = -1;      
    struct stat statbuff;  
    if(stat(path, &statbuff) < 0){  
        return filesize;  
    }else{  
       filesize = statbuff.st_size;  
   }  
   return filesize;  
}  

//<=0 error or not found
std::string getLogFilename(const char* path,std::string& id)
{
    DIR *d; 
    struct dirent *file; 
    struct stat buf;    
    string filename_latest="";
    //time_t filetime_latest = timeVal.tv_sec;
    char filename[64] ={0,};
    char timeString[64] = {0};
    struct timeval  timeVal;
    gettimeofday( &timeVal, NULL );
    strftime( timeString, sizeof(timeString), "%Y%m%d", localtime(&timeVal.tv_sec));

    //获取0点时间戳
    tm       *temptm; 
    temptm = localtime(&timeVal.tv_sec);     
    temptm->tm_hour = 0;    
    temptm->tm_min =0;    
    temptm->tm_sec = 0;
    time_t filetime_latest =mktime(temptm);

	
    if(!(d = opendir(path)))
    {
        //printf("error opendir %s!!!\n",path);
        //return -1;
        return "";
    }
    chdir(path);//Add this, so that it can scan the children dir
    while((file = readdir(d)) != NULL)
    {
        if(strncmp(file->d_name, ".", 1) == 0 ||strncmp(file->d_name, "..", 1) == 0)
            continue;
        if(stat(file->d_name, &buf) >= 0 && !S_ISDIR(buf.st_mode))
        {
            std::string filename = file->d_name;
	     int pos = filename.find('-');
	     std::string fileid = filename.substr(pos+1,id.length());
	     if(fileid ==  id && buf.st_mtime >= filetime_latest)
	     {
	     	 filetime_latest = buf.st_mtime;
		 filename_latest = filename;
	     }
            //printf("aaaa%s\n",file->d_name);
            //printf("file size=%d\n",buf.st_size);
            //printf("file last modify time=%d\n",buf.st_mtime);
        }

    }
    closedir(d);
    if(!filename_latest.empty())
    {
	std::string time = filename_latest.substr(0,8);
	if(time == timeString) //同一天
	{
		int pos = filename_latest.rfind('-');
		char logxh[10]={0,};
		std::string sxh = filename_latest.substr(pos+1,5);
		if(sxh.empty())
		{
			return "";
		}
		int ixh = atoi(sxh.c_str()) + 1;
		sprintf(logxh,"%05d",ixh);
		sprintf(filename,"%s%s-%s-%s.log",path,timeString,id.c_str(),logxh);
		//printf("getLogFilename1 %s !!!\n",filename);
	}
	else
	{
		sprintf(filename,"%s%s-%s-00001.log",path,timeString,id.c_str());
		//printf("getLogFilename2 %s!!!\n",filename);
	}
    	
    }
    else
    {
    	sprintf(filename,"%s%s-%s-00001.log",path,timeString,id.c_str());
	//printf("getLogFilename3 %s!!!\n",filename);
    }
    return filename;
}


std::string& trim(std::string& s) {
   if (s.empty()) {
     return s;
   }
   std::string::iterator c;
   // Erase whitespace before the string
   for (c = s.begin(); c != s.end() && ct_iswhitespace(*c++););
     s.erase(s.begin(), --c);
 
   // Erase whitespace after the string
   for (c = s.end(); c != s.begin() && ct_iswhitespace(*--c););
     s.erase(++c, s.end());
 
   return s;
 }
