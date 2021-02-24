#include "NodeStruct.h"

String getNodeTypeDisplayString(uint8_t nodeType) {
  switch (nodeType)
  {
    case NODE_TYPE_ID_ESP8266:     return F("ESP8266");
    case NODE_TYPE_ID_ESP32:       return F("ESP32");
  }
  return "Undefined";
}

NodeStruct::NodeStruct() :
  age(0), nodeType(0)
{
  for (uint8_t i = 0; i < 4; ++i) { ip[i] = 0; }
}
