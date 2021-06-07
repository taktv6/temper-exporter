#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <usb.h>
#include <string.h>
#include "temper.hpp"

const uint16_t VENDOR_ID = 0x0c45;
const uint16_t PRODUCT_ID = 0x7401;

#define INTERFACE1 0x00
#define INTERFACE2 0x01

#define DEVICE_CONFIG 0x01

const static int reqIntLen=8;
const static int timeoutMs=5000;

const static char uTemperature[] = { 0x01, -0x80, 0x33, 0x01, 0x00, 0x00, 0x00, 0x00 };

const static char uIni1[] = { 0x01, -0x7e, 0x77, 0x01, 0x00, 0x00, 0x00, 0x00 };

const static char uIni2[] = { 0x01, -0x7a, -0x01, 0x01, 0x00, 0x00, 0x00, 0x00 };

Temper::Temper::Temper() {
    try {
        usb_init();
        usb_find_busses();
        usb_find_devices();

        this->findDevice();

        this->detach();

        if (usb_set_configuration(this->devHandle, DEVICE_CONFIG) < 0) {
            throw std::runtime_error{"usb_set_configuration failed"};
        }

        if (usb_claim_interface(this->devHandle, INTERFACE1) < 0) {
            throw std::runtime_error{"usb_claim_interface failed for interface 1"};
        }

        if (usb_claim_interface(this->devHandle, INTERFACE2) < 0) {
            throw std::runtime_error{"usb_claim_interface failed for interface 2"};
        }

        this->iniControlTransfer();

        this->controlTransfer(uTemperature);
        this->interruptRead();

        this->controlTransfer(uIni1);
        this->interruptRead();

        this->controlTransfer(uIni2);
        this->interruptRead();
        this->interruptRead();
    } catch (...) {
        throw;
    }
}

Temper::Temper::~Temper() {
    usb_close(this->devHandle);
}

void Temper::Temper::detach() {
    try {
        this->_detach(INTERFACE1);
        this->_detach(INTERFACE2);
    } catch (...) {
        throw;
    }
}

void Temper::Temper::_detach(int iInterface) {
    auto ret = usb_detach_kernel_driver_np(this->devHandle, iInterface);
    if (!ret) {
        return;
    }

    if (errno == ENODATA) {
        return;
    }

    throw std::runtime_error{std::string("detach failed: ") + std::strerror(errno)};
    return;
}

void Temper::Temper::iniControlTransfer() {
    char q[] = { 0x01,0x01 };

    auto r = usb_control_msg(this->devHandle, 0x21, 0x09, 0x0201, 0x00, (char *) q, sizeof(q), timeoutMs);
    if (r < 0) {
        throw std::runtime_error{"Unable to transfer control"};
    }
}

void Temper::Temper::controlTransfer(const char *pquestion) {
    char q[reqIntLen];
    memcpy(q, pquestion, sizeof(q));

    if (auto r = usb_control_msg(this->devHandle, 0x21, 0x09, 0x0200, 0x01, (char *) q, reqIntLen, timeoutMs); r < 0) {
        throw std::runtime_error{std::string("usb_control_msg failed: ") + std::strerror(errno)};
    }
}

void Temper::Temper::interruptRead() {
    char answer[reqIntLen];
    std::memset(answer, 0, sizeof answer);

    if (auto r = usb_interrupt_read(this->devHandle, 0x82, answer, reqIntLen, timeoutMs); r != reqIntLen) {
        throw std::runtime_error{"usb_interrupt_read failed"};
    }
}

float Temper::Temper::interruptReadTemp() {
    char answer[reqIntLen];
    std::memset(answer, 0x00, sizeof answer);

    if (auto r = usb_interrupt_read(this->devHandle, 0x82, answer, reqIntLen, timeoutMs); r != reqIntLen) {
        throw std::runtime_error{"usb_interrupt_read failed"};
    }

    auto temp = (answer[3] & 0xFF) + (answer[2] << 8);
    return temp * (125.0 / 32000.0);
}

void Temper::Temper::findDevice() {
    struct usb_bus *bus;
    struct usb_device *dev;

    for (bus = usb_busses; bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
            if (dev->descriptor.idVendor != VENDOR_ID || dev->descriptor.idProduct != PRODUCT_ID ) {
                continue;
            }

            this->devHandle = usb_open(dev);
            if (!this->devHandle) {
                throw std::runtime_error{"usb_open failed"};
            }

            return;
        }
    }

    throw std::runtime_error{"device not found"};
}

float Temper::Temper::getTemp() {
    try {
        this->controlTransfer(uTemperature);
        return this->interruptReadTemp();
    } catch (...) {
        throw;
    }
}