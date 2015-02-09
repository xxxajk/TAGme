// hint for IDE
#include <Arduino.h>
#include "usb_settings.h"

uint8_t fromHost[MAX_USB_IO_SIZE];
uint8_t *dataFromHost = &fromHost[2];
uint8_t dataToHost[MAX_USB_IO_SIZE];
int dataFromHostSize;
int dataToHostSize;
uint16_t jtag_delay; // Delay in uS

#define RX rx_fromusb
#define TX tx_tousb
#define WriteToPin digitalWriteFast
#define ReadFromPin digitalReadFast

#define     JTAG_CMD_TAP_OUTPUT 0x00
#define       JTAG_CMD_SET_TRST 0x01
#define       JTAG_CMD_SET_SRST 0x02
#define     JTAG_CMD_READ_INPUT 0x03
#define JTAG_CMD_TAP_OUTPUT_EMU 0x04
#define      JTAG_CMD_SET_DELAY 0x05
#define  JTAG_CMD_SET_SRST_TRST 0x06
// These are not defined yet, but will be used for SWD, if I ever get to it.
#define             JTAG_CMD_R7 0x07
#define             JTAG_CMD_R8 0x08
#define             JTAG_CMD_R9 0x09
#define             JTAG_CMD_RA 0x0A
#define             JTAG_CMD_RB 0x0B
#define             JTAG_CMD_RC 0x0C
#define             JTAG_CMD_RD 0x0D
#define             JTAG_CMD_RE 0x0E
#define             JTAG_CMD_KINETIS_ERASE 0x0F // kinetis mass erase via EZ-port, to unwedge stuck chips.

//JTAG usb command mask
#define           JTAG_CMD_MASK 0x0f
#define          JTAG_DATA_MASK 0xf0

//output pins
// TEST DATA IN
#define            JTAG_PIN_TDI 9 // 3K R to ground
// TEST MODE SELECT
#define            JTAG_PIN_TMS 8 // 3K R to ground
// TEST RESET
#define           JTAG_PIN_TRST 7 // Not implemented yet
// SYSTEM RESET
#define           JTAG_PIN_SRST 12 // to system reset
// TEST CLOCK
#define            JTAG_PIN_TCK 11

//input pins
// TEST DATA OUT
#define            JTAG_PIN_TDO 10 // 3K R to ground
// Emulation data
#define            JTAG_PIN_EMU 6 // Not implemented yet
// Adaptive clocking TCK return signal (TCK sync)
#define           JTAG_PIN_RTCK 5 // Not implemented yet

#define                  EZP_CS 4 // Hold low and toggle JTAG_PIN_SRST to enter EZP SPI mode, or high for JTAG

#if 0
#define FLICK Serial1.print(v, HEX);
#define FLICKNL Serial1.println();
#else
#define FLICK
#define FLICKNL
#endif

#define FASTDELAYUP __asm__ volatile ("nop" :::);
#define SLOWDELAYUP delayMicroseconds(jtag_delay);
#define FASTDELAYDOWN delayMicroseconds(jtag_delay);
#define QUICKDELAYDOWN FASTDELAYUP FASTDELAYUP FASTDELAYUP FASTDELAYUP FASTDELAYUP FASTDELAYUP FASTDELAYUP

enum EZPORT_CMD {
        EZPORT_WREN = 0x06,
        EZPORT_WRDI = 0x04,
        EZPORT_RDSR = 0x05,
        EZPORT_READ = 0x03,
        EZPORT_FAST_READ = 0x0b,
        EZPORT_SP = 0x02,
        EZPORT_SE = 0xd8,
        EZPORT_BE = 0xc7,
        EZPORT_RESET = 0xb9,
        EZPORT_WRFCCOB = 0xba,
        EZPORT_FAST_RDFCCOB = 0xbb,
        EZPORT_WRFLEXRAM = 0xbc,
        EZPORT_RDFLEXRAM = 0xbd,
        EZPORT_FAST_RDFLEXRAM = 0xbe,
};

struct _ez_stat {
        uint8_t wip : 1;
        uint8_t wen : 1;
        uint8_t bedis : 1;
        uint8_t flexram : 1;
        uint8_t _rsvd0 : 2;
        uint8_t wef : 1;
        uint8_t fs : 1;
};

union ez_stat {
        _ez_stat status;
        uint8_t store;
};

