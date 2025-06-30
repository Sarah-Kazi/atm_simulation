# ATM Simulation in C

A simple C-based ATM simulation system that securely handles account management, PIN hashing using SHA-256, and SMS notifications via Twilio. It supports basic banking operations like withdrawals, deposits, transfers, and balance checks. All account data is stored in files for easy management and testing.

---

## Features

- Create accounts with hashed PINs
- Withdraw, deposit, and check balances
- Transfer funds and pay bills
- Send SMS notifications using Twilio API
- Generate transaction receipts

---

## Configuration

1. Copy `config.example.h` to `config.h`:
    ```bash
    cp config.example.h config.h
    ```

2. Open `config.h` and fill in your actual Twilio credentials:
    ```c
    #define ACCOUNT_SID "YOUR_TWILIO_ACCOUNT_SID"
   #define AUTH_TOKEN  "YOUR_TWILIO_AUTH_TOKEN"
   #define TWILIO_PHONE_NUMBER "+1YOUR_TWILIO_NUMBER"
   #define USER_PHONE_NUMBER "+91YOUR_NUMBER" 
    ```


---

## Compilation Instructions

### Prerequisite: Install libcurl

Make sure the libcurl development package is installed:

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install libcurl4-openssl-dev
 ```
### Compilation command

Compile using:
 ```bash
gcc atm.c -o atm_simulator -lcurl
./atm_simulator

 ```
