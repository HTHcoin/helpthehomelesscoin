/*
 * base64.cpp
 *
 *  Created on: 03/11/2018
 *      Author: manue
 */

#include "base64.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <regex>
base64::base64() {
	// TODO Auto-generated constructor stub


}

base64::~base64() {
	// TODO Auto-generated destructor stub
}


static const uint8_t from_base64[128] = {
   // 8 rows of 16 = 128
   // note: only require 123 entries, as we only lookup for <= z , which z=122
               255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
               255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
               255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255,  62, 255,  63,
                52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,   0, 255, 255, 255,
               255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
                15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255,  63,
               255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
                41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255, 255, 255, 255, 255
            };

static const char to_base64[65] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";


typedef unsigned char BYTE;

static inline bool is_base64(BYTE c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64::encode(std::string filename)
{

	std::ifstream infile;
	infile.open(filename, std::ifstream::binary);

	std::vector<char> data(100);

	std::string encodedData;

	infile.seekg(0, std::ios::end);
	size_t file_size_in_byte = infile.tellg();
	data.resize(file_size_in_byte);

	infile.seekg(0, std::ios::beg);
	infile.read(&data[0], file_size_in_byte);


	encodedData = base64::base64_encode(data,data.size());

	return encodedData;

}

std::vector<BYTE> base64::decode(std::string imgbase64)
{

	    return base64::base64_decode(imgbase64);
}



std::string base64::base64_encode(std::vector<char> buf,unsigned int bufLen) {

	std::string ret;

	try {

		 // Calculate how many bytes that needs to be added to get a multiple of 3
		   size_t missing = 0;
		   size_t ret_size = bufLen;
		   while ((ret_size % 3) != 0)
		   {
		      ++ret_size;
		      ++missing;
		   }

		   // Expand the return string size to a multiple of 4
		   ret_size = 4*ret_size/3;

		   ret.clear();
		   ret.reserve(ret_size);

		   for (size_t i = 0; i < ret_size/4; ++i)
		   {
		      // Read a group of three bytes (avoid buffer overrun by replacing with 0)
		      const size_t index = i*3;
		      const uint8_t b3_0 = (index+0 < bufLen) ? buf[index+0] : 0;
		      const uint8_t b3_1 = (index+1 < bufLen) ? buf[index+1] : 0;
		      const uint8_t b3_2 = (index+2 < bufLen) ? buf[index+2] : 0;

		      // Transform into four base 64 characters
		      const uint8_t b4_0 = ((b3_0 & 0xfc) >> 2);
		      const uint8_t b4_1 = ((b3_0 & 0x03) << 4) + ((b3_1 & 0xf0) >> 4);
		      const uint8_t b4_2 = ((b3_1 & 0x0f) << 2) + ((b3_2 & 0xc0) >> 6);
		      const uint8_t b4_3 = ((b3_2 & 0x3f) << 0);

		      // Add the base 64 characters to the return value
		      ret.push_back(to_base64[b4_0]);
		      ret.push_back(to_base64[b4_1]);
		      ret.push_back(to_base64[b4_2]);
		      ret.push_back(to_base64[b4_3]);
		   }

		   // Replace data that is invalid (always as many as there are missing bytes)
		   for (size_t i = 0; i != missing; ++i)
		      ret[ret_size - i - 1] = '=';

         LogPrintf("success: %s","encodebase64");

      return ret;
    }
    catch(std::exception& e) {

  		LogPrintf("exception encode: %s",e.what());

  		return ret;
       }
}



std::vector<BYTE> base64::base64_decode(std::string encoded_string) {
    size_t encoded_size = encoded_string.size();
    std::vector<BYTE> ret;
    ret.reserve(3*encoded_size/4);
try {
    for (size_t i=0; i<encoded_size; i += 4)
    {
    	BYTE b4[4];
        b4[0] = from_base64[encoded_string[i+0]];
        b4[1] = from_base64[encoded_string[i+1]];
        b4[2] = from_base64[encoded_string[i+2]];
        b4[3] = from_base64[encoded_string[i+3]];

        BYTE b3[3];
        b3[0] = ((b4[0] & 0x3f) << 2) + ((b4[1] & 0x30) >> 4);
        b3[1] = ((b4[1] & 0x0f) << 4) + ((b4[2] & 0x3c) >> 2);
        b3[2] = ((b4[2] & 0x03) << 6) + ((b4[3] & 0x3f));

        ret.push_back(b3[0]);
        ret.push_back(b3[1]);
        ret.push_back(b3[2]);
    }



  LogPrintf("success: %s","decodebase64");
    return ret;
  }
  catch(std::exception& e) {
        //Other errors
		std::vector<BYTE> vempty;
		LogPrintf("exception decode: %s",e.what());
	  return vempty;
     }
}



bool base64::base64Validator(std::string encoded_string)
{
	std::string expr="^([A-Za-z0-9+/]{4})*([A-Za-z0-9+/]{4}|[A-Za-z0-9+/]{3}=|[A-Za-z0-9+/]{2}==)$";

	 return base64::regexValidate(expr,encoded_string);

}



bool base64::regexValidate(std::string expr, std::string teststring)
{
    std::regex ex(expr);

    if(teststring.size()>1000)
    	teststring=teststring.substr(0,1000);

    if ( std::regex_match (teststring,ex) ) {
        return true;
    }

    return false;
}
