#ifndef _BASE_64_
#define _BASE_64_

// Encode binary input file writing result to output file
int decode64(FILE*,FILE*);

// Decode input base64 file writing results to output file
int encode64(FILE*,FILE*);

#endif // _BASE_64_

