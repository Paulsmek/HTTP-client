#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "buffer.h"

#define HOST "34.246.184.49"
#define PORT 8080
#define BUFLEN 4096

char *session_cookie = NULL;
char *library_token = NULL;

void parse_and_print_books(const char *json_str) {
    const char *pos = json_str;
    while ((pos = strstr(pos, "{\"id\":")) != NULL) {
        pos += 6; // Move past the "{\"id\":"

        // Extract the id
        int id = atoi(pos);
        printf("id: %d\n", id);

        // Extract the title
        pos = strstr(pos, "\"title\":\"");
        if (pos == NULL) break;
        pos += 9; // Move past the "\"title\":\""

        const char *title_end = strchr(pos, '\"');
        if (title_end == NULL) break;

        printf("title: ");
        while (pos < title_end) {
            putchar(*pos++);
        }
        putchar('\n');

        // Move past the rest of the book object
        pos = strchr(pos, '}');
        if (pos == NULL) break;
    }
}

void parse_and_print_book_details(const char *json_str) {
    const char *id_str = "\"id\":";
    const char *title_str = "\"title\":\"";
    const char *author_str = "\"author\":\"";
    const char *publisher_str = "\"publisher\":\"";
    const char *genre_str = "\"genre\":\"";
    const char *page_count_str = "\"page_count\":";

    const char *pos = json_str;

    // Extract id
    pos = strstr(pos, id_str);
    if (pos) {
        pos += strlen(id_str);
        int id = atoi(pos);
        printf("id: %d\n", id);
    }

    // Extract title
    pos = strstr(pos, title_str);
    if (pos) {
        pos += strlen(title_str);
        const char *end = strchr(pos, '\"');
        if (end) {
            printf("title: ");
            while (pos < end) {
                putchar(*pos++);
            }
            putchar('\n');
        }
    }

    // Extract author
    pos = strstr(pos, author_str);
    if (pos) {
        pos += strlen(author_str);
        const char *end = strchr(pos, '\"');
        if (end) {
            printf("author: ");
            while (pos < end) {
                putchar(*pos++);
            }
            putchar('\n');
        }
    }

    // Extract publisher
    pos = strstr(pos, publisher_str);
    if (pos) {
        pos += strlen(publisher_str);
        const char *end = strchr(pos, '\"');
        if (end) {
            printf("publisher: ");
            while (pos < end) {
                putchar(*pos++);
            }
            putchar('\n');
        }
    }

    // Extract genre
    pos = strstr(pos, genre_str);
    if (pos) {
        pos += strlen(genre_str);
        const char *end = strchr(pos, '\"');
        if (end) {
            printf("genre: ");
            while (pos < end) {
                putchar(*pos++);
            }
            putchar('\n');
        }
    }

    // Extract page count
    pos = strstr(pos, page_count_str);
    if (pos) {
        pos += strlen(page_count_str);
        int page_count = atoi(pos);
        printf("page_count: %d\n", page_count);
    }
}

void registerUser() {
    int ok = 0;
    char username[256], password[256];
    printf("username=");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;
    // Check if has space
    if (strchr(username, ' ') != NULL) {
        ok = 1;
    }

    printf("password=");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;
    // Check if has space
    if (strchr(password, ' ') != NULL) {
        ok = 1;
    }

    // Create JSON payload
    char payload[512];
    snprintf(payload, sizeof(payload), "{\"username\":\"%s\",\"password\":\"%s\"}", username, password);

    // Prepare the HTTP request
    char message[BUFLEN];
    snprintf(message, sizeof(message),
             "POST /api/v1/tema/auth/register HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %ld\r\n"
             "\r\n"
             "%s",
             HOST, strlen(payload), payload);

    // Open connection to the server
    int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    // Send the request
    send_to_server(sockfd, message);

    // Receive the response
    char *response = receive_from_server(sockfd);

    // Close the connection
    close_connection(sockfd);

    // Check the response
    if (ok == 1) {
        // Spaces in username/password
        printf("ERROR: Fara spatii in username sau password!\n");
    } else if (strstr(response, "201 Created") != NULL) {
        printf("SUCCESS: Utilizator înregistrat cu succes!\n");
    } else if (strstr(response, "400 Bad Request") != NULL) {
        printf("ERROR: Username-ul este deja folosit de către cineva!\n");
    } else if (strstr(response, "204 No Content") != NULL) {
        printf("ERROR: Fara spatii in username sau password!\n");
    } else {
        printf("ERROR: Eroare necunoscută\n");
    }

    // Free the response buffer
    free(response);
}

