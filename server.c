#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>

// Sabitler
#define PORT 8080         // Sunucunun dinleyece�i port numaras�
#define BUFFER_SIZE 1024  // Veri okuma ve yazma i�in tampon boyutu

// Global saya�lar
int client_count = 0;        // Ba�l� olan toplam istemci say�s�
int tea_count = 0;           // Sat�lan �ay say�s�
int doner_count = 0;         // Sat�lan d�ner say�s�
int beyti_count = 0;         // Sat�lan beyti say�s�

// Mutex (kilit), saya�lara thread-safe eri�im sa�lamak i�in
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // kar��l�kl� d��lama kilidi olu�turmak i�in kullan�lan bir veri t�r�d�r.

// �stemci ba�lant�s�n� i�lemek i�in kullan�lacak thread fonksiyonu
void* handle_client(void* client_socket) {
    int sock = *(int*)client_socket;  // Gelen soket dosya tan�mlay�c�s�n� al
    free(client_socket);              // Bellek s�z�nt�s�n� �nlemek i�in soket pointer'�n� serbest b�rak

    // �stemci say�s�n� art�r
    pthread_mutex_lock(&mutex);       // Mutex kilidi al
    client_count++;
    pthread_mutex_unlock(&mutex);     // Mutex kilidi b�rak

    char buffer[BUFFER_SIZE];         // �stemciden gelen mesajlar� tutacak tampon
    char response[BUFFER_SIZE];       // Sunucudan istemciye d�necek yan�t

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);              // Tamponu s�f�rla
        int bytes_read = read(sock, buffer, BUFFER_SIZE); // �stemciden veri oku
        if (bytes_read <= 0) {                       // Ba�lant� kesilirse d�ng�den ��k
            printf("Client disconnected.\n");
            break;
        }

        printf("Client: %s\n", buffer); // �stemciden gelen mesaj� ekrana yazd�r

        pthread_mutex_lock(&mutex);    // Mutex kilidi al

        // Gelen mesajlar� i�leyerek uygun yan�t olu�tur
        if (strcmp(buffer, "I want tea") == 0) {
            tea_count++;               // �ay say�s�n� art�r
            strcpy(response, "Here is your tea");
        } else if (strcmp(buffer, "I want doner") == 0) {
            doner_count++;             // D�ner say�s�n� art�r
            strcpy(response, "Here is your doner");
        } else if (strcmp(buffer, "I want beyti") == 0) {
            beyti_count++;             // Beyti say�s�n� art�r
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
            strcpy(response, "Invalid request"); // Ge�ersiz bir istek geldi�inde
        }

        pthread_mutex_unlock(&mutex);  // Mutex kilidi b�rak

        // �stemciye yan�t g�nder
        send(sock, response, strlen(response), 0);
    }

    // �stemci ba�lant�s� kesildi�inde istemci say�s�n� azalt
    pthread_mutex_lock(&mutex);
    client_count--;
    pthread_mutex_unlock(&mutex);

    close(sock);              // Soket ba�lant�s�n� kapat
    pthread_exit(NULL);       // Thread'i sonland�r
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Sunucu soketini olu�tur
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {                     // Hata kontrol�
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Soket se�eneklerini ayarla
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // setsockopt fonksiyonu, bir soketin �al��ma davran���n� de�i�tirmek i�in kullan�lan bir sistem �a�r�s�d�r.

    // Soketi belirli bir porta ba�la
    address.sin_family = AF_INET;            // IPv4 adres ailesi
    address.sin_addr.s_addr = INADDR_ANY;    // T�m a� aray�zlerini dinle
    address.sin_port = htons(PORT);          // PORT numaras�n� atan�r
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) { // bind sistemi �a�r�s�, soketi belirli bir IP adresine ve port numaras�na ba�lar.
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Gelen ba�lant�lar� dinle
    if (listen(server_fd, 3) < 0) {          // Maksimum 3 ba�lant� kuyru�u
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        // Yeni bir istemci ba�lant�s�n� kabul et
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        printf("New client connected.\n");

        // Her istemci i�in ayr� bir thread olu�tur
        pthread_t thread_id;
        int* client_socket = malloc(sizeof(int)); // Bellek ay�r
        *client_socket = new_socket;             // Soket tan�mlay�c�y� aktar
        pthread_create(&thread_id, NULL, handle_client, client_socket); // Yeni thread ba�lat. handle_client: �� par�ac��� taraf�ndan �al��t�r�lacak olan fonksiyondur. client_socket: �� par�ac���na, o istemciyle ileti�im kurmak i�in gerekli olan soket
    }

    close(server_fd); // Sunucu soketini kapat
    return 0;
}
