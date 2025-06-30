#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <curl/curl.h>
#include <stdint.h>
#include "config.h"

#define FILE_NAME "accounts.txt"
#define RECEIPT_FILE "receipt.txt"


#define CLEAR "clear"
#define BEEP() printf("\a")

#define COLOR_RESET "\033[0m"
#define COLOR_CYAN "\033[1;36m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_RED "\033[1;31m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_WHITE "\033[1;37m"

void sha256_hash(const char *input, char *output) {
    static const uint32_t k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    uint32_t h0 = 0x6a09e667, h1 = 0xbb67ae85, h2 = 0x3c6ef372, h3 = 0xa54ff53a;
    uint32_t h4 = 0x510e527f, h5 = 0x9b05688c, h6 = 0x1f83d9ab, h7 = 0x5be0cd19;
    
    uint8_t *msg = NULL;
    uint64_t new_len, orig_len = strlen(input);
    uint32_t i, j, w[64];
    uint32_t a, b, c, d, e, f, g, h, s0, s1, maj, t1, t2, ch;
    

    new_len = orig_len + 1;
    while (new_len % 64 != 56) new_len++;
    msg = (uint8_t *)malloc(new_len + 8);
    memcpy(msg, input, orig_len);
    msg[orig_len] = 0x80;
    for (i = orig_len + 1; i < new_len; i++) msg[i] = 0;
    

    uint64_t bit_len = orig_len * 8;
    for (i = 0; i < 8; i++) msg[new_len + i] = (bit_len >> (56 - i * 8)) & 0xff;
    new_len += 8;
    
    
    for (i = 0; i < new_len; i += 64) {
        
        for (j = 0; j < 16; j++) {
            w[j] = (msg[i + j * 4] << 24) | (msg[i + j * 4 + 1] << 16) | 
                   (msg[i + j * 4 + 2] << 8) | (msg[i + j * 4 + 3]);
        }
        for (j = 16; j < 64; j++) {
            s0 = (w[j-15] >> 7 | w[j-15] << 25) ^ (w[j-15] >> 18 | w[j-15] << 14) ^ (w[j-15] >> 3);
            s1 = (w[j-2] >> 17 | w[j-2] << 15) ^ (w[j-2] >> 19 | w[j-2] << 13) ^ (w[j-2] >> 10);
            w[j] = w[j-16] + s0 + w[j-7] + s1;
        }
        
        
        a = h0; b = h1; c = h2; d = h3;
        e = h4; f = h5; g = h6; h = h7;
        
       
        for (j = 0; j < 64; j++) {
            s1 = (e >> 6 | e << 26) ^ (e >> 11 | e << 21) ^ (e >> 25 | e << 7);
            ch = (e & f) ^ (~e & g);
            t1 = h + s1 + ch + k[j] + w[j];
            s0 = (a >> 2 | a << 30) ^ (a >> 13 | a << 19) ^ (a >> 22 | a << 10);
            maj = (a & b) ^ (a & c) ^ (b & c);
            t2 = s0 + maj;
            
            h = g; g = f; f = e; e = d + t1;
            d = c; c = b; b = a; a = t1 + t2;
        }
        
       
        h0 += a; h1 += b; h2 += c; h3 += d;
        h4 += e; h5 += f; h6 += g; h7 += h;
    }
    
    free(msg);
    
    
    sprintf(output, "%08x%08x%08x%08x%08x%08x%08x%08x", 
            h0, h1, h2, h3, h4, h5, h6, h7);
}

void delay(int milliseconds) {
    usleep(milliseconds * 1000);
}

char linux_linux_getch() {
    struct termios oldt, newt;
    char ch;
    tcgetattr(0, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(0, TCSANOW, &oldt);
    return ch;
}

void input_pin(char *pin_str) {
    char ch;
    printf("Enter 4-digit PIN: ");
    for (int i = 0; i < 4; i++) {
        ch = linux_linux_getch();
        if (ch >= '0' && ch <= '9') {
            pin_str[i] = ch;
            printf("*");
        } else {
            i--;
        }
    }
    pin_str[4] = '\0';
    printf("\n");
}

typedef struct {
    char username[50];
    char pin_hash[65];
    float balance;
    int is_activated;
} Account;

void print_receipt(const char* text) {
    FILE* rp = fopen(RECEIPT_FILE, "a");
    if (rp != NULL) {
        fprintf(rp, "%s\n", text);
        fclose(rp);
    }
}

void clear_screen() {
    system(CLEAR);
}

void banner(const char* text) {
    printf(COLOR_CYAN);
    printf("\n========================================\n");
    for (int i = 0; i < strlen(text); i++) {
        printf("%c", text[i]);
        delay(20);
    }
    printf("\n========================================\n");
    printf(COLOR_RESET);
}

void send_sms(const char* message) {
    CURL *curl = curl_easy_init();
    if (curl) {
        char url[256];
        char post_data[512];
        
        snprintf(url, sizeof(url), 
                "https://api.twilio.com/2010-04-01/Accounts/%s/Messages.json", 
                TWILIO_ACCOUNT_SID);
        snprintf(post_data, sizeof(post_data),
                "Body=%s&From=%s&To=%s", 
                message, TWILIO_PHONE_NUMBER, USER_PHONE_NUMBER);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
        curl_easy_setopt(curl, CURLOPT_USERNAME, TWILIO_ACCOUNT_SID);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, TWILIO_AUTH_TOKEN);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fopen("/dev/null", "w"));

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "SMS failed: %s\n", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
}