extern "C" {

        // limitations: TX and RX are locked to the size of one packet.
        // TO-DO: Lift this restriction.

        // Get only *this* packet's data.

        int rx_fromusb(void *buffer, uint32_t rxsize) {
                unsigned int i;
                uint32_t c = 0;
                uint8_t *p = (uint8_t *)buffer;
                usb_packet_t *rx_packet;

                if(!usb_configuration) return -1; // Possibly Okay to return -1 for an error.
                rx_packet = usb_rx(RXEP);
                if(!rx_packet) return 0; // This is not an error.
                for(i = rx_packet->index; c < rxsize && i < rx_packet->len;) {
                        *p++ = rx_packet->buf[i++];
                        c++;
                }
                usb_free(rx_packet);
                rx_packet = NULL;
                return c;
        }

        // Do not exceed MAX_USB_IO_SIZE

        int tx_tousb(void *buffer, uint32_t txsize) {
                uint32_t len;
                const uint8_t *src = (const uint8_t *)buffer;
                uint8_t *dest;
                usb_packet_t *tx_packet;

                if(txsize > (MAX_USB_IO_SIZE)) return -2; // Too big
                while(1) {
                        if(!usb_configuration) return -1;
                        if(usb_tx_packet_count(TXEP) < 2) {
                                tx_packet = usb_malloc();
                                if(tx_packet) break;
                        }
                        yield();
                }
                dest = tx_packet->buf;
                len = txsize;
                while(len--) {
                        *dest++ = *src++;
                }
                tx_packet->len = txsize;
                usb_tx(TXEP, tx_packet);
                tx_packet = NULL;
                return (txsize);
        }
}

void setup() {
        //long i;
        //long j;

        pinMode(LED_BUILTIN, OUTPUT);
        //WriteToPin(JTAG_PIN_TCK, 0);

        pinMode(JTAG_PIN_TDI, OUTPUT);
        pinMode(JTAG_PIN_TMS, OUTPUT);
        pinMode(JTAG_PIN_TRST, OUTPUT);
        pinMode(JTAG_PIN_SRST, OUTPUT);
        pinMode(JTAG_PIN_TCK, OUTPUT);
        pinMode(EZP_CS, OUTPUT);

        pinMode(JTAG_PIN_TDO, OUTPUT);
        WriteToPin(JTAG_PIN_TDO, 0);
        pinMode(JTAG_PIN_TDO, INPUT);
        pinMode(JTAG_PIN_EMU, INPUT);
        pinMode(JTAG_PIN_RTCK, INPUT);

        // perform reset
        WriteToPin(EZP_CS, 1); // default to JTAG
        WriteToPin(JTAG_PIN_TRST, 0);
        WriteToPin(JTAG_PIN_SRST, 0);
        delay(100);
        WriteToPin(JTAG_PIN_TCK, 0);
        WriteToPin(JTAG_PIN_TDI, 1);
        WriteToPin(JTAG_PIN_TMS, 1);
        WriteToPin(JTAG_PIN_TRST, 1);
        WriteToPin(JTAG_PIN_SRST, 1);

        dataFromHostSize = 0;
        dataToHostSize = 0;
        jtag_delay = 0;
        Serial1.begin(115200);
        Serial1.println();

        Serial1.println();
        Serial1.println("READY.");

}

