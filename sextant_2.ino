#include <math.h>//importe des fonctions trigonométriques indispensables pour nos calculs astro.

struct chiffres {// données reçues par la nano mises en variables globales.

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

struct retour {
      int Intercept;
      int Azimut;
    };
retour point;    

float Tangage,estimeLat,estimeLon;
byte annee,numJourMois,mois,heure,minute,seconde,hauteurOeil;
char astre;
int Intercept,Azimut;


void setup() {
  Serial.begin(57600);//ouvre la voie série entre la nano et la due.
  Serial1.begin(57600);//ouvre la voie série entre la due et la nano.

  
}
void loop() {
  
   if (Serial.available()) {// lecture du buffer de réception si il est plein.
    
    Serial.readBytes((byte *)&data, sizeof data);// lecture des données qui vont être stockées dans les variables globales.

   Tangage=data.Tangage;
   estimeLat=data.estimeLat;
   estimeLon=data.estimeLon;
   hauteurOeil=data.hauteurOeil;
   annee=data.annee;
   numJourMois=data.numJourMois;
   mois=data.mois;
   heure=data.heure;
   minute=data.minute;
   seconde=data.seconde;
   astre=data.astre;
   
  
   }
   while(Serial.available()>0) Serial.read();//vidage du buffer.
  
   
   /*Serial.print(Tangage);
   Serial.print("  ");
   Serial.print(estimeLat);
   Serial.print("  ");
   Serial.print(estimeLon);
   Serial.print("  ");
   Serial.print(hauteurOeil);
   Serial.print("  ");
   Serial.print(annee);
   Serial.print("  ");
   Serial.print(numJourMois);
   Serial.print("  ");
   Serial.print(mois);
   Serial.print("  ");
   Serial.print(heure);
   Serial.print("  ");
   Serial.print(minute);
   Serial.print("  ");
   Serial.print(seconde);
   Serial.print("  ");
   Serial.print(astre);
   Serial.print("  ");
   Serial.print(Intercept);
   Serial.print("  ");
   Serial.print(Azimut);
   Serial.println();*/
   
  int an=2000+annee;
  int m=mois;

  if (mois<=2) {
  an-=1;
  m+=12;
  }
  else {
    an=an;
    m=m;
  }
  
  int a=int(an/100);
  int b=2-a+int(a/4);

  double jJulien=long(365.25*an)+long(30.6001*(m+1))+(numJourMois+heure/24.0+minute/(24.0*60.0)+seconde/(24.0*60.0*60.0))+1720994.5+b;// calcul jour julien.
  double jJulien00h=long(365.25*an)+long(30.6001*(m+1))+numJourMois+1720994.5+b;//calcul jour julien à 00hUTC(permet de calculer TSMG).

  double T=(jJulien-2415020.0)/36525.0;// temps en siècles juliens(variable principales des équations de Jean Meeus).
  double T00h=(jJulien00h-2415020)/36525;//temps en siecles juliens à 00hUTC(permet de calculer TSMG).
  

  

    
    
  
  
  if (astre=='C') {
    
   
  double A=radians(153.23+22518.7541*T);//correction venus.
  double B=radians(216.57+45037.5082*T);//correction venus.
  double CD=radians(312.69+32964.3577*T);//correction jupiter.
  double D=radians(350.74+445267.1142*T-0.00144*sq(T));//correction lune.
  double E=radians(231.19+20.20*T);//inegalite de longue periode.
  double H=radians(353.40+65928.7155*T);//taille du slip de Mike Brant.

  double L=279.69668+36000.76892*T+0.0003025*sq(T)+0.00134*cos(A)+0.00154*cos(B)+0.00200*cos(CD)+0.00178*sin(E)+0.00179*sin(D);//longitude moyenne du soleil.

  double L1=L-long(L)+long(L)%360;

  
  double M=358.47583+35999.04975*T-0.000150*sq(T)-0.0000033*pow(T,3);//anomalie moyenne du soleil.

  double M1=M-long(M)+long(M)%360;

  
  double e=0.01675104-0.0000418*T-0.000000126*sq(T);//excentricité de l'orbite terrestre.
  
  double C1=(1.919460-0.004789*T-0.000014*sq(T))*sin(radians(M1));//équation du centre du soleil.

  double C2=(0.020094-0.0001*T)*sin(radians(M1));

  double C3=0.000293*sin(3*radians(M1));

  double C=C1+C2+C3;

  double lonVraie=L1+C;//longitude vraie du soleil.

  double v=M1+C;//anomalie vraie du soleil.

  double R=((1.0000002*(1-sq(e)))/(1+e*cos(radians(v)))+0.00000543*sin(A)+0.00001575*sin(B)+0.00001627*sin(CD)+0.00003076*cos(D)+0.00000927*sin(H));//rayon vecteur du soleil en UA.

  double obliquite=23.452294-0.0130125*T-0.00000164*sq(T)+0.000000503*pow(T,3);//obliquité de l'écliptique.

  double declinaison=degrees(asin(sin(radians(obliquite))*sin(radians(lonVraie))));

  double ascensionDroite=degrees(atan2(cos(radians(obliquite))*sin(radians(lonVraie)),cos(radians(lonVraie))));

  if (ascensionDroite<0) {
    ascensionDroite+=360;
  }
  else {
    ascensionDroite+=0;
  }

  double refraction=0;//calcul de correction de la réfraction atmosphérique selon Jean Meeus.

  if (Tangage>15.0) {
       refraction+=(58.294*tan(radians(90-Tangage))-0.0668*pow(tan(radians(90-Tangage)),3))/3600;
    }

    else if (Tangage>=0.0 && Tangage<4.0) {
       refraction+=(sq(6.6176-1.9037*log(Tangage+1.4)))/60;
    }

    else if (Tangage>=4.0 && Tangage<=15.0) {
       refraction+=(sq(4.3481-0.9466*log(Tangage-1.4)))/60;
    }

    double Oeil=(1.76/sqrt(hauteurOeil))/60;//correction de la hauteur d'oeil.

    
    double tempsSidGreen=0.276919398+100.0021359*T00h+0.000001075*sq(T00h);//temps sidéral à greenwich en révolutions.
    double tempsSidGreenHeures=(tempsSidGreen-long(tempsSidGreen))*24;//temps sidéral à greenwich en heures.

    double instantUT=(heure+minute/60.0+seconde/3600.0)*1.002737908;
    double TSMG1=instantUT+tempsSidGreenHeures;//temps sidéral à greenwich à n'importe quelle heure UT.
    double TSMG=TSMG1-long(TSMG1)+long(TSMG1)%24;

    double Diametre=degrees(atan(695510.0/(R*150000000.0)));// calcul de l'erreur de demi-diamètre(inutile pour notre système de visée au final).

    double Parallaxe=(6371/(R*150000000.0))*cos(radians(Tangage-Oeil));

    double AHL1=(TSMG*15+360)-ascensionDroite+estimeLon;
    double AHL=AHL1-long(AHL1)+long(AHL1)%360;

    
    double hauteurCalculee=degrees(asin(sin(radians(estimeLat))*sin(radians(declinaison))+cos(radians(estimeLat))*cos(radians(declinaison))*cos(radians(AHL))));

    double Azimut=0;

    if (AHL>=180 && AHL<360) {
       Azimut+=degrees(acos((sin(radians(declinaison))-sin(radians(estimeLat))*sin(radians(hauteurCalculee)))/(cos(radians(estimeLat))*cos(radians(hauteurCalculee)))));
    }
    else {
       Azimut+=360-degrees(acos((sin(radians(declinaison))-sin(radians(estimeLat))*sin(radians(hauteurCalculee)))/(cos(radians(estimeLat))*cos(radians(hauteurCalculee)))));
    }

    double hauteurVraie=Tangage-refraction-Oeil+Parallaxe;// la correction de demi-diamètre est enlevée car là on vise le milieu de l'astre.
    
    double Intercept=(hauteurVraie-hauteurCalculee)*60;

    
    
   

    

    point.Intercept=int(round(Intercept));
    point.Azimut=int(round(Azimut));

    

   
    

    
   Serial.print(point.Intercept);
   Serial.print("  ");
   Serial.print(point.Azimut);
   Serial.println();

    
    
    Serial1.write((byte *)&point,sizeof point);
    
    
    Serial1.flush();//cette instruction permet d'envoyer toutes les infos avant de passer à la suitre du programme.

    
  
    delay(500);
   
    



  }

  else if (astre=='D') {

    
  
  double omega1=259.183275-1934.1420*T+0.002078*sq(T)+0.0000022*pow(T,3);//longitude du noeud ascendant de la lune.

  double omega=omega1-long(omega1)+long(omega1)%360;

  double Lprim1=270.434164+481267.8831*T-0.001133*sq(T)+0.0000019*pow(T,3)+0.000233*sin(radians(51.2+20.2*T))+0.003964*sin(radians(346.56+132.87*T-0.0091731*sq(T)))+0.001964*sin(radians(omega));//longitude moyenne de la lune.

  double Lprim=Lprim1-long(Lprim1)+long(Lprim1)%360;
  
  double M1=358.475833+35999.0498*T-0.000150*sq(T)-0.0000033*pow(T,3)-0.001778*sin(radians(51.2+20.2*T));//anomalie moyenne du soleil.

  double M=M1-long(M1)+long(M1)%360;

  double Mprim1=296.104608+477198.8491*T+0.009192*sq(T)+0.0000144*pow(T,3)+0.000817*sin(radians(51.2+20.2*T))+0.003964*sin(radians(346.56+132.87*T-0.0091731*sq(T)))+0.002541*sin(radians(omega));//anomalie moyenne de la lune.

  double Mprim=Mprim1-long(Mprim1)+long(Mprim1)%360;

  double D1=350.737486+445267.1142*T-0.001436*sq(T)+0.0000019*pow(T,3)+0.002011*sin(radians(51.2+20.2*T))+0.001964*sin(radians(omega))+0.003964*sin(radians(346.56+132.87*T-0.0091731*sq(T)));//élongation moyenne de la lune.

  double D=D1-long(D1)+long(D1)%360;

  double F1=11.250889+483202.0251*T-0.003211*sq(T)-0.0000003*pow(T,3)+0.003964*sin(radians(346.56+132.87*T-0.0091731*sq(T)))-0.024691*sin(radians(omega))-0.004328*sin(radians(omega+275.05-2.3*T));//distance moyenne de la lune à son noeud ascendant.

  double F=F1-long(F1)+long(F1)%360;
  
  double e=1-0.002495*T-0.00000752*sq(T);

  double teta=Lprim+6.288750*sin(radians(Mprim))+1.274018*sin(radians(2*D-Mprim))+0.658309*sin(radians(2*D))+0.213616*sin(radians(2*Mprim))-0.185596*sin(radians(M))*e-0.114336*sin(radians(2*F))+0.058793*sin(radians(2*D-2*Mprim))+0.057212*sin(radians(2*D-M-Mprim))*e+0.053320*sin(radians(2*D+Mprim))+0.045874*sin(radians(2*D-M))*e+
  0.041024*sin(radians(Mprim-M))*e-0.034718*sin(radians(D))-0.030465*sin(radians(M+Mprim))*e+0.015326*sin(radians(2*D-2*F))-0.012528*sin(radians(2*F+Mprim))-0.010980*sin(radians(2*F-Mprim))+0.010674*sin(radians(4*D-Mprim))+0.010034*sin(radians(3*Mprim))+0.008548*sin(radians(4*D-2*Mprim))-0.007910*sin(radians(M-Mprim+2*D))*e-
  0.006783*sin(radians(2*D+M))*e+0.005162*sin(radians(Mprim-D))+0.005*sin(radians(M+D))*e+0.004049*sin(radians(Mprim-M+2*D))*e+0.003996*sin(radians(2*Mprim+2*D))+0.003862*sin(radians(4*D))+0.003665*sin(radians(2*D-3*Mprim))+0.002695*sin(radians(2*Mprim-M))*e+0.002602*sin(radians(Mprim-2*F-2*D))+0.002396*sin(radians(2*D-M-2*Mprim))*e
  -0.002349*sin(radians(Mprim+D))+0.002249*sin(radians(2*D-2*M))*sq(e)-0.002125*sin(radians(2*Mprim+M))*e-0.002079*sin(radians(2*M))*sq(e)+0.002059*sin(radians(2*D-Mprim-2*M))*sq(e)-0.001773*sin(radians(Mprim+2*D-2*F))-0.001595*sin(radians(2*F+2*D))+0.001220*sin(radians(4*D-M-Mprim))*e-0.001110*sin(radians(2*Mprim+2*F))
  +0.000892*sin(radians(Mprim-3*D))-0.000811*sin(radians(M+Mprim+2*D))*e+0.000761*sin(radians(4*D-M-2*Mprim))*e+0.000717*sin(radians(Mprim-2*M))*sq(e)+0.000704*sin(radians(Mprim-2*M-2*D))*sq(e)+0.000693*sin(radians(M-2*Mprim+2*D))*e+0.000598*sin(radians(2*D-M-2*F))*e+0.000550*sin(radians(Mprim+4*D))
  +0.000538*sin(radians(4*Mprim))+0.000521*sin(radians(4*D-M))*e+0.000486*sin(radians(2*Mprim-D));//longitude écliptique de la lune.

  double B=5.128189*sin(radians(F))+0.280606*sin(radians(Mprim+F))+0.277693*sin(radians(Mprim-F))+0.173238*sin(radians(2*D-F))+0.055413*sin(radians(2*D+F-Mprim))+0.046272*sin(radians(2*D-F-Mprim))+0.032573*sin(radians(2*D+F))+0.017198*sin(radians(2*Mprim+F))+0.009267*sin(radians(2*D+Mprim-F))+0.008823*sin(radians(2*Mprim-F))
  +0.008247*sin(radians(2*D-M-F))*e+0.004323*sin(radians(2*D-F-2*Mprim))+0.004200*sin(radians(2*D+F+Mprim))+0.003372*sin(radians(F-M-2*D))*e+0.002472*sin(radians(2*D+F-M-Mprim))*e+0.002222*sin(radians(2*D+F-M))*e+0.002072*sin(radians(2*D-F-M-Mprim))*e+0.001877*sin(radians(F-M+Mprim))*e+0.001828*sin(radians(4*D-F-Mprim))
  -0.001803*sin(radians(F+M))*e-0.001750*sin(radians(3*F))+0.001570*sin(radians(Mprim-M-F))*e-0.001487*sin(radians(F+D))-0.001481*sin(radians(F+M+Mprim))*e+0.001417*sin(radians(F-M-Mprim))*e+0.001350*sin(radians(F-M))*e+0.001330*sin(radians(F-D))+0.001106*sin(radians(F+3*Mprim))+0.001020*sin(radians(4*D-F))+0.000833*sin(radians(F+4*D-Mprim))
  +0.000781*sin(radians(Mprim-3*F))+0.000670*sin(radians(F+4*D-2*Mprim))+0.000606*sin(radians(2*D-3*F))+0.000597*sin(radians(2*D+2*Mprim-F))+0.000492*sin(radians(2*D+Mprim-M-F))*e+0.000450*sin(radians(2*Mprim-F-2*D))+0.000439*sin(radians(3*Mprim-F))+0.000423*sin(radians(F+2*D+2*Mprim))+0.000422*sin(radians(2*D-F-3*Mprim))
  -0.000367*sin(radians(M+F+2*D-Mprim))*e-0.000353*sin(radians(M+F+2*D))*e+0.000331*sin(radians(F+4*D))+0.000317*sin(radians(2*D+F-M+Mprim))*e+0.000306*sin(radians(2*D-2*M-F))*sq(e)-0.000283*sin(radians(Mprim+3*F));

  double W1=0.0004664*cos(radians(omega));
  double W2=0.0000754*cos(radians(omega+275.05-2.30*T));

  double beta=B*(1-W1-W2);//latitude écliptique de la lune.

  double Pi=0.950724+0.051818*cos(radians(Mprim))+0.009531*cos(radians(2*D-Mprim))+0.007843*cos(radians(2*D))+0.002824*cos(radians(2*Mprim))+0.000857*cos(radians(2*D+Mprim))+0.000533*cos(radians(2*D-M))*e+0.000401*cos(radians(2*D-M-Mprim))*e+0.000320*cos(radians(Mprim-M))*e-0.000271*cos(radians(D))-0.000264*cos(radians(M+Mprim))*e
  -0.000198*cos(radians(2*F-Mprim))+0.000173*cos(radians(3*Mprim))+0.000167*cos(radians(4*D-Mprim))-0.000111*cos(radians(M))*e+0.000103*cos(radians(4*D-2*Mprim))-0.000084*cos(radians(2*Mprim-2*D))-0.000083*cos(radians(2*D+M))*e+0.000079*cos(radians(2*D+2*Mprim))+0.000072*cos(radians(4*D))+0.000064*cos(radians(2*D-M+Mprim))*e
  -0.000063*cos(radians(2*D+M-Mprim))*e+0.000041*cos(radians(M+D))*e+0.000035*cos(radians(2*Mprim-M))*e-0.000033*cos(radians(3*Mprim-2*D))-0.000030*cos(radians(Mprim+D))-0.000029*cos(radians(2*F-2*D))-0.000029*cos(radians(2*Mprim+M))*e+0.000026*cos(radians(2*D-2*M))*sq(e)-0.000023*cos(radians(2*F-2*D+Mprim))
  +0.000019*cos(radians(4*D-M-Mprim))*e;//parallaxe de la lune.

  double R=(6378.14/sin(radians(Pi)));//distance entre le centre de la terre et de la lune en km.

  double obliquite=23.452294-0.0130125*T-0.00000164*sq(T)+0.000000503*pow(T,3);//obliquité de l'écliptique.


  
  
  double ascensionDroite=degrees(atan2((sin(radians(teta))*cos(radians(obliquite))-tan(radians(beta))*sin(radians(obliquite))),cos(radians(teta))));//ascension droite de la lune.

  if (ascensionDroite<0) {
    ascensionDroite+=360;
  }
  else {
    ascensionDroite+=0;
  }
  
  double declinaison=degrees(asin(sin(radians(beta))*cos(radians(obliquite))+cos(radians(beta))*sin(radians(obliquite))*sin(radians(teta))));//declinaison de la lune.

   

  double refraction=0;

  if (Tangage>15.0) {
       refraction+=(58.294*tan(radians(90-Tangage))-0.0668*pow(tan(radians(90-Tangage)),3))/3600;
    }

    else if (Tangage>=0.0 && Tangage<4.0) {
       refraction+=(sq(6.6176-1.9037*log(Tangage+1.4)))/60;
    }

    else if (Tangage>=4.0 && Tangage<=15.0) {
       refraction+=(sq(4.3481-0.9466*log(Tangage-1.4)))/60;
    }

    double Oeil=(1.76/sqrt(hauteurOeil))/60;

    
    double tempsSidGreen=0.276919398+100.0021359*T00h+0.000001075*sq(T00h);//temps sidéral à greenwich en révolutions.
    double tempsSidGreenHeures=(tempsSidGreen-long(tempsSidGreen))*24;//temps sidéral à greenwich en heures.

    double instantUT=(heure+minute/60.0+seconde/3600.0)*1.002737908;
    double TSMG1=instantUT+tempsSidGreenHeures;//temps sidéral à greenwich à n'importe quelle heure UT.
    double TSMG=TSMG1-long(TSMG1)+long(TSMG1)%24;

    double Diametre=degrees(atan(1750/R));//inutile pour notre système de visée au final.

    double Parallaxe=(6371/R)*cos(radians(Tangage-Oeil));

    double AHL1=(TSMG*15+360)-ascensionDroite+estimeLon;
    double AHL=AHL1-long(AHL1)+long(AHL1)%360;

    
    double hauteurCalculee=degrees(asin(sin(radians(estimeLat))*sin(radians(declinaison))+cos(radians(estimeLat))*cos(radians(declinaison))*cos(radians(AHL))));

    double Azimut=0;

    if (AHL>=180 && AHL<360) {
       Azimut+=degrees(acos((sin(radians(declinaison))-sin(radians(estimeLat))*sin(radians(hauteurCalculee)))/(cos(radians(estimeLat))*cos(radians(hauteurCalculee)))));
    }
    else {
       Azimut+=360-degrees(acos((sin(radians(declinaison))-sin(radians(estimeLat))*sin(radians(hauteurCalculee)))/(cos(radians(estimeLat))*cos(radians(hauteurCalculee)))));
    }

    double hauteurVraie=Tangage-refraction-Oeil+Parallaxe;
    
    double Intercept=(hauteurVraie-hauteurCalculee)*60;
  

    point.Intercept=int(round(Intercept));
    point.Azimut=int(round(Azimut));

     
   
    Serial1.write((byte *)&point,sizeof point);

    Serial1.flush();

    while(Serial1.available()) Serial1.read();
    
    
  
  


  

  }


  
  retour point={0,0};

  chiffres data={0,0,0,0,0,0,0,0,0,0,0};

  Tangage=0,estimeLat=0,estimeLon=0,hauteurOeil=0,numJourMois=0,mois=0,heure=0,minute=0,seconde=0,astre=0,Azimut=0,Intercept=0,annee=0;
  


  }
