#include "server.h"

void handle_login(int udp_socket, struct sockaddr_in client_addr, char* buffer, socklen_t client_addr_len) {
    char status[50] = "RLI ";
    char uid[UID_SIZE + 1];
    char password[PASSWORD_SIZE + 1];

    if (!read_uid_udp(buffer, uid) || !read_password_udp(buffer, password)) {
        strcat(status, "NOK\n");
        reply_msg(udp_socket, client_addr, client_addr_len, status);
        return;
    }

    if (verbose_mode){
        printf("Request type: Login\nuid: %s\n", uid);
    }


    //pthread_mutex_lock(&mutex);
    if (!verify_user_exists(uid)) {
        if (create_user(uid,password)) {
            strcat(status, "REG\n");
        } else {
            strcat(status, "NOK\n");
        }
    } else {
        //printf("before password\n");
        if (!verify_password_correct(uid, password)) {
            strcat(status, "NOK\n");
        } else if (is_user_login(uid)) {
            strcat(status, "NOK\n");
        } else {
            //printf("before change_user_login\n");
            change_user_login(uid);
            strcat(status, "OK\n");
        }
    }
    //pthread_mutex_unlock(&mutex);
    reply_msg(udp_socket, client_addr, client_addr_len, status);
}


void handle_logout(int udp_socket, struct sockaddr_in client_addr, char* buffer, socklen_t client_addr_len) {
    char uid[UID_SIZE + 1], password[PASSWORD_SIZE + 1];
    char status[50] = "RLO ";
    // sscanf(buffer, "LOU %s %s", uid, password);
    // uid[strlen(uid)] = '\0';
    // password[strlen(password)] = '\0';

    if (!read_uid_udp(buffer, uid) || !read_password_udp(buffer, password)) {
        strcat(status, "NOK\n");
        reply_msg(udp_socket, client_addr, client_addr_len, status);
        return;
    }

    if (verbose_mode){
        printf("Request type: Logout\nuid: %s\n", uid);
    }

    //pthread_mutex_lock(&mutex);
    if(!verify_user_exists(uid)){
        strcat(status, "UNR\n");
    } else{
        if (!verify_password_correct(uid, password)){
            strcat(status, "NOK\n");
        } else if (!is_user_login(uid)){
            strcat(status, "NOK\n");
        } else{
            change_user_login(uid);
            strcat(status, "OK\n");
        }
    }
    //pthread_mutex_unlock(&mutex);
    reply_msg(udp_socket, client_addr, client_addr_len, status);
}


void handle_unregister(int udp_socket, struct sockaddr_in client_addr, char* buffer, socklen_t client_addr_len) {
    char uid[UID_SIZE + 1], password[PASSWORD_SIZE + 1];
    char status[50] = "RUR ";
    // sscanf(buffer, "UNR %s %s", uid, password);
    // uid[strlen(uid)] = '\0';

    if (!read_uid_udp(buffer, uid) || !read_password_udp(buffer, password)) {
        strcat(status, "NOK\n");
        reply_msg(udp_socket, client_addr, client_addr_len, status);
        return;
    }

    if (verbose_mode){
        printf("Request type: Unregister\nuid: %s\n", uid);
    }

    //pthread_mutex_lock(&mutex);
    if (!verify_user_exists(uid)) {
        strcat(status, "NOK\n");
    } else {
        if (!verify_password_correct(uid, password)) {
            strcat(status, "NOK\n");
        } else if (!is_user_login(uid)) {
            strcat(status, "NOK\n");
        } else {
            delete_user(uid);
            strcat(status, "OK\n");
        }
    }
    //pthread_mutex_unlock(&mutex);
    reply_msg(udp_socket, client_addr, client_addr_len, status);
}



void handle_myauctions(int udp_socket, struct sockaddr_in client_addr, char* buffer, socklen_t client_addr_len) {
    char uid[UID_SIZE + 1];
    char status[9999] = "RMA ";
    // we need to guarantee that we read exactly three characters and a space afterwards
    if (!read_uid_udp(buffer, uid)) {
        strcat(status, "NOK\n");
        reply_msg(udp_socket, client_addr, client_addr_len, status);
        return;
    }

    if (verbose_mode){
        printf("Request type: Show user auctions\nuid: %s\n", uid);
    }

    //pthread_mutex_lock(&mutex);
    if (!is_user_login(uid)) {
        strcat(status, "NLG\n");
    } else {
        strcat(status, "OK");
        if (!user_auc_status(uid, status)) {
            sprintf(status, "RMA NOK");
        }
        strcat(status, "\n");
    }
    //pthread_mutex_unlock(&mutex);
    reply_msg(udp_socket, client_addr, client_addr_len, status);
}


void handle_mybids(int udp_socket, struct sockaddr_in client_addr, char* buffer, socklen_t client_addr_len) {
    char uid[UID_SIZE + 1];
    char status[9999] = "RMB ";
    // we need to guarantee that we read exactly three characters and a space afterwards
    if (!read_uid_udp(buffer, uid)) {
        strcat(status, "NOK\n");
        reply_msg(udp_socket, client_addr, client_addr_len, status);
        return;
    }

    if (verbose_mode){
        printf("Request type: Show user bids\nuid: %s\n", uid);
    }

    //pthread_mutex_lock(&mutex);
    if (!is_user_login(uid)) {
        strcat(status, "NLG\n");
    } else {
        strcat(status, "OK");
        if (!user_bids_status(uid, status)) {
            sprintf(status, "RMB NOK");
        }
        strcat(status, "\n");
    }
    //pthread_mutex_unlock(&mutex);
    reply_msg(udp_socket, client_addr, client_addr_len, status);
}


void handle_list(int udp_socket, struct sockaddr_in client_addr, char* buffer, socklen_t client_addr_len) {
    (void) buffer;
    char status[9999] = "RLS ";

    if (verbose_mode){
        printf("Request type: List auctions\n");
    }

    //pthread_mutex_lock(&mutex);
    if (access("auctions", F_OK) == -1) {
        strcat(status, "NOK\n");
    } else {
        strcat(status, "OK");
        append_auctions(status);
        //strcat(status, "\n");
    }
    //pthread_mutex_unlock(&mutex);
    reply_msg(udp_socket, client_addr, client_addr_len, status);
}

void handle_show_record(int udp_socket, struct sockaddr_in client_addr, char *buffer, socklen_t client_addr_len) {
    char auc_id[5];
    char status[9999] = "RRC ";

    if(!read_aid_udp(buffer, auc_id)){
        strcat(status, "NOK\n");
        reply_msg(udp_socket, client_addr, client_addr_len, status);
        return;
    }
    
    if (verbose_mode){
        printf("Request type: Show record\naid: %s\n", auc_id);
    }
    
    //pthread_mutex_lock(&mutex);
    if (!exists_auction(auc_id)) {
        strcat(status, "NOK\n");
        reply_msg(udp_socket, client_addr, client_addr_len, status);
        return;
    }
    strcat(status, "OK");
    get_auc_info(atoi(auc_id), status);
    strcat(status, "\n");
    //pthread_mutex_unlock(&mutex);
    reply_msg(udp_socket, client_addr, client_addr_len, status);
}