uint8_t jtag_tap_slowed(const uint8_t *out_buffer, uint16_t out_length, uint8_t *in_buffer) {
        uint8_t tms;
        uint8_t tdi;
        uint8_t out_data;
        uint8_t in_data = 0;
        uint16_t out_buffer_index = 0;
        uint16_t in_buffer_index = 0;
        uint16_t out_length_index = 0;
        uint8_t v;

        while(1) {
                //First TMS/TDI/TDO
                out_data = out_buffer[out_buffer_index++];
                tdi = out_data & 0x01;
                out_data = out_data >> 1;
                tms = out_data & 0x01;
                WriteToPin(JTAG_PIN_TDI, tdi);
                WriteToPin(JTAG_PIN_TMS, tms);
                WriteToPin(JTAG_PIN_TCK, 1);
                SLOWDELAYUP
                WriteToPin(JTAG_PIN_TCK, 0);
                FASTDELAYDOWN
                v = ReadFromPin(JTAG_PIN_TDO);
                in_data |= v ? 0x80 : 0;
                FLICK
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Second TMS/TDI/TDO
                out_data = out_data >> 1;
                tdi = out_data & 0x01;
                out_data = out_data >> 1;
                tms = out_data & 0x01;
                WriteToPin(JTAG_PIN_TDI, tdi);
                WriteToPin(JTAG_PIN_TMS, tms);
                WriteToPin(JTAG_PIN_TCK, 1);
                SLOWDELAYUP
                WriteToPin(JTAG_PIN_TCK, 0);
                FASTDELAYDOWN
                v = ReadFromPin(JTAG_PIN_TDO);
                in_data = in_data >> 1;
                in_data |= v ? 0x80 : 0;
                FLICK
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Third TMS/TDI/TDO
                out_data = out_data >> 1;
                tdi = out_data & 0x01;
                out_data = out_data >> 1;
                tms = out_data & 0x01;
                WriteToPin(JTAG_PIN_TDI, tdi);
                WriteToPin(JTAG_PIN_TMS, tms);
                WriteToPin(JTAG_PIN_TCK, 1);
                SLOWDELAYUP
                WriteToPin(JTAG_PIN_TCK, 0);
                FASTDELAYDOWN
                v = ReadFromPin(JTAG_PIN_TDO);
                in_data = in_data >> 1;
                in_data |= v ? 0x80 : 0;
                FLICK
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Fourth TMS/TDI/TDO
                out_data = out_data >> 1;
                tdi = out_data & 0x01;
                out_data = out_data >> 1;
                tms = out_data & 0x01;
                WriteToPin(JTAG_PIN_TDI, tdi);
                WriteToPin(JTAG_PIN_TMS, tms);
                WriteToPin(JTAG_PIN_TCK, 1);
                SLOWDELAYUP
                WriteToPin(JTAG_PIN_TCK, 0);
                FASTDELAYDOWN
                v = ReadFromPin(JTAG_PIN_TDO);
                in_data = in_data >> 1;
                in_data |= v ? 0x80 : 0;
                FLICK
                out_length_index++;
                //First TMS/TDI/TDO
                out_data = out_buffer[out_buffer_index++];
                tdi = out_data & 0x01;
                out_data = out_data >> 1;
                tms = out_data & 0x01;
                WriteToPin(JTAG_PIN_TDI, tdi);
                WriteToPin(JTAG_PIN_TMS, tms);
                WriteToPin(JTAG_PIN_TCK, 1);
                SLOWDELAYUP
                WriteToPin(JTAG_PIN_TCK, 0);
                FASTDELAYDOWN
                v = ReadFromPin(JTAG_PIN_TDO);
                in_data = in_data >> 1;
                in_data |= v ? 0x80 : 0;
                FLICK
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Second TMS/TDI/TDO
                out_data = out_data >> 1;
                tdi = out_data & 0x01;
                out_data = out_data >> 1;
                tms = out_data & 0x01;
                WriteToPin(JTAG_PIN_TDI, tdi);
                WriteToPin(JTAG_PIN_TMS, tms);
                WriteToPin(JTAG_PIN_TCK, 1);
                SLOWDELAYUP
                WriteToPin(JTAG_PIN_TCK, 0);
                FASTDELAYDOWN
                v = ReadFromPin(JTAG_PIN_TDO);
                in_data = in_data >> 1;
                in_data |= v ? 0x80 : 0;
                FLICK
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Third TMS/TDI/TDO
                out_data = out_data >> 1;
                tdi = out_data & 0x01;
                out_data = out_data >> 1;
                tms = out_data & 0x01;
                WriteToPin(JTAG_PIN_TDI, tdi);
                WriteToPin(JTAG_PIN_TMS, tms);
                WriteToPin(JTAG_PIN_TCK, 1);
                SLOWDELAYUP
                WriteToPin(JTAG_PIN_TCK, 0);
                FASTDELAYDOWN
                v = ReadFromPin(JTAG_PIN_TDO);
                in_data = in_data >> 1;
                in_data |= v ? 0x80 : 0;
                FLICK
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Fourth TMS/TDI/TDO
                out_data = out_data >> 1;
                tdi = out_data & 0x01;
                out_data = out_data >> 1;
                tms = out_data & 0x01;
                WriteToPin(JTAG_PIN_TDI, tdi);
                WriteToPin(JTAG_PIN_TMS, tms);
                WriteToPin(JTAG_PIN_TCK, 1);
                SLOWDELAYUP
                WriteToPin(JTAG_PIN_TCK, 0);
                FASTDELAYDOWN
                v = ReadFromPin(JTAG_PIN_TDO);
                in_data = in_data >> 1;
                in_data |= v ? 0x80 : 0;
                FLICK
                out_length_index++;
                in_buffer[in_buffer_index] = in_data;
                in_buffer_index++;
                in_data = 0;
                if(out_length_index >= out_length)
                        break;
        }
        FLICKNL
        if(out_length_index % 8)
                in_buffer[in_buffer_index] = in_data >> (8 - (out_length_index % 8));

        return (out_length + 7) / 8;
}

