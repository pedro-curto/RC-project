#include "client.h"

// ---------------------------- Auxiliary functions ----------------------------

void connect_UDP(char* IP, char* port, char* request, char* buffer) { // FIXME incluir um campo buffer_size? 
    int fd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    //printf("connect_UDP request: %s\n", request);
    // TODO devemos permitir que a função continue se qualquer um dos perrors ocorrer?
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) perror("Error creating socket.");
    memset(&hints, 0, sizeof hints);
    memset(&addr, 0, sizeof addr);
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket
    errcode = getaddrinfo(IP, port, &hints, &res);
    if (errcode != 0) perror("Error getting address info."); 
    n = sendto(fd, request, strlen(request), 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) perror("Error sending request.");
    addrlen = sizeof(addr);
    // usar sempre este buffer size? talvez só no list
    n = recvfrom(fd, buffer, 4095, 0, (struct sockaddr*) &addr, &addrlen);
    if (n == -1) perror("Error receiving response.");
    buffer[n] = '\0';
    freeaddrinfo(res);
    close(fd);
}


void connect_TCP(char* IP, char* port, char* request, char* buffer, size_t buffer_size) {
    int fd, asset_fd, fsize, to_read;//,  errcode;
    char asset_fname[ASSET_FNAME_SIZE + 1];
    ssize_t n;
    struct addrinfo hints, *res;
    //socklen_t addrlen;
    //struct sockaddr_in addr;
    // OPA UID password name start_value timeactive Fname Fsize Fdata OPA %s %s %s %d %d %s %d\n
    /*if (!strncmp(request, "OPA", 3)) { 
        openAuc = 1;
        sscanf(request, "OPA %*s %*s %*s %*d %*d %s %d", asset_fname, &fsize);
        //printf("connect_TCP asset_fname: %s, fsize: %d\n", asset_fname, fsize);
    }*/
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) perror("Error creating socket.\n");
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    if (getaddrinfo(IP, port, &hints, &res) != 0) printf("Error getting address info.\n");
    if (connect(fd, res->ai_addr, res->ai_addrlen) == -1) printf("Error connecting.\n");
    // writes the header in open
    if (write(fd, request, strlen(request)) == -1) perror("Error writing.\n");
    if (!strncmp(request, "OPA", 3)) {
        sscanf(request, "OPA %*s %*s %*s %*d %*d %s %d", asset_fname, &fsize); 
        // uses sendfile() to send the image
        asset_fd = open(asset_fname, O_RDONLY);
        if (asset_fd == -1) perror("Error opening file.\n");
        off_t offset = 0;
        ssize_t sent_bytes = sendfile(fd, asset_fd, &offset, fsize);
        if (sent_bytes == -1) perror("Error sending file.\n");
        if (write(fd, "\n", 1) == -1) perror("Error writing.\n");
        close(asset_fd);
    } else if (!strncmp(request, "SAS", 3)) {
        // reads the first four bytes, guaranteed to be SAS and a space
        to_read = 4;
        while (to_read > 0) {
            n = read(fd, buffer, 1);
            if (n == -1) perror("Error reading.\n");
            to_read -= n;
        }

    }
    n = read(fd, buffer, buffer_size - 1);
    if (n == -1) perror("Error reading.\n");
    buffer[n] = '\0';
    freeaddrinfo(res);
    close(fd);
    //return buffer;
}


