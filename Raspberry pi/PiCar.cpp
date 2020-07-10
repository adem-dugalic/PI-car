#include <iostream>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <wiringPi.h>
#include <sys/time.h>
#include <time.h>
#include <thread>
#include <future>
#include <chrono>
#include <string.h>
#include <cstdlib>


//motor controller
const int en1 = 7;
const int en2 = 3;

const int pin1 = 0;
const int pin2 = 2;
const int pin3 = 4;
const int pin4 = 5;

//ultrasonic sensor
const int outPin = 25;
const int echoPin = 27;

//light sensor
const int inL = 1;

int f = 0;



/**
 * Cleanup
 * Stops all motors. Sets all pins to LOW
 * 
*/
void cleanup()
{
    digitalWrite(en1, 0);
    digitalWrite(en2, 0);
    digitalWrite(pin1, 0);
    digitalWrite(pin2, 0);
    digitalWrite(pin3, 0);
    digitalWrite(pin4, 0);
}

/** 
 * Ultrasonic sensor 
 * Function that returns the distance
 * Works in a separate thread
*/
double distance()
{
    double sig,dis;
    struct timeval now,than;
    
    digitalWrite(outPin, LOW);
    digitalWrite(outPin, HIGH);
    sleep(0.0001);
    digitalWrite(outPin, LOW);
    
    while (digitalRead(echoPin)==0)
    {
        gettimeofday(&now, NULL);
        
    }
    while (digitalRead(echoPin)==1)
    {
        gettimeofday(&than, NULL); 
    }
    
    sig = than.tv_usec - now.tv_usec;
    dis = 0.017*sig;
    
    printf("Distance %0.2f\n",dis);
    return dis;
    
}

void roam(int trigger)
{
	std::cout<<"1" << std::endl;
	int random, counter = 0;
	double stop;
	std::cout<<"2" << std::endl;
	while(f == 1 && counter != 7)
	{
		std::cout<<"3" << std::endl;
		std::future<double> dis = std::async(std::launch::async, distance);
		std::cout<<"4" << std::endl;
		stop = dis.get();
		std::cout<<"5" << std::endl;
		random = rand() % 4;
		std::cout<<"6" << std::endl;
		//w
		if(random == 0)
		{
			if(stop<20)
			{
				cleanup();
				continue;
			}
			digitalWrite(en1,1);
			digitalWrite(en2,1);
			digitalWrite(pin1, 0);
			digitalWrite(pin2, 1);
			digitalWrite(pin3, 0);
			digitalWrite(pin4, 1);    
        }
        //a
        if(random == 1)
		{
			digitalWrite(en1,1);
			digitalWrite(en2,1);
            digitalWrite(pin1, 1);
            digitalWrite(pin2, 0);
            digitalWrite(pin3, 0);
            digitalWrite(pin4, 1);
                     
        }
        //s
        if(random == 2)
		{

			digitalWrite(en1,1);
			digitalWrite(en2,1);
            digitalWrite(pin1, 1);
            digitalWrite(pin2, 0);
            digitalWrite(pin3, 1);
            digitalWrite(pin4, 0);
                     
        }
        //d
        if(random == 3)
		{
			digitalWrite(en1,1);
			digitalWrite(en2,1);
			digitalWrite(pin1, 0);
			digitalWrite(pin2, 1);
			digitalWrite(pin3, 1);
			digitalWrite(pin4, 0);
                     
        }
        std::cout<<"7" << std::endl;
        write(trigger, (char*)&stop, sizeof(double));
        counter++;
        sleep(3);
	}
}



int main()
{

	//WiringPI setup
    wiringPiSetup();
    
	//Pin setup
    pinMode(pin1,OUTPUT);
    pinMode(pin2,OUTPUT);
    pinMode(pin3,OUTPUT);
    pinMode(pin4,OUTPUT);
    pinMode(en1,OUTPUT);
    pinMode(en2,OUTPUT);
    pinMode(outPin, OUTPUT); 
    pinMode(echoPin, INPUT); 
    pinMode(inL, INPUT);
    
    //Calling cleanup() in case the gpio pins were enabled
    cleanup();
    
    //Setting up the variables that will be used
    char lastrec='x';
    struct sockaddr_in serv; 
    socklen_t size = sizeof(serv);
    int fd;
    int conn;
    char message[1] = "";
    double result;
    
    //Creating a server
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8096);
    serv.sin_addr.s_addr = INADDR_ANY;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    bind(fd, (struct sockaddr *)&serv, sizeof(serv));
    listen(fd,2);
    
    //Accepting requests
    
    while(conn = accept(fd, (struct sockaddr *)&serv, &size))
    {
		while(recv(conn, message, 1, 0)>0)
		{ 	f = 0;
			std::cout<<"after rec" << std::endl;
			std::future<double> dis = std::async(std::launch::async, distance);
			result = dis.get();
			write(conn, (char*)&result, sizeof(double));
			printf("Message Received: %s\n", message);
            if(lastrec==message[0] && result> 20){
               memset(message, 0, sizeof(message));
               continue;
               }
             
                if(message[0]=='w')
                {
                    if(result<20)
                    {
                        memset(message, 0, sizeof(message));
                        cleanup();
                        continue;
                    }
                    digitalWrite(en1,1);
					digitalWrite(en2,1);
                    digitalWrite(pin1, 0);
                    digitalWrite(pin2, 1);
                    digitalWrite(pin3, 0);
                    digitalWrite(pin4, 1);
                     
                }
                std::cout<<"after w" << std::endl;
                if(message[0]=='s')
                {
					digitalWrite(en1,1);
					digitalWrite(en2,1);
                    digitalWrite(pin1, 1);
                    digitalWrite(pin2, 0);
                    digitalWrite(pin3, 1);
                    digitalWrite(pin4, 0);
                }
                std::cout<<"after s" << std::endl;
                
                if(message[0]=='a')
                {
					digitalWrite(en1,1);
					digitalWrite(en2,1);
                    digitalWrite(pin1, 1);
                    digitalWrite(pin2, 0);
                    digitalWrite(pin3, 0);
                    digitalWrite(pin4, 1);
                }
                
                if(message[0]=='d')
                {
					digitalWrite(en1,1);
					digitalWrite(en2,1);
                    digitalWrite(pin1, 0);
                    digitalWrite(pin2, 1);
                    digitalWrite(pin3, 1);
                    digitalWrite(pin4, 0);
                }
                
                if(message[0]=='r')
                {
					f = 1;
                    roam(conn);
                }
                
                if(message[0]=='x')
                {
                    cleanup();
                }
                lastrec = message[0];
                std::cout<<"after write" << std::endl;
                memset(message, 0, sizeof(message));
                std::cout<<"after memset" << std::endl;
		}
	}
	close(fd);
	close(conn);
	
}
