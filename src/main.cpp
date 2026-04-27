#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <LittleFS.h>

// Configuration du Wi-Fi Troll
const char* ssid = "Helha Guest 5GHz"; // Nom du réseau Wi-Fi

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
WebServer server(80);

void setup() {
  Serial.begin(115200);
  
  // 1. Initialisation du système de fichiers (LittleFS)
  if(!LittleFS.begin(true)){
    Serial.println("Erreur: Impossible de monter LittleFS");
    return;
  }

  // 2. Configuration du Point d'Accès Wi-Fi
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);
  Serial.println("Wi-Fi Troll activé !");

  // 3. Démarrage du serveur DNS (Redirige TOUT vers l'IP de l'ESP32)
  dnsServer.start(DNS_PORT, "*", apIP);

  // 4. Configuration des routes du Serveur Web
  
  // Route pour la page principale
  server.on("/", []() {
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  // Route pour charger l'image
  server.on("/troll.jpg", []() {
    File file = LittleFS.open("/troll.jpg", "r");
    server.streamFile(file, "image/jpeg");
    file.close();
  });

  // --- LIGNES SPÉCIALES POUR FORCER LA NOTIFICATION ---

  // Pour tromper spécifiquement les appareils Apple (iOS / Mac)
  server.on("/hotspot-detect.html", []() {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
  });

  // Pour tromper spécifiquement les appareils Android / Windows
  server.on("/generate_204", []() {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
  });

  // Additional detection URLs for better compatibility
  server.on("/ncsi.txt", []() {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
  });

  server.on("/success.txt", []() {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
  });

  server.on("/connecttest.txt", []() {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
  });

  server.on("/library/test/success.html", []() {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
  });

  // LE SECRET DU PORTAIL CAPTIF : Le "Catch-all"
  // Si le téléphone cherche n'importe quelle autre page (ex: captive.apple.com),
  // on le redirige de force vers notre page d'accueil (IP de l'ESP32)
  server.onNotFound([]() {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
  });

  server.begin();
}

void loop() {
  // Il faut faire tourner ces deux fonctions en boucle
  dnsServer.processNextRequest();
  server.handleClient();
}