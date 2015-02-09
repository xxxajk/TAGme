// ############################################################################
// ############################################################################
// ############################################################################
// Defaults for everything
// ############################################################################
// ############################################################################
// ############################################################################
#ifndef VENDOR_ID
        #define VENDOR_ID                       0x16C0
#endif

#ifndef MANUFACTURER_NAME
        #define MANUFACTURER_NAME               {'T','e','e','n','s','y','d','u','i','n','o'}
#endif


// ############################################################################
#ifndef CDC_ACM_SIZE
        #define CDC_ACM_SIZE                    16
#endif
#ifndef CDC_RX_SIZE
        #define CDC_RX_SIZE                     64
#endif
#ifndef CDC_TX_SIZE
        #define CDC_TX_SIZE                     64
#endif

// ############################################################################
#ifndef SEREMU_TX_SIZE
        #define SEREMU_TX_SIZE                  64
#endif
#ifndef SEREMU_RX_SIZE
        #define SEREMU_RX_SIZE                  32
#endif
#ifndef SEREMU_TX_INTERVAL
        #define SEREMU_TX_INTERVAL              1
#endif
#ifndef SEREMU_RX_INTERVAL
        #define SEREMU_RX_INTERVAL              2
#endif

// ############################################################################
#ifndef KEYBOARD_SIZE
        #define KEYBOARD_SIZE                   8
#endif
#ifndef KEYBOARD_INTERVAL
        #define KEYBOARD_INTERVAL               1
#endif

// ############################################################################
#ifndef MOUSE_SIZE
        #define MOUSE_SIZE                      8
#endif
#ifndef MOUSE_INTERVAL
        #define MOUSE_INTERVAL                  1
#endif

// ############################################################################
#ifndef JOYSTICK_SIZE
        #define JOYSTICK_SIZE                   16
#endif
#ifndef JOYSTICK_INTERVAL
        #define JOYSTICK_INTERVAL               2
#endif

// ############################################################################
#ifndef MIDI_TX_SIZE
        #define MIDI_TX_SIZE                    64
#endif
#ifndef MIDI_RX_SIZE
        #define MIDI_RX_SIZE                    64
#endif

// ############################################################################
#ifndef RAWHID_TX_SIZE
        #define RAWHID_TX_SIZE                  64
#endif
#ifndef RAWHID_TX_INTERVAL
        #define RAWHID_TX_INTERVAL              1
#endif
#ifndef RAWHID_RX_SIZE
        #define RAWHID_RX_SIZE                  64
#endif
#ifndef RAWHID_RX_INTERVAL
        #define RAWHID_RX_INTERVAL              1
#endif

// ############################################################################
#ifndef FLIGHTSIM_TX_SIZE
        #define FLIGHTSIM_TX_SIZE               64
#endif
#ifndef FLIGHTSIM_TX_INTERVAL
        #define FLIGHTSIM_TX_INTERVAL           1
#endif
#ifndef FLIGHTSIM_RX_SIZE
        #define FLIGHTSIM_RX_SIZE               64
#endif
#ifndef FLIGHTSIM_RX_INTERVAL
        #define FLIGHTSIM_RX_INTERVAL           1
#endif

// ############################################################################
// ############################################################################
// ############################################################################
// This massive amount of macros does the following for any possible combination:
// 1: Calculates the entire size of the device descriptor
// 2: Calculates Total endpoints
// 3: Generates the endpoint size table macro
// 4: Generates the endpoint configuration table macro
// 5: Assigns an unique endpoint for each pipe
// ############################################################################
// ############################################################################
// ############################################################################
#ifdef CDC_IAD_DESCRIPTOR
        #define CDC_IAD_DESCRIPTOR_LEN          8
#else
        #define CDC_IAD_DESCRIPTOR_LEN          0
#endif

