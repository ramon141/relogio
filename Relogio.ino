#include<Tone.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <virtuabotixRTC.h>



#define   clk   6
#define   dat   7
#define   rst   8

virtuabotixRTC   myRTC(clk, dat, rst);
LiquidCrystal_I2C lcd (0x27, 16, 2);

int ondeEstaSeta = 3;

// --- Constantes Auxiliares ---
#define   segL       0
#define   minL       37
#define   horL       17
#define   d_semL      4
#define   d_mesL     24
#define   mesL        3
#define   anoL     2021

#define RGB_R 12
#define RGB_G 4
#define RGB_B 2
#define pinLED 3
#define pinBuzzer 5
#define pinPHOTORE A1

static String temporizadorPrinted = "00:00";
static String cronometroPrinted = "00:00";
static bool isCronometro = false;

static String horaAlarme = "13:59:50";

static int horaHora = 14;
static int horaMinuto = 03;
static int horaSegundo = 10;

int portLM35 = A0;
static int temp = 0;


/*Inicio Musica*/

Tone tone1;

#define OCTAVE_OFFSET 0

int notes[] = { 0,
NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4,
NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5,
NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6,
NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7, NOTE_AS7, NOTE_B7
};

byte songAtual = 1;

char *song1 = ":d=4,o=5,b=160:c.6,e6,f#6,8a6,g.6,e6,c6,8a,8f#,8f#,8f#,2g,8p,8p,8f#,8f#,8f#,8g,a#.,8c6,8c6,8c6,c6";
char *song2 = ":d=4";
char *song3 = ":d=4,o=5,b=140:32p,c6,8f6,8e6,8d6,8c6,a.,8c6,8f6,8e6,8d6,8d#6,e.6,8e6,8e6,8c6,8d6,8c6,8e6,8c6,8d6,8a,8c6,8g,8a#,8a,8f";
char *song4 = ":d=4,o=5,b=160:8d#,8e,2p,8f#,8g,2p,8d#,8e,16p,8f#,8g,16p,8c6,8b,16p,8d#,8e,16p,8b,2a#,2p,16a,16g,16e,16d,2e";

/*Fim Musica*/
byte setaParaCimaChar[8]={B00100,B01110,B10101,B00100,B00100,B00100,B00100,B00100};
byte coracao[8]={B00000,B01010,B11111,B11111,B01110,B00100,B00000,B00000};

int interfaceAtual = 0;

//Sobe
#define pinBotao1 11
//Desce
#define pinBotao2 10
//Volta/avanca
#define pinBotao3 9

String mudarHora = "00:00:00";
String mudarData = "01/01/2021";

void drawInterface(int inter);

void setup(){
  
    Serial.begin(9600);
    
    tone1.begin(pinBuzzer);

    myRTC.setDS1302Time(segL, minL, horL, d_semL, d_mesL, mesL, anoL);

    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    //lcd.print("ramon");

    lcd.createChar(1, setaParaCimaChar);
    lcd.createChar(2, coracao);
    
    pinMode(pinBotao1, INPUT_PULLUP);
    pinMode(pinBotao2, INPUT_PULLUP);
    pinMode(pinBotao3, INPUT_PULLUP);
    pinMode(portLM35, INPUT);
    pinMode(pinPHOTORE, INPUT);
    pinMode(pinLED, OUTPUT);
    pinMode(pinBuzzer, OUTPUT);
    pinMode(RGB_R, OUTPUT);
    pinMode(RGB_G, OUTPUT);
    pinMode(RGB_B, OUTPUT);

    //Serial.println(lcd.print());

    drawInterface(interfaceAtual);
}