uint8_t jtag_tap(const uint8_t *out_buffer, uint16_t out_length, uint8_t *in_buffer) {
        if(jtag_delay) return jtag_tap_slowed(out_buffer, out_length, in_buffer);
        uint8_t tms;
        uint8_t tdi;
        uint8_t out_data;
        uint8_t in_data = 0;
        uint16_t out_buffer_index = 0;
        uint16_t in_buffer_index = 0;
        uint16_t out_length_index = 0;
        uint8_t v;

        while(1) {
                //First TMS/TDI/TDO
                out_data = out_buffer[out_buffer_index++];
                tdi = out_data & 0x01;
                out_data = out_data >> 1;
                tms = out_data & 0x01;
                WriteToPin(JTAG_PIN_TDI, tdi);
                WriteToPin(JTAG_PIN_TMS, tms);
                WriteToPin(JTAG_PIN_TCK, 1);
                FASTDELAYUP
                WriteToPin(JTAG_PIN_TCK, 0);
                QUICKDELAYDOWN
                v = ReadFromPin(JTAG_PIN_TDO);
                in_data |= v ? 0x80 : 0;
                FLICK
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Second TMS/TDI/TDO
                out_data = out_data >> 1;
                tdi = out_data & 0x01;
                out_data = out_data >> 1;
                tms = out_data & 0x01;
                WriteToPin(JTAG_PIN_TDI, tdi);
                WriteToPin(JTAG_PIN_TMS, tms);
                WriteToPin(JTAG_PIN_TCK, 1);
                FASTDELAYUP
                WriteToPin(JTAG_PIN_TCK, 0);
                QUICKDELAYDOWN
                v = ReadFromPin(JTAG_PIN_TDO);
                in_data = in_data >> 1;
                in_data |= v ? 0x80 : 0;
                FLICK
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Third TMS/TDI/TDO
                out_data = out_data >> 1;
                tdi = out_data & 0x01;
                out_data = out_data >> 1;
                tms = out_data & 0x01;
                WriteToPin(JTAG_PIN_TDI, tdi);
                WriteToPin(JTAG_PIN_TMS, tms);
                WriteToPin(JTAG_PIN_TCK, 1);
                FASTDELAYUP
                WriteToPin(JTAG_PIN_TCK, 0);
                QUICKDELAYDOWN
                v = ReadFromPin(JTAG_PIN_TDO);
                in_data = in_data >> 1;
                in_data |= v ? 0x80 : 0;
                FLICK
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Fourth TMS/TDI/TDO
                out_data = out_data >> 1;
                tdi = out_data & 0x01;
                out_data = out_data >> 1;
                tms = out_data & 0x01;
                WriteToPin(JTAG_PIN_TDI, tdi);
                WriteToPin(JTAG_PIN_TMS, tms);
                WriteToPin(JTAG_PIN_TCK, 1);
                FASTDELAYUP
                WriteToPin(JTAG_PIN_TCK, 0);
                QUICKDELAYDOWN
                v = ReadFromPin(JTAG_PIN_TDO);
                in_data = in_data >> 1;
                in_data |= v ? 0x80 : 0;
                FLICK
                out_length_index++;
                //First TMS/TDI/TDO
                out_data = out_buffer[out_buffer_index++];
                tdi = out_data & 0x01;
                out_data = out_data >> 1;
                tms = out_data & 0x01;
                WriteToPin(JTAG_PIN_TDI, tdi);
                WriteToPin(JTAG_PIN_TMS, tms);
                WriteToPin(JTAG_PIN_TCK, 1);
                FASTDELAYUP
                WriteToPin(JTAG_PIN_TCK, 0);
                QUICKDELAYDOWN
                v = ReadFromPin(JTAG_PIN_TDO);
                in_data = in_data >> 1;
                in_data |= v ? 0x80 : 0;
                FLICK
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Second TMS/TDI/TDO
                out_data = out_data >> 1;
                tdi = out_data & 0x01;
                out_data = out_data >> 1;
                tms = out_data & 0x01;
                WriteToPin(JTAG_PIN_TDI, tdi);
                WriteToPin(JTAG_PIN_TMS, tms);
                WriteToPin(JTAG_PIN_TCK, 1);
                FASTDELAYUP
                WriteToPin(JTAG_PIN_TCK, 0);
                QUICKDELAYDOWN
                v = ReadFromPin(JTAG_PIN_TDO);
                in_data = in_data >> 1;
                in_data |= v ? 0x80 : 0;
                FLICK
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Third TMS/TDI/TDO
                out_data = out_data >> 1;
                tdi = out_data & 0x01;
                out_data = out_data >> 1;
                tms = out_data & 0x01;
                WriteToPin(JTAG_PIN_TDI, tdi);
                WriteToPin(JTAG_PIN_TMS, tms);
                WriteToPin(JTAG_PIN_TCK, 1);
                FASTDELAYUP
                WriteToPin(JTAG_PIN_TCK, 0);
                QUICKDELAYDOWN
                v = ReadFromPin(JTAG_PIN_TDO);
                in_data = in_data >> 1;
                in_data |= v ? 0x80 : 0;
                FLICK
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Fourth TMS/TDI/TDO
                out_data = out_data >> 1;
                tdi = out_data & 0x01;
                out_data = out_data >> 1;
                tms = out_data & 0x01;
                WriteToPin(JTAG_PIN_TDI, tdi);
                WriteToPin(JTAG_PIN_TMS, tms);
                WriteToPin(JTAG_PIN_TCK, 1);
                FASTDELAYUP
                WriteToPin(JTAG_PIN_TCK, 0);
                QUICKDELAYDOWN
                v = ReadFromPin(JTAG_PIN_TDO);
                in_data = in_data >> 1;
                in_data |= v ? 0x80 : 0;
                FLICK
                out_length_index++;
                in_buffer[in_buffer_index] = in_data;
                in_buffer_index++;
                in_data = 0;
                if(out_length_index >= out_length)
                        break;
        }
        FLICKNL
        if(out_length_index % 8)
                in_buffer[in_buffer_index] = in_data >> (8 - (out_length_index % 8));

        return (out_length + 7) / 8;

}