int valid_filename(char *filename) {
    if (strlen(filename) > ASSET_FNAME_SIZE) return -1;
    if (strspn(filename, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.")) return -1;
    char *extension = strrchr(filename, '.');
    if (!extension || strlen(extension) != 4) return -1;
    if (!isalpha(extension[1]) || !isalpha(extension[2]) || !isalpha(extension[3])) return -1;
    return 1;
}

int getFileSize(char *filename) {
    struct stat st;
    int ret_stat;
    ret_stat = stat(filename, &st);
    if (ret_stat == -1 || st.st_size == 0) return -1;
    return st.st_size;
}


void SA_connect_TCP(char* IP, char* port, char* request, char* buffer, size_t buffer_size) {
    int fd, asset_fd, fsize, to_read, to_write;//,  errcode;
    char *ptr = buffer, asset_fname[ASSET_FNAME_SIZE + 1];
    ssize_t n;
    struct addrinfo hints, *res;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) perror("Error creating socket.\n");
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    if (getaddrinfo(IP, port, &hints, &res) != 0) printf("Error getting address info.\n");
    if (connect(fd, res->ai_addr, res->ai_addrlen) == -1) printf("Error connecting.\n");
    // writes the header in open
    n = write(fd, request, strlen(request));
    if (n == -1) perror("Error writing.\n");
    to_read = SA_RESPONSE_HEADER;
    while (to_read > 0) {
        n = read(fd, ptr, to_read);
        if (n == -1) perror("Error reading.\n");
        else if (n == 0) break;
        to_read -= n;
        ptr += n;
    }
    /*printf("SA_connect_TCP buffer after header: %s\n", buffer);
    printf("n: %d\n", n);
    printf("ptr: %s\n", ptr);*/

    if (strncmp(buffer, "RSA OK ", 7)) {
        printf("Answer was not RSA OK.\n");
        freeaddrinfo(res);
        close(fd);
        return;
    }
    char *temp_ptr = buffer;
    sscanf(temp_ptr, "RSA OK %24s %8d%n", asset_fname, &fsize, &n);
    temp_ptr += n;
    printf("SA_connect_TCP asset_fname: %s, fsize: %d\ntemp_ptr: %s\nn: %d", asset_fname, fsize, temp_ptr, n);
    char chunk[512];
    FILE *file = fopen(asset_fname, "wb");
    fwrite(temp_ptr, 1, n, file);
    size_t nleft = fsize;
    printf("check1\n");
    while (nleft > 0) {
        size_t to_read = (nleft < 512) ? nleft : 512;
        n = read(fd, temp_ptr, to_read);
        if (n == -1) {
            perror("Error reading Fdata.\n");
            fclose(file);
            close(fd);
            freeaddrinfo(res);
            return;
        }

        fwrite(temp_ptr, 1, n, file);
        temp_ptr += n;
        nleft -= n;
    }
    printf("check2\n");
    fclose(file);

    buffer[n] = '\0';
    freeaddrinfo(res);
    close(fd);
}



/*void SA_connect_TCP(char* IP, char* port, char* request, char* buffer, size_t buffer_size) {
    int fd, asset_fd, fsize, to_read, to_write;//,  errcode;
    char asset_fname[ASSET_FNAME_SIZE + 1];
    ssize_t n;
    struct addrinfo hints, *res;
    // OPA UID password name start_value timeactive Fname Fsize Fdata OPA %s %s %s %d %d %s %d\n
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) perror("Error creating socket.\n");
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    if (getaddrinfo(IP, port, &hints, &res) != 0) printf("Error getting address info.\n");
    if (connect(fd, res->ai_addr, res->ai_addrlen) == -1) printf("Error connecting.\n");
    // writes the header in open
    to_write = strlen(request);
    while (to_write > 0) {
        n = write(fd, request, to_write);
        if (n == -1) perror("Error writing.\n");
        to_write -= n;
        request += n;
    }
    // reads the first four bytes, guaranteed to be SAS and a space
    to_read = 4;
    while (to_read > 0) {
        n = read(fd, buffer, to_read);
        if (n == -1) perror("Error reading.\n");
        to_read -= n;
        buffer += n;
    }
    printf("SA_connect_TCP buffer after 4 bytes: %s\n", buffer);
    if (!strncmp(buffer, "SAS ", 4)) {
        to_read = 3;
        while (to_read > 0) {
            n = read(fd, buffer, to_read);
            if (n == -1) perror("Error reading.\n");
            to_read -= n;
            buffer += n;
        }
        printf("SA_connect_TCP buffer after 7 bytes: %s\n", buffer);
        if (!strncmp(buffer, ))

    }
    

    n = read(fd, buffer, buffer_size - 1);
    if (n == -1) perror("Error reading.\n");
    buffer[n] = '\0';
    freeaddrinfo(res);
    close(fd);
    //return buffer;
}*/


/*int readFile(char *filename, char *buffer, int size) {
    //size_t bufferSize = 100;
    int fd = open(filename, O_RDONLY);
    if (fd == -1) { 
        perror("Error opening file");
        return -1;
    }
    ssize_t n = read(fd, buffer, size);
    close(fd);
    if (n == -1) {
        perror("Error reading file");
        return -1;
    }
    return 0;
}*/


/*
nleft = nbytes;
while (nleft > 0) {
    nwritten = write(fd, ptr, nleft);
    if (nwritten <= 0) perror("Error writing.\n");
    nleft -= nwritten;
    ptr += nwritten;
}
nleft = nbytes2; 
ptr = buffer;

while (nleft > 0) {
    nread = read(fd, ptr, nleft);
    if (nread == -1) perror("Error reading.\n");
    else if (nread == 0) break;
    nleft -= nread;
    ptr += nread;
}
nread = nbytes2 - nleft;


*/