void play_rtttl(char *p)
{
 
  // Absolutely no error checking in here

  byte default_dur = 4;
  byte default_oct = 6;
  int bpm = 63;
  int num;
  long wholenote;
  long duration;
  byte note;
  byte scale;

  // format: d=N,o=N,b=NNN:
  // find the start (skip name, etc)

  while(*p != ':') p++;    // ignore name
  p++;                     // skip ':'

  // get default duration
  if(*p == 'd')
  {
    p++; p++;              // skip "d="
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    if(num > 0) default_dur = num;
    p++;                   // skip comma
  }

  Serial.print("ddur: "); Serial.println(default_dur, 10);

  // get default octave
  if(*p == 'o')
  {
    p++; p++;              // skip "o="
    num = *p++ - '0';
    if(num >= 3 && num <=7) default_oct = num;
    p++;                   // skip comma
  }

  Serial.print("doct: "); Serial.println(default_oct, 10);

  // get BPM
  if(*p == 'b')
  {
    p++; p++;              // skip "b="
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    bpm = num;
    p++;                   // skip colon
  }

  Serial.print("bpm: "); Serial.println(bpm, 10);

  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole note (in milliseconds)

  Serial.print("wn: "); Serial.println(wholenote, 10);


  // now begin note loop
  while(*p)
  {
    // first, get note duration, if available
    num = 0;
    while(isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    
    if(num) duration = wholenote / num;
    else duration = wholenote / default_dur;  // we will need to check if we are a dotted note after

    // now get the note
    note = 0;

    switch(*p)
    {
      case 'c':
        note = 1;
        break;
      case 'd':
        note = 3;
        break;
      case 'e':
        note = 5;
        break;
      case 'f':
        note = 6;
        break;
      case 'g':
        note = 8;
        break;
      case 'a':
        note = 10;
        break;
      case 'b':
        note = 12;
        break;
      case 'p':
      default:
        note = 0;
    }
    p++;

    // now, get optional '#' sharp
    if(*p == '#')
    {
      note++;
      p++;
    }

    // now, get optional '.' dotted note
    if(*p == '.')
    {
      duration += duration/2;
      p++;
    }
  
    // now, get scale
    if(isdigit(*p))
    {
      scale = *p - '0';
      p++;
    }
    else
    {
      scale = default_oct;
    }

    scale += OCTAVE_OFFSET;

    if(*p == ',')
      p++;       // skip comma for next note (or we may be at the end)

    // now play the note


    if(note)
    {
      Serial.print("Playing: ");
      Serial.print(scale, 10); Serial.print(' ');
      Serial.print(note, 10); Serial.print(" (");
      Serial.print(notes[(scale - 4) * 12 + note], 10);
      Serial.print(") ");
      Serial.println(duration, 10);
      tone1.play(notes[(scale - 4) * 12 + note]);
      delay(duration);
      tone1.stop();
    }
    else
    {
      Serial.print("Pausing: ");
      Serial.println(duration, 10);
      delay(duration);
    } 
  } 
}

String formatHora(long numero){
      int hora = numero / 3600;
      int minuto = (numero % 3600) / 60;
      int segundo = numero % 60;
      
      String retorno = ((hora > 9)? String(hora) :       "0" + String(hora)) + ":" + 
             ((minuto > 10)? String(minuto) :   "0" + String(minuto)) + ":" + 
             ((segundo > 10)? String(segundo) : "0" + String(segundo));

      Serial.println("Numero=" + String(numero)+ "Return="+retorno);
    
      return retorno;
    }

long getValuetRTCSeconds(){
  myRTC.updateTime(); 
  return ((long)myRTC.hours * 3600) + ((long)myRTC.minutes * 60) + (long)myRTC.seconds;
}

void playSong(){
  if(songAtual == 1) play_rtttl(song1);
  if(songAtual == 2) play_rtttl(song2);
  if(songAtual == 3) play_rtttl(song3);
  if(songAtual == 4) play_rtttl(song4);
  
  while(0);
  delay(1000);
}


void drawInterface(int inter){
    //Serial.println("interface" + interfaceAtual);
    interfaceAtual = inter;

    if(interfaceAtual == -1){
      lcd.clear();
      lcd.print("  Made with ");
      lcd.write(2);
      lcd.write(2);
      lcd.setCursor(0,1);
      lcd.print("    by Ramon");
    }

    if(interfaceAtual == 0){
        modificarRelogio();
    }

    if (inter == 1){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temporizador[*]");
        lcd.setCursor(0, 1);
        lcd.print("Cronometro[ ]");
    }

    if (inter == 2){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temporizador[ ]");
        lcd.setCursor(0, 1);
        lcd.print("Cronometro[*]");
    }

    if (inter == 3)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Cronometro[ ]");
        lcd.setCursor(0, 1);
        lcd.print("Alarme[*]");
    }

    if (inter == 4)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Alarme[ ]");
        lcd.setCursor(0, 1);
        lcd.print("Ajustar Hora[*]");
    }

    if (inter == 5)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Ajustar Hora[ ]");
        lcd.setCursor(0, 1);
        lcd.print("Ajustar Data[*]");
    }

    if (inter == 6)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Ajustar Data[ ]");
        lcd.setCursor(0, 1);
        lcd.print("Mudar musica[*]");
    }

    if (inter == 7)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Mudar musica[ ]");
        lcd.setCursor(0, 1);
        lcd.print("Voltar[*]");
    }

    if (inter == 8)
    { //Temporizador interface
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("00:00");
    }

    if (inter == 9)
    { //Cronometro interface
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("00:00");
    }

    if (inter == 10)
    { //Alarme interface
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("00:00:00");
    }
    if (inter == 11)
    { //Ajuste de Hora interface
        lcd.clear();
        mudarHora = formatHora(getValuetRTCSeconds());
        Serial.println("Mudar hora: "+mudarHora);
        lcd.print(mudarHora);

        ondeEstaSeta = 1;
        lcd.setCursor(0, 1);
        lcd.write(1);
        lcd.write(1);
    }

    if (inter == 12)
    { //Ajuste de Data interface
        lcd.clear();
        
        mudarData = (myRTC.dayofmonth > 9)? String(myRTC.dayofmonth) : "0" + String(myRTC.dayofmonth);
        mudarData += "/";
        mudarData += (myRTC.month > 9)? String(myRTC.month) : "0" + String(myRTC.month);
        mudarData += "/";
        mudarData += String(myRTC.year);
        
        lcd.print(mudarData);

        ondeEstaSeta = 1;
        lcd.setCursor(0, 1);
        lcd.write(1);
        lcd.write(1);
    }

    /*Incio do menu da musica*/
    if (inter == 13)
    {//Escolher musica
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Os simpsons[*]");
        lcd.setCursor(0, 1);
        lcd.print("Musica[ ]");
    }
    
    if (inter == 14)
    {//Escolher musica
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Os simpsons[ ]");
        lcd.setCursor(0, 1);
        lcd.print("Musica[*]");
    }

    if (inter == 15)
    {//Escolher musica
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Take on Me[ ]");
        lcd.setCursor(0, 1);
        lcd.print("Looney[*]");
    }

    if (inter == 16)
    {//Escolher musica
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Loney[ ]");
        lcd.setCursor(0, 1);
        lcd.print("PinkPanther[*]");
    }
    
    /*Fim do menu da musica*/
}




