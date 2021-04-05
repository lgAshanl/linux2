/**
 * @file main.c
 *
 * @brief PhoneBook client
 * @author Bobylev Igor, 
 * Contact: igor.v.bobylev@gmail.com
 *
 */

#include <stdio.h>
#include <memory.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#include "userdata.h"

#define g_badHandle -1

void read_surname(char* surname, uint32_t* surname_size) {
    printf("Write surname: ");
    scanf(SURNAME_SCANF_FORMAT, surname);
    *surname_size = strnlen(surname, MAX_SURNAME);
    if ((MAX_SURNAME == *surname_size) && (0 != surname[MAX_SURNAME-1])) {
        surname[MAX_SURNAME-1] = 0;
    }
}

void read_user(user_data_t* data) {
    // get name
    printf("Write name: ");
    scanf(NAME_SCANF_FORMAT, data->m_name);
    data->m_nameSize = strnlen(data->m_name, MAX_NAME);
    if ((MAX_NAME == data->m_nameSize) && (0 != data->m_name[MAX_NAME-1])) {
        data->m_name[MAX_NAME-1] = 0;
    }

    // get surname
    printf("Write surname: ");
    scanf(SURNAME_SCANF_FORMAT, data->m_surname);
    data->m_surnameSize = strnlen(data->m_surname, MAX_SURNAME);
    if ((MAX_SURNAME == data->m_surnameSize) && (0 != data->m_surname[MAX_SURNAME-1])) {
        data->m_surname[MAX_SURNAME-1] = 0;
    }

    // get age
    printf("Write age: ");
    scanf("%u", &data->m_age);

    // get phone
    printf("Write phone number: ");
    scanf(PHONE_NUMBER_SCANF_FORMAT, data->m_phone);
    data->m_phoneSize = strnlen(data->m_phone, MAX_PHONE_NUMBER);
    if ((MAX_PHONE_NUMBER == data->m_phoneSize) && (0 != data->m_phone[MAX_PHONE_NUMBER-1])) {
        data->m_phone[MAX_PHONE_NUMBER-1] = 0;
    }

    // get email
    printf("Write email: ");
    scanf("%s", data->m_email);
    data->m_emaiSize = strnlen(data->m_email, MAX_EMAIL);
    if ((MAX_EMAIL == data->m_emaiSize) && (0 != data->m_email[MAX_EMAIL-1])) {
        data->m_email[MAX_EMAIL-1] = 0;
    }
}

int get_user(const char* surname, const int surname_size, user_data_t* userdata) {
    const int fd = open("/dev/phoneBook", O_RDWR);
    if (g_badHandle == fd) {
        return errno;
    }

    const uint32_t iosize = 2 + surname_size;
    char* iobuf = malloc(sizeof(user_data_t));
    iobuf[0] = COMMAND_GET_USER;
    iobuf[1] = (unsigned char)(surname_size);
    memcpy(((uint8_t*)iobuf) + 2, surname, surname_size);

    int res = write(fd, iobuf, iosize);
    if (iosize != res) {
        free(iobuf);
        close(fd);
        return -res;
    }

    res = read(fd, iobuf, sizeof(user_data_t));

    if (sizeof(user_data_t) == res) {
        memcpy(userdata, iobuf, sizeof(user_data_t));
    }

    free(iobuf);
    close(fd);

    return (sizeof(user_data_t) == res) ? 0 : -res;
}

int add_user(user_data_t* userdata) {
    const int fd = open("/dev/phoneBook", O_RDWR);
    if (g_badHandle == fd) {
        return errno;
    }

    const uint32_t iosize = 1 + sizeof(user_data_t);
    char* iobuf = malloc(iosize);
    iobuf[0] = COMMAND_ADD_USER;
    memcpy(((uint8_t*)iobuf) + 1, userdata, sizeof(user_data_t));

    const int res = write(fd, iobuf, iosize);

    free(iobuf);
    close(fd);

    return (iosize == res) ? 0 : -res;
}

int del_user(const char* surname, const int surname_size) {
    const int fd = open("/dev/phoneBook", O_RDWR);
    if (g_badHandle == fd) {
        return errno;
    }

    const uint32_t iosize = 2 + surname_size;
    char* iobuf = malloc(iosize);
    iobuf[0] = COMMAND_DEL_USER;
    iobuf[1] = (unsigned char)(surname_size);
    memcpy(((uint8_t*)iobuf) + 2, surname, surname_size);

    const int res = write(fd, iobuf, iosize);

    free(iobuf);
    close(fd);

    return (iosize == res) ? 0 : -res;
}

int main() {
    char surname[MAX_SURNAME];
    uint32_t surname_size = 0;
    user_data_t data;

    while (1) {
        printf(">");
        char command[256] = { 0 };
        scanf("%s", command);

        if (0 == strcmp("get_user", command)) {
            read_surname(surname, &surname_size);
            memset(&data, 0, sizeof(user_data_t));
            const int res = get_user(surname, surname_size, &data);
            if (0 == res) {
                printf("Fount user for following fields\n");
                printf("Surname: %s\n", data.m_surname);
                printf("Name: %s\n", data.m_name);
                printf("Age: %d\n", data.m_age);
                printf("Phone number: %s\n", data.m_phone);
                printf("Email: %s\n", data.m_email);
            }
            else {
                printf("Failed with code: %d\n", res);
            }
        }

        else if (0 == strcmp("add_user", command)) {
            read_user(&data);
            const int res = add_user(&data);
            if (0 == res) {
                printf("OK\n");
            }
            else {
                printf("Failed with code: %d\n", res);
            }
        }

        else if (0 == strcmp("del_user", command)) {
            read_surname(surname, &surname_size);
            const int res = del_user(surname, surname_size);
            if (0 == res) {
                printf("Deleted successfully\n");
            }
            else {
                printf("Failed with code: %d\n", res);
            }
        }

        else if (0 == strcmp("exit", command)) {
            return 0;
        }
    }

    return 666;
}