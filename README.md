# Victron-Energy-Monitor-WaveShare
A Waveshare ESP32 S3 touch LCD 5B turned into a monitor screen for Victron devices (BMV712, Orion XS50, IP22, SmartSolar)

Ce projet permet d’utiliser une carte Waveshare ESP32 S3 Touch LCD 5B comme répéteur de plusieurs appareils Victron par liaison BlueTooth.
J’avais envie de regrouper sur un seul écran les données principales des quatre appareils Victron du bord :
-	BMV 712
-	Orion XS 50
-	Smart Solar 100/30
-	Chargeur IP22 12/30
Je trouve cela plus pratique que les petits afficheurs ronds Victron et je n’ai pas toujours envie d’avoir mon téléphone à la main, surtout en navigation pour consulter l’App Victron Connect, même si elle est très aboutie.
 ![Description de l'image]([IMAGES/IMG_20251203_140255.jpg)

LA CARTE WAVESHARE :
C’est une carte « All-In-One ». Ses avantages c’est qu’elle est plus grande que les afficheurs Victron : 5 pouces 1024*600, tactile, facile à brancher car elle a une entré pour l’alimentation de 7-36V et elle communique en Bluetooth avec les appareils Victron. Elle possède une  I/O CAN potentiellement interfaçable avec le réseau NMEA2000 du bord. Son prix est raisonnable, moins de 50€.
En savoir plus : https://www.waveshare.com/esp32-s3-touch-lcd-5.htm

LOGICIEL :
Le code a été développé avec l’IDE Arduino 2.3.6 avec l’aide d’Antropic Claude Sonnet Opus 4.5 Pro.
C’est du code C++ et LVGL ( https://lvgl.io/ )
Dépendances : Bibliothèques
-	LVGL 8.4.0
-	ESP32_Display_Panel 1.04
-	ESP32_IO_Expander 1.1.1
-	Esp-lib-utils 0.3.0

Paramétrage LVGL : Le fichier de paramétrage de LVGL « lv_conf.h » doit avoir été correctement paramétré par rapport à la carte utilisée. S’en assurer en testant les programmes « examples » de la library LVGL.
Dans ce fichier, ne pas oublier de sélectionner les polices de caractères utilisées par le projet (Paragraphe « Font usage »), Sélectionner également les paramètres suivants en particulier pour obtenir des couleurs en dégradées pour le fond d’écran.

PERSONNALISATION :
Des éléments doivent être impérativement personnalisés dans le code (Chapitre : « Personnalisation » sans le code) en fonction du bateau, de la batterie de servitude, et surtout des caractéristiques des appareils Victron :
-	Dans le fichier principal WS_IA_V8.XX_RXXX.INO :
o	Nom du bateau
-	Dans le fichier acquisition_BP.cpp :
o	Capacité du parc de batteries gérée par le BMV712 (A)
o	Puissance du panneau solaire (Watt)
o	Puissance alternateur (A)
o	Puissance chargeur de quai (A)
o	Adresse MAC de chaque appareil Victron
o	Clé de cryptage de chaque appareil Victron.
L’adresse MAC et la clé de cryptage de chaque appareil doivent être obtenue grâce à l’application VictronConnect disponible pour Android ou IOS. Dans l’application, pour chaque appareil, cliquer sur l’icône engrenage /  Menu 3 points / Infos produit /  Vérifier que le Bluetooth est activer  (Ne JAMAIS désactiver le Bluetooth, cela peut être irréversible). En bas de l’écran cliquez sur Données de Cryptage, sur l’écran suivant noter l’adresse MAC et la clé de cryptage. 
Ces deux informations devront être reportées dans le code source avec le format requis, par exemple :
-	6 octets de l’adresse MAC : f3 :81… (lettres en minuscules)
-	16 octets de la clé de cryptage : 0x4B, 0x05…( Lettres en majuscules)
Avertissement : La clé de cryptage d’un appareil Victron change si vous changez son code PIN. Si vous changez donc le code PIN d’un de vos appareils pour sécuriser son accès Bluetooth vous devrez obligatoirement modifier la clé de cryptage dans le code et le recompiler, sinon la carte Waveshare ne reconnaitra pas cet appareil.

IMAGES :
Les images de l’écran de splash ont été générées par Microsoft Copilot en « .PNG » et transformées en « .c » par le convertisseur LVGL.
https://lvgl.io/tools/imageconverter
-	Paramètres de conversion :
o	LVGL V8
o	Format : CF_TRUE_COLOR
o	C array
o	Dither images
L’image de fond d’écran du splash est 3 fois plus petite que l’écran, 341*200 au lieu de 1024*600. LVGL la zoome 3 fois pour que l’affichage soit plein écran. Le petit logo Victron est affiché à sa taille native.

REMARQUES SUR LE FONCTIONNEMENT :
-	Le rafraichissement des données est d’environ toutes les secondes. Excepté pour l’IP22 de notre bateau qui n’envoie les données que toutes les 20-30s. Je ne suis pas encore arrivé à éclaircir ce mystère. Peut-être qu’une prochaine version avec BLE asynchrone et mutex aura de meilleures performances
-	Le point rouge dans le coin de chaque jauge indique la réception de nouvelles données. Si l’appareil n’émet plus après 15s le voyant s’éteint (1mn pour l’IP22)
-	La production affichée en bas à gauche est la production journalière. Cette donnée est issue du Smart Solar
-	Le Status est le status détecté par le Smart Solar (Bulk, Absoption..)
-	Le TTG (Time To Go) est la durée estimée de l’autonomie de la batterie. Si Production > Consommation le TTG devient « Infini »
-	Les Ampères affichés en bas à droite sont issus du BMV712, c’est donc le bilan de ce qui est consommé et ce qui est produit. Si l’ampérage devient négatif (Consommation > Production) l’affichage de la valeur devient orange
-	Full indique dans combien de temps la batterie sera pleine en fonction de la production et de la consommation. Si Consommation > Production alors Full affiche «---« 








