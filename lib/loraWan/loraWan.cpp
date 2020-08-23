// Ensure the setting is correct in the LMIC config file located in
// LMIC library/project_config/lmic_project_config.h
// These two defines have no more effect than creating an error if the library setting
// is not correct
#define CFG_eu868 1
#define CFG_sx1276_radio 1
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "loraWan.h"
#include "config.h"

void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 10;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9,
    .dio = {2, 5, LMIC_UNUSED_PIN},
};

static volatile transmitionStatus_t txStatus;
static uint8_t * __rxPort;
static uint8_t * __rxBuffer;
static uint8_t * __rxSz;

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
             // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
            // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            txStatus =   TX_STATUS_UPLINK_SUCCESS;
            if (LMIC.txrxFlags & TXRX_ACK) {
              txStatus = TX_STATUS_UPLINK_ACKED;
              Serial.println(F("Received ack"));
            }
            if (LMIC.dataLen) {
              Serial.print(F("Received "));
              Serial.print(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
              if ( __rxPort != NULL) *__rxPort = LMIC.frame[LMIC.dataBeg-1];
              if ( __rxSz != NULL ) *__rxSz = LMIC.dataLen;
              if ( __rxBuffer != NULL ) memcpy(__rxBuffer,&LMIC.frame[LMIC.dataBeg],LMIC.dataLen);
              txStatus = TX_STATUS_UPLINK_ACKED_WITHDOWNLINK;
            }
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

/**
 * Setup the LoRaWan stack for TTN Europe
 */
void lorawan_setup() {
    txStatus = TX_STATUS_UNKNOWN;
    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();    
    LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);

    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
    LMIC_setLinkCheckMode(0);
    LMIC.dn2Dr = SF9;
    LMIC_setDrTxpow(DR_SF7,14);
}

/**
 * Send LoRaWan frame with ack or not
 * - txPort : port to transmit 
 * - txBuffer : message to transmit
 * - txSz : size of the message to transmit
 * - ack : true for message ack & downlink / false for pure uplink
 *   When Ack is false, the downlink buffer can be set to NULL as rxSz and rPort
 * - rxPort : where to write the port where downlink has been received
 * - rxBuffer : where the downlinked data will be stored
 * - rxSz : size of received data
 */
transmitionStatus_t lorawan_send(uint8_t txPort, uint8_t * txBuffer, uint8_t txSz, bool ack, uint8_t * rxPort, uint8_t * rxBuffer, uint8_t * rxSz){
  // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
        return TX_STATUS_ENDING_ERROR;
    } else {
        txStatus = TX_STATUS_UNKNOWN;
        __rxPort = rxPort;
        __rxBuffer = rxBuffer;
        __rxSz = rxSz;
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(txPort, txBuffer, txSz, ((ack)?1:0));
        // wait for completion
        uint64_t start = millis();
        while ( true ) {
          switch ( txStatus ) {
            case TX_STATUS_UNKNOWN:
            case TX_STATUS_JOINING:
            case TX_STATUS_JOINED:
                os_runloop_once();
                break;
            case TX_STATUS_UPLINK_SUCCESS:
            case TX_STATUS_UPLINK_ACKED:
            case TX_STATUS_UPLINK_ACKED_WITHDOWNLINK:
            case TX_STATUS_UPLINK_ACKED_WITHDOWNLINK_PENDING:
                return txStatus;
          }
          if ( millis() - start > LORA_TIMEOUT_MS ) {
            lorawan_setup();
            return TX_STATUS_TIMEOUT;
          }
        }        
    }
}