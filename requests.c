#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>     
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>      
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

// Disclaimer : fisier preluat din laborator (lucruri schimbate am adaugat 2 functii noi "compute_delete_request" , "compute_put_reuest"
// , si am aduagat la functiile din lab implementarea pentru cookies si JWT token)

// compute_get_request: construieste un mesaj HTTP GET
char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *token)
{
    // Alocam buffer pentru mesaj si linii intermediare
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Construim linia de start GET, cu sau fara parametri de query
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);  // Adaugam linia la mesaj

    // Daca avem token JWT, adaugam antetul Authorization
    if(token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // Adaugam antetul Host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Daca exista cookie-uri, le adaugam fiecare pe cate o linie Cookie
    if (cookies != NULL) {
        for (int i = 0; i < cookies_count; i++) {
            sprintf(line, "Cookie: %s", cookies[i]);
            compute_message(message, line);
        }
    }

    // Linia goala semnaleaza terminarea anteturilor
    compute_message(message, "");
    return message;  // Returnam cererea completa
}

// compute_post_request: construieste un mesaj HTTP POST
char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count, char *token)
{
    // Buffer pentru mesaj, linii si acumularea datelor in body
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    // Linia de start POST
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Adaugam Authorization daca exista token
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // Antet Host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Antet Content-Type
    if (content_type != NULL) {
        sprintf(line, "Content-Type: %s", content_type);
        compute_message(message, line);
    }

    // Daca avem date in body, concatenam si calculam lungimea
    int len = 0;
    if (body_data != NULL) {
        for (int i = 0; i < body_data_fields_count; i++) {
            strcat(body_data_buffer, body_data[i]);  // Adaugam campul la buffer
            len += strlen(body_data[i]);           // Crestem contorul de lungime
        }
        // Adaugam antetul Content-Length cu dimensiunea totala
        sprintf(line, "Content-Length: %u", len);
        compute_message(message, line);
    }

    // Adaugam cookie-urile daca exista
    if (cookies != NULL) {
        for (int i = 0; i < cookies_count; i++) {
            sprintf(line, "Cookie: %s", cookies[i]);
            compute_message(message, line);
        }
    }

    // Terminarea anteturilor cu linie goala
    compute_message(message, "");

    // Adaugam corpul cererii la mesaj
    memset(line, 0, LINELEN);
    strcat(message, body_data_buffer);

    free(line);  // Eliberam memoria pentru linie
    return message;
}


// compute_delete_request: construieste un mesaj HTTP DELETE
char *compute_delete_request(char *host, char *url, char *query_params, char **cookies, int cookies_count, char *token) {
    // Alocam buffer pentru mesaj si linii
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Construim linia de start DELETE cu sau fara parametri
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }
    compute_message(message, line);
    
    // Adaugam Authorization daca exista token
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }
    // Antet Host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Cookie-uri daca exista
    if (cookies != NULL) {
        for (int i = 0; i < cookies_count; i++) {
            sprintf(line, "Cookie: %s", cookies[i]);
            compute_message(message, line);
        }
    }
    
    // Terminam anteturile
    compute_message(message, "");
    return message;
}

// compute_put_request: construieste un mesaj HTTP PUT
char *compute_put_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count, char *token)
{
    // Buffere pentru mesaj, linie si date corp
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    // Linia de start PUT
    sprintf(line, "PUT %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Authorization token
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    // Antet Host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Antet Content-Type
    if (content_type != NULL) {
        sprintf(line, "Content-Type: %s", content_type);
        compute_message(message, line);
    }

    // Calculam si adaugam Content-Length pentru datele din body
    int len = 0;
    if (body_data != NULL) {
        for (int i = 0; i < body_data_fields_count; i++) {
            strcat(body_data_buffer, body_data[i]);
            len += strlen(body_data[i]);
        }
        sprintf(line, "Content-Length: %u", len);
        compute_message(message, line);
    }

    // Adaugam cookie-urile
    if (cookies != NULL) {
        for (int i = 0; i < cookies_count; i++) {
            sprintf(line, "Cookie: %s", cookies[i]);
            compute_message(message, line);
        }
    }

    // Linia goala: final antet
    compute_message(message, "");

    // Adaugam corpul la mesaj
    memset(line, 0, LINELEN);
    strcat(message, body_data_buffer);

    free(line);  // Eliberam memoria pentru linie
    return message;
}