uint8_t jtag_tap_output_emu_slowed(const uint8_t *out_buffer, uint16_t out_length, uint8_t *in_buffer) {
        uint8_t tms;
        uint8_t tdi;
        uint8_t out_data;
        uint8_t in_data = 0;
        uint16_t out_buffer_index = 0;
        uint16_t in_buffer_index = 0;
        uint16_t out_length_index = 0;

        while(1) {
                out_data = out_buffer[out_buffer_index++];

                //First TMS/TDI/TDO
                tdi = out_data & 0x01;
                tms = out_data & 0x02;
                WriteToPin(JTAG_PIN_TDI, tdi);
                FASTDELAYUP
                WriteToPin(JTAG_PIN_TMS, tms);
                FASTDELAYDOWN
                in_data = in_data >> 1;
                in_data |= ReadFromPin(JTAG_PIN_TDO) << 7;
                in_data = in_data >> 1;
                in_data |= ReadFromPin(JTAG_PIN_EMU) << 7;
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Second TMS/TDI/TDO
                out_data = out_data >> 2;
                tdi = out_data & 0x01;
                tms = out_data & 0x02;
                WriteToPin(JTAG_PIN_TDI, tdi);
                FASTDELAYUP
                WriteToPin(JTAG_PIN_TMS, tms);
                FASTDELAYDOWN
                in_data = in_data >> 1;
                in_data |= ReadFromPin(JTAG_PIN_TDO) << 7;
                in_data = in_data >> 1;
                in_data |= ReadFromPin(JTAG_PIN_EMU) << 7;
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Third TMS/TDI/TDO
                out_data = out_data >> 2;
                tdi = out_data & 0x01;
                tms = out_data & 0x02;
                WriteToPin(JTAG_PIN_TDI, tdi);
                FASTDELAYUP
                WriteToPin(JTAG_PIN_TMS, tms);
                FASTDELAYDOWN
                in_data = in_data >> 1;
                in_data |= ReadFromPin(JTAG_PIN_TDO) << 7;
                in_data = in_data >> 1;
                in_data |= ReadFromPin(JTAG_PIN_EMU) << 7;
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Fourth TMS/TDI/TDO
                out_data = out_data >> 2;
                tdi = out_data & 0x01;
                tms = out_data & 0x02;
                WriteToPin(JTAG_PIN_TDI, tdi);
                FASTDELAYUP
                WriteToPin(JTAG_PIN_TMS, tms);
                FASTDELAYDOWN
                in_data = in_data >> 1;
                in_data |= ReadFromPin(JTAG_PIN_TDO) << 7;
                in_data = in_data >> 1;
                in_data |= ReadFromPin(JTAG_PIN_EMU) << 7;
                out_length_index++;
                if(!(out_length_index % 4)) {
                        in_buffer[in_buffer_index] = in_data;
                        in_buffer_index++;
                        in_data = 0;
                }
                if(out_length_index >= out_length)
                        break;
        }

        if(out_length_index % 4)
                in_buffer[in_buffer_index] = in_data >> (8 - 2 * (out_length_index % 4));

        return (out_length + 3) / 4;
}

