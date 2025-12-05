# Victron-Energy-Monitor-WaveShare
A Waveshare ESP32 S3 touch LCD 5B turned into a monitor screen for Victron devices (BMV712, Orion XS50, IP22, SmartSolar)

Ce projet permet d’utiliser une carte Waveshare ESP32 S3 Touch LCD 5B comme répéteur de plusieurs appareils Victron par liaison BlueTooth.
J’avais envie de regrouper sur un seul écran les données principales des quatre appareils Victron du bord :
-	BMV 712
-	Orion XS 50
-	Smart Solar 100/30
-	Chargeur IP22 12/30
Je trouve cela plus pratique que les petits afficheurs ronds Victron et je n’ai pas toujours envie d’avoir mon téléphone à la main, surtout en navigation.

La carte Waveshare :
C’est une carte « All-In-One ». Ses avantages c’est qu’elle est plus grande que les afficheurs Victron : 5 pouces 1024*600, tactile, facile à brancher car elle a une entré pour l’alimentation de 7-36V et elle communique en Bluetooth avec les appareils Victron. Elle possède une  I/O CAN potentiellement interfaçable avec le réseau NMEA2000 du bord. Son prix est raisonnable, moins de 50€.
En savoir plus : https://www.waveshare.com/esp32-s3-touch-lcd-5.htm