void irParaInterface(int btnClick){
    /*
      Temporizador     1
      Cronometro       2
      Alarme           3
      Ajuste de hora   4
      Ajustar data     5
      Mudar musica     6
      Voltar           7
    */

    
    if (interfaceAtual >= 1 && interfaceAtual <= 7)
    { //Menu
        if (interfaceAtual > 1 && btnClick == pinBotao1)
        {
            drawInterface(interfaceAtual - 1);
        }
        else if (interfaceAtual < 7 && btnClick == pinBotao2)
        {
            drawInterface(interfaceAtual + 1);
        }
    }

    if (btnClick == pinBotao3)
    {
        if (interfaceAtual == 1)
        { //Desenhar o temporizador
            drawInterface(8);
        }
        else if (interfaceAtual == 2)
        { // Desenhar o cronometro
            drawInterface(9);
        }
        else if (interfaceAtual == 3)
        { //Desenhar o alarme
            drawInterface(10);
        }
        else if (interfaceAtual == 4)
        { //Desenhar o Ajustar Hora
            drawInterface(11);
        }
        else if (interfaceAtual == 5)
        { //Desenhar o Ajustar Data
            drawInterface(12);
        }
        else if (interfaceAtual == 6)
        { //Desenhar o Mudar Musica
            drawInterface(13);
        }
        else if (interfaceAtual == 7)
        { //Voltar
            drawInterface(0);
        }
    }
    //Serial.println("irParaInterface: btnClick=" + String(btnClick) + "      InterfaceAtual=" + String(interfaceAtual));
}

