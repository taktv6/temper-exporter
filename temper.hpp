#pragma once

#include <usb.h>

namespace Temper {
    class Temper {
            int dev_num;
            usb_dev_handle *devHandle;

            void findDevice();
            void detach();
            void _detach(int iInterface);
            void iniControlTransfer();
            void controlTransfer(const char *pquestion);
            void interruptRead();
            float interruptReadTemp();

        public:
            Temper();
            ~Temper();
            float getTemp();
    };  
}
