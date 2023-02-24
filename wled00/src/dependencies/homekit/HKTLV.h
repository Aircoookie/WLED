/*********************************************************************************
 *  MIT License
 *  
 *  Copyright (c) 2020-2022 Gregg E. Berman
 *  
 *  https://github.com/HomeSpan/HomeSpan
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *  
 ********************************************************************************/
 
#pragma once

#include "HKDebug.h"
#include <Arduino.h>
#include <stdint.h>

typedef enum {
  kTLVType_Method = 0x00,
  kTLVType_Identifier = 0x01,
  kTLVType_Salt = 0x02,
  kTLVType_PublicKey = 0x03,
  kTLVType_Proof = 0x04,
  kTLVType_EncryptedData = 0x05,
  kTLVType_State = 0x06,
  kTLVType_Error = 0x07,
  kTLVType_RetryDelay = 0x08,
  kTLVType_Certificate = 0x09,
  kTLVType_Signature = 0x0A,
  kTLVType_Permissions = 0x0B,
  kTLVType_FragmentData = 0x0C,
  kTLVType_FragmentLast = 0x0D,
  kTLVType_Flags = 0x13,
  kTLVType_Separator = 0xFF
} kTLVType;

class TLV {
  int cLen;              // total number of bytes in all defined TLV records, including TAG andf LEN (suitable for use as Content-Length in HTTP Body)
  int numTags;           // actual number of tags defined
  static const int maxTags = 10;

  struct tlv_t {
    kTLVType tag;         // TAG
    int len;             // LENGTH
    uint8_t * val;        // VALUE buffer
    int maxLen;          // maximum length of VALUE buffer
    const char * name;          // abbreviated name of this TAG
  };

  tlv_t tlv[maxTags];           // pointer to array of TLV record structures
  tlv_t *find(kTLVType tag);     // returns pointer to TLV record with matching TAG (or NULL if no match)

public:
  TLV();
  ~TLV();

  int create(kTLVType tag, int maxLen, const char *name);   // creates a new TLV record of type 'tag' with 'maxLen' bytes and display 'name'
  void clear();                             // clear all TLV structures
  int val(kTLVType tag);                     // returns VAL for TLV with matching TAG (or -1 if no match)
  int val(kTLVType tag, uint8_t val);        // sets and returns VAL for TLV with matching TAG (or -1 if no match)    
  uint8_t *buf(kTLVType tag);                // returns VAL Buffer for TLV with matching TAG (or NULL if no match)
  uint8_t *buf(kTLVType tag, int len);       // set length and returns VAL Buffer for TLV with matching TAG (or NULL if no match or if LEN>MAX)
  int len(kTLVType tag);                     // returns LEN for TLV matching TAG (or 0 if TAG is found but LEN not yet set; -1 if no match at all)
  void print();                             // prints all defined TLVs (those with length>0). For diagnostics/debugging only
  int deserialize(const uint8_t *tlv_buf, int length);  // unpacks nBytes of TLV content from single byte buffer into individual TLV records (return 1 on success, 0 if fail) 
  int serialize(uint8_t *tlvBuf);                // if tlvBuf!=NULL, packs all defined TLV records (LEN>0) into a single byte buffer, spitting large TLVs into separate 255-byte chunks.  Returns number of bytes (that would be) stored in buffer
}; // TLV8