//Cernea Mihnea-Ioan 324CB
#ifndef CLIENT_FUNCTIONS_H
#define CLIENT_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>    
#include <string.h>    
#include <arpa/inet.h>  
#include <stdbool.h>   

#include "parson.h"   
#include "requests.h"   
#include "helpers.h" 

#define HOST "63.32.125.183"
#define PORT 8081

// URL-uri pentru operatiuni admin
#define LOGIN_ADMIN_URL "/api/v1/tema/admin/login"       
#define LOGOUT_ADMIN_URL "/api/v1/tema/admin/logout"     
#define REGISTER_USER_URL "/api/v1/tema/admin/users"     
#define VISUALIZE_USERS_URL "/api/v1/tema/admin/users"   
#define DELETE_USER_URL "/api/v1/tema/admin/users/%s"    

// URL-uri pentru operatiuni user
#define LOGIN_USER_URL "/api/v1/tema/user/login"        
#define LOGOUT_USER_URL "/api/v1/tema/user/logout"       

// URL-uri pentru biblioteca si filme
#define GET_ACCESS_URL "/api/v1/tema/library/access"
#define GET_MOVIES_URL "/api/v1/tema/library/movies"
#define GET_MOVIE_URL "/api/v1/tema/library/movies/%s" 
#define ADD_MOVIE_URL "/api/v1/tema/library/movies" 
#define UPDATE_MOVIE_URL "/api/v1/tema/library/movies/%s"
#define DELETE_MOVIE_URL "/api/v1/tema/library/movies/%s"

// URL-uri pentru colectii
#define GET_COLLECTIONS_URL "/api/v1/tema/library/collections"              
#define ADD_COLLECTION_URL "/api/v1/tema/library/collections"                
#define GET_COLLECTION_URL "/api/v1/tema/library/collections/%s"              
#define DELETE_COLLECTION_URL "/api/v1/tema/library/collections/%s"          
#define ADD_MOVIE_TO_COLLECTION_URL "/api/v1/tema/library/collections/%s/movies"      
#define DELETE_MOVIE_FROM_COLLECTION_URL "/api/v1/tema/library/collections/%s/movies/%s"

// Stari autentificare
#define LOGOUT 0 // Neautentificat
#define ADMIN_LOGGEDIN 1 // Admin autentificat
#define USER_LOGGEDIN 2 // User autentificat

// Trimite cerere POST si returneaza raspunsul serverului
char *receive_post_request(int sockfd, char *host, char *url,
                           char *user[1], char **cookie,
                           int nmb_cookie, char *token);

// Trimite cerere PUT si returneaza raspunsul serverului

char *receive_put_request(int sockfd, char *host, char *url,
                          char *user[1], char **cookie,
                          int nmb_cookie, char *token);

// Trimite cerere GET si returneaza raspunsul serverului
char *receive_get_request(int sockfd, char *host,
                          char *url, char *cookies[1],
                          char *token);

//Trimite cerere DELETE si returneaza raspunsul serverului
char *receive_delete_request(int sockfd, char *host,
                             char *url, char *cookies[1],
                             char *token);

/**
 * Verifica daca status HTTP este SUCCESS
 */
bool http_status_is_success(const char *http_response);

// Extrage token JWT din corpul JSON al raspunsului
void extract_token(char *response, char *value_token);

// Obtine token JWT prin cerere GET la GET_ACCESS_URL
void obtain_jwt_token(int sockfd, char *host,
                      char **cookies, int nmb_cookie,
                      char *token);

/**
 * Verifica existenta antetului Set-Cookie in raspuns
 * Returneaza true daca gaseste un cookie valid
 */
bool valid_cookie(char *response);

// Citeste de la stdin date pentru crearea unui user si returneaza JSON
char *add_user();

// Parseaza raspuns JSON cu utilizatori si afiseaza lista
void parse_and_print_users(const char *response);

// Parseaza raspuns JSON cu filme si afiseaza lista
void parse_and_print_movies(const char *response);

// Parseaza corp JSON pentru o colectie si afiseaza detaliile

void parse_collection(char *response);

// Extrage ID colectie din raspunsul JSON
const int get_id_collection(char *response);

// Converteste un int la string C (aloca dinamica pentru buffer)

char *int_to_string(const int value);

#endif // CLIENT_FUNCTIONS_H
