#ifndef Mesh_h
#define Mesh_h

#include "Arduino.h"

class Mesh
{
  private:
    struct addressBook{
      String userName;
      uint8_t address;
    } myAddresses[6] = {{"Abby", 0xA1},{"Carlos", 0xB1},{"Kyle", 0xC1},{"Alex", 0xD1},{"Harman", 0xE1},{"Malik", 0xF1}}; 
    
    struct RadioHeader {
      uint8_t to_address;
      uint8_t from_address;
      uint8_t final_address;
      uint8_t ttl;
      char message_type;
    } receiveHeader = {0, 0, 0, 5, '\0'};
    
    struct ConnectionTable
    { 
      uint8_t value;
      uint8_t updates_til_reset;
    } c_t[6] = {{0,10}, {0,10}, {0,10}, {0,10}, {0,10}, {0,10}}; 
    
    uint8_t channel;
    uint8_t received_message[32];
  public:
    Mesh();
    void receiveMessage();
    void messageDecide(RadioHeader &message_header, int to_node_index, int from_node_index, int final_node_index);
    void relayMessage(int final_node_index, int from_node_index);
    void updateTable(int table_index);
    void sendMessage(RadioHeader &sendHeader);
    RadioHeader readUserInput();
    void broadcastMessage();
    int checkConnection(int final_node_index, int from_node_index);
    void scanChannels(); 
};

#endif

