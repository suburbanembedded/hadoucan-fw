#!/bin/bash

../firmware_encrypter/build/release/firmware_encrypter --key F0E1D2C3B4A5968778695A4B3C2D1E0F --iv 0F1E2D3C4B5A69788796A5B4C3D2E1F0 --infile ../hadoucan-fw/build/ram/release/hadoucan-fw/hadoucan-fw.bin --outfile ../hadoucan-fw/build/ram/release/hadoucan-fw/app.bin.enc --auxfile ../hadoucan-fw/build/ram/release/hadoucan-fw/app.bin.enc.xml

