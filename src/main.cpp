#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

//=====================================================
//Pines RFID
//=====================================================
uint8_t pinRSTfid = D3;
uint8_t pinCSfid = D8;


MFRC522 rfid(pinCSfid, pinRSTfid);

WiFiClient cliente;
HTTPClient clienteHtpp;

//---------------------------------------------------------------
//Indica si se encuentra disponible una tarjeta cerca del sensor
//---------------------------------------------------------------
bool tarjetaDisponible()
{
  if (rfid.PICC_IsNewCardPresent())
  {
    if (rfid.PICC_ReadCardSerial())
    {
      return true;
    }
  }
  return false;
}


//---------------------------------------------------------------
//Lee el UUID de la tarjeta que se encuentra en el sensor
//---------------------------------------------------------------
String leerTarjeta()
{
  String strId = "";
  for (int i = 0; i < rfid.uid.size; i++)
  {
    if(rfid.uid.uidByte[i] < 0x10){
      strId+="0";
    }
    strId += String(rfid.uid.uidByte[i], HEX);

    if(i + 1 < rfid.uid.size ){
      strId += "-";
      
    }
  }
  rfid.PICC_HaltA();
  strId.toUpperCase();
  return strId;
}



//Realiza peticion post servidor
void PeticionPost(String uuid){

  //Objeto json
  StaticJsonDocument<300> json;
  //Sring el cual se enviara en el cuerpo de la peticion
  String mensajePost;


  json["uuid"] = uuid;
   
  //Iniciamos nuestro cliente http indicando el clienteWifi 
  //y la ruta de nuestro servidor
  clienteHtpp.begin(cliente,"Url servidor");


  //convertimos el json a un string
  //el cual sera enviado
  serializeJson(json,mensajePost);

  //Realizamos la  peticion post
  int codigorespuesta = clienteHtpp.POST(mensajePost);

    //verificamos que se tenga un codigo de respuesta positivo
    if(codigorespuesta > 0){
      Serial.println(F("Peticion realizada con exito"));
    }else{
      Serial.println(F("Erro al realizar la peticion"));
      Serial.println(clienteHtpp.errorToString(codigorespuesta));
    }

 //libera memoria
 cliente.flush();
 clienteHtpp.end();


}
//----------------------------------------------------------------
//Loop que se ejecutara en la funcion principal
//---------------------------------------------------------------
void loopRfid(){
  
  if (tarjetaDisponible())
  {
    String uuid = leerTarjeta();

    Serial.println("Tarjeta detectada");
    Serial.println(uuid);


    PeticionPost(uuid);
  }
}



void setup() {
  // put your setup code here, to run once:

  SPI.begin();
  rfid.PCD_Init();
  Serial.begin(9600);

  
  //Indicamos a que red wifi se conectara
  WiFi.begin("nombre red","contra" );
  Serial.println("Conectando a red wifi");

  //Esperamos a una conexion 

  while (WiFi.status() != wl_status_t::WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("Conectado");
  
}

void loop() {

  //Loop para comprobar si se detecto una tarjeta rfid
  loopRfid();
}