// ############################################################################
// ############################################################################
// ############################################################################
#ifdef CDC_DATA_INTERFACE
        #define CDC_DATA_INTERFACE_LEN          (9+5+5+4+5+7+9+7+7)
        #define CDC_DATA_INTERFACE_EPS          (1+1+2)
        #define CDC_DATA_INTERFACE_EPCF         ENDPOINT_UNUSED, ENDPOINT_TRANSIMIT_ONLY, ENDPOINT_RECEIVE_ONLY, ENDPOINT_TRANS
        #define CDC_DATA_INTERFACE_EPSZ         64, CDC_ACM_SIZE, CDC_RX_SIZE, CDC_TX_SIZE
        #define CDC_ACM_ENDPOINT                2
        #define CDC_RX_ENDPOINT                 3
        #define CDC_TX_ENDPOINT                 4
#else
        #define CDC_DATA_INTERFACE_LEN          0
        #define CDC_DATA_INTERFACE_EPS          0
        #define CDC_DATA_INTERFACE_IF           0
        #define CDC_TX_ENDPOINT                 0
#endif

// ############################################################################
// ############################################################################
// ############################################################################
#ifdef MIDI_INTERFACE
        #define MIDI_INTERFACE_LEN              (9+7+6+6+9+9+9+5+9+5)
        #define MIDI_INTERFACE_EPS              2
        #ifdef CDC_DATA_INTERFACE_EPCF
                #define MIDI_INTERFACE_EPCF     CDC_DATA_INTERFACE_EPCF, ENDPOINT_TRANSIMIT_ONLY, ENDPOINT_RECEIVE_ONLY
                #define MIDI_INTERFACE_EPSZ     CDC_DATA_INTERFACE_EPSZ, MIDI_TX_SIZE, MIDI_RX_SIZE
                #define MIDI_TX_ENDPOINT (CDC_TX_ENDPOINT+1)
                #define MIDI_RX_ENDPOINT (CDC_TX_ENDPOINT+2)
        #else
                #define MIDI_INTERFACE_EPCF     ENDPOINT_TRANSIMIT_ONLY, ENDPOINT_RECEIVE_ONLY
                #define MIDI_INTERFACE_EPSZ     MIDI_TX_SIZE, MIDI_RX_SIZE
                #define MIDI_TX_ENDPOINT 1
                #define MIDI_RX_ENDPOINT 2
        #endif
#else
        #define MIDI_INTERFACE_LEN 0
        #define MIDI_INTERFACE_EPS 0
        #ifdef CDC_DATA_INTERFACE_EPCF
                #define MIDI_INTERFACE_EPCF CDC_DATA_INTERFACE_EPCF
                #define MIDI_INTERFACE_EPSZ CDC_DATA_INTERFACE_EPSZ
        #endif
        #define MIDI_RX_ENDPOINT CDC_TX_ENDPOINT
#endif

// ############################################################################
// ############################################################################
// ############################################################################
#ifdef KEYBOARD_INTERFACE
        #define KEYBOARD_INTERFACE_LEN  (9+9+7)
        #define KEYBOARD_INTERFACE_EPS 1
        #ifdef MIDI_INTERFACE_EPCF
                #define KEYBOARD_INTERFACE_EPCF MIDI_INTERFACE_EPCF, ENDPOINT_TRANSIMIT_ONLY
                #define KEYBOARD_INTERFACE_EPSZ MIDI_INTERFACE_EPSZ, KEYBOARD_SIZE
                #define KEYBOARD_ENDPOINT     (MIDI_RX_ENDPOINT + 1)
        #else
                #define KEYBOARD_INTERFACE_EPCF ENDPOINT_TRANSIMIT_ONLY
                #define KEYBOARD_INTERFACE_EPSZ KEYBOARD_SIZE
                #define KEYBOARD_ENDPOINT     1
        #endif
#else
        #define KEYBOARD_INTERFACE_LEN 0
        #define KEYBOARD_INTERFACE_EPS 0
        #ifdef MIDI_INTERFACE_EPCF
                #define KEYBOARD_INTERFACE_EPCF MIDI_INTERFACE_EPCF
                #define KEYBOARD_INTERFACE_EPSZ MIDI_INTERFACE_EPSZ
        #endif
        #define KEYBOARD_ENDPOINT     MIDI_RX_ENDPOINT
