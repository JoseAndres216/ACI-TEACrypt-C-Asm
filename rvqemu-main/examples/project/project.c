#include <stdint.h>

/* =========================================    EXTERN ASSEMBLER FUNCTION DECLARATION    ========================================= */

extern void tea_encrypt(uint32_t v[2], const uint32_t k[4], uint32_t sum);
extern void tea_decrypt(uint32_t v[2], const uint32_t k[4], uint32_t sum);

/* ======================================================    CONSTANTS   ========================================================= */

#define DELTA      0x9e3779b9
#define ROUNDS     32
#define BLOCK_SIZE 8
#define MAX_BUFFER 128

/* ===============================================    COMPLEMENTARY FUNCTIONS    ================================================= */

void print_char(char c) {
    volatile char *uart = (volatile char*)0x10000000;
    *uart = c;
}

void print_string(const char* str) {
    while (*str) {
        print_char(*str++);
    }
}

void print_hex(uint8_t *data, uint32_t len) {
    const char *hex = "0123456789ABCDEF";
    for (uint32_t i = 0; i < len; i++) {
        uint8_t b = data[i];
        print_char(hex[b >> 4]);
        print_char(hex[b & 0x0F]);
    }
    print_char('\n');
}

uint32_t str_len(const char *s) {
    uint32_t n = 0;
    while (s[n] != '\0') n++;
    return n;
}

void str_copy(uint8_t *dest, const char *src, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        dest[i] = (uint8_t)src[i];
    }
}

uint32_t add_padding(uint8_t *buffer, uint32_t len) {
    uint32_t pad = (len % BLOCK_SIZE == 0) ? 0 : (BLOCK_SIZE - (len % BLOCK_SIZE));
    for (uint32_t i = 0; i < pad; i++) {
        buffer[len + i] = (uint8_t)pad;
    }
    return len + pad;
}

char read_char() {
    volatile char *uart_data   = (volatile char*)0x10000000;
    volatile char *uart_status = (volatile char*)0x10000005;

    while ((*uart_status & 0x01) == 0);

    return *uart_data;
}


int get_user_input() {
    int valid_input = 0;
    int selected_idx;
    while (!valid_input)
    {
        print_char('\n');
        print_string("Choose a number to test any of the texts\n");
        print_string("1. HOLA1234\n");
        print_string("2. Mensaje de prueba para TEA\n");
        print_char('\n');
        char c;
        do {
            c = read_char();
        } while (c == '\r' || c == '\n');

        selected_idx = c - '1';
        print_string("Choosen text: ");
        print_char(selected_idx + '1');
        if (selected_idx >= 0 && selected_idx < 2) {
            valid_input = 1;
        } else
        {
            print_string("Invalid input.\n");
        }
    }
    return selected_idx;
}


/* ===================================================    ENCRYPT FUNCTIONS    =================================================== */

void tea_encrypt_block(uint32_t v[2], const uint32_t k[4]) {
    uint32_t sum = 0;
    for (int i = 0; i < ROUNDS; i++) {
        sum += DELTA;
        tea_encrypt(v, k, sum);
    }
}

uint32_t tea_encrypt_text(const char *text, uint8_t *output, const uint32_t key[4]) {
    uint32_t len = str_len(text);
    str_copy(output, text, len);

    // padding
    uint32_t padded_len = add_padding(output, len);

    // cifrado por bloques
    for (uint32_t i = 0; i < padded_len; i += BLOCK_SIZE) {
        tea_encrypt_block((uint32_t*)(output + i), key);
    }

    return padded_len;
}

/* ===================================================    DECRYPT FUNCTIONS    =================================================== */

void tea_decrypt_block(uint32_t v[2], const uint32_t k[4]) {
    uint32_t sum = DELTA * ROUNDS;
    for (int i = 0; i < ROUNDS; i++) {
        tea_decrypt(v, k, sum);
        sum -= DELTA;
    }
}

uint32_t tea_decrypt_text(uint8_t *data, uint32_t len, const uint32_t key[4]) {
    for (uint32_t i = 0; i < len; i += BLOCK_SIZE) {
        tea_decrypt_block((uint32_t*)(data + i), key);
    }

    uint8_t pad = data[len - 1];
    if (pad > 0 && pad <= BLOCK_SIZE) {
        len -= pad;
    }

    return len;
}

/* ==========================================================    MAIN   ========================================================== */

int main() {
    const uint32_t key[4] = {
        0x12345678, 0x9ABCDEF0, 0xFEDCBA98, 0x76543210
    };

    const char *tests[] = {
        "HOLA1234",
        "Mensaje de prueba para TEA"
    };

    print_string("================================================== AUTOMATIC TESTS  ==================================================\n");
    
    for (int t = 0; t < 2; t++) {
        uint8_t buffer[MAX_BUFFER];

        print_string("\nOriginal text: ");
        print_string(tests[t]);
        print_char('\n');

        // Encrypts
        uint32_t cipher_len = tea_encrypt_text(tests[t], buffer, key);
        print_string("Encrypted text: ");
        print_hex(buffer, cipher_len);

        // Decrypts
        uint32_t plain_len = tea_decrypt_text(buffer, cipher_len, key);
        buffer[plain_len] = '\0';
        print_string("Decrypted text: ");
        print_string((char*)buffer);
        print_char('\n');
    }

    while(1){
        print_string("==================================================    USER TEST     ==================================================\n");


        int selected_id = get_user_input();
        uint8_t buffer[MAX_BUFFER];

        print_string("\nOriginal text: ");
        print_string(tests[selected_id]);
        print_char('\n');

        // Encrypts
        uint32_t cipher_len = tea_encrypt_text(tests[selected_id], buffer, key);
        print_string("Encrypted text: ");
        print_hex(buffer, cipher_len);

        // Decrypts
        uint32_t plain_len = tea_decrypt_text(buffer, cipher_len, key);
        buffer[plain_len] = '\0';
        print_string("Decrypted text: ");
        print_string((char*)buffer);
        print_char('\n');
    }

    return 0;
}