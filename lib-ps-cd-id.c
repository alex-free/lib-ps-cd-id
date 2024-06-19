// PlayStation 1 and 2 CD Image Identification Library (Lib PS-CD-ID) by Alex Free (3-BSD license)

#include "lib-ps-cd-id.h"

char boot_file[11]; // returned by get_boot_file_name()

unsigned char psx_string[] = { // if it doesn't exist at known offset consider file invalid
    0x50, 0x4C, 0x41, 0x59, 0x53, 0x54, 0x41, 0x54, 0x49, 0x4F, 0x4E, 0x20 // PLAYSTATION<space>
};

int is_ps_cd(FILE *bin)
{
    // 10584000 bytes per minute * 71 minutes (max officially supported for PS1 and PS2 CD-ROMs) = 751464000 bytes max / 2352 bytes per sector = 319500 sector max
    // 10584000 bytes per minute * 80 minutes (max density read-able to any extent) = 846720000 bytes max / 2352 bytes per sector = 360000 sector max
    int sectors_max_71;
    sectors_max_71 = 319500;
    int sectors_max_80;
    sectors_max_80 = 360000;


    int bin_size;
    unsigned char check_for_match;

    fseek(bin, 0, SEEK_END);

    bin_size = ftell(bin);

    if(bin_size % 2352 != 0) // Minimum expected for correctness of detection functions below - 22-25
    {
	    printf("Error: bin file is not in MODE2/2352 format\n");
		return 0;
    } else if(bin_size < 0x930 * 25) { // Minimum expected for correctness of detection functions below - 22-25
	    printf("Error: bin file is too small to be valid\n");
		return 0;
    } else if((bin_size % 2352) > sectors_max_71) {
        printf("Info: Bin file is too large for a 71 Minute PSX-CDROM\n");
        if((bin_size % 2352) > sectors_max_80) 
        {
            printf("Error: Bin file is too large for even 80 Minute CD-Rs\n");
            return 0;
        }
    }

    bool is_psx_string;

	is_psx_string = true;

	for(int i =  0; i < 12; i++)
	{
		fseek(bin, (0x9320 + i), SEEK_SET);
		check_for_match = fgetc(bin);

		if(check_for_match != psx_string[i])
		{
			is_psx_string = false;
		}
	}

	if(!is_psx_string)
	{
		return 0;
	} else {
        return 1;
	}
}