#endif

// ############################################################################
// ############################################################################
// ############################################################################
#ifdef MOUSE_INTERFACE
        #define MOUSE_INTERFACE_LEN (9+9+7)
        #define MOUSE_INTERFACE_EPS 1
        #ifdef KEYBOARD_INTERFACE_EPCF
                #define MOUSE_INTERFACE_EPCF KEYBOARD_INTERFACE_EPCF, ENDPOINT_TRANSIMIT_ONLY
                #define MOUSE_INTERFACE_EPSZ KEYBOARD_INTERFACE_EPSZ, MOUSE_SIZE
                #define MOUSE_ENDPOINT        (KEYBOARD_ENDPOINT + 1)
        #else
                #define MOUSE_INTERFACE_EPCF ENDPOINT_TRANSIMIT_ONLY
                #define MOUSE_INTERFACE_EPSZ MOUSE_SIZE
                #define MOUSE_ENDPOINT        1
        #endif
#else
        #define MOUSE_INTERFACE_LEN 0
        #define MOUSE_INTERFACE_EPS 0
        #ifdef KEYBOARD_INTERFACE_EPCF
                #define MOUSE_INTERFACE_EPCF KEYBOARD_INTERFACE_EPCF
                #define MOUSE_INTERFACE_EPSZ KEYBOARD_INTERFACE_EPSZ
        #endif
        #define MOUSE_ENDPOINT KEYBOARD_ENDPOINT
#endif

// ############################################################################
// ############################################################################
// ############################################################################
#ifdef RAWHID_INTERFACE
        #define RAWHID_INTERFACE_LEN (9+9+7+7)
        #define RAWHID_INTERFACE_EPS 2
        #ifdef MOUSE_INTERFACE_EPCF
                #define RAWHID_INTERFACE_EPCF MOUSE_INTERFACE_EPCF, ENDPOINT_TRANSIMIT_ONLY, ENDPOINT_RECEIVE_ONLY
                #define RAWHID_INTERFACE_EPSZ MOUSE_INTERFACE_EPSZ, RAWHID_TX_SIZE,RAWHID_RX_SIZE
                #define RAWHID_TX_ENDPOINT    (MOUSE_ENDPOINT + 1)
                #define RAWHID_RX_ENDPOINT    (MOUSE_ENDPOINT + 2)
        #else
                #define RAWHID_INTERFACE_EPCF ENDPOINT_TRANSIMIT_ONLY, ENDPOINT_RECEIVE_ONLY
                #define RAWHID_INTERFACE_EPSZ RAWHID_TX_SIZE, RAWHID_RX_SIZE
                #define RAWHID_TX_ENDPOINT    1
                #define RAWHID_RX_ENDPOINT    2
        #endif
#else
        #define RAWHID_INTERFACE_LEN 0
        #define RAWHID_INTERFACE_EPS 0
        #ifdef MOUSE_INTERFACE_EPCF
                #define RAWHID_INTERFACE_EPCF MOUSE_INTERFACE_EPCF
                #define RAWHID_INTERFACE_EPSZ MOUSE_INTERFACE_EPSZ
        #endif
                #define RAWHID_RX_ENDPOINT    MOUSE_ENDPOINT

#endif

// ############################################################################
// ############################################################################
// ############################################################################
#ifdef FLIGHTSIM_INTERFACE
        #define FLIGHTSIM_INTERFACE_LEN (9+9+7+7)
        #define FLIGHTSIM_INTERFACE_EPS 2
        #ifdef RAWHID_INTERFACE_EPCF
                #define FLIGHTSIM_INTERFACE_EPCF RAWHID_INTERFACE_EPCF, ENDPOINT_TRANSIMIT_ONLY, ENDPOINT_RECEIVE_ONLY
                #define FLIGHTSIM_INTERFACE_EPSZ RAWHID_INTERFACE_EPSZ, FLIGHTSIM_TX_SIZE, FLIGHTSIM_RX_SIZE
                #define FLIGHTSIM_TX_ENDPOINT   (RAWHID_RX_ENDPOINT + 1)
                #define FLIGHTSIM_RX_ENDPOINT   (RAWHID_RX_ENDPOINT + 2)
        #else
                #define FLIGHTSIM_INTERFACE_EPCF ENDPOINT_TRANSIMIT_ONLY, ENDPOINT_RECEIVE_ONLY
                #define FLIGHTSIM_INTERFACE_EPSZ FLIGHTSIM_TX_SIZE, FLIGHTSIM_RX_SIZE
                #define FLIGHTSIM_TX_ENDPOINT   1
                #define FLIGHTSIM_RX_ENDPOINT   2
        #endif
