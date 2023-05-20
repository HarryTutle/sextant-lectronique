
/*bibliothèques nécessaires au programme*/


#include <SoftwareSerial.h>//permet d'émuler une seconde voie série.

SoftwareSerial retourSerial(11,12);// Rx=11 et Tx=12

#include <MPU6050.h>// gyrocompas.

#include <LiquidCrystal_I2C.h>//ecran lcd en i2C.

#include <Keypad.h>//le clavier.

#include <Wire.h>//le bus I2C.

#include <MechaQMC5883.h>//le compas GY271(modèle QMC).

#include <math.h>//fonctions maths en supplément (trigo notamment).

/*variables globales compas */

MechaQMC5883 capteur;

     int mx, my, mz; // déclaration des variables sur les axes x, y, z
     float angle;



/*variables globales clavier (keypad)*/


const byte LIGNES = 4;
const byte COLONNES = 4;

char keys [LIGNES][COLONNES] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'#', '0', '*', 'D'}
};
byte ligPins[LIGNES] = {5, 4, 3, 2};
byte colPins[COLONNES] = {9 , 8, 7, 6};

Keypad keypad = Keypad( makeKeymap(keys), ligPins, colPins, LIGNES, COLONNES );


      char Clavier[4];// variables globales utilisées pour stocker les données du clavier (point estimé, hauteur oeil).
      int compteur = 0;


/*variables pour le menu des options A,B,C et D (change également les switchs du clavier en interrupteurs "classiques")*/

      char preDataState=0;// permet de gérer le menu gitomètre, compas de relèvement, heure utc et droites de hauteur.

      char DataState=0;// variable de transfert pour grossomerdo changer les switchs du keypad en interrupteurs "permanents".



/*variables globales ecran (liquidcrystal lcd)*/

LiquidCrystal_I2C lcd(0x27,16,2); //ecran lcd (adresse I2C,lignes, colonnes)

/*variables globales gyrocompas (mpu6050)*/


MPU6050 accelgyro(0x69); // changement d'adresse du gyrocompas de 0x68 (par défaut) à 0x69.

int16_t ax, ay, az;
int16_t gx, gy, gz;
uint8_t Accel_range;
uint8_t Gyro_range;
      float Gite = 0;
      float Tangage = 0;
      float GiteInit=-0.93;//permet d'étalonner le gyrocompas en gite.
      float TangageInit=-2.12;//permet d'étalonner le gyrocompas en Tangage (particulièrement important pour la précision du sextant).

/*variables horloge externe (ds3231)*/

#define ADRESSE_I2C_RTC 0x68 //adresse de l'horloge.
byte seconde,minute,heure,numJourSemaine,numJourMois,mois,annee;
String jourDeLaSemaine[8]={"","Dim","Lun","Mar","Mer","Jeu","Ven","Sam"};

/*variable buzzer*/

       const int BUZZER=10;

/*variables de calcul*/

       int LatDeg=333;//  variables du point estimé, initialisées à 333 (arbitraire) pour pouvoir rentrer le 0 comme valeur dans les calculs.
       int LatMin=333;
       int LonDeg=333;
       int LonMin=333;
       byte hauteurOeil=250;
       float estimeLat=0;
       float estimeLon=0;
       int Intercept=0;
       int Azimut=0;

       struct retour {
        int Intercept;
        int Azimut;
       };

       retour point;
       
  
    

void setup() {
  keypad.setDebounceTime(10);// cette instruction évite les problèmes de rebonds avec le clavier.
  Serial.begin(57600);//ouvre la première liaison série pour communiquer entre les 2 cartes.
  retourSerial.begin(57600);//ouvre la seconde liaison série.
  
  capteur.init();
  //capteur.setMode(Mode_Continuous,ODR_200Hz,RNG_2G,OSR_256);pour le compas magnétique.

  Wire.begin();// initialise la liaison I2C.

  
  

  /* setup ecran*/

  lcd.init();
  lcd.backlight(); 
  

  lcd.home();
  lcd.setCursor(3,0);
  lcd.print("navigogol");
  lcd.setCursor(0,1);
  lcd.print("on est ou ?");
  delay(3000);
  lcd.clear();
  
  

  /* setup gyrocompas */

  
  accelgyro.initialize();

  lcd.home();
  lcd.print("Test gyro ...");
  lcd.setCursor(0,1);
  lcd.print(accelgyro.testConnection() ? "gyro ok" : "gyro pas ok");
  delay(1000);
  lcd.clear();
  

  
  
  /*configuration du Buzzer */

  pinMode(BUZZER,OUTPUT);
  
  
  
}

