#include "Tools.h"
#include "Size.h"

#define INF_PACKETTYPE 0
#define INF_HEADERLENGTH 1
#define INF_NETWORKTYPE 2
#define INF_VERSIONCODE 3
#define INF_PACKETNUMBER 4
#define INF_PACKETLENGTH 5
#define INF_RECVDSLOTNUMBER 6
#define INF_NUMPACKETRECVD 7
#define INF_CHANNELS 8
#define INF_SLOTNUMBER 9
#define INF_TIMESTAMP 10

/////////PacketTypes//////
#define AUDIO_SKIP_PACKET_TYPE 0
#define AUDIO_NORMAL_PACKET_TYPE 1
#define AUDIO_NOVIDEO_PACKET_TYPE 2
#define AUDIO_VERSION_PACKET_TYPE 3
#define AUDIO_CHANNEL_PACKET_TYPE 7
#define AUDIO_OPUS_PACKET_TYPE 9
#define AUDIO_NONMUXED_PACKET_TYPE 10
#define AUDIO_MUXED_PACKET_TYPE 11

#define MAXFIELDSINHEADER 15
#define MAXHEADERSIZE 100


//#define __AUDIO_HEADER_LENGTH__ 7


static int HeaderBitmap[] =
{
	8 /*INF_PACKETTYPE*/,
	6 /*INF_HEADERLENGTH*/,
	2 /*INF_NETWORKTYPE*/,
	5 /*INF_VERSIONCODE*/,
	31 /*INF_PACKETNUMBER*/,
	12 /*INF_PACKETLENGTH*/,
	3 /*INF_RECVDSLOTNUMBER*/,
	8 /*INF_NUMPACKETRECVD*/,
	2 /*INF_CHANNELS*/,
	3 /*INF_SLOTNUMBER*/,
	40 /*INF_TIMESTAMP*/
};

static int SupportedPacketTypes[] =
{
	AUDIO_SKIP_PACKET_TYPE,
	AUDIO_NORMAL_PACKET_TYPE,
	AUDIO_NOVIDEO_PACKET_TYPE,
	AUDIO_VERSION_PACKET_TYPE
}; //Only for Call


class CAudioPacketHeader {


	unsigned int m_nHeaderSizeInBit;
	unsigned int m_nHeaderSizeInByte;
	long long m_arrllInformation[MAXFIELDSINHEADER];

	unsigned char ma_uchHeader[MAXHEADERSIZE];
	int nNumberOfHeaderElements;
	//int CopyInformationToHeader(unsigned int * Information);
	void PutInformationToArray(int InfoType);

public:
	CAudioPacketHeader();
	//CAudioPacketHeader(unsigned int * Information);
	CAudioPacketHeader(unsigned char *Header);
	~CAudioPacketHeader();

	void CopyHeaderToInformation(unsigned char *Header);
	int GetHeaderInByteArray(unsigned char* data);

	int GetHeaderSize();

	void SetInformation(long long Information, int InfoType);
	long long GetInformation(int InfoType);

	long long GetFieldCapacity(int InfoType);

	bool IsPacketTypeSupported(unsigned int PacketType);
	bool IsPacketTypeSupported();

	void showDetails(string prefix);

};