String formatSeconds(int sec){
    String ret;
    int m = (sec / 60);
    int s = (sec % 60);
    if (m < 10)
    {
        ret = "0" + String(m) + ":";
    }
    else
    {
        ret = String(m) + ":";
    }
    if (s < 10)
    {
        ret = ret + "0" + String(s);
    }
    else
    {
        ret = ret + String(s);
    }
    return ret;
}

int getSeconds(String s){
    return (s.substring(0, 2).toInt() * 60) + s.substring(3, 5).toInt();
}



void startTemporizado(){
    int valueBegin = millis();
    
    int lastValueShowed = 0;
    int valueShow;
    
    int valorTempoInicial = getSeconds(temporizadorPrinted);
    for (int i = 0, quantSeg = 1; quantSeg > 0; i++){
      valueShow = millis();
      quantSeg = valorTempoInicial - ((int)(valueShow - valueBegin)/1000);
      
      if(lastValueShowed != quantSeg){
            lcd.clear();
            lastValueShowed = quantSeg;
            temporizadorPrinted = formatSeconds(quantSeg);
            lcd.print(temporizadorPrinted);
        }

        if (digitalRead(pinBotao1) == LOW || digitalRead(pinBotao2) == LOW){
            drawInterface(8);
            temporizadorPrinted = "00:00";
            return;
        }
    }

    playSong();
    if (digitalRead(pinBotao1) == LOW || digitalRead(pinBotao2) == LOW || digitalRead(pinBotao3) == LOW) return;
   
}



void startCronometro(){
    int valueBegin = getValuetRTCSeconds();
    int lastValueShowed = -1;
    int valueShow;
    
    cronometroPrinted = "00:00";
    isCronometro = true;
    
    while(true){
      valueShow = getValuetRTCSeconds() - valueBegin;
      
       if(isCronometro){
          if(valueShow != lastValueShowed){
              lcd.clear();
              lcd.setCursor(0, 0);
              cronometroPrinted = formatSeconds(valueShow);
              lcd.print(cronometroPrinted);
              lastValueShowed = valueShow;
          }
        }
        
        delay(100);
        
        if(digitalRead(pinBotao2) == LOW){
            isCronometro = false;
        }
        
        if(digitalRead(pinBotao3) == LOW){
          drawInterface(1);
          break;
        }
        
    }
}