#else
        #define FLIGHTSIM_INTERFACE_LEN 0
        #define FLIGHTSIM_INTERFACE_EPS 0
        #ifdef RAWHID_INTERFACE_EPCF
                #define FLIGHTSIM_INTERFACE_EPCF RAWHID_INTERFACE_EPCF
                #define FLIGHTSIM_INTERFACE_EPSZ RAWHID_INTERFACE_EPSZ
        #endif
        #define FLIGHTSIM_RX_ENDPOINT   RAWHID_RX_ENDPOINT

#endif

// ############################################################################
// ############################################################################
// ############################################################################
#ifdef SEREMU_INTERFACE
        #define SEREMU_INTERFACE_LEN (9+9+7+7)
        #define SEREMU_INTERFACE_EPS 2
        #ifdef FLIGHTSIM_INTERFACE_EPCF
                #define SEREMU_INTERFACE_EPCF FLIGHTSIM_INTERFACE_EPCF, ENDPOINT_TRANSIMIT_ONLY, ENDPOINT_RECEIVE_ONLY
                #define SEREMU_INTERFACE_EPSZ FLIGHTSIM_INTERFACE_EPSZ, SEREMU_TX_SIZE, SEREMU_RX_SIZE
                #define SEREMU_TX_ENDPOINT    (FLIGHTSIM_RX_ENDPOINT + 1)
                #define SEREMU_RX_ENDPOINT    (FLIGHTSIM_RX_ENDPOINT + 2)
        #else
                #define SEREMU_INTERFACE_EPCF ENDPOINT_TRANSIMIT_ONLY, ENDPOINT_RECEIVE_ONLY
                #define SEREMU_INTERFACE_EPSZ SEREMU_TX_SIZE, SEREMU_RX_SIZE
                #define SEREMU_TX_ENDPOINT    1
                #define SEREMU_RX_ENDPOINT    2
        #endif
#else
        #define SEREMU_INTERFACE_LEN 0
        #define SEREMU_INTERFACE_EPS 0
        #ifdef FLIGHTSIM_INTERFACE_EPCF
                #define SEREMU_INTERFACE_EPCF FLIGHTSIM_INTERFACE_EPCF
                #define SEREMU_INTERFACE_EPSZ FLIGHTSIM_INTERFACE_EPSZ
        #endif
        #define SEREMU_RX_ENDPOINT    FLIGHTSIM_RX_ENDPOINT

#endif

// ############################################################################
// ############################################################################
// ############################################################################
#ifdef JOYSTICK_INTERFACE
        #define JOYSTICK_INTERFACE_LEN (9+9+7)
        #define JOYSTICK_INTERFACE_EPS 1
        #ifdef SEREMU_INTERFACE_ECF
                #define JOYSTICK_INTERFACE_EPCF SEREMU_INTERFACE_EPCF, ENDPOINT_TRANSIMIT_ONLY
                #define JOYSTICK_INTERFACE_EPSZ SEREMU_INTERFACE_EPSZ, JOYSTICK_SIZE
                #define JOYSTICK_ENDPOINT     (SEREMU_RX_ENDPOINT + 1)
        #else
                #define JOYSTICK_INTERFACE_EPCF ENDPOINT_TRANSIMIT_ONLY
                #define JOYSTICK_INTERFACE_EPSZ JOYSTICK_SIZE
                #define JOYSTICK_ENDPOINT     1
        #endif