void loginUser() {
    char username[256], password[256];
    printf("username=");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    printf("password=");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    // Create JSON payload
    char payload[512];
    snprintf(payload, sizeof(payload), "{\"username\":\"%s\",\"password\":\"%s\"}", username, password);

    // Prepare the HTTP request
    char message[BUFLEN];
    snprintf(message, sizeof(message),
             "POST /api/v1/tema/auth/login HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %ld\r\n"
             "\r\n"
             "%s",
             HOST, strlen(payload), payload);

    // Open connection to the server
    int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    // Send the request
    send_to_server(sockfd, message);

    // Receive the response
    char *response = receive_from_server(sockfd);

    // Close the connection
    close_connection(sockfd);

    // Extract the session cookie
    char *cookie_start = strstr(response, "Set-Cookie: ");
    if (cookie_start) {
        cookie_start += 12; // Move past "Set-Cookie: "
        char *cookie_end = strstr(cookie_start, ";");
        if (cookie_end) {
            size_t cookie_len = cookie_end - cookie_start;
            session_cookie = malloc(cookie_len + 1);
            strncpy(session_cookie, cookie_start, cookie_len);
            session_cookie[cookie_len] = '\0';
            printf("SUCCESS: Utilizatorul a fost logat cu succes\n");
        }
    } else if (strstr(response, "400 Bad Request") != NULL) {
        printf("ERROR: Credențialele nu se potrivesc\n");
    } else if (strstr(response, "204 No Content") != NULL) {
        printf("ERROR: Fara spatii in username sau password!\n");
    } else {
        printf("ERROR: Eroare necunoscută\n");
    }

    // Free the response buffer
    free(response);
}


void enterLibrary() {
    if (session_cookie == NULL) {
        printf("ERROR: Trebuie să fiți autentificat pentru a accesa biblioteca\n");
        return;
    }

    // Prepare the HTTP request
    char message[BUFLEN];
    snprintf(message, sizeof(message),
             "GET /api/v1/tema/library/access HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Cookie: %s\r\n"
             "\r\n",
             HOST, session_cookie);

    // Open connection to the server
    int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    // Send the request
    send_to_server(sockfd, message);

    // Receive the response
    char *response = receive_from_server(sockfd);

    // Close the connection
    close_connection(sockfd);

    // Extract the library access token
    char *token_start = strstr(response, "{\"token\":\"");
    if (token_start) {
        token_start += 10; // Move past "{\"token\":\""
        char *token_end = strstr(token_start, "\"}");
        if (token_end) {
            size_t token_len = token_end - token_start;
            library_token = malloc(token_len + 1);
            strncpy(library_token, token_start, token_len);
            library_token[token_len] = '\0';
            printf("SUCCESS: Utilizatorul are acces la biblioteca\n");
        }
    } else {
        printf("ERROR: Eroare la accesarea bibliotecii\n");
    }

    // Free the response buffer
    free(response);
}

