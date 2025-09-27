//Cernea Mihnea-Ioan 324CB
#include "client_functions.h"

int main() {
    // Deschidem conexiunea TCP catre server (HOST, PORT)
    int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    // Alocam vectori pentru date dinamice: colectii, utilizator, film, cookie-uri
    char **collections = malloc(sizeof(char *));
    char **user = malloc(sizeof(char *));
    char **movie = malloc(sizeof(char *));
    char **cookies = malloc(sizeof(char *));
    // Buffer pentru token JWT si comanda citita de la tastatura
    char *token = malloc(BUFSIZ);
    char *command = malloc(50);
    
    // Starea initiala: neautentificat
    int logged_in = LOGOUT;
    // Flag pentru acces biblioteca
    bool library_acces = false;  

    // Bucla principala de citire comenzi
    while (1) {
        // Citim comanda de la stdin
        fgets(command, 50, stdin);
        // Daca utilizatorul tasteaza "exit", iesim din aplicatie
        if (strncmp(command, "exit" , 4) == 0) {
            break;
        } else {
            // Procesam comanda login_admin
            if (strncmp(command, "login_admin", 11) == 0) {
                if (!logged_in) {
                    // Citim credentiale admin: username si password
                    char *username = malloc(50);
                    char *password = malloc(50);
                    printf("username=");
                    fgets(username, 50, stdin);
                    username[strcspn(username, "\n")] = 0;
                    printf("password=");
                    fgets(password, 50, stdin);
                    password[strcspn(password, "\n")] = 0;

                    // Construim JSON cu parson
                    JSON_Value *val = json_value_init_object();
                    JSON_Object *obj = json_value_get_object(val);
                    json_object_set_string(obj, "username", username);
                    json_object_set_string(obj, "password", password);

                    // Serializam JSON pentru body
                    user[0] = json_serialize_to_string(val);

                    // Trimitem cererea POST pentru autentificare admin
                    char *response = receive_post_request(
                        sockfd, HOST, LOGIN_ADMIN_URL,
                        user, NULL, 0, NULL
                    );
                    // Extragem cookie-ul din antet daca exista
                    char *cookie = strstr(response, "Set-Cookie: ");
                    if (cookie != NULL) {
                        cookie += 12;  // saltam textul "Set-Cookie: "
                        char *end = strstr(cookie, ";");
                        if (end != NULL) {
                            *end = '\0';
                        }
                    }

                    // Verificam daca cookie-ul este valid si setam starea
                    if (valid_cookie(response)) {
                        logged_in = ADMIN_LOGGEDIN;
                        cookies[0] = cookie;
                        printf("SUCCESS: Admin autentificat cu succes\n");
                    } else if (strstr(response, "HTTP/1.1 500")){
                        printf("EROARE: Server invalid\n");
                    } else {
                        printf("EROARE: Credentialele nu se potrivesc\n");
                    }
                } else {
                    printf("EROARE: Deja sunteti logat\n");
                }

            // Procesam comanda add_user (numai admin)
            } else if (strncmp(command, "add_user", 8) == 0) {
                if (logged_in == ADMIN_LOGGEDIN) {
                    // Citim date user nou
                    char *username = malloc(50);
                    char *password = malloc(50);
                    printf("username=");
                    fgets(username, 50, stdin);
                    username[strcspn(username, "\n")] = 0;
                    printf("password=");
                    fgets(password, 50, stdin);
                    password[strcspn(password, "\n")] = 0;

                    // Construim JSON pentru utilizator
                    JSON_Value *val = json_value_init_object();
                    JSON_Object *obj = json_value_get_object(val);
                    json_object_set_string(obj, "username", username);
                    json_object_set_string(obj, "password", password);
                    user[0] = json_serialize_to_string(val);

                    // Trimitem cererea POST de inregistrare
                    char *response = receive_post_request(sockfd, HOST,
                        REGISTER_USER_URL, user, cookies, 1, NULL
                    );
                    // Verificam codul de status
                    if (strstr(response, "201 CREATED")) {
                        printf("SUCCESS: User adaugat cu succes\n");
                    } else if (strstr(response, "HTTP/1.1 500")){
                        printf("EROARE: Server invalid\n");
                    } else {
                        printf("EROARE: Informatii incomplete\n");
                    }
                } else {
                    printf("EROARE: Nu sunteti logat ca admin\n");
                }

            // Afisare utilizatori (numai admin)
            } else if (strncmp(command, "get_users", 9) == 0) {
                if (logged_in == ADMIN_LOGGEDIN) {
                    char *response = receive_get_request(
                        sockfd, HOST, VISUALIZE_USERS_URL, cookies, NULL
                    );
                    parse_and_print_users(response);
                } else {
                    printf("EROARE: Nu sunteti logat ca admin\n");
                }

            // Procesam comanda delete_user (numai admin)
            } else if (!strncmp(command, "delete_user", 11)) {
                if (logged_in == ADMIN_LOGGEDIN) {
                    // Citim username-ul de sters
                    char *username = malloc(50);
                    printf("username=");
                    fgets(username, 50, stdin);
                    username[strcspn(username, "\n")] = 0;

                    // Construim URL cu sprintf
                    char *url = malloc(100);
                    sprintf(url, DELETE_USER_URL, username);

                    // Trimitem DELETE
                    char *response = receive_delete_request(
                        sockfd, HOST, url, cookies, NULL
                    );
                    if (http_status_is_success(response)) {
                        printf("SUCCESS: User sters cu succes\n");
                    } else if (strstr(response, "HTTP/1.1 500")){
                        printf("EROARE: Server invalid\n");
                    } else {
                        printf("EROARE: Username-ul este invalid\n");
                    }
                } else {
                    printf("EROARE: Informatii incomplete\n");
                }

            // Logout admin
            } else if (!strncmp(command, "logout_admin", 12)) {
                if (!logged_in) {
                    printf("EROARE: Nu sunteti logat\n");
                } else if (logged_in == USER_LOGGEDIN) {
                    printf("EROARE: Sunteti logat ca user\n");
                } else {
                    char *response = receive_get_request(
                        sockfd, HOST, LOGOUT_ADMIN_URL, cookies, NULL
                    );
                    if (http_status_is_success(response)) {
                        printf("SUCCESS: Admin deconectat cu succes\n");
                        // Resetam cookie si token
                        memset(cookies[0], 0, strlen(cookies[0]));
                        memset(token, 0, strlen(token));
                        logged_in = LOGOUT;
                    } else if (strstr(response, "HTTP/1.1 500")){
                        printf("EROARE: Server invalid\n");
                    } else {
                        printf("EROARE: Logout admin a dat fail din alte motive\n");
                    }
                }
            }
            else if (!strncmp(command, "login", 5)) {
                // Comanda "login": autentificare utilizator
                if (logged_in == USER_LOGGEDIN) {
                    // Daca deja e user logat, afisam eroare
                    printf("ERROR: Deja sunteti logat ca user\n");
                } else if (logged_in == ADMIN_LOGGEDIN) {
                    // Daca e admin logat, nu poate face login user
                    printf("ERROR: Deja sunteti logat ca admin\n");
                } else {
                    // Citim credentialele user: folosim add_user() care returneaza JSON
                    user[0] = add_user();

                    // Trimitem cererea POST pentru login user
                    char *response = receive_post_request(sockfd, HOST, LOGIN_USER_URL,
                                                        user, NULL, 0, NULL);
                    // Daca status != 2xx, retry login
                    if (!http_status_is_success(response)) {
                        printf("EROARE: Credentiale gresite, reautentificare\n");
                        user[0] = add_user();
                        // Re-deschidem conexiunea si retrimit request
                        sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                        response = receive_post_request(sockfd, HOST, LOGIN_USER_URL,
                                                        user, NULL, 0, NULL);
                    }
                    // Extragem cookie-ul daca exista in antet
                    char *cookie = strstr(response, "Set-Cookie: ");
                    if (cookie != NULL) {
                        cookie += 12;  // saltam textul antetului
                        char *end = strstr(cookie, ";");
                        if (end != NULL) {
                            *end = '\0';  // terminator de sir la sfarsitul cookie
                        }
                    }
                    // Verificam cookie valid: daca da, salvam stare si cookie
                    if (valid_cookie(response)) {
                        logged_in = USER_LOGGEDIN;
                        cookies[0] = cookie;
                        printf("SUCCESS: User autentificat cu succes\n");
                    } else if (strstr(response, "HTTP/1.1 500")){
                        // Eroare server
                        printf("EROARE: Server invalid\n");
                    } else {
                        // Alte esecuri
                        printf("EROARE: Login user a dat fail din alte motive\n");
                    }
                }

            } else if (!strncmp(command, "logout", 6)) {
                // Comanda "logout": deconectare user
                if (!logged_in) {
                    // Daca nu e nimeni logat
                    printf("EROARE: Nu sunteti logat\n");
                } else if (logged_in == ADMIN_LOGGEDIN) {
                    // Daca e admin logat
                    printf("EROARE: Sunteti logat ca admin\n");
                } else {
                    // Trimitem GET pentru logout user
                    char *response = receive_get_request(sockfd, HOST, LOGOUT_USER_URL,
                                                        cookies, NULL);
                    if (http_status_is_success(response)) {
                        // Deconectare reusita: resetam cookie si token
                        printf("SUCCESS: User deconectat cu succes\n");
                        memset(cookies[0], 0, strlen(cookies[0]));
                        memset(token, 0, strlen(token));
                        logged_in = LOGOUT;
                    } else if (strstr(response, "HTTP/1.1 500")){
                        printf("EROARE: Server invalid\n");
                    } else {
                        printf("EROARE: Logout user a dat fail din alte motive\n");
                    }
                }

            } else if (!strncmp(command, "get_access", 10)) {
                // Comanda "get_access": solicitare token JWT
                if (logged_in) {
                    // Trimitem GET pentru acces bibliotecii
                    char *response = receive_get_request(sockfd, HOST, GET_ACCESS_URL,
                                                        cookies, token);
                    char *token_start = strstr(response, "token");
                    if (http_status_is_success(response)) {
                        if (token_start == NULL) {
                            printf("EROARE: Token invalid , access nepermis\n");
                        } else {
                            // Extragem si salvam tokenul
                            extract_token(response, token);
                            library_acces = true;
                            printf("SUCCESS: Token JWT primit\n");
                        }
                    } else if (strstr(response, "HTTP/1.1 500")){
                        printf("EROARE: Server invalid\n");
                    } else {
                        printf("EROARE : get access a dat fail din alte motive!\n");
                    }
                } else {
                    printf("EROARE: Nu sunteti logat\n");
                }

            } else if (!strncmp(command, "get_movies", 10)) {
                // Comanda "get_movies": afisare lista filme
                if (library_acces) {
                    char *response = receive_get_request(sockfd, HOST,
                                                        GET_MOVIES_URL, cookies, token);
                    // Daca token expirat (401), obtinem alt token si rechemam
                    if (strstr(response, "HTTP/1.1 401")) {
                        obtain_jwt_token(sockfd, HOST, cookies, 1, token);
                        response = receive_get_request(sockfd, HOST,
                                                    GET_MOVIES_URL, cookies, token);
                    }
                    parse_and_print_movies(response);
                } else {
                    printf("EROARE: Nu aveti acces la librarie!\n");
                }

            } else if (!strncmp(command, "get_movie", 9)) {
                // Comanda "get_movie": afisare detalii film
                if (library_acces) {
                    // Citim id-ul filmului
                    char *id = malloc(50);
                    printf("id=");
                    fgets(id, 50, stdin);
                    id[strcspn(id, "\n")] = 0;

                    // Construim URL cu id
                    char *url = malloc(100);
                    sprintf(url, GET_MOVIE_URL, id);

                    // Trimitem GET pentru film
                    char *response = receive_get_request(sockfd, HOST,
                                                        url, cookies, token);
                    if (strstr(response, "HTTP/1.1 500")){
                        printf("EROARE: Server invalid\n");
                    } else if (!http_status_is_success(response)) {
                        printf("EROARE: ID invalid\n");
                    } else {
                        // Parsam corp JSON si afisam campurile relevante
                        const char *json_start = strstr(response, "\r\n\r\n") + 4;
                        JSON_Value *value_response = json_parse_string(json_start);
                        JSON_Object *obj_response = json_value_get_object(value_response);

                        const char *title = json_object_get_string(obj_response, "title");
                        int year = (int)json_object_get_number(obj_response, "year");
                        const char *description = json_object_get_string(obj_response, "description");
                        const char *rating = json_object_get_string(obj_response, "rating");

                        // Reasamblam JSON frumos pentru afisare
                        JSON_Value *out_value = json_value_init_object();
                        JSON_Object *out_obj  = json_value_get_object(out_value);
                        json_object_set_string(out_obj, "title", title);
                        json_object_set_number(out_obj, "year", year);
                        json_object_set_string(out_obj, "description", description);
                        json_object_set_string(out_obj, "rating", rating);

                        char *out_str = json_serialize_to_string_pretty(out_value);
                        printf("%s\n", out_str);
                    }
                } else {
                    printf("EROARE: Nu aveti acces la librarie!\n");
                }
            }

            // Comanda “add_movie”: adauga un film in biblioteca
            else if (!strncmp(command, "add_movie", 9)
                    && strncmp(command, "add_movie_to_collection", 20)) {
                if (library_acces) {
                    // Citim datele filmului de la tastatura
                    char *year = malloc(10);        // anul aparitiei
                    double rating;                  // rating-ul filmului
                    char *title = malloc(100);      // titlul filmului
                    char *descrption = malloc(100); // descriere film

                    printf("title=");               // prompt pentru titlu
                    fgets(title, 100, stdin);
                    title[strcspn(title, "\n")] = 0;

                    printf("year=");                // prompt pentru an
                    fgets(year, 10, stdin);
                    year[strcspn(year, "\n")] = 0;

                    printf("description=");         // prompt pentru descriere
                    fgets(descrption, 100, stdin);
                    descrption[strcspn(descrption, "\n")] = 0;

                    printf("rating=");              // prompt pentru rating
                    scanf("%lf", &rating);
                    while (getchar() != '\n');      // curat buffer stdin

                    // Construim obiectul JSON cu Parson
                    JSON_Value *val = json_value_init_object();
                    JSON_Object *obj = json_value_get_object(val);
                    json_object_set_string(obj, "title", title);
                    json_object_set_number(obj, "year", atoi(year));
                    json_object_set_string(obj, "description", descrption);
                    json_object_set_number(obj, "rating", rating);

                    // Serializam JSON-ul
                    movie[0] = json_serialize_to_string(val);

                    // Trimitem cererea POST catre ADD_MOVIE_URL
                    char *response = receive_post_request(
                        sockfd, HOST, ADD_MOVIE_URL, movie, cookies, 1, token
                    );

                    // Verificam statusul HTTP si afisam mesaj
                    if (http_status_is_success(response))
                        printf("SUCCESS: Film adaugat\n");
                    else if (strstr(response, "HTTP/1.1 500"))
                        printf("EROARE: Server invalid\n");
                    else if (strstr(response, "HTTP/1.1 401"))
                        printf("EROARE: TOKEN expirat\n");
                    else
                        printf("EROARE: Date invalide\n");

                } else {
                    // Daca nu are token valid
                    printf("EROARE: Nu aveti acces la librarie!\n");
                }

            // Comanda “delete_movie”: sterge un film din biblioteca
            } else if (!strncmp(command, "delete_movie", 12)
                    && strncmp(command, "delete_movie_from_collection", 28)) {
                if (library_acces) {
                    // Citim id-ul filmului de sters
                    char *id_string = malloc(17);
                    printf("id=");
                    fgets(id_string, 17, stdin);
                    id_string[strcspn(id_string, "\n")] = 0;

                    // Construim URL-ul de stergere cu sprintf
                    char *url = malloc(100);
                    sprintf(url, DELETE_MOVIE_URL, id_string);

                    // Trimitem DELETE request
                    char *response = receive_delete_request(
                        sockfd, HOST, url, cookies, token
                    );

                    // Afisam mesaj in functie de status
                    if (http_status_is_success(response))
                        printf("SUCCESS: Film sters cu succes\n");
                    else if (strstr(response, "HTTP/1.1 500"))
                        printf("EROARE: Server invalid\n");
                    else if (strstr(response, "HTTP/1.1 401"))
                        printf("EROARE: TOKEN expirat\n");
                    else
                        printf("EROARE: ID invalid\n");

                } else {
                    printf("EROARE: Nu aveti acces la librarie!\n");
                }

            // Comanda “update_movie”: actualizeaza un film existent
            } else if (!strncmp(command, "update_movie", 12)) {
                if (library_acces) {
                    // Citim id-ul filmului de actualizat
                    char *id_string = malloc(17);
                    printf("id=");
                    fgets(id_string, 17, stdin);
                    id_string[strcspn(id_string, "\n")] = 0;

                    // Construim URL-ul pentru operatie
                    char *url = malloc(100);
                    sprintf(url, UPDATE_MOVIE_URL, id_string);

                    // Citim noile date ale filmului
                    char *year = malloc(10);
                    double rating;
                    char *title = malloc(100);
                    char *descrption = malloc(100);

                    printf("title=");
                    fgets(title, 100, stdin);
                    title[strcspn(title, "\n")] = 0;

                    printf("year=");
                    fgets(year, 10, stdin);
                    year[strcspn(year, "\n")] = 0;

                    printf("description=");
                    fgets(descrption, 100, stdin);
                    descrption[strcspn(descrption, "\n")] = 0;

                    printf("rating=");
                    scanf("%lf", &rating);
                    while (getchar() != '\n');

                    // Construim JSON-ul pentru update
                    JSON_Value *val = json_value_init_object();
                    JSON_Object *obj = json_value_get_object(val);
                    json_object_set_string(obj, "title", title);
                    json_object_set_number(obj, "year", atoi(year));
                    json_object_set_string(obj, "description", descrption);
                    json_object_set_number(obj, "rating", rating);

                    movie[0] = json_serialize_to_string(val);

                    // Trimitem PUT request pentru actualizare
                    char *response = receive_put_request(
                        sockfd, HOST, url, movie, cookies, 1, token
                    );

                    // Tratare erori si mesaj final
                    if (strstr(response, "HTTP/1.1 500"))
                        printf("EROARE: Server invalid\n");
                    if (!http_status_is_success(response))
                        printf("EROARE: ID invalid sau datele sunt incomplete\n");
                    else
                        printf("SUCCESS: Film actualizat\n");

                } else {
                    printf("EROARE: Nu aveti acces la librarie!\n");
                }
            }


            // Comanda “get_collections”: afiseaza toate colectiile disponibile in biblioteca
            else if (!strncmp(command, "get_collections", 15)) {
                if (library_acces) {
                    // Trimitem cererea GET catre endpoint-ul de liste colectii
                    char *response = receive_get_request(sockfd, HOST,
                                                        GET_COLLECTIONS_URL,
                                                        cookies, token);
                    // Daca serverul raspunde cu eroare interna
                    if (strstr(response, "HTTP/1.1 500")) {
                        printf("EROARE: Server invalid\n");
                    }
                    // Gasim inceputul corpului JSON (dupa anteturi)
                    const char *json_start = strstr(response, "\r\n\r\n");
                    json_start += 4;

                    // Parsam sirul JSON intr-o structura Parson
                    JSON_Value *response_value = json_parse_string(json_start);
                    JSON_Object *response_object = json_value_get_object(response_value);

                    // Preluam array-ul “collections” din obiect
                    JSON_Array *collections = json_object_get_array(response_object,
                                                                    "collections");

                    // Determinam numarul de colectii
                    int count = json_array_get_count(collections);
                    printf("SUCCESS: Lista colectiilor\n");

                    // Iteram si afisam fiecare colectie (#id: title)
                    for (int i = 0; i < count; i++) {
                        JSON_Object *collection = json_array_get_object(collections, i);
                        const char *title = json_object_get_string(collection, "title");
                        const int id = json_object_get_number(collection, "id");

                        if (title && id) {
                            printf("#%d: %s\n", id, title);
                        }
                    }

                    // Eliberam memoria JSON
                    json_value_free(response_value);
                } else {
                    // Nu a fost obtinut token JWT
                    printf("EROARE: Nu aveti acces la librarie!\n");
                }

            // Comanda “get_collection”: afiseaza detalii despre o colectie specifica
            } else if (!strncmp(command, "get_collection", 14)) {
                if (library_acces) {
                    // Citim ID-ul colectiei de la utilizator
                    char *id_string = malloc(17);
                    printf("id=");
                    fgets(id_string, 17, stdin);
                    id_string[strcspn(id_string, "\n")] = 0;

                    // Construim URL-ul cu ID-ul colectiei
                    char *url = malloc(100);
                    sprintf(url, GET_COLLECTION_URL, id_string);

                    // Trimitem cererea GET catre server
                    char *response = receive_get_request(sockfd, HOST, url,
                                                        cookies, token);
                    // Daca raspunsul are status 2xx, apelam functia care parseaza si afiseaza
                    if (http_status_is_success(response)) {
                        parse_collection(response);
                    } else if (strstr(response, "HTTP/1.1 500")) {
                        printf("EROARE: Server invalid\n");
                    } else {
                        printf("EROARE: ID-ul este invalid\n");
                    }

                    free(url);
                    free(id_string);
                } else {
                    printf("EROARE: Nu aveti acces la librarie!\n");
                }
            }

            else if (!strncmp(command, "add_collection", 14)) {
                // Citim titlul noii colectii
                char *title_collection = malloc(100);
                char *num_movies = malloc(17);
                char **movie_ids = malloc(1000 * sizeof(char *));
                printf("title=");
                fgets(title_collection, 100, stdin);
                title_collection[strcspn(title_collection, "\n")] = 0;
                // Citim numarul de filme de adaugat
                printf("num_movies=");
                fgets(num_movies, 17, stdin);
                num_movies[strcspn(num_movies, "\n")] = 0;

                int length = atoi(num_movies);
                // Alocam si citim ID-urile filmelor
                for (int i = 0; i < length; i++) {
                    printf("movie_id[%d]=" , i);
                    movie_ids[i] = malloc(17);
                    fgets(movie_ids[i], 17, stdin);
                    movie_ids[i][strcspn(movie_ids[i], "\n")] = 0;
                }
                // Construim JSON doar cu titlul colectiei
                JSON_Value *root_value = json_value_init_object();
                JSON_Object *root_object = json_value_get_object(root_value);
                json_object_set_string(root_object, "title", title_collection);
                
                collections[0] = json_serialize_to_string(root_value);
                // Trimitem POST pentru creare colectie 
                char *response = receive_post_request(sockfd, HOST, ADD_COLLECTION_URL, collections, cookies, 1, token);
                // Verificam daca a fost creat cu succes
                if (!http_status_is_success(response)) {
                    printf("EROARE: Date invalide (titlu)\n");
                } else {
                    // Extragem ID-ul colectiei nou create
                    char *id_collection = int_to_string(get_id_collection(response));
                    // Pentru fiecare film, trimitem POST catre adaugare in colectie
                    for (int i = 0; i < length; i++) {
                        
                        sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                        
                        char *url = malloc(100);
                        sprintf(url, ADD_MOVIE_TO_COLLECTION_URL, id_collection);

                        // Construim JSON payload cu ID-ul filmului
                        char **payload = malloc(sizeof(char *));
                        int id_movie = atoi(movie_ids[i]);
                        JSON_Value *val = json_value_init_object();
                        JSON_Object *obj = json_value_get_object(val);
                        json_object_set_number(obj, "id", id_movie);

                        payload[0] = json_serialize_to_string(val);

                        // Trimitem cererea POST pentru fiecare film
                        char *response = receive_post_request(sockfd, HOST, url, payload, cookies, 1, token);
                        if (strstr(response, "HTTP/1.1 500")){
                            printf("EROARE: Server invalid\n");
                        } else if (!http_status_is_success(response)) {
                            printf("EROARE: Filmul nu a putut fi adaugat, date invalide");
                        }
                    }
                    // Reafisam colectia finala cu toate filmele incluse
                    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
                    char *url = malloc(100);
                    sprintf(url, GET_COLLECTION_URL, id_collection);
                    response =receive_get_request(sockfd, HOST, url, cookies, token);

                    const char *json_start = strstr(response, "\r\n\r\n");
                    json_start += 4;

                    JSON_Value *root_value = json_parse_string(json_start);
                   

                    JSON_Object *root_object = json_value_get_object(root_value);  
                
                    const char *owner = json_object_get_string(root_object, "owner");
                    const char *title = json_object_get_string(root_object, "title");
                    
                    printf("SUCCESS: Colectie adaugata\n");

                    printf("title: %s\nowner: %s\n", title, owner);

                    
                    JSON_Array *movies = json_object_get_array(root_object, "movies");
                    size_t movie_count = json_array_get_count(movies);

                    for (size_t i = 0; i < movie_count; i++) {
                        JSON_Object *movie = json_array_get_object(movies, i);
                        int movie_id = (int)json_object_get_number(movie, "id");  
                        const char *movie_title = json_object_get_string(movie, "title");

                        printf("#%d: %s\n", movie_id, movie_title);
                    }

                    json_value_free(root_value);
                }
            }

            // Comanda “delete_collection”: sterge o colectie existenta
            else if (!strncmp(command, "delete_collection", 17)) {
                if (library_acces) {
                    // Citim ID-ul colectiei de sters
                    char *id_string = malloc(17);
                    printf("id=");
                    fgets(id_string, 17, stdin);
                    id_string[strcspn(id_string, "\n")] = 0;

                    // Construim URL-ul pentru DELETE cu ID-ul colectiei
                    char *url = malloc(100);
                    sprintf(url, DELETE_COLLECTION_URL, id_string);

                    // Trimitem cererea DELETE catre server
                    char *response = receive_delete_request(
                        sockfd, HOST, url, cookies, token
                    );

                    // Verificam statusul raspunsului si afisam mesaj potrivit
                    if (http_status_is_success(response)) {
                        printf("SUCCESS: Colectie stearsa\n");
                    } else if (strstr(response, "HTTP/1.1 500")) {
                        printf("EROARE: Server invalid\n");
                    } else {
                        // Fie ID invalid, fie userul nu e owner
                        printf("EROARE: ID-ul este invalid sau nu sunteti owner al acestei colectii\n");
                    }

                    free(id_string);
                    free(url);

                } else {
                    // Nu exista token JWT valid
                    printf("EROARE: Nu aveti acces la librarie!\n");
                }

            // Comanda “add_movie_to_collection”: adauga un film intr-o colectie existenta
            } else if (!strncmp(command, "add_movie_to_collection", 23)) {
                if (library_acces) {
                    // Citim ID-ul colectiei si apoi ID-ul filmului
                    char *id_collection = malloc(17);
                    char *id_movie      = malloc(17);

                    printf("collection_id=");
                    fgets(id_collection, 17, stdin);
                    id_collection[strcspn(id_collection, "\n")] = 0;

                    printf("movie_id=");
                    fgets(id_movie, 17, stdin);
                    id_movie[strcspn(id_movie, "\n")] = 0;

                    // Construim URL-ul POST pentru operatia de adaugare film
                    char *url = malloc(100);
                    sprintf(url, ADD_MOVIE_TO_COLLECTION_URL, id_collection);

                    // Construim payload JSON cu Parson: { "id": <movie_id> }
                    char **payload = malloc(sizeof(char *));
                    JSON_Value  *val = json_value_init_object();
                    JSON_Object *obj = json_value_get_object(val);
                    json_object_set_number(obj, "id", atoi(id_movie));
                    payload[0] = json_serialize_to_string(val);

                    // Trimitem POST catre server
                    char *response = receive_post_request(
                        sockfd, HOST, url, payload, cookies, 1, token
                    );

                    // Verificam rezultatul operatiei
                    if (http_status_is_success(response)) {
                        printf("SUCCESS: Film adaugat in colectie\n");
                    } else if (strstr(response, "HTTP/1.1 500")) {
                        printf("EROARE: Server invalid\n");
                    } else {
                        // Fie date invalide, fie nu e owner
                        printf("EROARE: Datele sunt invalide sau nu sunteti owner al acestei colectii\n");
                    }

                    // Eliberam memoria temporara
                    free(id_collection);
                    free(id_movie);
                    free(url);
                    free(payload);
                    json_value_free(val);

                } else {
                    printf("EROARE: Nu aveti acces la librarie!\n");
                }

            // Comanda “delete_movie_from_collection”: sterge un film dintr-o colectie
            } else if (!strncmp(command, "delete_movie_from_collection", 28)) {
                if (library_acces) {
                    // Citim ID-ul colectiei si ID-ul filmului
                    char *id_collection = malloc(17);
                    char *id_movie      = malloc(17);

                    printf("collection_id=");
                    fgets(id_collection, 17, stdin);
                    id_collection[strcspn(id_collection, "\n")] = 0;

                    printf("movie_id=");
                    fgets(id_movie, 17, stdin);
                    id_movie[strcspn(id_movie, "\n")] = 0;

                    // Construim URL-ul pentru DELETE cu ambele ID-uri
                    char *url = malloc(100);
                    sprintf(url, DELETE_MOVIE_FROM_COLLECTION_URL,
                            id_collection, id_movie);

                    // Trimitem cererea DELETE catre server
                    char *response = receive_delete_request(
                        sockfd, HOST, url, cookies, token
                    );

                    // Afisam mesaj in functie de rezultat
                    if (http_status_is_success(response)) {
                        printf("SUCCESS: Film sters din colectie\n");
                    } else if (strstr(response, "HTTP/1.1 500")) {
                        printf("EROARE: Server invalid\n");
                    } else {
                        // Fie ID invalid, fie nu e owner
                        printf("EROARE: ID-ul este invalid sau nu sunteti owner al acestei colectii\n");
                    }

                    // Eliberam memoria temporara
                    free(id_collection);
                    free(id_movie);
                    free(url);

                } else {
                    printf("EROARE: Nu aveti acces la librarie!\n");
                }

            // Comanda necunoscuta: afisam mesaj generic de eroare
            } else {
                printf("EROARE: Comanda necunoscuta %s", command);
            }

            // La sfarsit, redeschidem conexiunea pentru urmatoarea comanda
            sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
        }
    }
}