void create_account() {
    FILE *fp = fopen(FILE_NAME, "a+");
    if (fp == NULL) {
        printf(COLOR_RED "Error opening file!\n" COLOR_RESET);
        return;
    }

    Account acc;
    banner("Create New Account");
    printf("Enter username: ");
    scanf("%s", acc.username);
    
    char pin_str[5];
    input_pin(pin_str);
    sha256_hash(pin_str, acc.pin_hash);
    
    acc.balance = 0.0;
    acc.is_activated = 0;

    fprintf(fp, "%s %s %.2f %d\n", acc.username, acc.pin_hash, acc.balance, acc.is_activated);
    fclose(fp);

    printf(COLOR_GREEN "Account created. Please activate your card.\n" COLOR_RESET);
    print_receipt("Account created.");
    BEEP();
}

int find_account(const char *username, const char *pin_hash, Account *acc) {
    FILE *fp = fopen(FILE_NAME, "r");
    if (fp == NULL) return 0;

    while (fscanf(fp, "%s %s %f %d", acc->username, acc->pin_hash, &acc->balance, &acc->is_activated) != EOF) {
        if (strcmp(acc->username, username) == 0) {
            fclose(fp);
            if (strcmp(acc->pin_hash, pin_hash) == 0) return 1;
            else return -1;
        }
    }
    fclose(fp);
    return 0;
}

void update_account(Account updated) {
    FILE *fp = fopen(FILE_NAME, "r");
    FILE *temp = fopen("temp.txt", "w");
    Account acc;

    while (fscanf(fp, "%s %s %f %d", acc.username, acc.pin_hash, &acc.balance, &acc.is_activated) != EOF) {
        if (strcmp(acc.username, updated.username) == 0) {
            fprintf(temp, "%s %s %.2f %d\n", updated.username, updated.pin_hash, updated.balance, updated.is_activated);
        } else {
            fprintf(temp, "%s %s %.2f %d\n", acc.username, acc.pin_hash, acc.balance, acc.is_activated);
        }
    }

    fclose(fp);
    fclose(temp);
    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);
}

void deposit(Account *acc, float amount) {
    acc->balance += amount;
    update_account(*acc);

    char sms_text[100];
    sprintf(sms_text, "ATM Alert: Rs %.2f deposited. New balance: Rs %.2f", amount, acc->balance);
    send_sms(sms_text);

    printf(COLOR_GREEN "Deposited Rs %.2f\n" COLOR_RESET, amount);

    char receipt[100];
    sprintf(receipt, "Deposited: Rs %.2f", amount);
    print_receipt(receipt);

    BEEP();
}

void withdraw(Account *acc, float amount) {
    if (amount > acc->balance) {
        printf(COLOR_RED "Insufficient funds!\n" COLOR_RESET);
        return;
    }

    acc->balance -= amount;
    update_account(*acc);

    char sms_text[100];
    sprintf(sms_text, "ATM Alert: Rs %.2f withdrawn. New balance: Rs %.2f", amount, acc->balance);
    send_sms(sms_text);

    printf(COLOR_GREEN "Withdrawn: Rs %.2f\n" COLOR_RESET, amount);

    char receipt[100];
    sprintf(receipt, "Withdrawn: Rs %.2f", amount);
    print_receipt(receipt);

    BEEP();
}

void transfer_funds(Account *acc) {
    char recipient[50];
    float amount;
    printf("Enter recipient username: ");
    scanf("%s", recipient);
    printf("Enter amount to transfer: Rs ");
    scanf("%f", &amount);

    if (amount > acc->balance) {
        printf(COLOR_RED "Insufficient balance!\n" COLOR_RESET);
        return;
    }

    Account recipient_acc;
    int found = 0;

    FILE *fp = fopen(FILE_NAME, "r");
    while (fscanf(fp, "%s %s %f %d", recipient_acc.username, recipient_acc.pin_hash, &recipient_acc.balance, &recipient_acc.is_activated) != EOF) {
        if (strcmp(recipient_acc.username, recipient) == 0) {
            found = 1;
            break;
        }
    }
    fclose(fp);

    if (!found) {
        printf(COLOR_RED "Recipient not found.\n" COLOR_RESET);
        return;
    }

    recipient_acc.balance += amount;
    acc->balance -= amount;

    update_account(*acc);
    update_account(recipient_acc);

    char sms_text[150];
    sprintf(sms_text, "ATM Alert: Rs %.2f sent to %s. New balance: Rs %.2f", amount, recipient, acc->balance);
    send_sms(sms_text);

    printf(COLOR_GREEN "Transferred Rs %.2f to %s\n" COLOR_RESET, amount, recipient);

    char receipt[150];
    sprintf(receipt, "Transferred Rs %.2f to %s", amount, recipient);
    print_receipt(receipt);

    BEEP();
}

