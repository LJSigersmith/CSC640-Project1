# Client-Server Protocol

## **Protocol Overview**
The communication between the client and server is based on **Message** objects, which contain different types of data transmissions.

### **Message Class**
Each message contains the following fields:
- **`MessageType type`** – Defines the type of message.
- **`std::string content`** – The main content of the message.
- **`int sourceAddress`** – The sender's IP address.
- **`int destinationAddress`** – The recipient's IP address.

### **`MessageType` Enum**
| **Type**        | **Description** |
|----------------|---------------|
| `PULSE`        | Heartbeat message containing the client's file map. |
| `ACKNOWLEDGE`  | Server response to a `PULSE` message. Will eventually contain the file maps of all active clients. |
| `COMMAND`      | Not yet implemented. Intended for sending commands to the server. |
| `MESSAGE`      | A simple text message. |
| `UNKNOWN`      | Default case if the message type is unrecognized. |

---

## **Compilation Instructions**
### ** Standard Compilation**
```sh
g++ *.h *.cpp -std=c++17 -ljsoncpp -lcurl
```

### ** Fix for Ubuntu (If Linker Errors Occur)**
If you encounter linker errors related to JSONCPP on Ubuntu, try:
```sh
g++ *.h *.cpp -std=c++17 -I/usr/include/jsoncpp -ljsoncpp -lcurl
```

---

##  **Running the Program**
```sh
./a.out [server/client] [port]
```

### ** Arguments**
- **`[server/client]`** → `0` for **server**, `1` for **client**.
- **`[port]`** → The port number to use for communication.

### ** Example Usage**
#### **Start the Server**
```sh
./a.out 0 5001
```

#### **Start a Client**
```sh
./a.out 1 5001
```

---

## **Future Features**
- Implement **`COMMAND`** messages for executing server-side commands.
- Enhance the **`ACKNOWLEDGE`** message to include **file maps of all active clients**.
- Add full file content to server repsonse.
- Implement **peer-to-peer communication** between clients.

---

## **Dependencies**
Ensure you have the following installed:
- **GCC (g++)** `sudo apt install g++` (Linux) or `brew install gcc` (Mac)
- **JSONCPP** `sudo apt install libjsoncpp-dev`
- **cURL Library** `sudo apt install libcurl4-openssl-dev`

---

