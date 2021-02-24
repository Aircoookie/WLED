#ifndef DATASTRUCTS_NODESTRUCT_H
#define DATASTRUCTS_NODESTRUCT_H

/*********************************************************************************************\
* NodeStruct from the ESP Easy project (https://github.com/letscontrolit/ESPEasy)
\*********************************************************************************************/

#include <map>
#include <IPAddress.h>


#define NODE_TYPE_ID_UNDEFINED          0
#define NODE_TYPE_ID_ESP8266            1
#define NODE_TYPE_ID_ESP32              2

String getNodeTypeDisplayString(uint8_t nodeType);

/*********************************************************************************************\
* NodeStruct
\*********************************************************************************************/
struct NodeStruct
{
  NodeStruct();

  String    nodeName;
  IPAddress ip;
  uint8_t   unit;
  uint8_t   age;
  uint8_t   nodeType;
};
typedef std::map<uint8_t, NodeStruct> NodesMap;

#endif // DATASTRUCTS_NODESTRUCT_H