void loop() {
  
   /*variables globales utilisées dans le loop*/

  
   char key = keypad.getKey();//on stocke la touche pressée dans la variable key.
   
   

  /*mise en route des composants*/

    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    Gite = 0.98 * (Gite + float(gy) * 0.01 / 131) + 0.02 * atan2((double)ax, (double)az) * 180 / PI;
    Tangage = 0.98 * (Tangage + float(gx) * 0.01 / 131) + 0.02 * atan2((double)ay, (double)az) * 180 / PI;

    

    Wire.beginTransmission(ADRESSE_I2C_RTC);//reception heure et date
    Wire.write(0);//positionne le pointeur de registre sur 00h
    Wire.endTransmission();
    Wire.requestFrom(ADRESSE_I2C_RTC,7);//accède aux données de l'horloge à partir du registre 00h
    seconde=bcdToDec(Wire.read()& 0x7f);
    minute=bcdToDec(Wire.read());
    heure=bcdToDec(Wire.read()&0x3f);
    numJourSemaine=bcdToDec(Wire.read());
    numJourMois=bcdToDec(Wire.read());
    mois=bcdToDec(Wire.read());
    annee=bcdToDec(Wire.read());

    capteur.read(&mx, &my, &mz);
    angle = degrees(atan2((double)mx, (double)my));

     if (angle>=0) {
       angle=360-angle;
  }
     else {
       angle=abs(angle);
  }

  /*codage du menu*/

  DataState=key;//on stocke la touche dans DataState.Permet de garder ensuite avec predatastate en mémoire la touche pressée après la loop.
    
    if (DataState=='A'||DataState=='B'||DataState=='C'||DataState=='D') {

      preDataState=DataState;//change les switch du keypad en interrupteurs "classiques". PreDataState sert à faire fonctionner le menu principal.
      key=0;
      
      lcd.clear();//dès qu'on presse une touche, efface rapidement l'écran. ça évite les bugs avec l'écran lcd.
      
    }

    

   
    

  switch (preDataState) {
    case 'A':// la touche A active le gitomètre.
    
       noTone(BUZZER);
       lcd.home();
       lcd.print("gite:");
       lcd.setCursor(7,0);
       lcd.print(Gite);
       lcd.setCursor(0,1);
       lcd.print("tangage:");
       lcd.setCursor(10,1);
       lcd.print(Tangage);
       delay(10);

       break;

       case 'B':// la touche B active le compas de relèvement.

       noTone(BUZZER);
       lcd.home();
       lcd.print("relevement:");
       lcd.setCursor(3,1);
       lcd.print(angle);
       delay(100);

       break;

       case 'C':// la touche C active le mode sextant(soleil).

       
       
       if (LatDeg==333 && LatMin==333 && LonDeg==333 && LonMin==333 && hauteurOeil==250) {
       
       lcd.home();
       lcd.print("point estime");
       lcd.setCursor(0,1);
       lcd.print("LatDeg:");
       

          if (key!=NO_KEY && key!='*' && key!='#') {
           Clavier[compteur++]=key;
           Clavier[compteur]='\0';// indique l'arrêt de la chaîne de caractères.
           lcd.setCursor(9,1);
           lcd.print(Clavier);
       }
          else if (key=='*') {
           LatDeg=atoi(Clavier);// convertit char en int.
           Clavier[4]={0};
           compteur=0;
           lcd.clear();
       }
          else if (key=='#') {
           LatDeg=atoi(Clavier)*-1;
           Clavier[4]={0};
           compteur=0;
           lcd.clear();
       }
       }
      else if (LatDeg!=333 && LatMin==333 && LonDeg==333 && LonMin==333 && hauteurOeil==250) {
       
       
       lcd.home();
       lcd.print("point estime");
       lcd.setCursor(0,1);
       lcd.print("LatMin:");
       lcd.setCursor(9,1);
       
       
          if (key!=NO_KEY && key!='*' && key !='#') {
            Clavier[compteur++]=key;
            Clavier[compteur]='\0';
            lcd.setCursor(9,1);
            lcd.print(Clavier);
       }

          else if (key=='*') {
           LatMin=atoi(Clavier);
           Clavier[4]={0};
           compteur=0;
           lcd.clear();
       }

          else if (key=='#') {
           LatMin=atoi(Clavier)*-1;
           Clavier[4]={0};
           compteur=0;
           lcd.clear();
       }
  
      }
      else if (LatDeg!=333 && LatMin!=333 && LonDeg==333 && LonMin==333 && hauteurOeil==250) {

       
       lcd.home();
       lcd.print("point estime");
       lcd.setCursor(0,1);
       lcd.print("LonDeg:");
       

          if (key!=NO_KEY && key!='*' && key!='#') {
           Clavier[compteur++]=key;
           Clavier[compteur]='\0';
           lcd.setCursor(9,1);
           lcd.print(Clavier);
       }

          else if (key=='*') {
           LonDeg=atoi(Clavier);
           Clavier[4]=0;
           compteur=0;
           lcd.clear();
       }

          else if (key=='#') {
           LonDeg=atoi(Clavier)*-1;
           Clavier[4]=0;
           compteur=0;
           lcd.clear();
       }
      }

      else if (LatDeg!=333 && LatMin!=333 && LonDeg!=333 && LonMin==333 && hauteurOeil==250) {

       
       lcd.home();
       lcd.print("point estime");
       lcd.setCursor(0,1);
       lcd.print("LonMin:");
       

          if (key!=NO_KEY && key!='*' && key!='#') {
           Clavier[compteur++]=key;
           Clavier[compteur]='\0';
           lcd.setCursor(9,1);
           lcd.print(Clavier);
       }

          else if (key=='*') {
           LonMin=atoi(Clavier);
           Clavier[4]={0};
           compteur=0;
           lcd.clear();
       }

          else if (key=='#') {
           LonMin=atoi(Clavier)*-1;
           Clavier[4]={0};
           compteur=0;
           lcd.clear();
       }
      }

       else if (LatDeg!=333 && LatMin!=333 && LonDeg!=333 && LonMin!=333 && hauteurOeil==250) {
       
       lcd.home();
       lcd.print("hauteur oeil:");
       

           if (key!=NO_KEY && key!='*' && key!='#') {
             Clavier[compteur++]=key;
             Clavier[compteur]='\0';
             lcd.setCursor(0,1);
             lcd.print(Clavier);
             }

           else if (key=='*' || key=='#') {
              hauteurOeil=atoi(Clavier);
              Clavier[4]={0};
              compteur=0;
              lcd.clear();
             }
             
      
             }

             
       


      else if (hauteurOeil!=250 && LatDeg!=333 && LatMin!=333 && LonDeg!=333 && LonMin!=333) {

        
        
       
         lcd.home();
         lcd.print("angle soleil:");
         lcd.setCursor(0,1);
         lcd.print(Tangage);
       
       
      if (key==NO_KEY) {
       
          if (Gite==constrain(Gite,0,2) ) {// facilite la précision de la visée. Si ça buzze c'est bon, on peut appuyer pour lancer la machine.
           tone(BUZZER,500);
       }
          else {
           noTone(BUZZER);
       }
          
          delay(100); 
      }
  
         
       else if (key!=NO_KEY) {
            noTone(BUZZER);
            lcd.clear();

            estimeLat=LatDeg+LatMin/60.00;
            estimeLon=LonDeg+LonMin/60.00;
            char astre='C';

           struct chiffres {// données à envoyer sous forme de variable structure.

             float Tangage;
             float estimeLat;
             float estimeLon;
             byte hauteurOeil;
             byte annee;
             byte numJourMois;
             byte mois;
             byte heure;
             byte minute;
             byte seconde;
             char astre;
             
        };

           chiffres data;

           data.Tangage=Tangage+TangageInit;
           data.estimeLat=estimeLat;
           data.estimeLon=estimeLon;
           data.hauteurOeil=hauteurOeil;
           data.annee=annee;
           data.numJourMois=numJourMois;
           data.mois=mois;
           data.heure=heure;
           data.minute=minute;
           data.seconde=seconde;
           data.astre=astre;
           
           
          
        
           Serial.write((byte *)&data, sizeof data);

           Serial.flush();
        
           

          delay(3000);
          
          
          
          while (retourSerial.available()>0) {
  
          retourSerial.readBytes((byte *)&point, sizeof point);

          
         
          Intercept=point.Intercept;
          Azimut=point.Azimut;

          
          
          
          
          lcd.home();
          lcd.print("inter et azim:");
          lcd.setCursor(0,3);
          lcd.print(Intercept);
          delay(4000);
          lcd.clear();
          lcd.home();
          lcd.print("caca chiant");
          lcd.setCursor(0,3);
          lcd.print(Azimut);
          lcd.clear();
          
          estimeLat=0,estimeLon=0,LatDeg=333,LatMin=333,LonDeg=333,LonMin=333,hauteurOeil=250;
          }
  
      }   
      }
       
         
          

       

      

       break;

       case 'D'://active le mode sextant(lune).

       if (LatDeg==333 && LatMin==333 && LonDeg==333 && LonMin==333 && hauteurOeil==250) {
       
       lcd.home();
       lcd.print("point estime");
       lcd.setCursor(0,1);
       lcd.print("LatDeg:");
       

          if (key!=NO_KEY && key!='*' && key!='#') {
           Clavier[compteur++]=key;
           Clavier[compteur]='\0';// indique l'arrêt de la chaîne de caractères.
           lcd.setCursor(9,1);
           lcd.print(Clavier);
       }
          else if (key=='*') {
           LatDeg=atoi(Clavier);// convertit char en int.
           Clavier[4]={0};
           compteur=0;
           lcd.clear();
       }
          else if (key=='#') {
           LatDeg=atoi(Clavier)*-1;
           Clavier[4]={0};
           compteur=0;
           lcd.clear();
       }
       }
      else if (LatDeg!=333 && LatMin==333 && LonDeg==333 && LonMin==333 && hauteurOeil==250) {
       
       
       lcd.home();
       lcd.print("point estime");
       lcd.setCursor(0,1);
       lcd.print("LatMin:");
       lcd.setCursor(9,1);
       
       
          if (key!=NO_KEY && key!='*' && key !='#') {
            Clavier[compteur++]=key;
            Clavier[compteur]='\0';
            lcd.setCursor(9,1);
            lcd.print(Clavier);
       }

          else if (key=='*') {
           LatMin=atoi(Clavier);
           Clavier[4]={0};
           compteur=0;
           lcd.clear();
       }

          else if (key=='#') {
           LatMin=atoi(Clavier)*-1;
           Clavier[4]={0};
           compteur=0;
           lcd.clear();
       }
  
      }
      else if (LatDeg!=333 && LatMin!=333 && LonDeg==333 && LonMin==333 && hauteurOeil==250) {

       
       lcd.home();
       lcd.print("point estime");
       lcd.setCursor(0,1);
       lcd.print("LonDeg:");
       

          if (key!=NO_KEY && key!='*' && key!='#') {
           Clavier[compteur++]=key;
           Clavier[compteur]='\0';
           lcd.setCursor(9,1);
           lcd.print(Clavier);
       }

          else if (key=='*') {
           LonDeg=atoi(Clavier);
           Clavier[4]=0;
           compteur=0;
           lcd.clear();
       }

          else if (key=='#') {
           LonDeg=atoi(Clavier)*-1;
           Clavier[4]=0;
           compteur=0;
           lcd.clear();
       }
      }

      else if (LatDeg!=333 && LatMin!=333 && LonDeg!=333 && LonMin==333 && hauteurOeil==250) {

       
       lcd.home();
       lcd.print("point estime");
       lcd.setCursor(0,1);
       lcd.print("LonMin:");
       

          if (key!=NO_KEY && key!='*' && key!='#') {
           Clavier[compteur++]=key;
           Clavier[compteur]='\0';
           lcd.setCursor(9,1);
           lcd.print(Clavier);
       }

          else if (key=='*') {
           LonMin=atoi(Clavier);
           Clavier[4]={0};
           compteur=0;
           lcd.clear();
       }

          else if (key=='#') {
           LonMin=atoi(Clavier)*-1;
           Clavier[4]={0};
           compteur=0;
           lcd.clear();
       }
      }

       else if (LatDeg!=333 && LatMin!=333 && LonDeg!=333 && LonMin!=333 && hauteurOeil==250) {
       
       lcd.home();
       lcd.print("hauteur oeil:");
       

           if (key!=NO_KEY && key!='*' && key!='#') {
             Clavier[compteur++]=key;
             Clavier[compteur]='\0';
             lcd.setCursor(0,1);
             lcd.print(Clavier);
             }

           else if (key=='*' || key=='#') {
              hauteurOeil=atoi(Clavier);
              Clavier[4]={0};
              compteur=0;
              lcd.clear();
             }
             
      
             }

             
       


      else if (hauteurOeil!=250 && LatDeg!=333 && LatMin!=333 && LonDeg!=333 && LonMin!=333) {

        
        
       
         lcd.home();
         lcd.print("angle lune:");
         lcd.setCursor(0,1);
         lcd.print(Tangage);
       
       
      if (key==NO_KEY) {
       
          if (Gite==constrain(Gite,0,2) ) {// facilite la précision de la visée. Si ça buzze c'est bon, on peut appuyer pour lancer la machine.
           tone(BUZZER,500);
       }
          else {
           noTone(BUZZER);
       }
          
          delay(100); 
      }
  
         
       else if (key!=NO_KEY) {
            noTone(BUZZER);
            lcd.clear();

            estimeLat=LatDeg+LatMin/60.00;
            estimeLon=LonDeg+LonMin/60.00;
            char astre='D';

           struct chiffres {// données à envoyer sous forme de variable structure.

             float Tangage;
             float estimeLat;
             float estimeLon;
             byte hauteurOeil;
             byte annee;
             byte numJourMois;
             byte mois;
             byte heure;
             byte minute;
             byte seconde;
             char astre;
             
          
        };

           chiffres data;

           data.Tangage=Tangage+TangageInit;
           data.estimeLat=estimeLat;
           data.estimeLon=estimeLon;
           data.hauteurOeil=hauteurOeil;
           data.annee=annee;
           data.numJourMois=numJourMois;
           data.mois=mois;
           data.heure=heure;
           data.minute=minute;
           data.seconde=seconde;
           data.astre=astre;
           

           while(Serial.available()>0) Serial.read();

        
           Serial.write((byte *)&data, sizeof data);

           Serial.flush();
        
       

          delay(3000);

           
          
          
          while (retourSerial.available()>0) {
  
          retourSerial.readBytes((byte *)&point, sizeof point);

          

          
          Intercept=point.Intercept;
          Azimut=point.Azimut;

          
          
          lcd.home();
          lcd.print("inter et azim:");
          lcd.setCursor(0,1);
          lcd.print(Intercept);
          delay(4000);
          lcd.home();
          lcd.print("caca chiant:");
          lcd.setCursor(0,1);
          lcd.print(Azimut);
          lcd.clear();
          estimeLat=0,estimeLon=0,LatDeg=333,LatMin=333,LonDeg=333,LonMin=333,hauteurOeil=250;
  }
      }

         
      }
       

       
          
      

   
      
         

       break;

       default:// affiche l'horloge UTC quand aucune touche du menu n'est pressée.

       noTone(BUZZER);
      lcd.home();
      lcd.print("date:");
      lcd.setCursor(7,0);
      lcd.print(numJourMois);
      lcd.setCursor(9,0);
      lcd.print("/");
      lcd.setCursor(11,0);
      lcd.print(mois);
      lcd.setCursor(13,0);
      lcd.print("/");
      lcd.setCursor(14,0);
      lcd.print(annee);
      lcd.setCursor(0,1);
      lcd.print("UTC:");
      lcd.setCursor(5,1);
      lcd.print(heure);
      lcd.setCursor(7,1);
      lcd.print("/");
      lcd.setCursor(8,1);
      lcd.print(minute);
      lcd.setCursor(10,1);
      lcd.print("/");
      lcd.setCursor(11,1);
      lcd.print(seconde);

    
      delay(100);
     

      break;
  }

      
      
  
}



byte bcdToDec(byte val){return((val/16*10)+(val%16));} //fonction qui change décimaux codés binaires en décimaux normaux.