uint8_t jtag_tap_output_emu(const uint8_t *out_buffer, uint16_t out_length, uint8_t *in_buffer) {
        if(jtag_delay) return jtag_tap_output_emu_slowed(out_buffer, out_length, in_buffer);
        uint8_t tms;
        uint8_t tdi;
        uint8_t out_data;
        uint8_t in_data = 0;
        uint16_t out_buffer_index = 0;
        uint16_t in_buffer_index = 0;
        uint16_t out_length_index = 0;

        while(1) {
                out_data = out_buffer[out_buffer_index++];

                //First TMS/TDI/TDO
                tdi = out_data & 0x01;
                tms = out_data & 0x02;
                WriteToPin(JTAG_PIN_TDI, tdi);
                FASTDELAYUP
                WriteToPin(JTAG_PIN_TMS, tms);
                QUICKDELAYDOWN
                in_data = in_data >> 1;
                in_data |= ReadFromPin(JTAG_PIN_TDO) << 7;
                in_data = in_data >> 1;
                in_data |= ReadFromPin(JTAG_PIN_EMU) << 7;
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Second TMS/TDI/TDO
                out_data = out_data >> 2;
                tdi = out_data & 0x01;
                tms = out_data & 0x02;
                WriteToPin(JTAG_PIN_TDI, tdi);
                FASTDELAYUP
                WriteToPin(JTAG_PIN_TMS, tms);
                QUICKDELAYDOWN
                in_data = in_data >> 1;
                in_data |= ReadFromPin(JTAG_PIN_TDO) << 7;
                in_data = in_data >> 1;
                in_data |= ReadFromPin(JTAG_PIN_EMU) << 7;
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Third TMS/TDI/TDO
                out_data = out_data >> 2;
                tdi = out_data & 0x01;
                tms = out_data & 0x02;
                WriteToPin(JTAG_PIN_TDI, tdi);
                FASTDELAYUP
                WriteToPin(JTAG_PIN_TMS, tms);
                QUICKDELAYDOWN
                in_data = in_data >> 1;
                in_data |= ReadFromPin(JTAG_PIN_TDO) << 7;
                in_data = in_data >> 1;
                in_data |= ReadFromPin(JTAG_PIN_EMU) << 7;
                out_length_index++;
                if(out_length_index >= out_length)
                        break;

                //Fourth TMS/TDI/TDO
                out_data = out_data >> 2;
                tdi = out_data & 0x01;
                tms = out_data & 0x02;
                WriteToPin(JTAG_PIN_TDI, tdi);
                FASTDELAYUP
                WriteToPin(JTAG_PIN_TMS, tms);
                QUICKDELAYDOWN
                in_data = in_data >> 1;
                in_data |= ReadFromPin(JTAG_PIN_TDO) << 7;
                in_data = in_data >> 1;
                in_data |= ReadFromPin(JTAG_PIN_EMU) << 7;
                out_length_index++;
                if(!(out_length_index % 4)) {
                        in_buffer[in_buffer_index] = in_data;
                        in_buffer_index++;
                        in_data = 0;
                }
                if(out_length_index >= out_length)
                        break;
        }

        if(out_length_index % 4)
                in_buffer[in_buffer_index] = in_data >> (8 - 2 * (out_length_index % 4));

        return (out_length + 3) / 4;
}

uint8_t jtag_read_input(void) {
        uint8_t x;
        Serial1.print("read ");
        x = (ReadFromPin(JTAG_PIN_TDO) | (ReadFromPin(JTAG_PIN_EMU) << 1));
        Serial1.println(x);
        return x;
}

void jtag_set_srst(uint8_t srst) {
        WriteToPin(JTAG_PIN_SRST, srst);
}

void jtag_set_trst(uint8_t trst) {
        WriteToPin(JTAG_PIN_TRST, trst);
}

void jtag_set_trst_srst(uint8_t trst, uint8_t srst) {
        WriteToPin(JTAG_PIN_SRST, srst);
        WriteToPin(JTAG_PIN_TRST, trst);
}

// clock out one byte, while reading

