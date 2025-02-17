#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <cstring>

enum class MessageType {
    PULSE, // PULSE will include filemap
    COMMAND,
    MESSAGE,
    ACKNOWLEDGE,
    UNKNOWN
};

static const std::unordered_map<std::string, MessageType> msgTypeMap = {
    {"PULSE", MessageType::PULSE},
    {"COMMAND", MessageType::COMMAND},
    {"MESSAGE", MessageType::MESSAGE},
};

static const std::unordered_map<MessageType, std::string> msgTypeToStr = {
    {MessageType::PULSE, "PULSE"},
    {MessageType::COMMAND, "COMMAND"},
    {MessageType::MESSAGE, "MESSAGE"},
};

using namespace std;
class Message {

    public :
        Message();

        MessageType type;
        int sourceAddress, destinationAddress;
        string content;
        int size;

        char* serialize(size_t& dataSize) const;
        static Message deserialize(const char* buffer, size_t dataSize);

        string messageTypeToString(MessageType type);

    };

#endif