#include <M5Stack.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

extern float left;
extern float right;

int getTwosComplement(int num, int bits) {
  // Calculate the two's complement
  return (num & (1 << (bits - 1))) ? (num - (1 << bits)) : num;
}

// UUID HID
static BLEUUID serviceUUID("1812");
// UUID Report Charcteristic
static BLEUUID charUUID("2a4d");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;

static BLEAdvertisedDevice *myDevice;

std::vector<BLEAdvertisedDevice *> myDevices;

static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {

  const int INDEX_LEVER_UPDOWN = 2;
  const int INDEX_LEVER_LEFTRIGHT = 1;
  const float unit = 12.0;

  int x = getTwosComplement(pData[INDEX_LEVER_LEFTRIGHT], 8);
  int y = getTwosComplement(pData[INDEX_LEVER_UPDOWN], 8);

  if (y < 0) {
    left = unit * y;
    right = unit * y;

    Serial.printf("UP %d\n", y);
  } else if (y > 0) {
    Serial.printf("DOWN %d\n", y);
    left = unit * y;
    right = unit * y;
  }

  if (x > 0) {
    Serial.printf("RIGHT %d\n", x);
    left = 0;
    right = unit * x;
  } else if (x < 0) {
    Serial.printf("LEFT %d\n", x);
    left = unit * x;
    right = 0;
  }

  if (pData[1] == 0 && pData[2] == 0) {
    left = 0;
    right = 0;
    //  Serial.println("IDLE ");
  }
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient *pclient) {
  }

  void onDisconnect(BLEClient *pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {

  Serial.printf("myDevices=%d\n", myDevices.size());
  for (int i = 0; i < myDevices.size(); i++) {
    Serial.print("Forming a connection to ");
    Serial.println(myDevices.at(i)->getAddress().toString().c_str());

    BLEClient *pClient = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevices.at(i));  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");


    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");

    std::map<uint16_t, BLERemoteCharacteristic *> *pCharacteristicMap;
    pCharacteristicMap = pRemoteService->getCharacteristicsByHandle();
    if (pCharacteristicMap == nullptr) {
      Serial.println("pCharacteristicMap=null");
      return false;
    }

    for (auto itr = pCharacteristicMap->begin(); itr != pCharacteristicMap->end(); ++itr) {
      Serial.print("UUID: ");
      Serial.println(itr->second->getUUID().toString().c_str());
      if (itr->second->getUUID().equals(charUUID)) {
        Serial.print("CharUUID matched: ");
        Serial.println(itr->second->getUUID().toString().c_str());
        if (itr->second->canNotify()) {
          Serial.print("Notify registerd: ");
          Serial.println(itr->second->getUUID().toString().c_str());
          itr->second->registerForNotify(notifyCallback);
        }
      }
    }
  }
  connected = true;
  return true;
}
/**
   Scan for BLE servers and find the first one that advertises the service we are looking for.
*/
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  /**
       Called for each advertising BLE server.
    */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      myDevices.push_back(myDevice);
      doConnect = true;
      doScan = true;


      Serial.print("BLE Device found !!!!");

    }  // Found our server
  }    // onResult
};

void ble_task(void *param) {
  while (true) {
    // If the flag "doConnect" is true then we have scanned for and found the desired
    // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
    // connected we set the connected flag to be true.
    if (doConnect == true) {
      Serial.println("We are now  start connected to the BLE Server.");
      if (connectToServer()) {
        Serial.println("We are now connected to the BLE Server.");
      } else {
        Serial.println("We have failed to connect to the server; there is nothin more we will do.");
      }
      doConnect = false;
    }

    // If we are connected to a peer BLE Server, update the characteristic each time we are reached
    // with the current time since boot.
    if (connected) {
    } else if (doScan) {
      BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
    }

    delay(10);  // Delay a second between loops.
  }
}



void ble_control_setup() {

  BLEDevice::init("");
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30, false);
  
  M5.Lcd.fillRect(220, 0, 100, 60, TFT_BLACK); // 设置特定区域为黑色

  M5.Lcd.setCursor(220, 0*18); M5.Lcd.print("Not Connected");
  M5.Lcd.setCursor(230, 1*18); M5.Lcd.print("X :");
  M5.Lcd.setCursor(230, 2*18); M5.Lcd.print("Y :");

  xTaskCreatePinnedToCore(ble_task, "bleTask", 8192, NULL, 10, NULL, 1);
}

void show_control_status() {
    if (connected) {
        M5.Lcd.setCursor(220, 0*18); M5.Lcd.print("Connected    ");
        M5.Lcd.setCursor(272, 1*18); M5.Lcd.printf("%3d ", (int)left);
        M5.Lcd.setCursor(272, 2*18); M5.Lcd.printf("%3d ", (int)right);
    } else {
      M5.Lcd.setCursor(220, 0*18); M5.Lcd.print("Not Connected");
    }
}