void changeInterface(int btnClick){

    if (interfaceAtual == 8)
    {   //Modificar o temporizador
        //Pin1 aumenta
        //Pin2 diminui
        //Pin3 comeca
        if (btnClick == pinBotao1)
        {
            if (getSeconds(temporizadorPrinted) < 5999)
            {
                lcd.clear();
                lcd.setCursor(0, 0);
                temporizadorPrinted = formatSeconds(getSeconds(temporizadorPrinted) + 1);
                lcd.print(temporizadorPrinted);
            }
        }
        if (btnClick == pinBotao2)
        {
            if (getSeconds(temporizadorPrinted) > 0)
            {
                lcd.clear();
                lcd.setCursor(0, 0);
                temporizadorPrinted = formatSeconds(getSeconds(temporizadorPrinted) - 1);
                lcd.print(temporizadorPrinted);
            }
        }
        if (btnClick == pinBotao3)
        {
            if (getSeconds(temporizadorPrinted) > 0)
            {
                startTemporizado();
            }
            else
            {
                drawInterface(1);
            }
        }
    } else if (interfaceAtual == 9 && btnClick == pinBotao3){ //Modificar o cronometro
        Serial.println("entra: "+String(isCronometro));
        
        startCronometro();
        
    } else if (interfaceAtual == 10){//Modificar o alarme
        //Serial.println("entrou");
        
        //Serial.println(horaAlarme.substring(0, 5));
        //Serial.println(getSeconds(horaAlarme.substring(0, 5)) + 1);
//        Serial.println(formatSeconds(getSeconds(horaAlarme.substring(0, 5)) + 1));

        if(btnClick == pinBotao2){
            horaAlarme = formatSeconds(getSeconds(horaAlarme.substring(0, 5)) + 1) + ":00";
        } else if(btnClick == pinBotao1){
            horaAlarme = formatSeconds(getSeconds(horaAlarme.substring(0, 5)) + 1) + ":00";
        } else if(btnClick == pinBotao3){
            drawInterface(0);
            return;
        }
        //Serial.println("entrou\n\n");

        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(horaAlarme);
    } else if (interfaceAtual == 11){//Modificar a hora
      //Serial.println("entrou aqui");
      
      if(btnClick == pinBotao2){
        lcd.clear();
        
        lcd.print(mudarHora);

        ondeEstaSeta = (ondeEstaSeta >= 3)? 1 : ++ondeEstaSeta;
        Serial.println(String(ondeEstaSeta));
        
        if(ondeEstaSeta == 1){//seta para cima
          lcd.setCursor(0,1);
        } else if(ondeEstaSeta == 2){
          lcd.setCursor(3,1);
        } else if(ondeEstaSeta == 3){
          lcd.setCursor(6,1);
        }
        
        lcd.write(1);
        lcd.write(1);
      
      } else if(btnClick == pinBotao1){
          if(ondeEstaSeta == 1){//seta no campo de hora
            int horaL = (mudarHora.substring(0,2)).toInt();
            horaL = (horaL >= 23)? 0 : ++horaL;
            String colocar = (horaL > 9)? String(horaL) : "0" + String(horaL);
            mudarHora = colocar + mudarHora.substring(2,8);
            
          } else if(ondeEstaSeta == 2){//seta no campo de minuto
            int minutoL = (mudarHora.substring(3,5)).toInt();
            
            String minutoLAux = (minutoL > 9)? String(minutoL) : "0" + String(minutoL);
            
            minutoL = (minutoL >= 59)? 0 : ++minutoL;
            String colocar = (minutoL > 9)? String(minutoL) : "0" + String(minutoL);
            
            mudarHora.replace((":" + minutoLAux + ":"), (":" + colocar + ":")); 
          
          } else if(ondeEstaSeta == 3){//seta no campo de segundo
            int segundoL = (mudarHora.substring(6,8)).toInt();
            
            segundoL = (segundoL >= 59)? 0 : ++segundoL;
            
            String colocar = (segundoL > 9)? String(segundoL) : "0" + String(segundoL);
            mudarHora = mudarHora.substring(0,6) + colocar;
          }
          lcd.clear();
        
          lcd.print(mudarHora);
          
          if(ondeEstaSeta == 1){//seta para cima
            lcd.setCursor(0,1);
          } else if(ondeEstaSeta == 2){
            lcd.setCursor(3,1);
          } else if(ondeEstaSeta == 3){
            lcd.setCursor(6,1);
          }
          
          lcd.write(1);
          lcd.write(1); 
      } else if(btnClick == pinBotao3){
        myRTC.setDS1302Time(/*Sgundo*/(mudarHora.substring(6,8)).toInt(), /*Minuto*/(mudarHora.substring(3,5)).toInt(), /*Hora*/ (mudarHora.substring(0,2)).toInt(), d_semL, d_mesL, mesL, anoL);
        drawInterface(0);
      }
      
    } else if(interfaceAtual == 12){
        if(btnClick == pinBotao2){
          lcd.clear();
        
          lcd.print(mudarData);
  
          ondeEstaSeta = (ondeEstaSeta >= 3)? 1 : ++ondeEstaSeta;
          
          if(ondeEstaSeta == 1){//seta para cima
            lcd.setCursor(0,1);
          } else if(ondeEstaSeta == 2){
            lcd.setCursor(3,1);
          } else if(ondeEstaSeta == 3){
            lcd.setCursor(6,1);
            
            lcd.write(1);
            lcd.write(1);
          }
          
          lcd.write(1);
          lcd.write(1);
        } else if(btnClick == pinBotao1){
          if(ondeEstaSeta == 1){//Altera dia
            int diaL = (mudarData.substring(0,2)).toInt();
            diaL = (diaL >= 31)? 1 : ++diaL;
            String colocar = (diaL > 9)? String(diaL) : "0" + String(diaL);
            mudarData = colocar + mudarData.substring(2,10);
          
          } else if(ondeEstaSeta == 2){//Altera dia
            int mesLocal = (mudarData.substring(3,5)).toInt();
            mesLocal = (mesLocal >= 12)? 1 : ++mesLocal;
            String colocar = (mesLocal > 9)? String(mesLocal) : "0" + String(mesLocal);
            mudarData = mudarData.substring(0,3) + colocar + mudarData.substring(5,10);
            
          } else if(ondeEstaSeta == 3){//Altera dia
            int anoLocal = (mudarData.substring(6,10)).toInt();
            anoLocal = (anoLocal >= 2050)? 2021 : ++anoLocal;
            String colocar = (anoLocal > 9)? String(anoLocal) : "0" + String(anoLocal);
            mudarData = mudarData.substring(0,6) + colocar;
          }
          
          lcd.clear();
        
          lcd.print(mudarData);
          
          if(ondeEstaSeta == 1){//seta para cima
            lcd.setCursor(0,1);
          } else if(ondeEstaSeta == 2){
            lcd.setCursor(3,1);
          } else if(ondeEstaSeta == 3){
            lcd.setCursor(6,1);
            lcd.write(1);
            lcd.write(1);
          }
          
          lcd.write(1);
          lcd.write(1);
        } else if(btnClick == pinBotao3){
          myRTC.setDS1302Time(segL, minL, horL, d_semL, /*Dia*/ (mudarData.substring(0,2)).toInt(), /*mes*/ (mudarData.substring(3,5)).toInt(), /*Ano*/ (mudarData.substring(6,10)).toInt());
          
          drawInterface(0);
        }
      
    }else if (interfaceAtual >= 13 && interfaceAtual <= 16){ //Menu da musica   
        if (interfaceAtual > 13 && btnClick == pinBotao1){
            drawInterface(interfaceAtual - 1);
        }
        else if (interfaceAtual < 16 && btnClick == pinBotao2)
        {
            drawInterface(interfaceAtual + 1);
        } else if(btnClick == pinBotao3){
          
          if(interfaceAtual == 13) songAtual = 1;
          if(interfaceAtual == 14) songAtual = 2;
          if(interfaceAtual == 15) songAtual = 3;
          if(interfaceAtual == 16) songAtual = 4;

          drawInterface(6);
          playSong();
        }
    }
}

