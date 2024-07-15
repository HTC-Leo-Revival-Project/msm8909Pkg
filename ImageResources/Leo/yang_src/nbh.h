#ifndef _INCLUDE_NBH_H
#define _INCLUDE_NBH_H_ 

struct HTCIMAGEHEADER {
	char device[32];
	unsigned long sectiontypes[32];
	unsigned long sectionoffsets[32];
	unsigned long sectionlengths[32];
	char CID[32];
	char version[16];
	char language[16];
};

extern struct HTCIMAGEHEADER HTCIMAGEHEADER;

/* nbh.c */
int bufferedReadWrite(FILE *input, FILE *output, unsigned long length);

/* nbhextract.c */
void extractNBH(char *filename);

#endif