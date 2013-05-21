//
//  main.c
//  audioConverter
//
//  Created by Rohan Jyoti on 5/18/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <AudioToolbox/AudioToolbox.h>

#ifndef audioConverter_h
#define audioConverter_h
static void errcheck(OSStatus error, const char *operation);
static char *getFormatName(char *stringToParse);
char *gFN_name;
char *gFN_ret_val;

#pragma mark user data struct
typedef struct mAudioConverterSettings
{
	AudioStreamBasicDescription inputFormat;
	AudioStreamBasicDescription outputFormat;
	
	AudioFileID inputFile;
	AudioFileID outputFile;
	
	UInt64 inputFilePacketIndex;
	UInt64 inputFilePacketCount;
	UInt32 inputFilePacketMaxSize;
	AudioStreamPacketDescription *inputFilePacketDescription;
	
	void *sourceBuffer;
} mAudioConverterSettings;

void audioConvert(mAudioConverterSettings *mACS);
#endif


#pragma mark utility functions
static void errCheck(OSStatus error, const char *operation)
{
	if(error == noErr) return;
	
	char errorString[20];
	*(UInt32 *)(errorString + 1) = CFSwapInt32HostToBig(error);
	//see if 4-char code
	if(isprint(errorString[1]) && isprint(errorString[2]) && isprint(errorString[3]) && isprint(errorString[4]))
	{
		errorString[0] = errorString[5] = '\'';
		errorString[6] = '\0';
	}
	else
		sprintf(errorString, "%d", (int)error); //format as integer instead
	
	fprintf(stderr, "Error: %s (%s)\n", operation, errorString);
	exit(1);
}

static char *getFormatName(char *stringToParse)
{
	char *dot_pos = strchr(stringToParse, '.');
	int num_dots = 0;
	long pos = -1;
	while(dot_pos != NULL)
	{
		num_dots++;
		if(pos == -1) 
		{
			pos = dot_pos-stringToParse+1;
		}
		dot_pos = strchr(dot_pos+1, '.');
	}
	if(pos == -1) return "Error";
	
	size_t total_length = strlen(stringToParse);
	gFN_name = (char *)malloc(pos * sizeof(char));
	gFN_ret_val = (char *)malloc((total_length-pos) * sizeof(char));
	strncpy(gFN_name, stringToParse, pos-1);
	strncpy(gFN_ret_val, &stringToParse[pos], total_length - strlen(gFN_name)+1);
	return gFN_ret_val;	
}

void audioConvert(mAudioConverterSettings *mACS)
{
	AudioConverterRef audioConverter;
	errCheck(AudioConverterNew(&mACS->inputFormat, &mACS->outputFormat, &audioConverter), "audioConvert AudioConvertNew error");
	
	//Next we need to figure out how big of a packet description array we need to allocate.
}

#pragma mark converter callback function


#pragma mark main function
int main (int argc, const char * argv[])
{
	mAudioConverterSettings mACS = {0}; //zero out the struct
	//========== Open Input File ==========
	const char *iFN = "/EMPLICIT/zTemp/Archetek-Stay.m4a";
	char *iF = (char *)malloc(strlen(iFN) * sizeof(char));
	strncpy(iF, iFN, strlen(iFN));
	CFStringRef inputName = CFStringCreateWithCString(kCFAllocatorDefault, iF, 0);
	CFURLRef inputFileURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, inputName, kCFURLPOSIXPathStyle, false);
	errCheck(AudioFileOpenURL(inputFileURL, kAudioFileReadPermission, 0, &mACS.inputFile), "inputFile AudioFileOpenURL error");
	CFRelease(inputFileURL);
	
	//========== Get Input Format ==========
	UInt32 propSize = sizeof(mACS.inputFormat);
	errCheck(AudioFileGetProperty(mACS.inputFile, kAudioFilePropertyDataFormat, &propSize, &mACS.inputFormat), "Input Format AudioFileGetProperty error");
	
	//we still want to get the total number of packets in the source file and the size of the largest possible packet
	propSize = sizeof(mACS.inputFilePacketCount);
	errCheck(AudioFileGetProperty(mACS.inputFile, kAudioFilePropertyAudioDataPacketCount, &propSize, &mACS.inputFilePacketCount), "Input Format Packet Count AudioFileGetProperty error");
	propSize = sizeof(mACS.inputFilePacketMaxSize);
	errCheck(AudioFileGetProperty(mACS.inputFile, kAudioFilePropertyMaximumPacketSize, &propSize, &mACS.inputFilePacketMaxSize), "Input Format Max Packet size AudioFileGetProperty error");
	
	//========== Set Up Output File ==========
	mACS.outputFormat.mSampleRate = 44100.0;
	mACS.outputFormat.mFormatID = kAudioFormatLinearPCM;
	mACS.outputFormat.mFormatFlags = kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	mACS.outputFormat.mBytesPerPacket = 4;
	mACS.outputFormat.mFramesPerPacket = 1;
	mACS.outputFormat.mBytesPerFrame = 4; //mBytesPerPacket / mFramesPerPacket
	mACS.outputFormat.mChannelsPerFrame = 2; //stereo
	mACS.outputFormat.mBitsPerChannel = 16; //So overall we have 16-bit 44100Hz CD quality stereo linearPCM
	
	
	const char *oFN = "OutputFile.aif";
	char *oF = (char *)malloc(strlen(oFN) * sizeof(char));
	strncpy(oF, oFN, strlen(oFN));
	CFStringRef outputName = CFStringCreateWithCString(kCFAllocatorDefault, oF, 0);
	CFURLRef outputFileURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, outputName, kCFURLPOSIXPathStyle, false);
	errCheck(AudioFileCreateWithURL(outputFileURL, kAudioFileAIFFType, &mACS.outputFormat, kAudioFileFlags_EraseFile, &mACS.outputFile), "Output File AudioFileCreateWithURL error");
	CFRelease(outputFileURL);
	
	//========== Perform Conversion ==========
	fprintf(stdout, "Audio file converting from %s to %s\n", getFormatName(iF), getFormatName(oF));
	//audioConvert(&mACS);
	
	//cleanup:
	AudioFileClose(mACS.inputFile);
	AudioFileClose(mACS.outputFile);
	free(oF);
	free(iF);
	if(gFN_name) free(gFN_name);
	if(gFN_ret_val) free(gFN_ret_val);
	
	
	
    return 0;
}

