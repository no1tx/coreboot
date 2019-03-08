#!/bin/bash
make
cbfstool build/coreboot.rom add-payload -f UEFIPAYLOAD.fd -n img/tianocore
cbfstool build/coreboot.rom add -f font.pf2 -n font.pf2 -t raw
cbfstool build/coreboot.rom add -f background.tga -n background.tga -t raw