uint8_t SPI_one_byte(uint8_t out_data) {
        uint8_t tdi;
        uint8_t v;
        uint8_t in_data = 0;

        // 7
        FASTDELAYUP
        tdi = (out_data & 0x80) == 0x80 ? 0x01 : 0;
        WriteToPin(JTAG_PIN_TDI, tdi);
        WriteToPin(JTAG_PIN_TCK, 0);
        FASTDELAYUP
        v = ReadFromPin(JTAG_PIN_TDO);
        WriteToPin(JTAG_PIN_TCK, 1);
        in_data |= v ? 0x01 : 0;
        in_data << 1;
        out_data << 1;

        // 6
        FASTDELAYUP
        tdi = (out_data & 0x80) == 0x80 ? 0x01 : 0;
        WriteToPin(JTAG_PIN_TDI, tdi);
        WriteToPin(JTAG_PIN_TCK, 0);
        FASTDELAYUP
        v = ReadFromPin(JTAG_PIN_TDO);
        WriteToPin(JTAG_PIN_TCK, 1);
        in_data |= v ? 0x01 : 0;
        in_data << 1;
        out_data << 1;

        // 5
        FASTDELAYUP
        tdi = (out_data & 0x80) == 0x80 ? 0x01 : 0;
        WriteToPin(JTAG_PIN_TDI, tdi);
        WriteToPin(JTAG_PIN_TCK, 0);
        FASTDELAYUP
        v = ReadFromPin(JTAG_PIN_TDO);
        WriteToPin(JTAG_PIN_TCK, 1);
        in_data |= v ? 0x01 : 0;
        in_data << 1;
        out_data << 1;

        // 4
        FASTDELAYUP
        tdi = (out_data & 0x80) == 0x80 ? 0x01 : 0;
        WriteToPin(JTAG_PIN_TDI, tdi);
        WriteToPin(JTAG_PIN_TCK, 0);
        FASTDELAYUP
        v = ReadFromPin(JTAG_PIN_TDO);
        WriteToPin(JTAG_PIN_TCK, 1);
        in_data |= v ? 0x01 : 0;
        in_data << 1;
        out_data << 1;

        // 3
        FASTDELAYUP
        tdi = (out_data & 0x80) == 0x80 ? 0x01 : 0;
        WriteToPin(JTAG_PIN_TDI, tdi);
        WriteToPin(JTAG_PIN_TCK, 0);
        FASTDELAYUP
        v = ReadFromPin(JTAG_PIN_TDO);
        WriteToPin(JTAG_PIN_TCK, 1);
        in_data |= v ? 0x01 : 0;
        in_data << 1;
        out_data << 1;

        // 2
        FASTDELAYUP
        tdi = (out_data & 0x80) == 0x80 ? 0x01 : 0;
        WriteToPin(JTAG_PIN_TDI, tdi);
        WriteToPin(JTAG_PIN_TCK, 0);
        FASTDELAYUP
        v = ReadFromPin(JTAG_PIN_TDO);
        WriteToPin(JTAG_PIN_TCK, 1);
        in_data |= v ? 0x01 : 0;
        in_data << 1;
        out_data << 1;

        // 1
        FASTDELAYUP
        tdi = (out_data & 0x80) == 0x80 ? 0x01 : 0;
        WriteToPin(JTAG_PIN_TDI, tdi);
        WriteToPin(JTAG_PIN_TCK, 0);
        FASTDELAYUP
        v = ReadFromPin(JTAG_PIN_TDO);
        WriteToPin(JTAG_PIN_TCK, 1);
        in_data |= v ? 0x01 : 0;
        in_data << 1;
        out_data << 1;

        // 0
        FASTDELAYUP
        tdi = (out_data & 0x80) == 0x80 ? 0x01 : 0;
        WriteToPin(JTAG_PIN_TDI, tdi);
        WriteToPin(JTAG_PIN_TCK, 0);
        FASTDELAYUP
        v = ReadFromPin(JTAG_PIN_TDO);
        WriteToPin(JTAG_PIN_TCK, 1);
        in_data |= v ? 0x01 : 0;

        return in_data;
}

ez_stat ezport_status;

void check_status() {
        WriteToPin(EZP_CS, 0); // select
        SPI_one_byte(EZPORT_RDSR);
        ezport_status.store = SPI_one_byte(0);
        WriteToPin(EZP_CS, 1); // deselect.
}

uint8_t write_protected() {
        return ((ezport_status.status.fs));
}

uint8_t erase_protected() {
        return ((ezport_status.status.bedis));
}

uint8_t write_busy() {
        return (ezport_status.status.wip);
}

void bulk_erase(void) {
        WriteToPin(EZP_CS, 0); // select
        SPI_one_byte(EZPORT_BE);
        WriteToPin(EZP_CS, 1); // deselect.
        do {
                WriteToPin(LED_BUILTIN, 0);
                delay(100);
                WriteToPin(LED_BUILTIN, 1);
                delay(100);
                check_status();
        } while(write_busy());
        delay(1000); // Delay 1 second, should be way more than enough.
}

uint8_t kinetis_mass_erase() {
        uint8_t rv = 0;
        // perform reset
        WriteToPin(EZP_CS, 0); // default to EZP
        WriteToPin(JTAG_PIN_TRST, 0);
        WriteToPin(JTAG_PIN_SRST, 0);
        WriteToPin(JTAG_PIN_TCK, 1);
        WriteToPin(JTAG_PIN_TDI, 1);
        WriteToPin(JTAG_PIN_TMS, 1);

        delay(100);

        WriteToPin(JTAG_PIN_TRST, 1);
        WriteToPin(JTAG_PIN_SRST, 1);
        delay(100);

        WriteToPin(EZP_CS, 1); // deselect.
#if 1
        check_status();
        if(erase_protected()) {
                rv = 1; // Erase protected, fail
        } else {
                // enable writes
                WriteToPin(EZP_CS, 0); // select
                SPI_one_byte(EZPORT_WREN);
                WriteToPin(EZP_CS, 1); // deselect.
                check_status();
                if(write_protected()) {
                        rv = 2; // Write protected, fail
                } else {
                        bulk_erase();
                }
        }
#endif
        // perform reset
        WriteToPin(EZP_CS, 1); // default to JTAG
        WriteToPin(JTAG_PIN_TRST, 0);
        WriteToPin(JTAG_PIN_SRST, 0);
        WriteToPin(JTAG_PIN_TCK, 0);
        WriteToPin(JTAG_PIN_TDI, 1);
        WriteToPin(JTAG_PIN_TMS, 1);
        delay(100);
        WriteToPin(JTAG_PIN_TRST, 1);
        WriteToPin(JTAG_PIN_SRST, 1);

        if(rv) {
                //for(int i = 0; i < 10; i++) {
                while(1) {
                        WriteToPin(LED_BUILTIN, 0);
                        delay(1000);
                        WriteToPin(LED_BUILTIN, 1);
                        delay(1000);
                }
        }
        return (rv);
}

