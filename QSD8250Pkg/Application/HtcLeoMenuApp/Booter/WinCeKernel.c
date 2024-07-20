/*
 * 2008 (c) STMicroelectronics, Inc.
 * Author: Ryan Chen <Ryan.Chen at st.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Opsycon AB, Sweden.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Windows CE Binary Image Data Format
 * The binary image (.bin) file format organizes data by sections. Each section contains 
 * a section header that specifies the starting address, length, and checksum for that 
 * section. 
 *
 * Refer to <http://msdn.microsoft.com/en-us/library/ms924510.aspx>
 *
 * The following table shows the .bin file format.
 * Field            Length (bytes)    Description 
 * Sync bytes (optional)    7        Byte 0 is B, indicating a .bin file format. 
 *                        Bytes 1-6 are reserved and set to 0, 0, 0, F, F, \n.
 * Run-time image address    4        Physical starting address of the run-time image. 
 * Run-time image length    4        Physical length, in bytes, of the run-time image. 
 * Record Address        4        Physical starting address of data record. 
 *                        If this value is zero, the Record Address is the end of 
 *                        the file, and record length contains the starting address 
 *                        of the run-time image.
 * Record length        4        Length of record data, in bytes. 
 * Record checksum        4        Signed 32-bit sum of record data bytes. 
 * Record data            Record length    Record data. 
 */
#define WINCE_IMAGE_SYNC_SIZE 7
#define WINCE_IMAGE_SYNC      "B000FF\n"

typedef struct {
    unsigned char sync_bytes[WINCE_IMAGE_SYNC_SIZE];
    unsigned int img_addr;
    unsigned int img_length;
} type_wince_image_header;

int check_sum(unsigned char * buf, int len, unsigned int checksum)
{
    unsigned int count,i;

    for (i = 0,count = 0 ; i < len ; i++)
        count += buf[i];

    if (count == checksum)
        return 0;

    return 1;
}

/* ======================================================================
 * Determine if a valid WinCE image exists at the given memory location.
 * First looks at the image header field, the makes sure that it is
 * WinCE image.
 * ====================================================================== */
int valid_wince_image (unsigned long addr)
{
    type_wince_image_header *p = (type_wince_image_header *)addr;

    if(strcmp((char *)p->sync_bytes, (char *)WINCE_IMAGE_SYNC) != 0)
        return 0;

    return 1;
}

/* ======================================================================
 * A very simple WinCE image loader, assumes the image is valid, returns the
 * entry point address.
 * ====================================================================== */
unsigned long load_wince_image (unsigned long addr)
{
    unsigned char *p = (unsigned char *)addr;
    u32 start_addr, total_length;
    u32 record_addr, record_length, record_checksum;
    u32 i = 0;

    if(valid_wince_image(addr) == 0)
        return ~0;

    printf("WINCE image is found: ");
    p += WINCE_IMAGE_SYNC_SIZE;
    start_addr = (u32)(p[3]<<24) + (u32)(p[2]<<16) + (u32)(p[1]<<8) + (u32)p[0];
    p += 4;
    total_length = (u32)(p[3]<<24) + (u32)(p[2]<<16) + (u32)(p[1]<<8) + (u32)p[0];
    printf(" Start Address = 0x%x @ Total Length = 0x%x\n", start_addr, total_length);
    p += 4;

    /* read each records */
    while(1) {
        record_length = (u32)(p[7]<<24) + (u32)(p[6]<<16) + (u32)(p[5]<<8) + (u32)p[4];
        record_checksum = (u32)(p[11]<<24) + (u32)(p[10]<<16) + (u32)(p[9]<<8) + (u32)p[8];
        record_addr = (u32)(p[3]<<24) + (u32)(p[2]<<16) + (u32)(p[1]<<8) + (u32)p[0];
        if(record_addr == 0)
            break;

        if(check_sum((unsigned char *)&p[12], record_length, record_checksum) != 0) {
            printf("Checksum Error!\n");
            return (unsigned long)~0;
        }
        memcpy ((void *)record_addr, (const void *)&p[12], (unsigned long)record_length);
        printf("Region %d: Loading from 0x%x to 0x%x @ Length 0x%x\n", i, (unsigned int)&p[12], \
            (unsigned int)record_addr, record_length);
        p = p + 12 + record_length;
        i++;
    }

    /* the lastest checksun should be zero */
    if(record_checksum != 0) {
        printf("Checksum Error!\n");
        return (unsigned long)~0;
    }

    /* the lastest length is entry address */
    return (unsigned long)record_length;
}

/* ======================================================================
 * Interpreter command to boot WinCE from a memory image.  The image can
 * be an WinCE image.  WinCE image do not need the
 * bootline and other parameters.
 * ====================================================================== */
int do_bootwince (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    unsigned long addr;             /* Address of image            */

    /*
     * Check the loadaddr variable.
     * If we don't know where the image is then we're done.
     */
    if (argc < 2)
        addr = load_addr;
    else
        addr = simple_strtoul (argv[1], NULL, 16);

#if defined(CONFIG_CMD_NET)
    /* Check to see if we need to tftp the image ourselves before starting */
    if ((argc == 2) && (strcmp (argv[1], "tftp") == 0)) {
        if (NetLoop (TFTP) <= 0)
            return 1;
        printf ("Automatic boot of WinCE image at address 0x%08lx ... \n", addr);
    }
#endif

    /*
     * If the data at the load address is an WinCE image, then
     * treat it like an WinCE image. Otherwise, return 1
     */
    if (valid_wince_image (addr)) {
        addr = load_wince_image (addr);
    } else {
        puts ("## Not an WinCE image, exit!\n");
        return 1;
        /* leave addr as load_addr */
    }

    printf ("## Starting Wince at 0x%08lx ...\n", addr);

    ((void (*)(void)) addr) ();

    puts ("## WinCE terminated\n");
    return 1;
}
