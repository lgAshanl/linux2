/**
 * @file userdata.h
 *
 * @brief PhoneBook userdata
 * @author Bobylev Igor, 
 * Contact: igor.v.bobylev@gmail.com
 *
 */

#pragma once

#ifndef PHONEBOOK_USERDATA_H
#define PHONEBOOK_USERDATA_H 1

#define MAX_NAME 32
#define NAME_SCANF_FORMAT "%31s"
#define MAX_SURNAME 64
#define SURNAME_SCANF_FORMAT "%63s"
#define MAX_PHONE_NUMBER 16
#define PHONE_NUMBER_SCANF_FORMAT "%15s"
#define MAX_EMAIL 32
#define EMAIL_SCANF_FORMAT "%31s"

#define COMMAND_GET_USER (char)0x01
#define COMMAND_ADD_USER (char)0x02
#define COMMAND_DEL_USER (char)0x03

struct user_data {
    char m_name[MAX_NAME];
    uint32_t m_nameSize;

    char m_surname[MAX_SURNAME];
    uint32_t m_surnameSize;

    uint32_t m_age;

    char m_phone[MAX_PHONE_NUMBER];
    uint32_t m_phoneSize;

    char m_email[MAX_EMAIL];
    uint32_t m_emaiSize;
};

typedef struct user_data user_data_t;

#endif // PHONEBOOK_USERDATA_H