#else
        #define JOYSTICK_INTERFACE_LEN 0
        #define JOYSTICK_INTERFACE_EPS 0
        #ifdef SEREMU_INTERFACE_EPCF
                #define JOYSTICK_INTERFACE_EPCF SEREMU_INTERFACE_EPCF
                #define JOYSTICK_INTERFACE_EPSZ SEREMU_INTERFACE_EPSZ
        #endif
        #define JOYSTICK_ENDPOINT     SEREMU_RX_ENDPOINT

#endif

// ############################################################################
// ############################################################################
// ############################################################################
#ifdef USB_CUSTOM_INTERFACE
        #ifndef USB_CUSTOM_INTERFACE_LEN
                #error "USB_CUSTOM_INTERFACE_LEN not defined in sketch_settings.h"
        #endif
        #ifndef USB_CUSTOM_INTERFACE_EPS
                #error "USB_CUSTOM_INTERFACE_EPS not defined in sketch_settings.h"
        #endif

        #ifdef USB_CUSTOM_INTERFACE_EPCFG
                #ifdef JOYSTICK_INTERFACE_EPCF
                        #define USB_INTERFACE_EPCF JOYSTICK_INTERFACE_EPCF, USB_CUSTOM_INTERFACE_EPCFG
                        #define USB_INTERFACE_EPSZ JOYSTICK_INTERFACE_EPSZ, USB_CUSTOM_INTERFACE_EPSZ
                #else
                        #define USB_INTERFACE_EPCF USB_CUSTOM_INTERFACE_EPCFG
                        #define USB_INTERFACE_EPSZ USB_CUSTOM_INTERFACE_EPSZ
                #endif
        #else
                #ifdef JOYSTICK_INTERFACE_EPCF
                        #define USB_INTERFACE_EPCF JOYSTICK_INTERFACE_EPCF
                        #define USB_INTERFACE_EPSZ JOYSTICK_INTERFACE_EPSZ
                #else
                        #define USB_INTERFACE_EPCF
                        #define USB_INTERFACE_EPSZ
                #endif
        #endif
#else
        #define USB_CUSTOM_INTERFACE_LEN 0
        #define USB_CUSTOM_INTERFACE_EPS 0
        #ifdef JOYSTICK_INTERFACE_EPCF
                #define USB_INTERFACE_EPCF JOYSTICK_INTERFACE_EPCF
                #define USB_INTERFACE_EPSZ JOYSTICK_INTERFACE_EPSZ
        #endif
#endif
// WHEW!

// Let GCC calculate the endpoint counts for us
#define TOTAL_NUM_ENDPOINTS ( \
        CDC_DATA_INTERFACE_EPS + MIDI_INTERFACE_EPS + KEYBOARD_INTERFACE_EPS + \
        MOUSE_INTERFACE_EPS + RAWHID_INTERFACE_EPS + FLIGHTSIM_INTERFACE_EPS + \
        SEREMU_INTERFACE_EPS + JOYSTICK_INTERFACE_EPS + USB_CUSTOM_INTERFACE_EPS)

#if (TOTAL_NUM_ENDPOINTS > 15)
#error "Total endpoint count is more than 15, too many interfaces."
#endif
#if TOTAL_NUM_ENDPOINTS
// This macro can be used throughout the code to 'disable' the teensyduino USB code
#define NUM_ENDPOINTS TOTAL_NUM_ENDPOINTS
#endif

#ifdef NUM_ENDPOINTS

// Why count manually when compiler can do this for us!
#define MANUFACTURER_NAME_LEN sizeof((char [])MANUFACTURER_NAME)
#define PRODUCT_NAME_LEN sizeof((char [])PRODUCT_NAME)
#define MAIN_DEVICE_DESCRIPTOR_LEN              9