void getBooks() {
    if (library_token == NULL) {
        printf("ERROR: Trebuie să aveți acces la bibliotecă pentru a obține lista de cărți\n");
        return;
    }

    // Prepare the HTTP request
    char message[BUFLEN];
    snprintf(message, sizeof(message),
             "GET /api/v1/tema/library/books HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Authorization: Bearer %s\r\n"
             "\r\n",
             HOST, library_token);

    // Open connection to the server
    int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    // Send the request
    send_to_server(sockfd, message);

    // Receive the response
    char *response = receive_from_server(sockfd);

    // Close the connection
    close_connection(sockfd);

    // Print the response
    if (strstr(response, "200 OK") != NULL) {
        printf("SUCCESS:\n");
        char *json_start = strstr(response, "\r\n\r\n");
        if (json_start) {
            json_start += 4; // Move past the "\r\n\r\n"
            parse_and_print_books(json_start);
        } else {
            printf("ERROR: Failed to find JSON part in the response\n");
        }
    } else {
        printf("ERROR: Eroare la obținerea listei de cărți\n");
    }

    // Free the response buffer
    free(response);
}

void getBook() {
    if (library_token == NULL) {
        printf("ERROR: Trebuie să aveți acces la bibliotecă pentru a obține detalii despre o carte\n");
        return;
    }

    char book_id[256];
    printf("id=");
    fgets(book_id, sizeof(book_id), stdin);
    book_id[strcspn(book_id, "\n")] = 0;

    // Prepare the HTTP request
    char message[BUFLEN];
    snprintf(message, sizeof(message),
             "GET /api/v1/tema/library/books/%s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Authorization: Bearer %s\r\n"
             "\r\n",
             book_id, HOST, library_token);

    // Open connection to the server
    int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    // Send the request
    send_to_server(sockfd, message);

    // Receive the response
    char *response = receive_from_server(sockfd);

    // Close the connection
    close_connection(sockfd);

    // Check the response
    if (strstr(response, "200 OK") != NULL) {
        printf("SUCCESS: \n");
        char *json_start = strstr(response, "\r\n\r\n");
        if (json_start) {
            json_start += 4; // Move past the "\r\n\r\n"
            parse_and_print_book_details(json_start);
        } else {
            printf("ERROR: Failed to find JSON part in the response\n");
        }
    } else if (strstr(response, "404 Not Found") != NULL) {
        printf("ERROR: Cartea cu id=%s nu există!\n", book_id);
    } else {
        printf("ERROR: Eroare necunoscută\n");
    }

    // Free the response buffer
    free(response);
}

void addBook() {
    int bad_fields = 0;
    if (library_token == NULL) {
        printf("ERROR: Trebuie să aveți acces la bibliotecă pentru a adăuga o carte\n");
        return;
    }

    char title[256], author[256], genre[256], publisher[256], page_count[256];
    printf("title=");
    fgets(title, sizeof(title), stdin);
    title[strcspn(title, "\n")] = 0;
    if (strlen(title) == 0) {
        bad_fields = 1;
    }

    printf("author=");
    fgets(author, sizeof(author), stdin);
    author[strcspn(author, "\n")] = 0;
    if (strlen(author) == 0) {
        bad_fields = 1;
    }

    printf("genre=");
    fgets(genre, sizeof(genre), stdin);
    genre[strcspn(genre, "\n")] = 0;
    if (strlen(genre) == 0) {
        bad_fields = 1;
    }

    printf("publisher=");
    fgets(publisher, sizeof(publisher), stdin);
    publisher[strcspn(publisher, "\n")] = 0;
    if (strlen(publisher) == 0) {
       bad_fields = 1;
    }

    printf("page_count=");
    fgets(page_count, sizeof(page_count), stdin);
    page_count[strcspn(page_count, "\n")] = 0;
    if (strlen(page_count) == 0) {
        bad_fields = 1;
    }

    // Validate page_count
    char *endptr;
    long pages = strtol(page_count, &endptr, 10);
    if (*endptr != '\0' || pages <= 0) {
        printf("ERROR: Tip de date incorect pentru numărul de pagini\n");
        return;
    }

    // Create JSON payload
    char payload[512];
    snprintf(payload, sizeof(payload), "{\"title\":\"%s\",\"author\":\"%s\",\"genre\":\"%s\",\"page_count\":%ld,\"publisher\":\"%s\"}",
             title, author, genre, pages, publisher);

    // Prepare the HTTP request
    char message[BUFLEN];
    snprintf(message, sizeof(message),
             "POST /api/v1/tema/library/books HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %ld\r\n"
             "Authorization: Bearer %s\r\n"
             "\r\n"
             "%s",
             HOST, strlen(payload), library_token, payload);

    // Open connection to the server
    int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    // Send the request
    send_to_server(sockfd, message);

    // Receive the response
    char *response = receive_from_server(sockfd);

    // Close the connection
    close_connection(sockfd);

    // Check the response
    if (bad_fields == 1) {
        // if a book field is empty
        printf("ERROR: Nu sunt suficiente date despre carte\n");
    }
    if (strstr(response, "200 OK") != NULL) {
        printf("SUCCESS: Cartea a fost adăugată cu succes!\n");
    } else {
        printf("ERROR: Eroare la adăugarea cărții\n");
    }

    // Free the response buffer
    free(response);
}


