#include "HKTLV.h"

//////////////////////////////////////
// TLV constructor()

TLV::TLV(){
  numTags = 0;

  //////////////////////////
  // Create TLV8 records  //
  //////////////////////////

  create(kTLVType_State, 1, "STATE");
  create(kTLVType_PublicKey, 384, "PUBKEY");
  create(kTLVType_Method, 1, "METHOD");
  create(kTLVType_Salt, 16, "SALT");
  create(kTLVType_Error, 1, "ERROR");
  create(kTLVType_Proof, 64, "PROOF");
  create(kTLVType_EncryptedData, 1024, "ENC.DATA");
  create(kTLVType_Signature, 64, "SIGNATURE");
  create(kTLVType_Identifier, 64, "IDENTIFIER");
  create(kTLVType_Permissions, 1, "PERMISSION");
}

//////////////////////////////////////
// TLV deconstructor()

TLV::~TLV() {
  while (numTags > 0) {
    delete[] tlv[numTags - 1].val;
    numTags--;
  }
}

//////////////////////////////////////
// TLV create(tag, maxLen, name)

int TLV::create(kTLVType tag, int maxLen, const char *name){
  if(numTags >= maxTags) {
    EHK_DEBUG("\n*** ERROR: Can't create new TLC tag type with name='");
    EHK_DEBUG(name);
    EHK_DEBUG("' - exceeded number of records reserved\n\n");
    return 0;
  }

  tlv[numTags].tag = tag;
  tlv[numTags].maxLen = maxLen;
  tlv[numTags].name = name;
  tlv[numTags].len = -1;
  tlv[numTags].val = new uint8_t[maxLen];
  numTags++;

  return 1;
}

//////////////////////////////////////
// TLV find(tag)

typename TLV::tlv_t *TLV::find(kTLVType tag){
  for(int i = 0; i < numTags; i++){
    if(tlv[i].tag == tag)
      return tlv + i;
  }

  return NULL;
}

//////////////////////////////////////
// TLV clear()

void TLV::clear() {
  cLen = 0;

  for(int i = 0; i < numTags; i++) {
    tlv[i].len = -1;
  }
}

//////////////////////////////////////
// TLV val(tag)

int TLV::val(kTLVType tag) {

  tlv_t *tlv = find(tag);

  if(tlv && tlv->len > 0) {
    return tlv->val[0];
  }

  return -1;
}

//////////////////////////////////////
// TLV val(tag, val)

int TLV::val(kTLVType tag, uint8_t val) {

  tlv_t *tlv = find(tag);

  if(tlv) {
    tlv->val[0] = val;
    tlv->len = 1;
    cLen += tlv->len + 2;
    return(val);
  }

  return(-1);
}

//////////////////////////////////////
// TLV buf(tag)

uint8_t *TLV::buf(kTLVType tag) {

  tlv_t *tlv = find(tag);

  if(tlv) {
    return tlv->val;
  }

  return NULL;
}

//////////////////////////////////////
// TLV buf(tag, len)

uint8_t *TLV::buf(kTLVType tag, int len) {

  tlv_t *tlv = find(tag);
  
  if(tlv && len <= tlv->maxLen) {
    tlv->len = len;
    cLen += tlv->len;

    for(int i = 0; i < tlv->len; i += 255) {
      cLen += 2;
    }

    return tlv->val;
  }
  
  return NULL;
}

//////////////////////////////////////
// TLV print()

void TLV::print() {
  char buf[3];

  for(int i = 0; i < numTags; i++) {

    if(tlv[i].len > 0){
      EHK_DEBUG(tlv[i].name);
      EHK_DEBUG("(");
      EHK_DEBUG(tlv[i].len);
      EHK_DEBUG(") ");

      for(int j = 0; j < tlv[i].len; j++) {
        sprintf(buf, "%02X", tlv[i].val[j]);
        EHK_DEBUG(buf);
      }

      EHK_DEBUG("\n");
    } // len>0
  } // loop over all TLVs
}

//////////////////////////////////////
// TLV len(tag)

int TLV::len(kTLVType tag){
  
  tlv_t *tlv = find(tag);

  if(tlv) {
    return(tlv->len > 0 ? tlv->len : 0);
  }

  return(-1);
}

//////////////////////////////////////
// TLV serialize(tlvBuf)

int TLV::serialize(uint8_t * tlvBuf) {
  int n = 0;
  int nBytes;

  for(int i = 0; i < numTags; i++) {    
    if((nBytes = tlv[i].len) > 0) {
      for(int j = 0; j < tlv[i].len; j += 255, nBytes -= 255) {
        if(tlvBuf != NULL) {
          *tlvBuf++ = tlv[i].tag;
          *tlvBuf++ = nBytes > 255 ? 255 : nBytes;
          memcpy(tlvBuf, tlv[i].val + j, nBytes > 255 ? 255 : nBytes);
          tlvBuf += nBytes > 255 ? 255 : nBytes;
        }
        n += (nBytes > 255 ? 255 : nBytes) + 2;      
      } // j-loop
    } // len>0
  } // loop over all TLVs
  return(n);  
}

//////////////////////////////////////
// TLV deserialize(tlvBuf, nBytes)

int TLV::deserialize(const uint8_t *tlv_buf, int length) {
  clear();

  size_t i = 0;

  while (i < length) {
    kTLVType tag = (kTLVType) tlv_buf[i];
    size_t size = 0;

    if (!find(tag)) {
      clear();
      return 0;
    }

    // check if there are any other more chunks in tlv with the same tag and create daata.
    size_t j = i;

    while(j < length && tlv_buf[j] == tag) {
      size_t chunk_size = tlv_buf[++j];
      size += chunk_size;
      j += chunk_size + 1;
    }

    if (size != 0) {
      uint8_t * val = buf(tag, size);
      if (!val) {
        clear();
        return 0;
      }

      uint8_t * p = val;

      size_t remaining = size;
      while (remaining) {
        size_t chunk_size = tlv_buf[++i];
        memcpy(p, &tlv_buf[++i], chunk_size);
        p += chunk_size;
        i += chunk_size;
        remaining -= chunk_size;
      }
    }
  }

  return 1;
}