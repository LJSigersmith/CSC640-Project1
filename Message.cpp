#include "Message.h"

using namespace std;

Message::Message() {
    type = MessageType::PULSE;
    sourceAddress = inet_addr("0.0.0.0");
    destinationAddress = inet_addr("0.0.0.0");
    content = "";
}

// ChatGPT wrote serialize() and deserialize()
// **Serialize Message into a dynamically allocated char buffer**
char* Message::serialize(size_t& dataSize) const {
    size_t contentLength = content.size();
    dataSize = sizeof(type) + sizeof(sourceAddress) + sizeof(destinationAddress) + sizeof(int) + contentLength;
    
    char* buffer = new char[dataSize]; // Allocate memory
    char* ptr = buffer;

    // Copy MessageType
    memcpy(ptr, &type, sizeof(type));
    ptr += sizeof(type);

    // Copy Source Address
    memcpy(ptr, &sourceAddress, sizeof(sourceAddress));
    ptr += sizeof(sourceAddress);

    // Copy Destination Address
    memcpy(ptr, &destinationAddress, sizeof(destinationAddress));
    ptr += sizeof(destinationAddress);

    // Copy Content Size
    int contentSize = static_cast<int>(contentLength);
    memcpy(ptr, &contentSize, sizeof(contentSize));
    ptr += sizeof(contentSize);

    // Copy Content Data
    memcpy(ptr, content.data(), contentLength);

    return buffer;
}

// **Deserialize Message from a char buffer**
Message Message::deserialize(const char* buffer, size_t dataSize) {
    Message msg;
    const char* ptr = buffer;

    // Read MessageType
    memcpy(&msg.type, ptr, sizeof(msg.type));
    ptr += sizeof(msg.type);

    // Read Source Address
    memcpy(&msg.sourceAddress, ptr, sizeof(msg.sourceAddress));
    ptr += sizeof(msg.sourceAddress);

    // Read Destination Address
    memcpy(&msg.destinationAddress, ptr, sizeof(msg.destinationAddress));
    ptr += sizeof(msg.destinationAddress);

    // Read Content Size
    int contentSize;
    memcpy(&contentSize, ptr, sizeof(contentSize));
    ptr += sizeof(contentSize);

    // Read Content Data
    msg.content.assign(ptr, contentSize);

    return msg;
}

std::string Message::messageTypeToString(MessageType type) {
    auto it = msgTypeToStr.find(type);
    return (it != msgTypeToStr.end()) ? it->second : "UNKNOWN";
}