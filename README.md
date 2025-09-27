# Web Client 

## Description
The Web Client application allows interaction with a REST API for user management, authentication, JWT token handling, and operations on movies and collections. All HTTP requests are manually built and sent over a TCP socket.

## Functionality
1. **Authentication and Session**  
   - `login_admin <admin_username> <admin_password>`: obtain a session cookie for the admin  
   - `login <admin_username> <username> <password>`: obtain a session cookie for a user  
   - `logout_admin`: close the admin session  
   - `logout`: close the user session  
   - `get_access`: retrieve a JWT token for library operations  

2. **User Management (admin only)**  
   - `add_user <username> <password>`: add a new user  
   - `get_users`: list all users (requires cookie)  
   - `delete_user <id>`: delete user by id  

3. **Movie Operations**  
   - `get_movies`: retrieve the complete list of movies  
   - `get_movie <id>`: retrieve details of a specific movie  
   - `add_movie <title> <year> <description> <review>`: add a new movie  
   - `delete_movie <id>`: delete a movie by id  
   - `update_movie <id> <new_title> <new_year> <new_description> <new_review>`: update movie information  

4. **Movie Collection Operations**  
   - `get_collections`: list all collections  
   - `get_collection <id>`: get details of a specific collection  
   - `add_collection <title> <initial_number_of_movies> <movie_data_set>`: add a new collection with an initial set of movies  
   - `delete_collection <id>`: delete a collection  
   - `add_movie_to_collection <collection_id> <movie_id>`: add a movie to a collection  
   - `delete_movie_from_collection <collection_id> <movie_id>`: remove a movie from a collection  

## General Workflow
1. Open a TCP connection to the server (host, port).  
2. Read the command from stdin.  
3. After identifying the command, build the HTTP request (GET, POST, DELETE, PUT) using functions from `requests.c` and `helpers.c`.  
4. For requests with a JSON body, use the Parson library to create and serialize JSON objects.  
5. Send the message to the server via socket and receive the full response.  
6. Parse the headers for status, cookie, or token; process the JSON body with Parson.  
7. Display the result in the console.  
8. Reopen the connection for the next command.  

## JSON Parsing Library
- We use [Parson](https://github.com/kgabis/parson) for:  
  - Dynamic construction of JSON objects (`json_value_init_object`, `json_object_set_*`)  
  - Serialization into strings and pretty strings (for displaying messages in JSON format) (`json_serialize_to_string`, `json_serialize_to_string_pretty`)  
  - Parsing JSON responses (`json_parse_string`, `json_value_get_object`, `json_object_get_*`, `json_object_get_array`)  
- Justification: Parson is a lightweight C library, very useful especially for extracting movie arrays via `json_object_get_array`. It greatly simplified all JSON file processing.  

## File Structure
- **client.c**, **client_functions.c**, **requests.c**: main client code and HTTP communication functions  
- **helpers.c**, **helpers.h**, **buffer.c**, **buffer.h**, **requests.h**
  - Base library for HTTP messages, data buffers, and utility functions  
  - **client_functions.h**: prototypes for request and JSON parsing functions  
  - **requests.h**: prototypes for HTTP request construction functions  
  - **helpers.h**: declarations for socket and buffer functions  

## Error Handling
I aimed to handle all possible situations in communication with the server and command processing. The client code manages the following error types:

1. **HTTP Response Errors (status codes)**  
   - **500 Internal Server Error**  
     If the response contains `"HTTP/1.1 500"`, display:  
     ```c
     "ERROR: Invalid server";
     ```
   - **Invalid JWT Token**  
     - For operations requiring a JWT token (e.g., `get_movies`, `add_movie`, `delete_movie`, `update_movie`, `get_collections`, etc.)  
     - In `get_access`, if the token is missing or invalid:  
       ```c
       "ERROR: Invalid token, access denied."
       "ERROR: You do not have access to the library!"
       ```
   - **Invalid Data**  
     If submitted data is incomplete or incorrect (e.g., login/register for user/admin, add movie):  
     ```c
     "ERROR: Invalid or incomplete data."
     ```

2. **Authentication / Session Errors**  
   - If a user or admin tries to execute a command without being logged in:  
     ```c
     "ERROR: Not logged in"
     "ERROR: Already logged in as admin/user"
     ```
   - If a command is executed without sufficient rights (e.g., `add_user` by a normal user):  
     ```c
     "ERROR: Permission denied."
     "ERROR: Not logged in as admin"
     ```

3. **Command Usage Errors**  
   - Unknown command or wrong number of arguments:  
     ```c
     "ERROR: Unknown command <command>";
     ```

## Notes 
- The Parson library was taken from [https://github.com/kgabis/parson](https://github.com/kgabis/parson). 

## Credits

Author: Cernea Mihnea-Ioan
