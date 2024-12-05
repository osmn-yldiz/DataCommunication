#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080         // Sunucunun dinlediði port numarasý
#define BUFFER_SIZE 1024  // Veri tamponu boyutu (Geçici Depolama)

int main() {
    int sock = 0;                           // Ýstemci soketi dosya tanýmlayýcýsý
    struct sockaddr_in server_address;      // Sunucu adres bilgilerini tutar
    char buffer[BUFFER_SIZE];               // Sunucudan gelen yanýt için tampon
    char message[BUFFER_SIZE];              // Sunucuya gönderilecek mesaj için tampon

    // Soket oluþtur
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { // socket fonksiyonu 3 parametre alýr. Af_INET IPv4 protokolünü belirtir. SOCK_STREAM, TCP protokolünü ifade eder. 
        perror("Socket creation error");    // Hata durumunda hata mesajý yazdýr
        exit(EXIT_FAILURE);                 // Programý sonlandýr
    }

    // Sunucu adresini ayarla
    server_address.sin_family = AF_INET;           // IPv4 adres ailesi
    server_address.sin_port = htons(PORT);         // Port numarasýný ayarla

    // IP adresini binary formata çevir ve sunucu adresine ata
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) { // inet_pton fonksiyonu, IP adresini metin biçiminden (örn. "127.0.0.1") binary (ikili) biçime dönüþtürür. // Dönüþ deðeri 1:Çeviri baþarýlý, 0: Geçersiz IP Adresi, -1: Hata
        perror("Invalid address");                 // Geçersiz adres hata mesajý
        exit(EXIT_FAILURE);
    }

    // Sunucuya baðlan
    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) { // sock: soketin dosya tanýmlayýcýsýdýr. Bu soket, sunucu ile baðlantýyý gerçekleþtirecektir.
        perror("Connection failed");               // Baðlantý hatasý durumunda
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server. Type your request ('I want tea', 'I want doner', 'I want beyti').\n");

    while (1) {
        memset(message, 0, BUFFER_SIZE);           // Gönderilecek mesajý sýfýrla
        printf("You: ");
        fgets(message, BUFFER_SIZE, stdin);        // Kullanýcýdan girdi al. fgets: kullanýcýdan mesaj alýnýyor ve bu mesaj sunucuya gönderiliyor.
        message[strcspn(message, "\n")] = '\0';    // Yeni satýr karakterini temizle

        // Mesajý sunucuya gönder
        send(sock, message, strlen(message), 0);

        // Sunucudan gelen yanýtý oku
        memset(buffer, 0, BUFFER_SIZE);            // Yanýt tamponunu sýfýrla
        int bytes_read = read(sock, buffer, BUFFER_SIZE); // Sunucudan veri oku
        if (bytes_read > 0) {
            printf("Server: %s\n", buffer);        // Sunucunun yanýtýný yazdýr
        }
    }

    close(sock); // Soketi kapat
    return 0;    // Programý sonlandýr
}