void modificarRelogio(){
    lcd.clear();
    lcd.setCursor(0,0);

    horaSegundo++;
    if(horaSegundo >= 60){
        horaSegundo=0;
        horaMinuto++;
    }
    if(horaMinuto >= 60){
        horaMinuto = 0;
        horaHora++;
    }
    if(horaHora >= 24){
        horaHora = 0;
    }
myRTC.updateTime();
    String mostrah = (myRTC.hours > 9)? String(myRTC.hours) : "0" + String(myRTC.hours);
    mostrah += ":";
    mostrah += (myRTC.minutes > 9)? String(myRTC.minutes) : "0" + String(myRTC.minutes);
    mostrah += ":";
    mostrah += (myRTC.seconds > 9)? String(myRTC.seconds) : "0" + String(myRTC.seconds);
    
    String mostrad = (myRTC.dayofmonth > 9)? String(myRTC.dayofmonth) : "0" + String(myRTC.dayofmonth);
    mostrad += "/";
    mostrad += (myRTC.month > 9)? String(myRTC.month) : "0" + String(myRTC.month);
    mostrad += "/";
    mostrad += String(myRTC.year);

    if(mostrah == horaAlarme){
        playSong();
    }

    


    //temp = map(((analogRead(portLM35) - 20) * 3.04), 0, 1023, -40, 125);
    //temp = (int)((float(analogRead(LM35))*5/(1023))/0.01);
    temp = (int)((analogRead(portLM35) * 0.0048828125 * 100));
    
    String tm = String(temp)+"C";

    //Serial.println((String(5+(242*temp)/165)) + ", 118, "+ String(242 - ((242*temp)/165)));
    
  if(temp < 20){
    analogWrite(RGB_R, 64);
    analogWrite(RGB_G, 83);
    analogWrite(RGB_B, 247);
  } else if (temp >= 21 && temp <= 22){
    analogWrite(RGB_R, 64);
    analogWrite(RGB_G, 189);
    analogWrite(RGB_B, 247);
  } else if (temp == 23 || temp == 24){
    analogWrite(RGB_R, 64);
    analogWrite(RGB_G, 247);
    analogWrite(RGB_B, 125);
  } else if (temp == 25 || temp == 26){
    analogWrite(RGB_R, 204);
    analogWrite(RGB_G, 247);
    analogWrite(RGB_B, 64);
  } else if (temp == 27 || temp == 28){
    analogWrite(RGB_R, 226);
    analogWrite(RGB_G, 247);
    analogWrite(RGB_B, 64);
  } else if (temp == 29 || temp == 27){
    analogWrite(RGB_R, 247);
    analogWrite(RGB_G, 229);
    analogWrite(RGB_B, 64);
  } else if (temp == 31 || temp <= 35){
    analogWrite(RGB_R, 247);
    analogWrite(RGB_G, 149);
    analogWrite(RGB_B, 64);
  } else {
    analogWrite(RGB_R, 247);
    analogWrite(RGB_G, 64);
    analogWrite(RGB_B, 64);
  }

    
//    analogWrite(RGB_R, 5 + ((242*temp)/165));
//    analogWrite(RGB_G, 118);
//    analogWrite(RGB_B, 242 - ((242*temp)/165));

    lcd.print(mostrah);
    lcd.setCursor(0,1);
    lcd.print(mostrad);
    lcd.setCursor(13,0);
    lcd.print(tm);
}

