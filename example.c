// example by Alex Free
#include "lib-ps-cd-id.h"
#include  <string.h>

FILE *bin;

int main (int argc, const char * argv[]) 
{
    printf("Lib PS-CD-ID example %s by Alex Free\n", VERSION);

    if(argc != 2) {
        printf("Error: incorrect number of arguments\n\nUsage:\n\nexample <track 1 bin file of PlayStation 1/2 CD game>\n");
        return 1;
    }

    bin = fopen(argv[1], "rb+");

    if(bin == NULL)
    {
        printf("Error: Can not open: %s\n", argv[1]);
        return 1;
    }
    
    int valid = is_ps_cd(bin);

    if(!valid)
    {
        printf("\n%s does not appears to be a PlayStation 1 or 2 CD image\n", argv[1]);
        return 1;
    } else if(valid) {
        printf("\n%s appears to be a PlayStation 1 or 2 CD image\n", argv[1]);

        int got_boot_file = get_boot_file_name(bin);
        
        if(got_boot_file)
        {
            printf("boot file: %s\n", boot_file);

            if((strncmp(boot_file, "SCES_028.35", 11) == 0)) // Spyro: Year of the Dragon (Europe) (En,Fr,De,Es,It) (Rev 0) / Spyro: Year of the Dragon (Europe) (En,Fr,De,Es,It) (Rev 1)
	        {
		        // 000093C7 09 0A
                int rev;
                rev = id_rev(bin, 0x93C7, 0x09, 0x0A);

		        if(rev == 0)
                {
			        printf("Rev 0\n");
        		} else if(rev == 1) {
			        printf("Rev 1\n");
                } else if (rev == 2) {
                    printf("Unknown version\n");
                }
            }
        } else if(!got_boot_file) {
            printf("Could not id %s\n", argv[1]);
        }
    }

    fclose(bin);
}