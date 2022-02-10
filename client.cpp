#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "nlohmann/json.hpp"
#include <json-c/json.h>
#include <iostream>
using namespace std;
using json = nlohmann::json;

//functia prelucreaza raspunsul de la server si extrage
//cookie-ul primit
void extractcookie(char **response, char **cookie) {

    char *cookie1, cookie2[256], text[256], *p;
    cookie1 = strstr(*response, "Set-Cookie:");
    p = strtok(cookie1, ";");
    strcpy(*cookie, p + 12);

}

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;

    char *host = (char *)"34.118.48.238";
    int cookiesAuthCount = 1;
    char **cookiesAuth = (char**) malloc(cookiesAuthCount * sizeof(char*));
    cookiesAuth[0] = (char*) malloc(1000);
    string username, password, title, author, genre, publisher;
    string page_count;

    
    //array pentru reprezentarea unei comenzi la tastatura
    char command[256];
    char content[256];
    //array pentru prelucrarea rutei de acees
    char idbook[256];
    //ma ajuta variabila pentru a arata daca este logat sau nu
    int alreadylog = 0;
    char *token = NULL;

    while(1) {
        cin.getline(command, 256);
        //se restabileste conexiunea de fiecare data cand adaugam o comanda
        sockfd = open_connection((char*)"34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
        string sir;

   
        if(strcmp(command, "login") == 0) {
            //verificam daca suntem deja logati
            if (alreadylog == 1) {
                puts("Already logged in!");
            } else {
                cout<<"\n-----------------Login:---------------------------\n";
                //introducem numele si parola utilizatorului din sistem
                cout<<"Username: ";
                getline(cin, username, '\n');
                cout<<"Password: ";
                getline(cin, password, '\n');
                //construim un obiect json care contine cele doua campuri de mai sus
                //pentru trimiterea catre server
                json database;
                database["username"] = username;
                database["password"] = password;
                //in acest string se va depozita
                //obiectul json cu newline intre cele doua campuri
                sir = database.dump(4);
            
                char content[256];
                strcpy(content, sir.c_str());
                //adaugam resursele obtinute
                message = compute_post_request(host, (char*)"/api/v1/tema/auth/login", (char*)"application/json", content, cookiesAuth, cookiesAuthCount, NULL);
                puts(message);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                //afisam ce primim de la server
                puts(response);
                //in cazul in care nu se obtin erori se extrage cookie-ul
                if(strstr(response, "error") == NULL) {
                    alreadylog = 1;
                    extractcookie(&response, &(cookiesAuth[0]));
                }
            }
        }   

        if(strcmp(command, "register") == 0) { 
            cout<<"\n-----------------Register:---------------------------\n";
            //introducem numele si parola utilizatorului din sistem
            cout<<"Username: ";
            getline(cin, username, '\n');
            cout<<"Password: ";
            getline(cin, password, '\n');
            //construim un obiect json care contine cele doua campuri de mai sus
            //pentru trimiterea catre server
            json database;
            database["username"] = username;
            database["password"] = password;

            //in acest string se va depozita
            //obiectul json cu newline intre cele doua campuri
            sir = database.dump(4);
            
            strcpy(content, sir.c_str());
            //adaugam resursele obtinute
            message = compute_post_request(host, (char*)"/api/v1/tema/auth/register", (char*)"application/json", content, cookiesAuth, cookiesAuthCount, NULL);
            puts(message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            puts(response);
        }   

        if(strcmp(command, "enter_library") == 0) { 
            //intrarea in librarie este influentata de faptul
            //daca cineva este logat sau nu
            if(alreadylog == 0) {
                printf("You are not connected!\n");
            } else {
                cout<<"\n-----------------Enter in a library:---------------------------\n";
                //se face intrarea in librarie
                message = compute_get_request(host, (char*)"/api/v1/tema/library/access", NULL, cookiesAuth, cookiesAuthCount, NULL);
                puts(message);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                puts(response);
                //se extrage tokenul obtinut din response
                token = (char*) malloc(1000);
                char *p = strstr(response, "token");
                if(p != NULL) {
                    strcpy(token, p + 8);
                    token[strlen(token) - 2] = '\0';
                }
            }

        }   
        
        if(strcmp(command, "get_books") == 0) { 
            //in cazul in care tokenul este null
            //inseamna ca nu s-a dat comanda enter_library
            if(token == NULL) {
                cout<<"Not access in library\n";
            } else {
                cout<<"\n-----------------Please choose some books:)---------------------------\n";
                message = compute_get_request(host, (char*)"/api/v1/tema/library/books", NULL, NULL, 0, token);
                //afisam toate cartile userului conectat
                puts(message);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                puts(response);  

            }



        }   

        if(strcmp(command, "get_book") == 0) { 
            //in cazul in care tokenul este null
            //inseamna ca nu s-a dat comanda enter_library
            if(token == NULL) {
                cout<<"Not access in library\n";
            } else {
                cout<<"\n-----------------Get a book!---------------------------\n";
                //se citeste id-ul cerut pentru afisarea cartii
                int id;
                cout<<"Please Sir choose a book using the index ";
                cin>>id;
                //ruta de acces se contruieste pe baza id-ului
                sprintf(idbook, "/api/v1/tema/library/books/%d", id);
                message = compute_get_request(host, idbook, NULL, NULL, 0, token);
                //se afiseaza datele cartii cerute
                puts(message);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                puts(response);
            }

        }   

        if(strcmp(command, "add_book") == 0) { 
            //in cazul in care tokenul este null
            //inseamna ca nu s-a dat comanda enter_library
            if(token == NULL) {
                cout<<"Not access in library\n";
            } else {
                cout<<"\n-----------------Please add the book!---------------------------\n";
                //se citesc datele pentru noua carte
                //care va urma sa fie aduagata
                cout<<"title: ";
                getline(cin, title, '\n');
                cout<<"author ";
                getline(cin, author, '\n');
                cout<<"genre: ";
                getline(cin, genre, '\n');
                cout<<"publisher: ";
                getline(cin, publisher, '\n');
                cout<<"page_count: ";
                getline(cin, page_count, '\n');
                //se construieste obiectul json pe baza informatiilor de mai sus
                json book;
                book["title"] = title;
                book["author"] = author;
                book["genre"] = genre;
                book["publisher"] = publisher;
                book["page_count"] = page_count;
                //in acest string se va depozita
                //obiectul json cu newline intre toate campurile cartii
                sir = book.dump(4);    
                strcpy(content, sir.c_str());

                message = compute_post_request(host, (char*)"/api/v1/tema/library/books", (char*)"application/json", content, NULL, 0, token);
                puts(message);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                puts(response);    
            }

        }   

        if(strcmp(command, "delete_book") == 0) { 
            //in cazul in care tokenul este null
            //inseamna ca nu s-a dat comanda enter_library
            if(token == NULL) {
                cout<<"Not access in library\n";
            } else {
                cout<<"\n-----------------Delete this book!--------------------------\n";
                int id;
                //se citeste id-ul cartii care va urma sa fie stearsa
                cout<<"Please Sir choose a book using the index ";
                cin>>id;
                sprintf(idbook, "/api/v1/tema/library/books/%d", id);
                //functia delete se va ocupa de stergerea cartii din baza de date
                message = compute_delete_request(host, idbook, NULL, NULL, 0, token);
                puts(message);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                puts(response);   
            } 
     
        }   

        if(strcmp(command, "logout") == 0) { 
            cout<<"\n-----------------Logout!---------------------------\n";
            //in cazul in care se face logout, se reseteaza parametrii  dati
            alreadylog = 0;
            free(token);
            token = NULL;
            message = compute_get_request(host, (char*)"/api/v1/tema/auth/logout", NULL, cookiesAuth, cookiesAuthCount, NULL);
            puts(message);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            puts(response);
        }   

        if(strcmp(command, "exit") == 0) { 
            cout<<"\n-----------------Goodbye my friend!---------------------------\n";
            close_connection(sockfd);
            break;
        }   

        close_connection(sockfd);

    }


    free(message);
    free(response);
    free(cookiesAuth[0]);
    free(cookiesAuth);
    return 0;
}