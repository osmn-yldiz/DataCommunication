#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080         // Sunucunun dinledi�i port numaras�
#define BUFFER_SIZE 1024  // Veri tamponu boyutu (Ge�ici Depolama)

int main() {
    int sock = 0;                           // �stemci soketi dosya tan�mlay�c�s�
    struct sockaddr_in server_address;      // Sunucu adres bilgilerini tutar
    char buffer[BUFFER_SIZE];               // Sunucudan gelen yan�t i�in tampon
    char message[BUFFER_SIZE];              // Sunucuya g�nderilecek mesaj i�in tampon

    // Soket olu�tur
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { // socket fonksiyonu 3 parametre al�r. Af_INET IPv4 protokol�n� belirtir. SOCK_STREAM, TCP protokol�n� ifade eder. 
        perror("Socket creation error");    // Hata durumunda hata mesaj� yazd�r
        exit(EXIT_FAILURE);                 // Program� sonland�r
    }

    // Sunucu adresini ayarla
    server_address.sin_family = AF_INET;           // IPv4 adres ailesi
    server_address.sin_port = htons(PORT);         // Port numaras�n� ayarla

    // IP adresini binary formata �evir ve sunucu adresine ata
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) { // inet_pton fonksiyonu, IP adresini metin bi�iminden (�rn. "127.0.0.1") binary (ikili) bi�ime d�n��t�r�r. // D�n�� de�eri 1:�eviri ba�ar�l�, 0: Ge�ersiz IP Adresi, -1: Hata
        perror("Invalid address");                 // Ge�ersiz adres hata mesaj�
        exit(EXIT_FAILURE);
    }

    // Sunucuya ba�lan
    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) { // sock: soketin dosya tan�mlay�c�s�d�r. Bu soket, sunucu ile ba�lant�y� ger�ekle�tirecektir.
        perror("Connection failed");               // Ba�lant� hatas� durumunda
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server. Type your request ('I want tea', 'I want doner', 'I want beyti').\n");

    while (1) {
        memset(message, 0, BUFFER_SIZE);           // G�nderilecek mesaj� s�f�rla
        printf("You: ");
        fgets(message, BUFFER_SIZE, stdin);        // Kullan�c�dan girdi al. fgets: kullan�c�dan mesaj al�n�yor ve bu mesaj sunucuya g�nderiliyor.
        message[strcspn(message, "\n")] = '\0';    // Yeni sat�r karakterini temizle

        // Mesaj� sunucuya g�nder
        send(sock, message, strlen(message), 0);

        // Sunucudan gelen yan�t� oku
        memset(buffer, 0, BUFFER_SIZE);            // Yan�t tamponunu s�f�rla
        int bytes_read = read(sock, buffer, BUFFER_SIZE); // Sunucudan veri oku
        if (bytes_read > 0) {
            printf("Server: %s\n", buffer);        // Sunucunun yan�t�n� yazd�r
        }
    }

    close(sock); // Soketi kapat
    return 0;    // Program� sonland�r
}
