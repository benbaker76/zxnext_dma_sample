#define RTM_28MHZ               0x03

#define SAMPLE_BANK             0xC000
#define SAMPLE_MMU              MMU_C000

#define SAMPLE_BUFFER_SIZE      8192

#define printCls()				printf("%c", 12)
#define printAt(row, col, str)	printf("\x16%c%c%s", (col + 1), (row + 1), (str))