void loop() {
        //while(1) {
        //        jtag_tap(dataFromHost,128,dataToHost);
        //}
        int n;
        int p;
        p = RX(fromHost, MAX_USB_IO_SIZE);
        //if(dataFromHostSize < 0) WriteToPin(LED_BUILTIN, 0);
        if(p > 1) {
                dataFromHostSize = fromHost[0] + (fromHost[1] << 8);
        } else {
                dataFromHostSize = 0;
        }
        if(dataFromHostSize > 0) {
                WriteToPin(LED_BUILTIN, 1);
                //                Serial1.print("Got ");
                //                Serial1.print(dataFromHostSize, HEX);
                //                Serial1.print(" Bytes, CMD ");
                //                for(n = 0; n < dataFromHostSize; n++) {
                //                        Serial1.print(dataFromHost[n], HEX);
                //                        Serial1.print(" ");
                //                }
                dataToHostSize = 0;
                switch(dataFromHost[0] & JTAG_CMD_MASK) {
                        case JTAG_CMD_TAP_OUTPUT:
                                dataFromHostSize--;
                                dataFromHostSize *= 4; // because it is 2bit packed.
                                if(dataFromHost[0] & JTAG_DATA_MASK) {
                                        dataFromHostSize -= (4 - ((dataFromHost[0] & JTAG_DATA_MASK) >> 4));
                                }
                                dataToHostSize = jtag_tap(&dataFromHost[1], dataFromHostSize, dataToHost);
                                break;
                        case JTAG_CMD_TAP_OUTPUT_EMU:
                                dataFromHostSize--;
                                dataFromHostSize *= 4; // because it is 2bit packed.
                                if(dataFromHost[0] & JTAG_DATA_MASK) {
                                        dataFromHostSize -= (4 - ((dataFromHost[0] & JTAG_DATA_MASK) >> 4));
                                }
                                dataToHostSize = jtag_tap_output_emu(&dataFromHost[1], dataFromHostSize, dataToHost);

                                break;

                        case JTAG_CMD_READ_INPUT:
                                dataToHost[0] = jtag_read_input();
                                dataToHostSize = 1;
                                break;

                        case JTAG_CMD_SET_SRST:
                                jtag_set_srst(dataFromHost[1]&1);
                                dataToHost[0] = 0; //TODO: what to output here?
                                dataToHostSize = 1;
                                break;

                        case JTAG_CMD_SET_TRST:
                                jtag_set_trst(dataFromHost[1]&1);
                                dataToHost[0] = 0; //TODO: what to output here?
                                dataToHostSize = 1;
                                break;

                        case JTAG_CMD_SET_DELAY:
                                jtag_delay = dataFromHost[1];
                                dataToHost[0] = 0; //TODO: what to output here?
                                dataToHostSize = 1;
                                break;

                        case JTAG_CMD_SET_SRST_TRST:
                                jtag_set_trst_srst((dataFromHost[1] >> 1) &1, dataFromHost[1]&1);
                                dataToHost[0] = 0; //TODO: what to output here?
                                dataToHostSize = 1;
                                break;

                        case JTAG_CMD_KINETIS_ERASE:
                                kinetis_mass_erase();
                                dataToHost[0] = 0;
                                dataToHostSize = 1;
                                break;

                        default: //REPORT ERROR?
                                break;

                }
                if(dataToHostSize) {
                        //                        Serial1.print("-> ");
                        //                        Serial1.println(dataToHostSize, HEX);
                        uint8_t *ptr = dataToHost;
                        if(dataToHostSize) {
#if 0
                                for(n = 0; n < dataToHostSize; n++) {
                                        if(dataToHost[n] < 0x0f) Serial1.print("0");
                                        Serial1.print(dataToHost[n], HEX);
                                        Serial1.print(" ");
                                }
                                Serial1.println(" ");
#endif
                                int sendsize = dataToHostSize;
                                if(sendsize > MAX_USB_IO_SIZE) sendsize = MAX_USB_IO_SIZE;
                                for(n = -1; n != sendsize; n = TX(ptr, sendsize));
                        }
                }

        }
        WriteToPin(LED_BUILTIN, 0);

}