void loop(){
/*  play_rtttl(song1);
  Serial.println("Done.");
  while(0);
  delay(1000);
*/
    if(interfaceAtual == -1){
      if (digitalRead(pinBotao1) == LOW || digitalRead(pinBotao2) == LOW || digitalRead(pinBotao3) == LOW){
          drawInterface(0);
      }
    } else if(interfaceAtual == 0){
        for(int i = 0; i < 10; i++){
            if(i == 9){
                modificarRelogio();
            }
            delay(89);
            if (digitalRead(pinBotao3) == LOW){
                drawInterface(1);
                break;
            }
            
            if (digitalRead(pinBotao1) == LOW){
                drawInterface(-1);
                break;
            }
        }
        


    } else if (interfaceAtual <= 7) {
        if (digitalRead(pinBotao1) == LOW)
        {
            irParaInterface(pinBotao1);
        }
        if (digitalRead(pinBotao2) == LOW)
        {
            irParaInterface(pinBotao2);
        }
        if (digitalRead(pinBotao3) == LOW)
        {
            irParaInterface(pinBotao3);
        }
    } else if (interfaceAtual > 7) {
        if (digitalRead(pinBotao1) == LOW)
        {
            changeInterface(pinBotao1);
        }
        if (digitalRead(pinBotao2) == LOW)
        {
            changeInterface(pinBotao2);
        }
        if (digitalRead(pinBotao3) == LOW)
        {
            changeInterface(pinBotao3);
        }
    }

//    Serial.println("110");
    //Serial.println(analogRead(pinPHOTORE));
    if(analogRead(pinPHOTORE) <= 500){
        digitalWrite(pinLED, HIGH);
    } else {
        digitalWrite(pinLED, LOW);
    }
    delay(110);
    
    
}
