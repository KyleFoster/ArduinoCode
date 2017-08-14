#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  #include "Wire.h"
#endif

MPU6050 mpu;

// uncomment "OUTPUT_READABLE_WORLDACCEL" if you want to see acceleration
// components with gravity removed and adjusted for the world frame of
// reference (yaw is relative to initial orientation, since no magnetometer
// is present in this case). Could be quite handy in some cases.
#define OUTPUT_READABLE_WORLDACCEL

#define LED_PIN 13 
#define calibrating_led 3 //Led blinks for 25 seconds while the MPU-6050 self calibrates
#define activate_led 4 //Represents the ignition of a smoke generating device

int i = 0; //Counter to capture initial values after calibration 
int initial_x = 0; 
int initial_y = 0;
int initial_z = 0;

bool useSerial = true; //Set to true to enable Serial Monitoring
bool blinkState = false;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}

void setup() 
{
    pinMode(LED_PIN, OUTPUT);
    pinMode(calibrating_led, OUTPUT);
    pinMode(activate_led, OUTPUT);
    
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

    if (useSerial)
    {
    Serial.begin(115200);
    }
    while (!Serial); // wait for Leonardo enumeration, others continue immediately
    Serial.println(F("Initializing I2C devices..."));
    mpu.initialize();

    Serial.println(F("Testing device connections..."));
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
    Serial.println(F("Initializing DMP..."));
    devStatus = mpu.dmpInitialize();

    mpu.setXGyroOffset(220);
    mpu.setYGyroOffset(76);
    mpu.setZGyroOffset(-85);
    mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

    // make sure it worked (returns 0 if so)
    if (devStatus == 0) 
    {
      mpu.setDMPEnabled(true);
  
      attachInterrupt(0, dmpDataReady, RISING);
      mpuIntStatus = mpu.getIntStatus();
      dmpReady = true;
  
      packetSize = mpu.dmpGetFIFOPacketSize();
      
      // Stall for 25 seconds while DMP self-calibrates 
      for (int x = 0; x < 50; x++) 
      {
      digitalWrite(calibrating_led, HIGH);
      delay(100);
      digitalWrite(calibrating_led, LOW);
      delay(400);
      }
    } 
}

void loop() 
{
    if (!dmpReady) return;

    // wait for MPU interrupt or extra packet(s) available
    while (!mpuInterrupt && fifoCount < packetSize) {}

    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();
    fifoCount = mpu.getFIFOCount();

    // check for overflow
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        mpu.resetFIFO();

    } else if (mpuIntStatus & 0x02) {
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

        mpu.getFIFOBytes(fifoBuffer, packetSize);
        fifoCount -= packetSize;

        #ifdef OUTPUT_READABLE_WORLDACCEL
          // display initial world-frame acceleration, adjusted to remove gravity
          // and rotated based on known orientation from quaternion
          mpu.dmpGetQuaternion(&q, fifoBuffer);
          mpu.dmpGetAccel(&aa, fifoBuffer);
          mpu.dmpGetGravity(&gravity, &q);
          mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
          mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
          Serial.print("aworld\t");
          Serial.print(aaWorld.x);
          Serial.print("\t");
          Serial.print(aaWorld.y);
          Serial.print("\t");
          Serial.println(aaWorld.z);
        #endif

        if (i == 0)
        {
          initial_x = aaWorld.x;
          initial_y = aaWorld.y;
          initial_z = aaWorld.z;
          Serial.print("Initial values\t");
          Serial.print(initial_x);
          Serial.print("\t");
          Serial.print(initial_y);
          Serial.print("\t");
          Serial.println(initial_z);
        }

        if (abs(aaWorld.x) >= (500 * abs(initial_x)) && abs(aaWorld.y) >= (500 * abs(initial_y)) && abs(aaWorld.z) >= (3.25 * abs(initial_z))) 
        {
          for (int i = 0; i < 10; i++)
          {
          digitalWrite(activate_led, HIGH);
          delay(100);
          digitalWrite(activate_led, LOW);
          delay(150);
          }
        }
        blinkState = !blinkState;
        digitalWrite(LED_PIN, blinkState);
        i++;
    }
}
