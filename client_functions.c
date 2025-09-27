//Cernea Mihnea-Ioan 324CB
#include "client_functions.h"

/**
 * receive_post_request: trimite o cerere HTTP POST si primeste raspunsul
 * Returneaza: sirul cu raspunsul serverului
 */
char *receive_post_request(int sockfd, char *host, char *url, char *user[1], char **cookie, int nmb_cookie, char *token) {
    // Construim mesajul POST folosind functia helper
    char *message = compute_post_request(host, url, "application/json", user, 1, cookie, nmb_cookie, token);
    // Trimitem cererea catre server
    send_to_server(sockfd, message);
    // Primim raspunsul de la server
    char *response = receive_from_server(sockfd);
    return response;
}

/**
 * receive_put_request: trimite o cerere HTTP PUT si primeste raspunsul
 * Similar cu POST, dar pentru actualizarea unei resurse
 */
char *receive_put_request(int sockfd, char *host, char *url, char *user[1], char **cookie, int nmb_cookie, char *token) {
    char *message = compute_put_request(host, url, "application/json", user, 1, cookie, nmb_cookie, token);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    return response;
}

//receive_get_request: trimite o cerere HTTP GET si primeste raspunsul
char *receive_get_request(int sockfd, char *host, char *url, char *cookies[1], char *token) {
    char *message = compute_get_request(host, url, NULL, cookies, 1, token);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    return response;
}

//receive_delete_request: trimite o cerere HTTP DELETE si primeste raspunsul
char *receive_delete_request(int sockfd, char *host, char *url, char *cookies[1], char *token) {
    char *message = compute_delete_request(host, url, NULL, cookies, 1, token);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    return response;
}

// http_status_is_success: verifica daca statusul HTTP este 2xx
bool http_status_is_success(const char *http_response) {
    int status = 0;
    // Extragem codul de status din raspuns
    if (sscanf(http_response, "HTTP/%*d.%*d %d", &status) != 1) {
        return false; // nu s-a putut parsa statusul
    }
    return (status >= 200 && status < 300);
}

// extract_token: extrage token-ul JWT din corpul JSON al raspunsului
void extract_token(char *response, char *value_token) {
    // Gasim inceputul JSON-ului ('{') in raspuns
    char *token = strchr(response, '{');
    // Parseaza JSON-ul din acel punct
    JSON_Value *jwt = json_parse_string(token);
    // Extrage campul "token" din obiectul JSON
    strcpy(value_token,(char *)json_object_get_string(json_object(jwt), "token"));
    // Eliberam structura JSON
    json_value_free(jwt);
}

// obtain_jwt_token: obtine un token JWT printr-o cerere GET catre URL-ul de acces
void obtain_jwt_token(int sockfd, char *host, char **cookies, int nmb_cookie, char *token) {
    // Trimite cererea GET pentru acces
    char *resp = receive_get_request(sockfd, host, GET_ACCESS_URL, cookies, token);
    // Verificam daca statusul nu este success
    if (!http_status_is_success(resp)) {
        if (strstr(resp, "HTTP/1.1 500")) {
            printf("EROARE: Server invalid\n");
        } else {
            printf("EROARE: get access a dat fail din alte motive!\n");
        }
        free(resp);
    }
    // Verificam prezenta campului token
    if (!strstr(resp, "token")) {
        printf("EROARE: Token invalid, access nepermis\n");
        free(resp);
    }
    // Extragem si salvam token-ul
    extract_token(resp, token);
    free(resp);
    printf("SUCCESS: Token JWT primit\n");
}

/**
 * valid_cookie: verifica si izoleaza cookie-ul Set-Cookie din antet
 * Returneaza: true daca gaseste cookie, false altfel
 */
bool valid_cookie(char *response) {
    // Cautam antetul Set-Cookie
    char *cookie = strstr(response, "Set-Cookie: ");
    if (cookie != NULL) {
        cookie += 12; // sarim peste textul "Set-Cookie: "
        // Gasim sfarsitul cookie-ului (';') si il terminam
        char *end = strstr(cookie, ";");
        if (end != NULL) {
            *end = '\0';
        }
    }
    return (cookie != NULL);
}

/**
 * add_user: citeste de la stdin datele pentru crearea unui user (admin_username, username, password)
 * si le serializeaza in JSON
 * Returneaza: sir JSON cu datele utilizatorului
 */
char *add_user() {
    // Alocam buffer pentru inputul de la tastatura
    char *admin_username = malloc(50);
    char *username = malloc(50);
    char *password = malloc(50);
    // Citim admin_username
    printf("admin_username=");
    fgets(admin_username, 50, stdin);
    admin_username[strcspn(admin_username, "\n")] = 0; // eliminam \n
    // Citim username
    printf("username=");
    fgets(username, 50, stdin);
    username[strcspn(username, "\n")] = 0;
    // Citim password
    printf("password=");
    fgets(password, 50, stdin);
    password[strcspn(password, "\n")] = 0;

    // Construim obiect JSON cu parson
    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);
    json_object_set_string(obj, "admin_username", admin_username);
    json_object_set_string(obj, "username", username);
    json_object_set_string(obj, "password", password);

    return json_serialize_to_string(val);
}

