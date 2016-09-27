#include <ESP8266WiFi.h>

const IPAddress local_IP(192,168,4,21);
const IPAddress gateway(192,168,4,9);
const IPAddress subnet(255,255,255,0);

const char WiFiAPPSK[] = "totara1234";
const char WiFiSSID[] = "Totara-Text";

const int ANALOG_PIN = A0;
const int DIGITAL_PIN = 12; 
const int LED_PIN = 5;

WiFiServer server(80);

void setup() 
{
  Serial.begin(115200);
  pinMode(DIGITAL_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(WiFiSSID, WiFiAPPSK);
  Serial.println("Setup wifi with id: "+String(WiFiSSID)+" and password "+String(WiFiAPPSK));
  server.begin();

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
}

String wrap(String rawString,String Length){
  int rawLength = rawString.length();
  int maxLength = Length.toInt(); 
  int newLineStartingIndex = 0;
  int startingIndex = 0;
  int endingIndex = 0;
  int currentIndex = 0;

  String doneString = "";
  
  bool finishedString = false;
  
  while(!finishedString){ //while the string is not finished, keep doing this loop
    
    currentIndex = startingIndex + maxLength; //Set the current index to startingIndex of the string plus max length
    
    if((currentIndex > rawLength)||(currentIndex<0)){//check to make sure the CurrentIndex is in range of the char array
      finishedString = true;
      break;    
    }
    
     while(rawString[currentIndex] == ' '){ //if the index has landed on a space, move forward untill a letter is found
        currentIndex++;
        if(currentIndex > rawLength){
            break;
        }
      }
           
      while(rawString[currentIndex] != ' '){ //if the index has landed on not a space, move backward untill a space is found, this will exit at the index for the start of the next line
          currentIndex--;
      }
      newLineStartingIndex = currentIndex;     
      
      while(rawString[currentIndex] == ' '){ //go back untill you find the ending index of the first word
          currentIndex--;
      }
      endingIndex = currentIndex+1; //record the ending index 

      
      doneString += rawString.substring(startingIndex, endingIndex)+"<br>";
      startingIndex = newLineStartingIndex+1;
  }
  doneString += (rawString.substring(startingIndex, rawLength) + "<br>"); //add the rest of the raw string onto the end
  return doneString;  
}

void loop(){
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  
  String req = client.readString();// Read the full request string
  Serial.println(req);
  client.flush();

  bool invalidInput = false;
  
  int CPLstartIndex = req.indexOf("CharsPerLine="); //get the starting index of the start of the chars per line string value passed in through the post request
  int CPLendIndex = req.indexOf("&",CPLstartIndex); //get the ending index of the end of the chars per line string value by finding the next & symbol linking the form data

  int RTIstartIndex = req.indexOf("RawTextInput="); //get the starting index of the start of the raw text input string value passed in through the post request
  int RTIendIndex = req.indexOf("/n",RTIstartIndex); //get the ending index of the end of the  raw text input string value by finding the newline char
  
  String CharsPerLine = req.substring(CPLstartIndex+13,CPLendIndex);

  String RawTextInput = req.substring(RTIstartIndex+13,RTIendIndex);

  if(CharsPerLine.length() > 5){ //Invalid input if the length of the chars per line string (Which should have a normal length of 0-3 chars) Goes above a length of 5
    invalidInput = true;
  }
  
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n<body>\r\n\r\n";

  s += "<form action=\"action_page.php\" method=\"post\">\r\n";
  s += "Max Charchers Per Line:<br>\r\n";
  s += "<input type=\"text\" name=\"CharsPerLine\" value=\"12\"><br>\r\n";
  
  s += "Text to format:<br>\r\n";
  s += "<input type=\"text\" name=\"RawTextInput\" value=\"The quick brown fox jumps over the lazy dog\" is an English-language pangram—a phrase that contains all of the letters of the alphabet\">\r\n";
  s += "<br><br>\r\n";
  s += "<input type=\"submit\" value=\"Submit\">\r\n";
  s += "</form>\r\n\r\n";

  if(!invalidInput){
    s += "<p>With a Wrapping length of "+CharsPerLine+" and a Raw Text Input of ";
    s += RawTextInput+" The Result was as follows.</p>";
    RawTextInput.replace("+", " ");
    String OutputText = wrap(RawTextInput,CharsPerLine);
    s += "<p>"+OutputText+".</p>";
    Serial.println(OutputText);
  }
  
  
  s += "</body>\n";
  s += "</html>\n";

  //Serial.print(s);
  client.print(s); //Submit the data to the client
  delay(10);
  Serial.println("Client disonnected");
}