int get_boot_file_name(FILE *bin) 
{
/*
info from https://problemkaputt.de/psx-spx.htm#hardwarenumbers

Software (CDROM Game Codes)
  SCES-NNNNN Sony Computer Europe Software
  SCED-NNNNN Sony Computer Europe Demo
  SLES-NNNNN Sony Licensed Europe Software
  SLED-NNNNN Sony Licensed Europe Demo
  SCPS-NNNNN Sony Computer Japan Software
  SLPS-NNNNN Sony Licensed Japan Software
  SLPM-NNNNN Sony Licensed Japan ... maybe promo/demo?
  SCUS-NNNNN Sony Computer USA Software
  SLUS-NNNNN Sony Licensed USA Software
  PAPX-NNNNN Demo ...?
  PCPX-NNNNN Club ...?
  LSP-NNNNNN Lightspan series (non-retail educational games)
Note: Multi-disc games have more than one game code. The game code for Disc 1 is also printed on the CD cover, and used in memory card filenames. The per-disk game codes are printed on the discs, and are used as boot-executable name in SYSTEM.CNF file. There is no fixed rule for the multi-disc numbering; some games are using increasing numbers of XNNNN or NNNNX (with X increasing from 0 upwards), and some are randomly using values like NNNXX and NNNYY for different discs.
*/
    bool match;
    int sector_byte;
    unsigned char possible_directory_record_sectors_buf[0x1800];

    fseek(bin, (0xCA20 + 0x18), SEEK_SET);// 0x930 * 22 = Directory Record Sector + 0x18 to skip header
            
    for(int directory_record_sector = 0; directory_record_sector < 3; directory_record_sector++){
        //printf("\n\nDirectory Record Sector %d\n\n", directory_record_sector);        
        for(int i=0; i < 0x800; i++)
        {
            possible_directory_record_sectors_buf[i + (directory_record_sector * 0x800)] = fgetc(bin);
            //printf("%02X ", possible_directory_record_sectors_buf[i + (directory_record_sector * 0x800)]);
            //printf("Directory Sector Multi: %d\n", (directory_record_sector * 0x800)) ;
        }
        fseek(bin, (0x118 + 0x18), SEEK_CUR); // skip EDC/EEC/Header
    }

    fseek(bin, 0, SEEK_SET);

    for(sector_byte=0; sector_byte < (0x1800 - 5); sector_byte++) // - 5 to avoid over shooting odd sized buffer, it won't miss anything
    {
        // Sony Computer Europe Software
        const unsigned char SCES[] = { 
            0x53, 0x43, 0x45, 0x53, 0x5F // SCES_
        };

        match = true;
        for(int i=0; i < 5; i++)
        {                
            if((SCES[i] != possible_directory_record_sectors_buf[sector_byte + i]))
            {
                match = false;
            }
        }

        if(match) 
        {
            break;
        }

        // Sony Computer Europe Demo
        const unsigned char SCED[] = { 
            0x53, 0x43, 0x45, 0x44, 0x5F // SCED_
        };

        match = true;
        for(int i=0; i < 5; i++)
        {                
            if((SCED[i] != possible_directory_record_sectors_buf[sector_byte + i]))
            {
                match = false;
            }
        }

        if(match) 
        {
            break;
        }
        
        // Sony Licensed Europe Software
        const unsigned char SLES[] = { 
            0x53, 0x4C, 0x45, 0x53 // SLES (We do not check for underscore because the  Formula One 99 (Europe) (En,Fr,De,It) (prototype 8/12/1999) bootfile name line in SYSTEM.CNF is BOOT = cdrom:\SLES1979.02;1)
        };

        match = true;
        for(int i=0; i < 4; i++)
        {                
            if((SLES[i] != possible_directory_record_sectors_buf[sector_byte + i]))
            {
                match = false;
            }
        }


        if(match) 
        {
            break;
        }

        // Sony Licensed Europe Demo
        const unsigned char SLED[] = { 
            0x53, 0x4C, 0x45, 0x53, 0x5F // SLED_
        };

        match = true;
        for(int i=0; i < 5; i++)
        {                
            if((SLED[i] != possible_directory_record_sectors_buf[sector_byte + i]))
            {
                match = false;
            }
        }

        if(match) 
        {
            break;
        }

        // Sony Computer Japan Software
        const unsigned char SCPS[] = { 
            0x53, 0x43, 0x50, 0x53, 0x5F // SCPS_
        };

        match = true;
        for(int i=0; i < 5; i++)
        {                
            if((SCPS[i] != possible_directory_record_sectors_buf[sector_byte + i]))
            {
                match = false;
            }
        }

        if(match) 
        {
            break;
        }

        // Sony Licensed Japan Software
        const unsigned char SLPS[] = { 
            0x53, 0x4C, 0x50, 0x53, 0x5F // SLPS_
        };

        match = true;
        for(int i=0; i < 5; i++)
        {                
            if((SLPS[i] != possible_directory_record_sectors_buf[sector_byte + i]))
            {
                match = false;
            }
        }

        if(match) 
        {
            break;
        }

        // Sony Licensed Japan ... maybe promo/demo?
        const unsigned char SLPM[] = { 
            0x53, 0x4C, 0x50, 0x4D, 0x5F // SLPM_
        };

        match = true;
        for(int i=0; i < 5; i++)
        {                
            if((SLPM[i] != possible_directory_record_sectors_buf[sector_byte + i]))
            {
                match = false;
            }
        }

        if(match) 
        {
            break;
        }

        // Sony Computer USA Software   
        const unsigned char SCUS[] = { 
            0x53, 0x43, 0x55, 0x53, 0x5F // SCUS_
        };

        match = true;
        for(int i=0; i < 5; i++)
        {                
            if((SCUS[i] != possible_directory_record_sectors_buf[sector_byte + i]))
            {
                match = false;
            }
        }

        if(match) 
        {
            break;
        }

        // Sony Licensed USA Software
        const unsigned char SLUS[] = { 
            0x53, 0x4C, 0x55, 0x53, 0x5F // SLUS_
        };

        match = true;
        for(int i=0; i < 5; i++)
        {                
            if((SLUS[i] != possible_directory_record_sectors_buf[sector_byte + i]))
            {
                match = false;
            }
        }

        if(match) 
        {
            break;
        }

        // Demo ...?
        const unsigned char PAPX[] = { 
            0x50, 0x41, 0x50, 0x58, 0x5F // PAPX_
        };

        match = true;
        for(int i=0; i < 5; i++)
        {                
            if((PAPX[i] != possible_directory_record_sectors_buf[sector_byte + i]))
            {
                match = false;
            }
        }

        if(match) 
        {
            break;
        }

        // Club ...?
        const unsigned char PCPX[] = { 
            0x50, 0x45, 0x50, 0x58, 0x5F // PCPX_
        };

        match = true;
        for(int i=0; i < 5; i++)
        {                
            if((PCPX[i] != possible_directory_record_sectors_buf[sector_byte + i]))
            {
                match = false;
            }
        }

        if(match) 
        {
            break;
        }
    }

    if(match)
    {
        for(int i = 0; i < 11; i++)
        {
            boot_file[i] = possible_directory_record_sectors_buf[sector_byte + i];
        }
        return 1;
    }

    printf("Error: Unsupported executable name prefix\n");
    return 0;
}

int id_rev(FILE *bin, const unsigned int difference_offset, const unsigned char old_byte, const unsigned char new_byte)
{
    unsigned char ver_check_val;
	// <Address with first difference> <original byte> <different byte from original at same address>
	fseek(bin, difference_offset, SEEK_SET);
	ver_check_val = fgetc(bin);
	fseek(bin, 0, SEEK_SET);

	if(ver_check_val == old_byte)
	{
	    return 0; // version containing old byte
	} else if(ver_check_val == new_byte) {
        return 1; // version containing new byte
    } else {
    	printf("Unknown version\n");
        return 2;
	}    
}
