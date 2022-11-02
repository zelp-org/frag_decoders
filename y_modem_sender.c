#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <Windows.h>
#include <io.h>

#define PACKET_SEQNO_INDEX      (1)
#define PACKET_SEQNO_COMP_INDEX (2)

#define PACKET_HEADER           (3)
#define PACKET_TRAILER          (2)
#define PACKET_OVERHEAD         (PACKET_HEADER + PACKET_TRAILER)
#define PACKET_SIZE             (128)
#define PACKET_1K_SIZE          (1024)

#define FILE_NAME_LENGTH        (256)
#define FILE_SIZE_LENGTH        (16)

#define SOH                     (0x01)
/* start of 128-byte data packet */
#define STX                     (0x02)  /* start of 1024-byte data packet */
#define EOT                     (0x04)  /* end of transmission */
#define ACK                     (0x06)  /* acknowledge */
#define NAK                     (0x15)  /* negative acknowledge */
#define CA                      (0x18)  /* two of these in succession aborts transfer */
#define CRC16                   (0x43)  /* 'C' == 0x43, request 16-bit CRC */

#define ABORT1                  (0x41)  /* 'A' == 0x41, abort by user */
#define ABORT2                  (0x61)  /* 'a' == 0x61, abort by user */

void flush_serial(HANDLE hport)
{
    char c;
    uint32_t got = 1;
    while (got && ReadFile(hport, &c, 1, &got, NULL));
}

uint8_t read_serial(HANDLE hport)
{
    // wait for character
//    DWORD dwEventMask = 0;
//    WaitCommEvent(hport, &dwEventMask, NULL);

    uint32_t chars_read = 0;
    uint8_t c;

    while (!chars_read)
    {
        if (!ReadFile(hport, &c, 1, &chars_read, NULL))
            continue;
    }

    return c;
}

bool write_serial(HANDLE hport, uint8_t* buf, uint32_t len)
{
    uint32_t wrote = 0, total_wrote = 0;

    while (wrote != len)
    {
        if (!WriteFile(hport, buf, len, &wrote, NULL))
            return false;

        total_wrote += wrote;
    }

    return true;
}

static uint16_t crc_update(uint16_t crc_in, int incr)
{
    uint16_t xor = crc_in >> 15;
    uint16_t out = crc_in << 1;

    if (incr)
        out++;

    if (xor)
        out ^= 0x1021;

    return out;
}

static uint16_t crc16(const uint8_t* data, uint32_t size)
{
    uint16_t crc, i;

    for (crc = 0; size > 0; size--, data++)
        for (i = 0x80; i; i >>= 1)
            crc = crc_update(crc, *data & i);

    for (i = 0; i < 16; i++)
        crc = crc_update(crc, 0);

    return crc;
}
static uint16_t swap16(uint16_t in)
{
    return (in >> 8) | ((in & 0xff) << 8);
}

void calc_crc(uint8_t* dest, uint8_t* buf, uint32_t len)
{
    uint16_t crc = swap16(crc16(buf, len));
    memcpy(dest, &crc, 2);
}

