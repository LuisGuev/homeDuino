/*
 * Name: Luis Guevara
 * Date:4/16/2018
 * Function:Webserver program that takes in AJAX requests and processes them, enabling
 * home automation for lights and a fan.
 * 
 * 
 * 
 * 
 * 
 */




#include <Ethernet.h>//library included for ethernet shield capabilities
#include <SD.h>//library included for SD capabilities
#define buff 50 //buffer size enough to get http requests



IPAddress ip(192, 168, 1, 12); // IP of board
EthernetServer server(80);  // server needs to be at port 80
File page;               // this will be our page on the SD
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };//mac address
char HTTPREQUEST[buff] = {0}; // buffered HTTP request stored as null terminated string
char index = 0;              // index into HTTPREQUEST buffer
boolean LED_state[4] = {0}; // stores the states of the LEDs

void setup()
{
    //Ethernet port on arduino is 10, so initialize it
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
    
    if (!SD.begin(4))//if the SD card cant be read
    {
        return;    //then just return and don't continue
    }
    
    if (!SD.exists("index.htm"))//if the index.html file does not exist in the SD card
    {
        return;  // then just return and don't continue
    }

    //if the SD card can be read as well as the index.htm file, then we can proceed

    //outputs declared for the LED's being controlled
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(8, OUTPUT);
    pinMode(9, OUTPUT);

    //begin the ethernet and the server capabilities
    Ethernet.begin(mac, ip);  
    server.begin(); 
}

void loop()
{
    EthernetClient client = server.available(); //fetches the client connecting

    if (client) {//if there is a client connecting
      
        boolean currentLineIsBlank;
        currentLineIsBlank = true; //boolean for stating that it is a fresh request
      
        while (client.connected()) {//while the client is connected, we process the requests
          
            if (client.available()) {//if the client is available
              
                char c = client.read(); // read a character from the client
                
                //stores the request in the array
                if (index < (buff - 1))//if the current index is less than the buffer -1 (needed for the null termination character)
                {
                    HTTPREQUEST[index] = c;// put the character into the request array
                    index++;//cycle through the next index
                }

                if (c == '\n' && currentLineIsBlank)//if the full request was recieved (terminated by \n) and it is a new request to process
                {
                    client.println("HTTP/1.1 200 OK");//request is good, recieved
                    if (compString(HTTPREQUEST, "ajaxin"))//if there's an AJAX request, use XML file
                    {
                        client.println("Content-Type: text/xml");
                        client.println("Connection: keep-alive");
                        client.println();
                        LEDupdate(); //call a function to update the LED's
                        XMLUpdate(client);//send updated XML file, changing values of lights 
                    }
                    
                    else //else, its a web page request
                    {  

                        //send the rest of the header
                        client.println("Content-Type: text/html");
                        client.println("Connection: keep-alive");
                        client.println();
                       
                        page = SD.open("index.htm");        // open web page file
                       
                        if (page)//if the page exists
                        {
                            while(page.available())//if the page is available
                            {  
                                client.write(page.read());//send the page to the client
                            }
                            page.close();//close the page
                        }
                    }
          
                    //finally, reset buffer and the index, for a new request
                    index = 0;
                    clearString(HTTPREQUEST, buff);
                    break;
                }
 
                if (c == '\n')//check for the new line character in the request array
                {
                    currentLineIsBlank = true;//if so, say that the array is empty
                } 
                
                else if (c != '\r') //if not, a character was recieved from the client
                {
                    currentLineIsBlank = false;//so we say the array is not empty
                }
            } 
        }
   
        delay(5);//give time for web browser to recieve data
        client.stop();//finally, close the client
    } 
   
}


//function to update state of LED's and to control them
void LEDupdate(void)
{
    // LED 1, front porch
    if (compString(HTTPREQUEST, "LED1=1"))
    {
        LED_state[0] = 1;  // save LED state
        digitalWrite(6, HIGH);//turn on
    }
    else if (compString(HTTPREQUEST, "LED1=0"))
    {
        LED_state[0] = 0;  // save LED state
        digitalWrite(6, LOW);//turn off
    }


    
    // LED 2, inside 
    if (compString(HTTPREQUEST, "LED2=1")) 
    {
        LED_state[1] = 1;  // save LED state
        digitalWrite(7, HIGH);//turn on
    }
    else if (compString(HTTPREQUEST, "LED2=0")) 
    {
        LED_state[1] = 0;  // save LED state
        digitalWrite(7, LOW);//turn off
    }

    
    // LED 3, flood light
    if (compString(HTTPREQUEST, "LED3=1")) 
    {
        LED_state[2] = 1;  // save LED state
        digitalWrite(8, HIGH);//turn on
    }
    else if (compString(HTTPREQUEST, "LED3=0")) 
    {
        LED_state[2] = 0;  // save LED state
        digitalWrite(8, LOW);//turn off
    }

    
    //LED 4, using as a fan input
    if (compString(HTTPREQUEST, "LED4=1")) 
    {
        LED_state[3] = 1;  // save LED state (or fan)
        digitalWrite(9, HIGH);//turn on
    }
    else if (compString(HTTPREQUEST, "LED4=0")) 
    {
        LED_state[3] = 0;  // save LED state (or fan)
        digitalWrite(9, LOW);//turn off

        
    }
}

// function that clears the string
void clearString(char *str, char l)
{
    for (int i = 0; i < l; i++)
    {
    str[i] = 0;
    }
}

//function that searches for a string inside another string
char compString(char *str, char *str2)
{
    char foundChar = 0;//used to compare with the string
    char index = 0;//index character for iterating
    char l;//stores length of string
    l = strlen(str);//get length of string
    
    if (strlen(str2) > l)//if string is greater than length of str2
    {
        return 0;
    }
    while (index < l)//if the index is less than the length
    {
        if (str[index] == str2[foundChar]) 
        {   foundChar++;
            if (strlen(str2) == foundChar) 
            {
            return 1;
            }
        }
        else
        {
        foundChar = 0;
        }
        index++;
    }
    return 0;
}



//function that sends the XML file that is updated
void XMLUpdate(EthernetClient cl)
{
    
    cl.print("<?xml version = \"1.0\" ?>");

    

    // LED1
    cl.print("<LED>");
    if (LED_state[0])//if on, it is checked
    {
        cl.print("checked");
    }
    else
    {
        cl.print("unchecked");//else unchecked
    }
    cl.println("</LED>");

    
    
    // LED2
    cl.print("<LED>");
    if (LED_state[1])//if on, it is checked
    {
        cl.print("checked");
    }
    else
    {
        cl.print("unchecked");//else unchecked
    }
     cl.println("</LED>");

     
    
    // LED3
    cl.print("<LED>");
    if (LED_state[2])//if on, it is checked
    {
        cl.print("checked");
    }
    else
    {
        cl.print("unchecked");//else unchecked
    }
    cl.println("</LED>");
    
    
    
    // LED4
    cl.print("<LED>");
    if (LED_state[3])//if on, it is checked
    {
        cl.print("checked");
    }
    else
    {
        cl.print("unchecked");//else unchecked
    }
    cl.println("</LED>");
    
   cl.print("</inputs>");
}