//parse_and_print_users: parseaza raspunsul JSON si afiseaza lista de utilizatori
void parse_and_print_users(const char *response) {
    // Daca serverul a dat eroare 500
    if (strstr(response, "HTTP/1.1 500")){
        printf("EROARE: Server invalid\n"); 
        return;
    } else if(!http_status_is_success(response)) {
        printf("EROARE: Afisarea userilor a dat fail din alte motive\n");
        return;
    }

    // Sarim peste antete pana la corp
    const char *json_start = strstr(response, "\r\n\r\n") + 4;
    JSON_Value *root_value = json_parse_string(json_start);
    JSON_Object *root_object = json_value_get_object(root_value);
    JSON_Array *users = json_object_get_array(root_object, "users");
    size_t count = json_array_get_count(users);

    printf("SUCCESS: Lista utilizatorilor\n");
    for (size_t i = 0; i < count; i++) {
        JSON_Object *user = json_array_get_object(users, i);
        int id = (int)json_object_get_number(user, "id");
        const char *username = json_object_get_string(user, "username");
        const char *password = json_object_get_string(user, "password");
        printf("#%d %s:%s\n", id, username, password);
    }
    json_value_free(root_value);
}

// parse_and_print_movies: parseaza raspunsul JSON si afiseaza lista de filme
void parse_and_print_movies(const char *response) {
    // Verificam eroare 401 (token expirat)
    if (strstr(response, "HTTP/1.1 401")) {
        printf("EROARE: TOKEN expirat\n");
        return;
    }
    // Alte erori
    if(!http_status_is_success(response)) {
        printf("EROARE: Afisarea filmelor a dat fail din alte motive\n");
        return;
    }

    // Extragem payload ul
    const char *json_start = strstr(response, "\r\n\r\n") + 4;

    // Creeam noul pachet de date json
    JSON_Value *root_value = json_parse_string(json_start);
    JSON_Object *root_object = json_value_get_object(root_value);
    JSON_Array *movies = json_object_get_array(root_object, "movies");
    int count = (int)json_array_get_count(movies);

    printf("SUCCESS: Lista filmelor\n");
    for (int i = 0; i < count; i++) {
        JSON_Object *movie = json_array_get_object(movies, i);
        int id = (int)json_object_get_number(movie, "id");
        const char *title = json_object_get_string(movie, "title");
        printf("#%d %s\n", id, title);
    }
    json_value_free(root_value);
}

/**
 * parse_collection: parseaza raspunsul HTTP si afiseaza detaliile unei colectii
 * Cauta corpul JSON dupa secventa de separare "\r\n\r\n" si il parseaza.
 * Afiseaza titlul, proprietarul si lista filmelor cu id si titlu.
 */
void parse_collection(char *response) {
    // Gasim delimitatorul intre antet si corpul JSON
    const char *json_start = strstr(response, "\r\n\r\n");
    // Sarim peste secventa de delimitare pentru a ajunge la inceputul JSON-ului
    json_start += 4;

    // Parseaza continutul JSON in structura interna
    JSON_Value *root_value = json_parse_string(json_start);

    printf("SUCCESS: Detalii colectie\n");
    
    // Obtinem obiectul principal din JSON
    JSON_Object *root_object = json_value_get_object(root_value);
    
    // Extragem campurile string "owner" si "title"
    const char *owner = json_object_get_string(root_object, "owner");
    const char *title = json_object_get_string(root_object, "title");

    // Afisam titlul si proprietarul colectiei
    printf("title: %s\nowner: %s\n", title, owner);

    // Extragem array-ul de filme
    JSON_Array *movies = json_object_get_array(root_object, "movies");
    size_t movie_count = json_array_get_count(movies);

    // Iteram peste fiecare film si afisam id-ul si titlul
    for (size_t i = 0; i < movie_count; i++) {
        JSON_Object *movie = json_array_get_object(movies, i);
        int movie_id = (int)json_object_get_number(movie, "id");  
        const char *movie_title = json_object_get_string(movie, "title");
        printf("#%d: %s\n", movie_id, movie_title);
    }

    // Eliberam memoria alocata pentru JSON
    json_value_free(root_value);
}

/**
 * get_id_collection: parseaza raspunsul HTTP pentru a extrage id-ul colectiei
 * Returneaza: id-ul numeric extras din corpul JSON
 */
const int get_id_collection(char *response) {
    // Gasim inceputul corpului JSON si sarim peste antet
    const char *json_start = strstr(response, "\r\n\r\n");
    json_start += 4;

    // Parseaza JSON-ul
    JSON_Value *response_value = json_parse_string(json_start);
    JSON_Object *response_object = json_value_get_object(response_value);

    // Extragem campul numeric "id"
    const int id = (int)json_object_get_number(response_object, "id");
    
    // Eliberam memoria JSON
    json_value_free(response_value);
    // Returnam id-ul colectiei
    return id;                           
}

/**
 * int_to_string: converteste un numar intreg intr-un sir C
 * Returneaza: buffer alocat cu sirul rezultat (apelantul trebuie sa elibereze memoria)
 */
char *int_to_string(const int value) {
    // Alocam suficient pentru un intreg semnat (max 11 caractere + terminator)
    char *buffer = malloc(12);
    if (buffer == NULL) {
        // Daca nu se poate aloca, returnam NULL
        return NULL;
    }

    // Formatam valoarea in sir cu snprintf pentru siguranta
    snprintf(buffer, 12, "%d", value);
    return buffer;
}