bool send_file(HANDLE hport, FILE* hfile, char* filename, uint32_t filesize)
{
    bool ret = false;

    flush_serial(hport);

    printf("wait for 'ready'\n");

    // await 'c'
    while ('C' != read_serial(hport))
        Sleep(1);

    printf("sending header\n");
    uint8_t header[PACKET_OVERHEAD + PACKET_SIZE] = { 0 };
    header[0] = SOH;
    header[1] = 0;
    header[2] = 0xff;
    char* p = (char*)header + 3;
    p += sprintf(p, "%s", filename);
    p++;
    p += sprintf(p, "%u", filesize);
    
    calc_crc(header + 128 + 3, header + 3, 128);
    write_serial(hport, header, sizeof(header));

    // see what the remote thinks of that
    if (ACK != read_serial(hport))
    {
        printf("did not receive ACK\n");
        goto err;
    }
    else
        printf("they like our header!\n");

    // await 'c'
    printf("wait for 'ready'\n");
    while ('C' != read_serial(hport))
        Sleep(1);

    uint8_t packet[PACKET_OVERHEAD + PACKET_1K_SIZE];

    uint32_t avail = filesize;
    uint8_t seq = 1;
    while (avail)
    {
        // form the packet
        uint32_t chunk = avail;
        if (chunk > PACKET_1K_SIZE)
            chunk = PACKET_1K_SIZE;
        memset(packet, 0, sizeof(packet));
        packet[0] = STX;
        packet[1] = seq;
        packet[2] = 0xff - seq;
        if (chunk != fread(packet + 3, 1, chunk, hfile))
        {
            printf("file read error!\n");
            goto err;
        }
        calc_crc(packet + PACKET_1K_SIZE + PACKET_HEADER, packet + PACKET_HEADER, PACKET_1K_SIZE);

        // send packet. we will try up to 5 times
        uint8_t attempts_left = 5;
        while (attempts_left)
        {
            printf("send packet %u (%u bytes remain): ", seq, avail);
            write_serial(hport, packet, sizeof(packet));

            // see what the remote thinks of that
            if (ACK != read_serial(hport))
            {
                printf("ERROR, ");
                if (attempts_left)
                    printf(" try again.\n");
                else
                    printf("\n");                
                attempts_left--;
            }
            else
            {
                printf("OK\n");
                break;
            }
        }

        if (!attempts_left)
        {
            printf("transfer failed :-(\n");
            goto err;
        }

        avail -= chunk;
        seq++;
    }

    uint32_t eot_attempts = 5;
    do
    {
        printf("send EOT\n");

        // send EOT
        uint8_t eot = EOT;
        write_serial(hport, &eot, 1);

        // see what the remote thinks of that
        if (ACK != read_serial(hport))
        {
            printf("did not receive ACK\n");
            Sleep(100);
        }
        else
            break;
    } while (eot_attempts--);

    // send final null packet
    // recycle header buffer
    header[0] = SOH;
    header[1] = 0;
    header[2] = 0xff;
    memset(header + 3, 0, 128);
    calc_crc(header + 128 + 3, header + 3, 128);

    uint32_t finish_attempts = 5;
    do
    {
        printf("send last null packet\n");

        write_serial(hport, header, sizeof(header));

        // see what the remote thinks of that
        if (ACK != read_serial(hport))
        {
            printf("did not receive ACK\n");
            Sleep(100);
        }
        else
            break;
    } while (finish_attempts--);

    rewind(hfile);
    return true;
err:
    rewind(hfile);
    return ret;
}

int main(int argc, char* argv[])
{
    uint8_t port;
    const char* filename;
    char portname[32];
    HANDLE hport = 0;
    FILE* hfile = 0;
    
    // enought args, com port can be parsed, file readable
    if (argc != 3 || 1 != sscanf(argv[1],"%hhu",&port))
    {
        printf("usage:\n");
        printf("ymodem_sender.exe [com port number] [file]\n");
        exit(0);
    }
    
    filename = argv[2];
    sprintf(portname, "\\\\.\\COM%u", port);
 
    hfile = fopen(argv[2], "rb");
    if (!hfile)
    {
        printf("could not open file \"%s\"\n", filename);
        goto end;
    }
    fseek(hfile, 0, SEEK_END);
    uint32_t filesize = ftell(hfile);
    rewind(hfile);

    hport = CreateFileA(portname, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hport == INVALID_HANDLE_VALUE)
    {
        printf("could not open port COM%u\n", port);
        goto end;
    }

    // configure the serial port 

    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(hport, &dcb))
    {
        printf("error retrieving com port settings\n");
        goto end;
    }

    dcb.BaudRate = CBR_115200;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;

    if (!SetCommState(hport, &dcb))
    {
        printf("error setting com port settings\n");
        goto end;
    }

    COMMTIMEOUTS timeouts = { 0 };

    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hport, &timeouts))
    {
        printf("error setting com port timeout settings\n");
        goto end;
    }

    // configure port for character receive recognition
    if (!SetCommMask(hport, EV_RXCHAR))
    {
        printf("error configuring port for character receive\n");
        goto end;
    }

    while (!send_file(hport, hfile, filename, filesize))
    {
        printf("error. will retry.\n");
    }

    printf("file send completed, will now monitor serial\n");
    printf("--------------------------------------------\n\n");

    while (1)
        printf("%c", read_serial(hport));

end:
    if (hport)
        CloseHandle(hport);
    if (hfile)
        fclose(hfile);
}