void bill_payment(Account *acc) {
    float amount;
    printf("Enter bill amount: Rs ");
    scanf("%f", &amount);

    if (amount > acc->balance) {
        printf(COLOR_RED "Insufficient balance!\n" COLOR_RESET);
        return;
    }

    acc->balance -= amount;
    update_account(*acc);

    char sms_text[100];
    sprintf(sms_text, "ATM Alert: Rs %.2f paid for bills. New balance: Rs %.2f", amount, acc->balance);
    send_sms(sms_text);

    printf(COLOR_GREEN "Bill of Rs %.2f paid successfully.\n" COLOR_RESET, amount);

    char receipt[100];
    sprintf(receipt, "Bill Payment: Rs %.2f", amount);
    print_receipt(receipt);

    BEEP();
}

void atm_menu(Account acc) {
    int choice;
    float amount;
    char receipt_line[100];
    char new_pin_str[5];
    char new_pin_hash[65];

    while (1) {
        banner("ATM Menu");
        printf("1. Balance Inquiry\n2. Deposit\n3. Withdraw\n4. Fund Transfer\n5. Bill Payment\n6. Change PIN\n7. Print Receipt\n8. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf(COLOR_YELLOW "Current Balance: Rs %.2f\n" COLOR_RESET, acc.balance);
                sprintf(receipt_line, "Balance Inquiry: Rs %.2f", acc.balance);
                print_receipt(receipt_line);
                BEEP();
                break;
            case 2:
                printf("Enter amount to deposit: Rs ");
                scanf("%f", &amount);
                deposit(&acc, amount);
                break;
            case 3:
                printf("Enter amount to withdraw: Rs ");
                scanf("%f", &amount);
                withdraw(&acc, amount);
                break;
            case 4:
                transfer_funds(&acc);
                break;
            case 5:
                bill_payment(&acc);
                break;
            case 6:
                printf("Enter new PIN: ");
                input_pin(new_pin_str);
                sha256_hash(new_pin_str, new_pin_hash);
                strcpy(acc.pin_hash, new_pin_hash);
                update_account(acc);
                printf(COLOR_GREEN "PIN changed successfully.\n" COLOR_RESET);
                print_receipt("PIN Changed Successfully");
                BEEP();
                break;
            case 7:
                banner("Transaction Receipt");
                FILE *rp = fopen(RECEIPT_FILE, "r");
                if (rp != NULL) {
                    char line[100];
                    while (fgets(line, sizeof(line), rp)) {
                        printf(COLOR_WHITE "%s" COLOR_RESET, line);
                        delay(50);
                    }
                    fclose(rp);
                } else {
                    printf("No receipt found.\n");
                }
                printf("---------------------------\n");
                break;
            case 8:
                printf(COLOR_MAGENTA "Logging out...\n" COLOR_RESET);
                BEEP();
                return;
            default:
                printf(COLOR_RED "Invalid choice.\n" COLOR_RESET);
        }
    }
}

void activate_card() {
    char username[50];
    char pin_str[5];
    char pin_hash[65];
    Account acc;

    printf("Enter username: ");
    scanf("%s", username);
    input_pin(pin_str);
    sha256_hash(pin_str, pin_hash);

    int res = find_account(username, pin_hash, &acc);
    if (res == 1) {
        if (!acc.is_activated) {
            acc.is_activated = 1;
            update_account(acc);
            printf(COLOR_GREEN "Card activated successfully.\n" COLOR_RESET);
            print_receipt("Card Activated Successfully");
            BEEP();
        } else {
            printf(COLOR_YELLOW "Card is already activated.\n" COLOR_RESET);
        }
    } else if (res == -1) {
        printf(COLOR_RED "Incorrect PIN.\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "Username not found.\n" COLOR_RESET);
    }
}

void login() {
    char username[50];
    char pin_str[5];
    char pin_hash[65];
    Account acc;

    printf("Enter username: ");
    scanf("%s", username);
    input_pin(pin_str);
    sha256_hash(pin_str, pin_hash);

    int res = find_account(username, pin_hash, &acc);
    if (res == 1) {
        if (!acc.is_activated) {
            printf(COLOR_RED "Card not activated. Please activate first.\n" COLOR_RESET);
            return;
        }
        atm_menu(acc);
    } else if (res == -1) {
        printf(COLOR_RED "Incorrect PIN.\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "Username not found!\n" COLOR_RESET);
    }
}

int main() {
    int choice;
    while (1) {
        clear_screen();
        banner("Welcome to Terminal ATM");
        printf("1. Create Account\n2. Activate Card\n3. Login\n4. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: create_account(); break;
            case 2: activate_card(); break;
            case 3: login(); break;
            case 4: printf(COLOR_MAGENTA "Thank you!\n" COLOR_RESET); exit(0);
            default: printf(COLOR_RED "Invalid choice!\n" COLOR_RESET);
        }
        printf("\nPress any key to continue...");
        linux_linux_getch();
    }
    return 0;
}
