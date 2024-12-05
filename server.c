#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>

// Sabitler
#define PORT 8080         // Sunucunun dinleyeceði port numarasý
#define BUFFER_SIZE 1024  // Veri okuma ve yazma için tampon boyutu

// Global sayaçlar
int client_count = 0;        // Baðlý olan toplam istemci sayýsý
int tea_count = 0;           // Satýlan çay sayýsý
int doner_count = 0;         // Satýlan döner sayýsý
int beyti_count = 0;         // Satýlan beyti sayýsý

// Mutex (kilit), sayaçlara thread-safe eriþim saðlamak için
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // karþýlýklý dýþlama kilidi oluþturmak için kullanýlan bir veri türüdür.

// Ýstemci baðlantýsýný iþlemek için kullanýlacak thread fonksiyonu
void* handle_client(void* client_socket) {
    int sock = *(int*)client_socket;  // Gelen soket dosya tanýmlayýcýsýný al
    free(client_socket);              // Bellek sýzýntýsýný önlemek için soket pointer'ýný serbest býrak

    // Ýstemci sayýsýný artýr
    pthread_mutex_lock(&mutex);       // Mutex kilidi al
    client_count++;
    pthread_mutex_unlock(&mutex);     // Mutex kilidi býrak

    char buffer[BUFFER_SIZE];         // Ýstemciden gelen mesajlarý tutacak tampon
    char response[BUFFER_SIZE];       // Sunucudan istemciye dönecek yanýt

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);              // Tamponu sýfýrla
        int bytes_read = read(sock, buffer, BUFFER_SIZE); // Ýstemciden veri oku
        if (bytes_read <= 0) {                       // Baðlantý kesilirse döngüden çýk
            printf("Client disconnected.\n");
            break;
        }

        printf("Client: %s\n", buffer); // Ýstemciden gelen mesajý ekrana yazdýr

        pthread_mutex_lock(&mutex);    // Mutex kilidi al

        // Gelen mesajlarý iþleyerek uygun yanýt oluþtur
        if (strcmp(buffer, "I want tea") == 0) {
            tea_count++;               // Çay sayýsýný artýr
            strcpy(response, "Here is your tea");
        } else if (strcmp(buffer, "I want doner") == 0) {
            doner_count++;             // Döner sayýsýný artýr
            strcpy(response, "Here is your doner");
        } else if (strcmp(buffer, "I want beyti") == 0) {
            beyti_count++;             // Beyti sayýsýný artýr
            strcpy(response, "Here is your beyti");
        } else if (strcmp(buffer, "How many tea have been sold?") == 0) {
            snprintf(response, BUFFER_SIZE, "%d tea have been sold so far.", tea_count);
        } else if (strcmp(buffer, "How many doner have been sold?") == 0) {
            snprintf(response, BUFFER_SIZE, "%d doner have been sold so far.", doner_count);
        } else if (strcmp(buffer, "How many beyti have been sold?") == 0) {
            snprintf(response, BUFFER_SIZE, "%d beyti have been sold so far.", beyti_count);
        } else if (strcmp(buffer, "How many clients are connected?") == 0) {
            snprintf(response, BUFFER_SIZE, "We currently have %d clients.", client_count);
        } else {
            strcpy(response, "Invalid request"); // Geçersiz bir istek geldiðinde
        }

        pthread_mutex_unlock(&mutex);  // Mutex kilidi býrak

        // Ýstemciye yanýt gönder
        send(sock, response, strlen(response), 0);
    }

    // Ýstemci baðlantýsý kesildiðinde istemci sayýsýný azalt
    pthread_mutex_lock(&mutex);
    client_count--;
    pthread_mutex_unlock(&mutex);

    close(sock);              // Soket baðlantýsýný kapat
    pthread_exit(NULL);       // Thread'i sonlandýr
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Sunucu soketini oluþtur
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {                     // Hata kontrolü
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Soket seçeneklerini ayarla
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // setsockopt fonksiyonu, bir soketin çalýþma davranýþýný deðiþtirmek için kullanýlan bir sistem çaðrýsýdýr.

    // Soketi belirli bir porta baðla
    address.sin_family = AF_INET;            // IPv4 adres ailesi
    address.sin_addr.s_addr = INADDR_ANY;    // Tüm að arayüzlerini dinle
    address.sin_port = htons(PORT);          // PORT numarasýný atanýr
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) { // bind sistemi çaðrýsý, soketi belirli bir IP adresine ve port numarasýna baðlar.
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Gelen baðlantýlarý dinle
    if (listen(server_fd, 3) < 0) {          // Maksimum 3 baðlantý kuyruðu
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        // Yeni bir istemci baðlantýsýný kabul et
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        printf("New client connected.\n");

        // Her istemci için ayrý bir thread oluþtur
        pthread_t thread_id;
        int* client_socket = malloc(sizeof(int)); // Bellek ayýr
        *client_socket = new_socket;             // Soket tanýmlayýcýyý aktar
        pthread_create(&thread_id, NULL, handle_client, client_socket); // Yeni thread baþlat. handle_client: Ýþ parçacýðý tarafýndan çalýþtýrýlacak olan fonksiyondur. client_socket: Ýþ parçacýðýna, o istemciyle iletiþim kurmak için gerekli olan soket
    }

    close(server_fd); // Sunucu soketini kapat
    return 0;
}
