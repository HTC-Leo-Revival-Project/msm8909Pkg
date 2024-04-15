    // Storage - SD card
    //
    Device (SDC2)
    {
        Name (_HID, "QCOM2466")
        Name (_CID, "ACPI\QCOM2466")
        Name (_UID, 1)

        Method (_CRS, 0x0, NotSerialized) {
            Name (RBUF, ResourceTemplate ()
            {
                // SDCC2 register address space
                Memory32Fixed (ReadWrite, 0xA0400000, 0x00100000)
 
                //Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive, , , ) {FIXME}
     
            })
            Return (RBUF)
        }
 
        Device (EMMC) {
            Method (_ADR) {
                Return (8)
            }
 
            Method (_RMV) {
                Return (0)
            }
        }
    }