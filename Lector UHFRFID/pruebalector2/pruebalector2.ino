/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#include <M5Unified.h>
#include <M5GFX.h>
#include "UNIT_UHF_RFID.h"

M5GFX display;
Unit_UHF_RFID uhf;
String info = "";

void setup() {
    M5.begin();
    Serial.begin(115200);
    uhf.begin(&Serial2, 115200, 1, 2, false);
    while (1) {
        info = uhf.getVersion();
        if (info != "ERROR") {
            Serial.println(info);
            break;
        }
    }
    uhf.setTxPower(2600);
    M5.Display.fillRect(0, 0, 320, 240, WHITE);
    M5.Display.setTextColor(BLACK);
    M5.Display.setFont(&fonts::FreeMonoBold9pt7b);
    M5.Display.setCursor(0, 0);
    M5.Display.println("Unit RFID UHF init...");
}

uint8_t write_buffer[]  = {0xab, 0xcd, 0xef, 0xdd};
uint8_t reade_buffer[4] = {0};

void log(String info) {
    Serial.println(info);
    M5.Display.println(info);
}

void loop() {
        log("polling once");
        uint8_t result = uhf.pollingOnce();
        // result = pollingMultiple(uint16_t polling_count);  Can be scanned repeatedly multiple times.
        Serial.printf("scan result: %d\r\n", result);
        if (result > 0) {
            for (uint8_t i = 0; i < result; i++) {
                log("pc: " + uhf.cards[i].pc_str);
                log("rssi: " + uhf.cards[i].rssi_str);
                log("epc: " + uhf.cards[i].epc_str);
                log("-----------------");
                delay(10);
            }
        }
        delay(2000);
        M5.Display.fillScreen(WHITE);
        M5.Display.setCursor(0, 0);

        if (uhf.select(uhf.cards[0].epc)) {
            log("Select OK");
        } else {
            log("Select ERROR");
        }
        log("Current Select EPC:");
        log(uhf.selectInfo());
        delay(2000);
        M5.Display.fillScreen(WHITE);
        M5.Display.setCursor(0, 0);

        log("Write Data...");
        if (uhf.writeCard(write_buffer, sizeof(write_buffer), 0x04, 0, 0x00000000)) {
            log("Write OK");
        } else {
            log("Write ERROR");
        }
        delay(1000);
        log("Read Data...");
        if (uhf.readCard(reade_buffer, sizeof(reade_buffer), 0x04, 0, 0x00000000)) {
            log("Read OK");
            log("Data Content");
            for (uint8_t i = 0; i < sizeof(reade_buffer); i++) {
                Serial.printf("%x", reade_buffer[i]);
                M5.Display.printf("%x", reade_buffer[i]);
            }
        } else {
            log("Read ERROR");
        }
        delay(2000);
        M5.Display.fillScreen(WHITE);
        M5.Display.setCursor(0, 0);
} 