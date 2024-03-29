#include "server.h"

void handle_login(int udp_socket, struct sockaddr_in client_addr, char* buffer, socklen_t client_addr_len) {
    char status[50] = "RLI ";
    char uid[UID_SIZE + 1];
    char password[PASSWORD_SIZE + 1];

    if (!read_uid_udp(buffer, uid) || !read_password_udp(buffer, password)) {
        strcat(status, "ERR\n");
        reply_msg(udp_socket, client_addr, client_addr_len, status);
        return;
    }

    if (verbose_mode){
        printf("Request type: Login\nuid: %s\n", uid);
    }


    if (!verify_user_exists(uid)) {
        if (create_user(uid,password)) {
            strcat(status, "REG\n");
        } else {
            strcat(status, "NOK\n");
        }
    } else {
        if (!verify_password_correct(uid, password)) {
            strcat(status, "NOK\n");
        } else if (is_user_login(uid)) {
            strcat(status, "NOK\n");
        } else {
            change_user_login(uid);
            strcat(status, "OK\n");
        }
    }
    reply_msg(udp_socket, client_addr, client_addr_len, status);
}


void handle_logout(int udp_socket, struct sockaddr_in client_addr, char* buffer, socklen_t client_addr_len) {
    char uid[UID_SIZE + 1], password[PASSWORD_SIZE + 1];
    char status[50] = "RLO ";

    if (!read_uid_udp(buffer, uid) || !read_password_udp(buffer, password)) {
        strcat(status, "ERR\n");
        reply_msg(udp_socket, client_addr, client_addr_len, status);
        return;
    }

    if (verbose_mode){
        printf("Request type: Logout\nuid: %s\n", uid);
    }

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
    reply_msg(udp_socket, client_addr, client_addr_len, status);
}


void handle_unregister(int udp_socket, struct sockaddr_in client_addr, char* buffer, socklen_t client_addr_len) {
    char uid[UID_SIZE + 1], password[PASSWORD_SIZE + 1];
    char status[50] = "RUR ";

    if (!read_uid_udp(buffer, uid) || !read_password_udp(buffer, password)) {
        strcat(status, "ERR\n");
        reply_msg(udp_socket, client_addr, client_addr_len, status);
        return;
    }

    if (verbose_mode){
        printf("Request type: Unregister\nuid: %s\n", uid);
    }

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
    reply_msg(udp_socket, client_addr, client_addr_len, status);
}



void handle_myauctions(int udp_socket, struct sockaddr_in client_addr, char* buffer, socklen_t client_addr_len) {
    char uid[UID_SIZE + 1];
    char status[9999] = "RMA ";
    if (!read_uid_udp_final(buffer, uid)) {
        strcat(status, "ERR\n");
        reply_msg(udp_socket, client_addr, client_addr_len, status);
        return;
    }

    if (verbose_mode){
        printf("Request type: Show user auctions\nuid: %s\n", uid);
    }

    if (!is_user_login(uid)) {
        strcat(status, "NLG\n");
    } else {
        strcat(status, "OK");
        if (!user_auc_status(uid, status)) {
            sprintf(status, "RMA NOK");
        }
        strcat(status, "\n");
    }
    reply_msg(udp_socket, client_addr, client_addr_len, status);
}


void handle_mybids(int udp_socket, struct sockaddr_in client_addr, char* buffer, socklen_t client_addr_len) {
    char uid[UID_SIZE + 1];
    char status[9999] = "RMB ";
    if (!read_uid_udp_final(buffer, uid)) {
        strcat(status, "ERR\n");
        reply_msg(udp_socket, client_addr, client_addr_len, status);
        return;
    }

    if (verbose_mode) {
        printf("Request type: Show user bids\nuid: %s\n", uid);
    }

    if (!is_user_login(uid)) {
        strcat(status, "NLG\n");
    } else {
        strcat(status, "OK");
        if (!user_bids_status(uid, status)) {
            sprintf(status, "RMB NOK");
        }
        strcat(status, "\n");
    }
    reply_msg(udp_socket, client_addr, client_addr_len, status);
}


void handle_list(int udp_socket, struct sockaddr_in client_addr, char* buffer, socklen_t client_addr_len) {
    char status[9999] = "RLS ";

    if (verbose_mode){
        printf("Request type: List auctions\n");
    }

    if (buffer[3] != '\n') {
        strcat(status, "ERR\n");
        reply_msg(udp_socket, client_addr, client_addr_len, status);
        return;
    }

    if (!exists_auctions()) {
        strcat(status, "NOK\n");
    } else {
        strcat(status, "OK");
        append_auctions(status);
    }
    reply_msg(udp_socket, client_addr, client_addr_len, status);
}

void handle_show_record(int udp_socket, struct sockaddr_in client_addr, char *buffer, socklen_t client_addr_len) {
    char auc_id[5];
    char status[9999] = "RRC ";

    if(!read_aid_udp(buffer, auc_id)){
        strcat(status, "ERR\n");
        reply_msg(udp_socket, client_addr, client_addr_len, status);
        return;
    }
    
    if (verbose_mode){
        printf("Request type: Show record\naid: %s\n", auc_id);
    }

    if (!exists_auction(auc_id)) {
        strcat(status, "NOK\n");
        reply_msg(udp_socket, client_addr, client_addr_len, status);
        return;
    }
    strcat(status, "OK");
    get_auc_info(atoi(auc_id), status);
    strcat(status, "\n");
    reply_msg(udp_socket, client_addr, client_addr_len, status);
}
