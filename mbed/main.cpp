#include "Motor.h"
#include "HALLFX_ENCODER.h"
#include "Servo.h"

Motor Wheel_Left(p21, p6, p5); // pwm, fwd, rev 
Motor Wheel_Right(p22, p8, p7); // pwm, fwd, rev
HALLFX_ENCODER Wheel_Encoder_Left(p23);
HALLFX_ENCODER Wheel_Encoder_Right(p26);
DigitalOut Motor_Toy(p10);
Servo Servo_Camera(p24);
Servo Servo_Food(p25);
AnalogOut speaker(p18);  // Sound
Serial pi(USBTX,USBRX);

// Leds for testing 
DigitalOut myled1(LED1);
DigitalOut myled2(LED2);
DigitalOut myled3(LED3);
DigitalOut myled4(LED4);

volatile char bnum = 0;

int main() 
{
    // Setup pi
    pi.baud(9600);
    //pi.attach(&dev_recv, Serial::RxIrq);
    
    float camera_location = 0.5;
    Servo_Food = 0;
    
    while(1)
    {
        // Get command over serial 
        myled1 = !myled1;
        while(pi.readable()) 
        {
            myled1 = !myled1;
            bnum = pi.getc();
            // Control Section
            switch(bnum)
            {
                case '0': // reset mbed
                    Motor_Toy = 0;
                    Servo_Food = 0;
                    Servo_Camera = 0.5;
                    Wheel_Right.speed(0);
                    Wheel_Left.speed(0);
                    break;
                case '1': // move forward
                    Wheel_Left.speed(-1);
                    Wheel_Right.speed(-1);
                    
                    int count = 0;
                    while(bnum != '0')
                    {
                        count++;
                        wait(0.02);
                        //count is just an interval to check the location.
                        if (count == 10)
                        {
                            // Get positions of both encoders
                            int leftWheelPosition = Wheel_Encoder_Left.read();
                            int rightWheelPosition = Wheel_Encoder_Right.read();
                            //adjust left wheel to catch up with right
                            while (leftWheelPosition < rightWheelPosition)
                            {
                                Wheel_Right.speed(-.5);
                                leftWheelPosition = Wheel_Encoder_Left.read();
                                wait(0.02);
                            }
                            while (rightWheelPosition < leftWheelPosition)
                            {
                                Wheel_Left.speed(-.5); 
                                rightWheelPosition = Wheel_Encoder_Right.read(); 
                                wait(0.02);
                            }
                            Wheel_Encoder_Left.reset();
                            Wheel_Encoder_Right.reset();
                            count = 0; 
                        }
                        bnum = pi.getc(); 
                    }
                    Wheel_Left.speed(0);
                    Wheel_Right.speed(0);
                    break;
                case '2': // move backwards
                    Wheel_Left.speed(1.0);
                    Wheel_Right.speed(1.0);
                    
                    int count2 = 0;
                    while(bnum != '0')
                    {
                        count++;
                        wait(0.02);
                        //count is just an interval to check the location.
                        if (count2 == 10)
                        {
                            // Get positions of both encoders
                            int leftWheelPosition = Wheel_Encoder_Left.read();
                            int rightWheelPosition = Wheel_Encoder_Right.read();
                            //adjust left wheel to catch up with right
                            while (leftWheelPosition < rightWheelPosition)
                            {
                                Wheel_Right.speed(.5);
                                leftWheelPosition = Wheel_Encoder_Left.read();
                                wait(0.02);
                            }
                            while (rightWheelPosition < leftWheelPosition)
                            {
                                Wheel_Left.speed(.5); 
                                rightWheelPosition = Wheel_Encoder_Right.read(); 
                                wait(0.02);
                            }
                            Wheel_Encoder_Left.reset();
                            Wheel_Encoder_Right.reset();
                            count2 = 0; 
                        }
                        bnum = pi.getc(); 
                    }
                    Wheel_Left.speed(0);
                    Wheel_Right.speed(0);
                    break;
                case '3': // turn left
                    Wheel_Left.speed(0.8);
                    Wheel_Right.speed(-0.8);
                    break;
                case '4': // turn right
                    Wheel_Left.speed(-0.8);
                    Wheel_Right.speed(0.8);
                    break;
                case '5': // play audio
                    // Not used
                    break;
                case '6': // dispense food
                    for(float p=0; p<0.6; p += 0.1) 
                    {
                        Servo_Food = p;
                        wait(0.2);
                    }
                    wait(0.2);
                    Servo_Food = 0;
                    break;
                case '7': // camera up
                    if(camera_location != 1.0)
                    {
                        camera_location += 0.1;
                        Servo_Camera = camera_location;
                    }
                    break;
                case '8': // camera down
                    if(camera_location != 0.0)
                    {
                        camera_location -= 0.1;
                        Servo_Camera = camera_location;
                    }
                    break;
                case '9': // toy on
                    Motor_Toy = 1;
                    break;
                case 'A': // toy off
                    Motor_Toy = 0;
                    break;
                default:
                    //Motor_Toy = 0;
                    Servo_Food = 0;
            }
            wait(0.2);
        }
    }
}