void deleteBook() {
    if (library_token == NULL) {
        printf("ERROR: Trebuie să aveți acces la bibliotecă pentru a șterge o carte\n");
        return;
    }

    char book_id[256];
    printf("id=");
    fgets(book_id, sizeof(book_id), stdin);
    book_id[strcspn(book_id, "\n")] = 0;

    // Prepare the HTTP request
    char message[BUFLEN];
    snprintf(message, sizeof(message),
             "DELETE /api/v1/tema/library/books/%s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Authorization: Bearer %s\r\n"
             "\r\n",
             book_id, HOST, library_token);

    // Open connection to the server
    int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    // Send the request
    send_to_server(sockfd, message);

    // Receive the response
    char *response = receive_from_server(sockfd);

    // Close the connection
    close_connection(sockfd);

    // Check the response
    if (strstr(response, "200 OK") != NULL) {
        printf("SUCCESS: Cartea cu id %s a fost stearsa cu succes!\n", book_id);
    } else {
        printf("ERROR: Eroare la ștergerea cărții\n");
    }

    // Free the response buffer
    free(response);
}

void logoutUser() {
    if (session_cookie == NULL) {
        printf("ERROR: Trebuie să fiți autentificat pentru a vă deloga\n");
        return;
    }

    // Prepare the HTTP request
    char message[BUFLEN];
    snprintf(message, sizeof(message),
             "GET /api/v1/tema/auth/logout HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Cookie: %s\r\n"
             "\r\n",
             HOST, session_cookie);

    // Open connection to the server
    int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    // Send the request
    send_to_server(sockfd, message);

    // Receive the response
    char *response = receive_from_server(sockfd);

    // Close the connection
    close_connection(sockfd);

    // Check the response
    if (strstr(response, "200 OK") != NULL) {
        printf("SUCCESS: Utilizatorul a fost delogat cu succes!\n");
        free(session_cookie);
        session_cookie = NULL;
        if (library_token != NULL) {
            free(library_token);
            library_token = NULL;
        }
    } else {
        printf("ERROR: Eroare la delogare\n");
    }

    // Free the response buffer
    free(response);
}

int main() {
    char command[256];

    while (1) {
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;
        // Handle commands
        if (strcmp(command, "register") == 0) {
            registerUser();
        } else if (strcmp(command, "login") == 0) {
            loginUser();
        } else if (strcmp(command, "enter_library") == 0) {
            enterLibrary();
        } else if (strcmp(command, "get_books") == 0) {
            getBooks();
        } else if (strcmp(command, "get_book") == 0) {
            getBook();
        } else if (strcmp(command, "add_book") == 0) {
            addBook();
        } else if (strcmp(command, "delete_book") == 0) {
            deleteBook();
        } else if (strcmp(command, "logout") == 0) {
            logoutUser();
        } else if (strcmp(command, "exit") == 0) {
            break;
        } else {
            printf("Unknown command\n");
        }
    }
    // free cookie/token
    if (session_cookie != NULL) {
        free(session_cookie);
    }
    if (library_token != NULL) {
        free(library_token);
    }

    return 0;
}
