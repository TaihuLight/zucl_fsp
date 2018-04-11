#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MaxFrames 5000
#define NoOfBRAMContRows 2	// Number of BRAM content rows

/* prototypes */
void mygetopt(int argc, char **argv);
int parsePartial(int argc, char **argv, int i, int j);
void help(void);
int file_handling(void);
unsigned int GetNextWord(FILE *ptr, int *flag);
int updateFLR(int val, int who);
int getColIndex(void);

void dumpPartial(void);
int getBufferByte(int BufferIndex, int Frame, int ByteIndex);
int getBufferBytePartial(int Frame, int ByteIndex);

int ByteSwap(unsigned int *inData, unsigned int *outData, int inSize);

int FillBuffer(FILE *ptr, int WordCount, int BufferIndex, bool isPartial);
void bufferHeaderAndFooter(void);
int WriteFullBitstream(char *FullBitstreamFileName);
int WriteFullBitstreamPartial(char *ModuleBitstreamFileName);
int WriteModuleBitstream(char *ModuleBitstreamFileName);
int WriteModuleBitstreamHeader(FILE *OutBitfilePtr);
int WriteModuleBitstreamCommand(FILE *OutBitfilePtr);
int WriteModuleBitstreamFooter(FILE *OutBitfilePtr);
int WriteModuleBitstreamData(FILE *OutBitfilePtr, int BA, int Offset, int Blocks, int FramesPerBlock);

int WritePartialBitstream(char *PartialBitstreamFileName);
int WriteDebugPartialBitstream(char *PartialBitstreamFileName);

void scan(FILE *in_ptr, int BufferIndex);

void echoCLBs(void);
int echoRAMs(char mode);
void echoCLKs(void);

void InitPartialMask(void);
void echoTXT(int level,char *msg);

void ParseRsgOptions(char **argv, int i, char type);
void ParseBitManipulations(char **argv, int i, char type);
void SetBit(int buffer, int col, int row, int offset);
void ClearBit(int buffer, int col, int row, int offset);
int TestBit(int buffer, int col, int row, int offset);
int TestRSGUsage(int buffer, int col, int row);
void ParseBoundingBox(char **argv, int i, int *left, int *right, int *top, int *bottom);
void ParseClbMask(char **argv, int i);
void ParseBRamMask(char **argv, int i);
void ParseBRamRoutingMask(char **argv, int i);
int CheckPartialMaskHight(void);
void ParseLinkPitFileOptions(char **argv, int i);
int LinkPitFile(char *PitFileName, int col, int row);
int UpdateRSG(unsigned Value, int Col, int Row, char type);
int ParsePitHeader(FILE *PitPtr, int *Top, int *Bottom, char ModuleDevice[]);
int CheckLinkModulePlacement(char ModuleDevice[], int col, int row, int Top, int Bottom);
int GetLeftResourceStringIndex(int col);
int GetModuleHight(char Device[], int Top, int Bottom);
int GetModuleStartRowByteIndex(char Device[], int row);
int GetPitStreamSize(char ModuleDevice[], int ModuleHight);
int GetNumberOfResourceColumns(unsigned char Resource);
int GetNumberOfSkippedResourceColumns(unsigned char Resource);
int countOtherColumns(int col, char countSymbol, char matchSymbol);
int WriteBufferByte(int BufferIndex, int Frame, int ByteIndex, char WriteByte);

char *input_file;
char *output_file;
char *file1;				/* the first file parameter */
char *file2;
char *file3;
FILE *in_ptr0;
FILE *in_ptr1;
FILE *out_ptr;
FILE *msg_ptr;

int verbose;
int header;

int Speed;
int Delay;
int twoInputMode;
int WritePartial;
//int ColBoundingBox1=0;
//int ColBoundingBox2=0;
//int RowBoundingBox1=0;
//int RowBoundingBox2=0;
char *PartialFile;
int DumpBitstream;
int WriteFullBitstreamMode;
int WriteModuleBitstreamMode;
int DumpCLBs;
char *DumpBitstreamFileName;
char *FullBitstreamFileName;
char *ModuleBitstreamFileName;

char *PartialBitstreamFileName;

int LinkPitFileMode;
// global variables and functions' declarations for the Resource Replication
int RepRsc;
char *TypeOfResource;
void ParseRepRsc(char **argv, int i);
int RscCl1;
int RscRw1;
int RscCl2;
int RscRw2;

int RepClb(int Cl1, int Rw1, int Cl2, int Rw2, bool IsReplaced);
int RepBRAM(int Cl1, int Rw1, int Cl2, int Rw2, bool IsReplaced);
int duplicate_FPGA_region(int MJA1, int MJA2, int MJA3, bool IsReplaced);
int cut_FPGA_region(int MJA1, int MJA2);

int cutout_region[50000];
int end_cutout;

//void ParseMetaHeader(char **argv, int i);
//char *metadata="";

struct LinkInformation {
	int PitCol;
	int PitRow;
	char *PitFileName;
};
struct LinkInformation LinkInformationList[20];

//int PitCol[20], PitRow[20];
//char *PitFileName[20];
long int EndOfHeaderFilePosition;
long int SyncSequenceFilePosition;
char DirtyFrameMask[MaxFrames];

int WritePartialByMask;
char *PartialFileByMask;

int FLR;  				// frane length 
int userFLR;  			// frame length may be set by user (later)
//int deviceFLR = -1;			// frame length can be set by writing a device ID
int writeFLR; 			// frame length can be set by writing the Frame Length Register FLR

unsigned char InitialHeader[50000];
int InitialHeaderSize;
unsigned char InitialFooter[1000000];
int InitialFooterSize;
short PartialMaskCLB[250][250];
short PartialMaskRAM[250][250];
unsigned FrameBufferPartial[30000][150];	// this is the complete bitstream of a partial region
unsigned FrameBuffer[2][30000][150];	// this is the complete bitstream of a XC7Z010
int FrameBufferState[5000];         // -1 stream is empty  

unsigned ShadowCol[500];			// A configuration column is firstly written into a shadow reg...
int ShadowState;
int CLBState[6][100][60][50];	// denotes for each CLB if the entire frame contains some information
int DeviceID;

int ClbMaskActive;
int BRamMaskActive;
int BRamRoutingMaskActive;
int ClbLeft, ClbRight, ClbTop, ClbBottom;
int BRamLeft, BRamRight, BRamTop, BRamBottom;
int BRamRoutingLeft, BRamRoutingRight, BRamRoutingTop, BRamRoutingBottom;
unsigned char ModuleResourceString[200];

int WriteNOOP(int times, FILE *OutBitfilePtr);
int WriteNoOfWords(int Words, bool AddFrame, FILE *OutBitfilePtr);
int WriteFAR(int FARValue, FILE *OutBitfilePtr);
int getFAR(int RscCl1, int RscRw1);
int WriteRemaniningHeader(FILE *OutBitfilePtr);
int WriteIDCODE(FILE *OutBitfilePtr);
int WriteCMDReg(int CMDValue, FILE *OutBitfilePtr);
int WriteCTL0Reg(int CTL0Value, FILE *OutBitfilePtr);
int WriteMASKReg(int MASKValue, FILE *OutBitfilePtr);
int WriteSYNC(FILE *OutBitfilePtr);
int calNewFAR(int island);
