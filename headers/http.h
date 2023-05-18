#ifndef HTTP_H
#define HTTP_H

unsigned char http_recv(int main_socket,char *http_response,char **body_location);
unsigned char http_write(int main_socket, char *http_request);
unsigned char http_connect(int *main_socket,char *node_ip_address,char *port);


#endif
