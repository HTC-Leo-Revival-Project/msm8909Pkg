/*
 * nbgen.c
 *
 *  Created on: Mar 22, 2011
 *      Author: cedesmith
 */

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdint.h>


struct NbgPart
{
	char* fileName;
	int start;
	int end;
};
struct NbgData
{
	char header[2*0x800];
	int noParts;
	struct NbgPart parts[16];
};
struct NbgData data;

struct PartEntry {
  uint8_t BootInd;
  uint8_t FirstHead;
  uint8_t FirstSector;
  uint8_t FirstTrack;
  uint8_t FileSystem;
  uint8_t LastHead;
  uint8_t LastSector;
  uint8_t LastTrack;
  uint32_t StartSector;
  uint32_t TotalSectors;
};

//#define blocks(x)((x)/0x20000+((x)%0x20000!=0?1:0))
void save(char* file, int nb);
int blocks(size_t bytes);
void PartSetCHS(struct PartEntry* part);

int main(int argc, char* argv[])
{
	printf("nbgen v1.0 by cedesmith\n");
	if(argc<2){
		fprintf(stderr, "	Usage: os.nb|os.payload\n", argv[0]);
		return 1;
	}

	struct stat st;
	if(stat("lk.bin", &st)!=0){
		fprintf(stderr, "lk.bin not found\n", argv[1]);
		return 3;
	}
	data.parts[0].fileName="lk.bin";
	data.parts[0].start=2;
	data.parts[0].end=blocks(st.st_size+0x800*2)*64;
	data.noParts++;

	//make file header
	memset(data.header, 0, 0x800);										//fill 1st sector with 00
	data.header[0]=0xE9;												//fill signature bytes
	data.header[1]=0xFD;
	data.header[2]=0xFF;
	data.header[512-2]=0x55;
	data.header[512-1]=0xAA;
	//write partition
	struct PartEntry* part = (struct PartEntry*)(data.header+0x1BE);
	part->BootInd=0;
	part->FileSystem=0x23;
	part->StartSector=data.parts[0].start;
	part->TotalSectors=data.parts[0].end-data.parts[0].start;
	PartSetCHS(part);


	//write MSFLASH50
	memset(data.header+0x800, 0xFF, 0x800);								//fill 2nd sector with FF
	memset(data.header+0x800, 00, 0x64);
	strncpy(data.header+0x800, "MSFLSH50", sizeof("MSFLSH50")-1);		//MSFLSH50 signature

	//save
	int nb=0;
	if(strcasecmp(strrchr(argv[1], '.'), ".nb")==0) nb=1;
	save(argv[1], nb);
	return 0;
}
void PartSetCHS(struct PartEntry* part)
{
	uint32_t first=part->StartSector, last=part->StartSector+part->TotalSectors-1;
	part->FirstHead=(uint8_t)(first%0x40 & 0xFF);
	part->FirstTrack=(uint8_t)(first/0x40 & 0xFF);
	part->FirstSector=(uint8_t)(((((first/0x40)>>8)<<6) & 0xFF)+1);
	part->LastHead=(uint8_t) (last%0x40 & 0xFF);
	part->LastTrack=(uint8_t)(last/0x40 & 0xFF);
	part->LastSector=(uint8_t)(((((last/40)>>8)<<6) & 0xFF)+1);
}

int blocks(size_t bytes)
{
	// 1 physical nand block = 64 sectors = 0x20000 bytes
	return bytes/0x20000+(bytes%0x20000!=0 ? 1 : 0);
}
void writetag(uint32_t no, uint32_t tag, FILE* out)
{
	fwrite(&no, sizeof(no), 1, out);
	fwrite(&tag, sizeof(tag), 1, out);
}
void save(char* file, int nb)
{
	FILE* out=fopen(file, "w");
	if(out==NULL){
		fprintf(stderr, "failed to open %s\n", file);
		exit(20);
	}
	uint32_t sectorNo=0x0, tag=0xFFFBFFFD;

	// write file header block
	fwrite(data.header, 1, 0x800, out);
	if(nb) writetag(sectorNo, tag, out);
	sectorNo++;

	//write MSFLASH50 block
	fwrite(data.header+0x800, 1, 0x800, out);
	if(nb) writetag(sectorNo, tag, out);
	sectorNo++;

	char sector[0x800];
	for(int i=0; i<data.noParts; i++){
		struct NbgPart* part=&data.parts[i];

		//write empty sectors before part
		memset(sector, 0xFF, 0x800);
		for(int is=sectorNo; is<part->start; is++){
			fwrite(sector, 0x800, 1, out);
			if(nb) writetag(0xFFFFFFFF, 0xFFFFFFFF, out);
			sectorNo++;
		}

		//write part from file
		printf("writing  %s\n", part->fileName);
		FILE* in=fopen(part->fileName, "r");
		if(in==NULL){
			fprintf(stderr, "Failed to open %s\n", part->fileName);
			exit(21);
		}
		while(!feof(in)){
			int readed=0;
			memset(sector, 0xFF, 0x800);
			while(!feof(in) && readed<0x800) readed+=fread(sector+readed, 1, 0x800-readed, in);
			fwrite(sector, 0x800, 1, out);
			if(nb){
				tag=0xFFFFFFFF;
				for(int ib=0; ib<0x800; ib++)
					if(sector[ib]!=0xFF)
						tag=(i==0?0xFFFBFFFD:0xFFFBFFFF);
				if(tag!=0xFFFFFFFF) writetag(sectorNo, tag, out);
				else writetag(0xFFFFFFFF, 0xFFFFFFFF, out);
			}
			sectorNo++;
		}
		fclose(in);

		//printf("write empty sectors from %d to %d\n", sectorNo, part->end);
		//write empty sectors at the end
		memset(sector, 0xFF, 0x800);
		for(int is=sectorNo; is<part->end; is++){
			fwrite(sector, 0x800, 1, out);
			if(nb) writetag(0xFFFFFFFF, 0xFFFFFFFF, out);
			sectorNo++;
		}
	} //for(int i=0; i<data.noParts; i++)

	fclose(out);
}