// Let GCC calculate Descriptor offsets for us.
// Length numbers are from the sections in the descriptor.
// These values are in this exact order
#define CDC_IAD_DESCRIPTOR_OFFSET               (MAIN_DEVICE_DESCRIPTOR_LEN)
#define CDC_DATA_INTERFACE_OFFSET               (CDC_IAD_DESCRIPTOR_OFFSET + CDC_IAD_DESCRIPTOR_LEN)
#define MIDI_INTERFACE_OFFSET                   (CDC_DATA_INTERFACE_OFFSET + CDC_DATA_INTERFACE_LEN)
#define KEYBOARD_DESC_OFFSET                    (MIDI_INTERFACE_OFFSET + MIDI_INTERFACE_LEN)
#define MOUSE_DESC_OFFSET                       (KEYBOARD_DESC_OFFSET + KEYBOARD_INTERFACE_LEN)
#define RAWHID_DESC_OFFSET                      (MOUSE_DESC_OFFSET + MOUSE_INTERFACE_LEN)
#define FLIGHTSIM_DESC_OFFSET                   (RAWHID_DESC_OFFSET + RAWHID_INTERFACE_LEN)
#define SEREMU_DESC_OFFSET                      (FLIGHTSIM_DESC_OFFSET + FLIGHTSIM_INTERFACE_LEN)
#define JOYSTICK_DESC_OFFSET                    (SEREMU_DESC_OFFSET + SEREMU_INTERFACE_LEN)
#define USB_CUSTOM_OFFSET                       (JOYSTICK_DESC_OFFSET + JOYSTICK_INTERFACE_LEN)

#define CONFIG_DESC_SIZE (USB_CUSTOM_OFFSET + USB_CUSTOM_INTERFACE_LEN)
#endif

// Every composite device uses this size for EndPoint Zero.
#define EP0_SIZE                64

#include <usb_mem.h>

#ifdef __cplusplus
extern "C" {
#endif

void usb_init(void);
void usb_init_serialnumber(void);
void usb_isr(void);
usb_packet_t *usb_rx(uint32_t endpoint);
uint32_t usb_tx_byte_count(uint32_t endpoint);
uint32_t usb_tx_packet_count(uint32_t endpoint);
void usb_tx(uint32_t endpoint, usb_packet_t *packet);
void usb_tx_isr(uint32_t endpoint, usb_packet_t *packet);

extern volatile uint8_t usb_configuration;

extern uint16_t usb_rx_byte_count_data[NUM_ENDPOINTS];
static inline uint32_t usb_rx_byte_count(uint32_t endpoint) __attribute__((always_inline));
static inline uint32_t usb_rx_byte_count(uint32_t endpoint)
{
        endpoint--;
        if (endpoint >= NUM_ENDPOINTS) return 0;
        return usb_rx_byte_count_data[endpoint];
}

#ifdef CDC_DATA_INTERFACE
extern uint32_t usb_cdc_line_coding[2];
extern volatile uint8_t usb_cdc_line_rtsdtr;
extern volatile uint8_t usb_cdc_transmit_flush_timer;
extern void usb_serial_flush_callback(void);
#endif

#ifdef SEREMU_INTERFACE
extern volatile uint8_t usb_seremu_transmit_flush_timer;
extern void usb_seremu_flush_callback(void);
#endif

#ifdef KEYBOARD_INTERFACE
extern uint8_t keyboard_modifier_keys;
extern uint8_t keyboard_keys[6];
extern uint8_t keyboard_protocol;
extern uint8_t keyboard_idle_config;
extern uint8_t keyboard_idle_count;
extern volatile uint8_t keyboard_leds;
#endif

#ifdef MIDI_INTERFACE
extern void usb_midi_flush_output(void);
#endif

#ifdef FLIGHTSIM_INTERFACE
extern void usb_flightsim_flush_callback(void);
#endif

extern const uint8_t usb_endpoint_config_table[NUM_ENDPOINTS];

typedef struct {
        uint16_t        wValue;
        uint16_t        wIndex;
        const uint8_t   *addr;
        uint16_t        length;
} usb_descriptor_list_t;

extern const usb_descriptor_list_t usb_descriptor_list[];

#ifdef __cplusplus
}
#endif
