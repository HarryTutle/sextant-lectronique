# sextant-lectronique
fabriquer un sextant électronique avec arduino.

Avec deux cartes arduino et un gyrocompas électronique, permet d'obtenir l'azimut et l'intercept donc sa position par rapport à la Lune et le Soleil. Le sextant peut aussi faire des relèvements via un compas électronique intégré, et simplement indiquer les angles de roulis et de tangage avec le gyro.
La première carte, une arduino uno simple, permet de capter les angles du gyro, l'heure via une horloge externe et le cap via le compas. Elle envoie par liaison série les infos à une seconde carte arduino duo (il faut une duo, car capable de faire des calculs sur des variables double, indispensable pour les équations du mouvement de la Lune). Cette dernière fait les calculs pour fournir azimut et intercept.

Le sextant est intégré dans un boitier, avec un viseur de lunette astro pour faire une visée précise. Les formules utilisées pour programmer la arduino duo proviennent de l'ouvrage de Jean Meeus "calculs astronomiques pour amateurs".

Actuellement le sextant fonctionne assez bien avec le Soleil (précision de 30 km environ) mais pas terrible pour la Lune (environ 120 km de précision). Cela peut s'expliquer peut-être par le nombre élevé de calculs à faire pour les mouvements de la Lune, et donc un cumul des erreurs plus conséquent.

Circuit et montage:

![circuit-electronique-du-sextant-modif1-1024x907](https://github.com/HarryTutle/sextant-lectronique/assets/82940602/d82fb0de-2a6a-4f24-8a50-8bd7ec44afd6)


Quelques photos:

![montage1-1024x576](https://github.com/HarryTutle/sextant-lectronique/assets/82940602/48b0adb8-8985-4145-b0a2-3da68d377287)

![montage9-1024x576](https://github.com/HarryTutle/sextant-lectronique/assets/82940602/8fd73f2e-ce66-4f09-aa58-df04967e3123)

