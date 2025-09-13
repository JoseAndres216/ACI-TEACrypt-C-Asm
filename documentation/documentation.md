**Arquitectura de software**

Para llevar a cabo el proyecto se desarrolló una arquitectura que implementa una interfaz de usuario en C, la cual se encarga de interactuar y recibir las entradas de usuario y,haciendo uso de funciones auxiliares externas realizadas en ensamblador, realiza las 32 rondas de cifrado o descifrado según las necesidades del usuario. Este software implementa una una simulación bare-metal a partir del uso de QEMU,de modo que simula un entorno independiente de sistema operativo, que no utiliza librerías externas y depende únicamente de las funcionalidades desarrolladas para su funcionamiento.

**Funcionalidades**

Para la complementación de bloques de 64 bits, y para su posterior análisis en forma de bloques completos, se implementó la función add_padding, la cual toma las entradas a cifrar que no completan uno o varios bloques de 64 bits y los completa hasta llenarlos, tal como se observa en en el siguiente código:

```c
uint32_t add_padding(uint8_t *buffer, uint32_t len) {
    uint32_t pad = (len % BLOCK_SIZE == 0) ? 0 : (BLOCK_SIZE - (len % BLOCK_SIZE));
    for (uint32_t i = 0; i < pad; i++) {
        buffer[len + i] = (uint8_t)pad;
    }
    return len + pad;
}
```

Por otra parte, para la lectura de entradas por parte del usuario se implementaron las funciones read_char y get_user_input. La función read_char se encarga de leer constantemente los inputs de teclado por parte del usuario; esta funcionalidad se plantea para leer las entradas del usuario a la hora de seleccionar el texto a cifrar y descifrar. Por otra parte, la función get_user_input muestra las posibilidades de opciones de texto a cifrar y llama a la función read_char para la lectura de selección.

```c

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

```

Ahora bien, las funciones tea_encrypt_block y tea_encrypt_text trabajan colectivamente para el cifrado de texto en forma de bloques. la función principal, tea_encrypt_text toma el texto seleccionado por el usuario y la clave proporcionada por el mismo, la fracciona en bloques, agrega el padding necesario para rellenar el último bloque en caso de estar incompleto y así, bloque por bloque, llamar a la función tea_encrypt_block que invoca a la funcionalidad externa de ensamblador para llevar a cabo cada ronda de cifrado.


```c

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

```

La función tea_encrypt toma la clave dada por el usuario y la información proveniente de la función tea_encrypt_block y realiza la el encriptado de v0 y v1, retornandolos respectivamente cifrados en los 8 bits del registro a0.

```s

.section .text
.globl tea_encrypt

tea_encrypt:
    addi sp, sp, -16
    sw   s0, 12(sp)
    sw   s1, 8(sp)

    lw   t0, 0(a0)
    lw   t1, 4(a0)

    lw   t2, 0(a1)
    lw   t3, 4(a1)
    lw   t4, 8(a1)
    lw   t5, 12(a1)

    slli t6, t1, 4
    add  t6, t6, t2
    add  s0, t1, a2
    srli s1, t1, 5
    add  s1, s1, t3
    xor  t6, t6, s0
    xor  t6, t6, s1
    add  t0, t0, t6 

    slli t6, t0, 4
    add  t6, t6, t4
    add  s0, t0, a2
    srli s1, t0, 5
    add  s1, s1, t5
    xor  t6, t6, s0
    xor  t6, t6, s1
    add  t1, t1, t6

    sw t0, 0(a0)
    sw t1, 4(a0)

    lw   s1, 8(sp)
    lw   s0, 12(sp)
    addi sp, sp, 16
    ret


```

Del mismo modo, las funciones tea_decrypt_block, tea_decrypt_text y tea_decrypt, llevan a cabo el proceso de desencriptado del texto dado por el usuario y con la clave proporcionada.

```c

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

```

```s

.section .text
.globl tea_decrypt
.type tea_decrypt, @function

tea_decrypt:
    addi sp, sp, -16
    sw   s0, 12(sp)
    sw   s1, 8(sp)

    lw   t0, 0(a0)
    lw   t1, 4(a0)
    lw   t2, 0(a1)
    lw   t3, 4(a1)
    lw   t4, 8(a1)
    lw   t5, 12(a1)

    slli t6, t0, 4
    add  t6, t6, t4
    add  s0, t0, a2
    srli s1, t0, 5
    add  s1, s1, t5
    xor  t6, t6, s0
    xor  t6, t6, s1
    sub  t1, t1, t6

    slli t6, t1, 4
    add  t6, t6, t2
    add  s0, t1, a2
    srli s1, t1, 5
    add  s1, s1, t3
    xor  t6, t6, s0
    xor  t6, t6, s1
    sub  t0, t0, t6

    sw t0, 0(a0)
    sw t1, 4(a0)

    lw   s1, 8(sp)
    lw   s0, 12(sp)
    addi sp, sp, 16
    ret

```

**Evidencias de ejecución**

Al ejecutar los comandos de la sección de instrucciones, se realiza una primera prueba sobre un arreglo que contiene los textos "HOLA1234" y "Mensaje de prueba para TEA", enviando ambos mensajes de uno por uno para probar su cifrado.

![image](/img/automatic_tests.png)

Posteriormente, el sistema se mantiene a la espera de cualquier entrada de usuario, con el fin de realizar una prueba de cifrado con un texto específico. La siguientes imagenes muestran pruebas sobre ambos mensajes, la primera ante la entrada "1" por parte del usuario, y la segunda ante la entrada de "2":

![image](/img/user_test_1.png)

![image](/img/user_test_2.png)

**Discusión de resultados**

Cómo se observa en las evidencias incluidas, ambos mensajes experimentan un correcto encriptado y desencriptado; tanto para el caso en que no se requiere rellenado o padding, o casos de varios bloques completos, como para un solo bloque completamente lleno, los mensajes son cifrados en impresos a nivel de consola en su forma hexadecimal, y posteriormente, se aprecia un correcto descifrado al obtener el mensaje originalmente previsto.

**Instrucciones para compilar y utilizar el sistema**

Inicialmente, es necesario abrir una terminal en el directorio principal (dentro del directorio llamado rvqemu-main). En una primera terminal, se deben ejecutar los siguientes comandos (en este orden específico):

```bash

./run.sh

cd /home/rvqemu-dev/workspace/examples/project

./build.sh

./run-qemu.sh

```

Posteriormente, en una segunda terminal, es necesario llevar a cabo el siguiente flujo de comandos (de igual forma, siguiendo el orden indicado):

```bash

docker exec -it rvqemu /bin/bash

cd /home/rvqemu-dev/workspace/examples/project

gdb-multiarch project.elf

target remote :1234

continue

```

Una vez hecho esto, en la primera terminal se habrán realizado los ejemplos de prueba vistos anteriormente y el sistema se mantendrá a la espera de una entrada (1 o 2) que le permita cifrar o descifrar el texto seleccionado.