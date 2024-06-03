#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

using namespace std;


class Device {
private:
    string name;
    string data;
    string macAddress;

public:
    Device(string n, string d, string mac) : name(n), data(d), macAddress(mac) {}

    // Generate and set the parity bit based on the data
    string D_P() {
        int countOnes = 0;
        for (char c : data) {
            if (c == '1') {
                countOnes++;
            }
        }
        char parityBit = (countOnes % 2 == 0) ? '0' : '1'; // Even parity
        return data + parityBit;
    }

    void sendMsg(Device* receiver) {
        string dataWithParity = D_P();
        cout << name << " sending data to " << receiver->name << ": " << dataWithParity << endl;
        receiver->receiveData(dataWithParity);
    }

    void receiveData(const string& receivedDataWithParity) {
        char receivedParityBit = receivedDataWithParity.back();
        string receivedData = receivedDataWithParity.substr(0, receivedDataWithParity.size() - 1);

        int countOnes = 0;
        for (char c : receivedData) {
            if (c == '1') {
                countOnes++;
            }
        }
        char computedParityBit = (countOnes % 2 == 0) ? '0' : '1';

        if (computedParityBit == receivedParityBit) {
            cout << name << " received data : " << receivedData << endl;
        } else {
            cout << name << " received data with incorrect parity: " << receivedData << endl;
        }
    }
    string getName() const {
        return name;
    }
};

// Stop-and-Wait protocol

class StopAndWait {
public:
    static bool waitForAck() {
        // Simulating transmission time
        this_thread::sleep_for(chrono::milliseconds(1000));
        bool ackReceived = (rand() % 100) < 70; // 70% chance of successful transmission

        if (ackReceived) {
            return true;
        }
        return false;
    }
};
// Hub class representing the hub which connects devices
class Hub {
public:
    vector<Device*> devices;

    void connect(Device* dev) {
        devices.push_back(dev);
    }

    void sendMsgToDevice(Device* sender, const string& dataWithParity) {
        for (Device* dev : devices) {
            if (dev != sender) {
                dev->receiveData(dataWithParity);
            }
        }
    }
};

// Function to check if the medium is busy

bool isMediumBusy() {
    bool randomNum = (rand() % 100)<70; 
    return randomNum; 
}

// CSMA access control
void csmaAccessControl(Device* sender) {
    int x=1;
    while (isMediumBusy()) {
        cout << "Medium is busy. " << sender->getName() << " waiting "<<x<<" seconds before retrying...\n\n" << endl;
        
        this_thread::sleep_for(chrono::milliseconds(x*1000)); // back off applied
        x++;
    }
    cout<<"\n\nMedium is  idle and "<<sender->getName()<<" can now send data\n\n";
}

// Switch class representing the switch which connects devices
class Switch {
private:
    map<string, Device*> device_Map;

public:
    void connect(Device* dev, string macAddress) {
        device_Map[macAddress] = dev;
    }

    void sendMsg_ToDevice(Device* sender, string macAddress, const string& dataWithParity) {
        Device* dev = device_Map[macAddress];
        csmaAccessControl(sender);
        if (dev != nullptr && dev != sender) {
            cout << "\n\t\tSending data: "<<sender->D_P()<<"\n\n";
            for (int i=0; i<dataWithParity.length(); i++) {
                if (StopAndWait::waitForAck()) {
                    cout << "\n( "<<dataWithParity[i]<<" ) sent and received by "<<dev->getName()<<endl;
                } else {
                    cout<< "\n( "<<dataWithParity[i]<<" ) not sent as no ACK received "<<endl;
                    cout<<"\n\t\tSending ( "<<dataWithParity[i]<<" ) again\n"<<endl;
                    i--;
                }
            }
            cout << endl;
        dev->receiveData(dataWithParity);    
        } else {
            cout << "Error: Device with MAC address " << macAddress << " not found." << endl;
        }
    }
};


// Bridge class representing the bridge which connects hubs
class Bridge {
private:
    map<string, Hub*> hub_Map;

public:
    void connect(Hub* hub, string macAddress) {
        hub_Map[macAddress] = hub;
    }

    void sendDataToDevice(Hub* sender, string receiverMacAddress, const string& dataWithParity) {
            Hub* hub = hub_Map[receiverMacAddress];
           
                for (Device* dev : hub->devices) {
                    dev->receiveData(dataWithParity);
                }
        }
    
};


int main() {
    // Create devices with some data
    Device device1("Device_1", "101010", "M:I:N:A:M:01");
    Device device2("Device_2", "010101", "A:L:I:Z:A:02");
    Device device3("Device_3", "110011", "S:O:B:I:YA:03");
    Device device4("Device_4", "001100", "M:I:N:A:M:04");
    Device device5("Device_5", "001110", "A:L:I:Z:A:05");
    Device device6("Device_6", "001101", "A:L:I:Z:A:06");
    Device device7("Device_7", "1101100", "A:L:I:Z:A:07");
    Device device8("Device_8", "101100", "A:L:I:Z:A:08");
    Device device9("Device_9", "001111", "A:L:I:Z:A:09");
    Device device10("Device_10", "1001100","A:L:I:Z:A:10");

    // Create hubs
    Hub hub1;
    Hub hub2;

    // Connect devices to the hubs

    hub1.connect(&device1);
    hub1.connect(&device2);
    hub1.connect(&device3);
    hub1.connect(&device4);
    hub1.connect(&device5);
    

    hub2.connect(&device6);
    hub2.connect(&device7);
    hub2.connect(&device8);
    hub2.connect(&device9);
    hub2.connect(&device10);

    // Create a switch
    Switch switchObj;

    // Connect devices to the switch with MAC addresses
    switchObj.connect(&device1, "M:I:N:A:M:01");
    switchObj.connect(&device2, "A:L:I:Z:A:02");
    switchObj.connect(&device3, "S:O:B:I:YA:03");
    switchObj.connect(&device4, "M:I:N:A:M:04");

    // Create a bridge
    Bridge bridge;

    // Connect hubs to the bridge with MAC addresses
    bridge.connect(&hub1, "M:I:N:A:M:05");
    bridge.connect(&hub2, "S:O:B:I:YA:06");

    // Generate parity for devices' data
    device1.D_P();
    device2.D_P();
    device3.D_P();
    device4.D_P();
    while(true)
    {
    int choice;
    cout << "\n\n\n\t\tSelect the mode of transmission:\n\n"
         << "1. Device to Device directly\n"
         << "2. Device to Device through Hub\n"
         << "3. Device to Device through Switch\n"
         << "4. Device to Device through Bridge\n\n"
         << "\t\tEnter your choice: \t";
    cin >> choice;
    cout<<endl;
    switch (choice) {
        case 1:
        cout<<"\nSENDING DATA BETWEEN DEVICES\n\n";
            device1.sendMsg(&device2);
            break;
        case 2:
        cout<<"\nSENDING DATA THROUGH HUB\n\n";
            hub1.sendMsgToDevice (&device1, device1.D_P());
            break;
        case 3:
            cout<<"\nSENDING DATA THROUGH SWITCH\n\n";
            switchObj.sendMsg_ToDevice(&device1, "A:L:I:Z:A:02", device1.D_P());
            break;
        case 4:
            cout<<"\nSENDING DATA TO DEVICES FROM HUB TO HUB THROUGH BRIDGE\n\n";
            bridge.sendDataToDevice(&hub1, "S:O:B:I:YA:06", device1.D_P());
            break;
        default:
            cout << "Invalid choice. Exiting..." << endl;
            return 1;
    }
    }

    return 0;
}