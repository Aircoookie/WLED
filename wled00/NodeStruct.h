#ifndef WLED_NODESTRUCT_H
#define WLED_NODESTRUCT_H

/*********************************************************************************************\
* NodeStruct from the ESP Easy project (https://github.com/letscontrolit/ESPEasy)
\*********************************************************************************************/

#include <map>
#include <IPAddress.h>

#define NODE_TYPE_ID_UNDEFINED        0
#define NODE_TYPE_ID_ESP8266         82
#define NODE_TYPE_ID_ESP32           32

/*********************************************************************************************\
* NodeStruct
\*********************************************************************************************/
struct NodeStruct
{
  String    nodeName;
  IPAddress ip;
  uint8_t   unit;
  uint8_t   age;
  uint8_t   nodeType;
  uint32_t  build;

  NodeStruct() : age(0), nodeType(0), build(0)
  {
    for (uint8_t i = 0; i < 4; ++i) { ip[i] = 0; }
  }
};
typedef std::map<uint8_t, NodeStruct> NodesMap;

#endif // WLED_NODESTRUCT_H
