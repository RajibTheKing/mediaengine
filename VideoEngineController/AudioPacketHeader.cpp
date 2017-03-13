
#include "AudioPacketHeader.h"
#include "LogPrinter.h"

CAudioPacketHeader::CAudioPacketHeader()
{
	nNumberOfHeaderElements = sizeof(HeaderBitmap)/sizeof(int);

	int headerSizeInBit = 0;
	for (int i = 0; i < nNumberOfHeaderElements; i++)
	{
		headerSizeInBit += HeaderBitmap[i];
	}

	m_nHeaderSizeInBit = headerSizeInBit;
	m_nHeaderSizeInByte = (headerSizeInBit + 7) / 8;
	m_nProcessingHeaderSizeInByte = m_nHeaderSizeInByte;
	memset(m_arrllInformation, 0, sizeof(m_arrllInformation));
	memset(ma_uchHeader, 0, m_nHeaderSizeInByte);
}

//CAudioPacketHeader::CAudioPacketHeader(unsigned int * Information)
//{
//	CAudioPacketHeader();
//	CopyInformationToHeader(Information);
//}

CAudioPacketHeader::CAudioPacketHeader(unsigned char *Header)
{
	CAudioPacketHeader();
	CopyHeaderToInformation(Header);
}

CAudioPacketHeader::~CAudioPacketHeader()
{
	memset(m_arrllInformation, 0, sizeof(m_arrllInformation));
	memset(ma_uchHeader, 0, m_nHeaderSizeInByte);
}

//int CAudioPacketHeader::CopyInformationToHeader(unsigned int * Information)
//{
//	memcpy(m_arrllInformation, Information, m_nHeaderSizeInByte);
//	for (int i = 0; i < nNumberOfHeaderElements; i++)
//	{
//		SetInformation(Information[i], i);
//	}
//	return m_nHeaderSizeInByte;
//}

long long CAudioPacketHeader::GetFieldCapacity(int InfoType)
{
	return 1LL << HeaderBitmap[InfoType];
}


int CAudioPacketHeader::GetHeaderInByteArray(unsigned char* data)
{
	memcpy(data, ma_uchHeader, m_nHeaderSizeInByte);
	return m_nHeaderSizeInByte;
}

int CAudioPacketHeader::GetHeaderSize()
{
	return m_nHeaderSizeInByte;
}

bool CAudioPacketHeader::IsPacketTypeSupported(unsigned int PacketType)
{
	int nPacketTypes = sizeof(SupportedPacketTypes) / sizeof(int);
	for (int i = 0; i < nPacketTypes; i++)
	{
		if (SupportedPacketTypes[i] == PacketType) return true;
	}
	return false;
}

bool CAudioPacketHeader::IsPacketTypeSupported()
{
	unsigned int iPackeType = GetInformation(INF_PACKETTYPE);
	return IsPacketTypeSupported(iPackeType);
}

void CAudioPacketHeader::SetInformation(long long Information, int InfoType)
{
    Information = (Information & ((1LL << HeaderBitmap[InfoType]) - 1));
	m_arrllInformation[InfoType] = Information;

	int infoStartBit = 0;
	for (int i = 0; i < InfoType; i++)
	{
		infoStartBit += HeaderBitmap[i];
	}
	int infoStartByte = infoStartBit / 8;
	int infoStartBitOfByte = infoStartBit % 8;

	int numberOfBitsIn1stByte;
	if (8 - infoStartBitOfByte >= HeaderBitmap[InfoType])
	{
		numberOfBitsIn1stByte = min(HeaderBitmap[InfoType], 8 - infoStartBitOfByte);
		ma_uchHeader[infoStartByte] &= ~( ((1 << numberOfBitsIn1stByte) - 1) << (8 - infoStartBitOfByte - numberOfBitsIn1stByte) ) ;
		ma_uchHeader[infoStartByte] |= (Information >> (HeaderBitmap[InfoType] - numberOfBitsIn1stByte)) << (8 - infoStartBitOfByte - numberOfBitsIn1stByte);
	}
	else
	{
		numberOfBitsIn1stByte = 8 - infoStartBitOfByte;
		ma_uchHeader[infoStartByte] &=  ~((1<<numberOfBitsIn1stByte) - 1);
		ma_uchHeader[infoStartByte] |= (Information >> (HeaderBitmap[InfoType] - numberOfBitsIn1stByte));

		int remainingBits = HeaderBitmap[InfoType] - numberOfBitsIn1stByte;
		int remainingBytes = remainingBits / 8;
		int nBitsInLastByte = remainingBits % 8;
		int byte = 1;

		for(int i = 0; i < remainingBytes; i ++)
		{
			ma_uchHeader[infoStartByte + byte] = Information >> (HeaderBitmap[InfoType] - numberOfBitsIn1stByte - 8 * byte);
			byte++;
		}

		if(nBitsInLastByte)
		{
			ma_uchHeader[infoStartByte + byte] &=  ~ ( ( (1<<nBitsInLastByte) - 1) << (8 - nBitsInLastByte));
			ma_uchHeader[infoStartByte + byte] |= 0xFF & (Information << (8-nBitsInLastByte) );
		}
	}
}

void CAudioPacketHeader::CopyHeaderToInformation(unsigned char *Header)
{
	memcpy(ma_uchHeader, Header, m_nHeaderSizeInByte);

	for (int i = 0; i < nNumberOfHeaderElements; i++)
	{
		//CLogPrinter_WriteLog(CLogPrinter::INFO, INSTENT_TEST_LOG,"#PutInformationToArray#");
		PutInformationToArray(i);
		if(INF_HEADERLENGTH == i && m_nHeaderSizeInByte != m_arrllInformation[INF_HEADERLENGTH]) {
			m_nProcessingHeaderSizeInByte = m_arrllInformation[INF_HEADERLENGTH];
		}
	}
}

long long CAudioPacketHeader::GetInformation(int InfoType)
{
	return m_arrllInformation[InfoType];
}

bool CAudioPacketHeader::PutInformationToArray(int InfoType)
{
	unsigned long long Information = 0;
	int infoStartBit = 0;
	for (int i = 0; i < InfoType; i++)
	{
		infoStartBit += HeaderBitmap[i];
	}
	int infoStartByte = infoStartBit / 8;
	int infoStartBitOfByte = infoStartBit % 8;

	if( infoStartByte + HeaderBitmap[InfoType] > (m_nProcessingHeaderSizeInByte << 3))
	{
		m_arrllInformation[InfoType] = -1;
		return false;
	}
    
	if (infoStartBitOfByte + HeaderBitmap[InfoType] <= 8)//fits in 1 byte
	{
		unsigned char temp = (ma_uchHeader[infoStartByte] << infoStartBitOfByte);
		Information = (temp >>( 8 - HeaderBitmap[InfoType]));
	}
	else
	{
		int nBitesToCopy = HeaderBitmap[InfoType] + infoStartBitOfByte;
		int nBytesToCopy = nBitesToCopy / 8 + (nBitesToCopy % 8 != 0);
		for (int i = 0; i < nBytesToCopy; i++)
		{
			Information <<= 8;
			Information += ma_uchHeader[infoStartByte + i];
		}

		int shift = ((HeaderBitmap[InfoType] + infoStartBitOfByte ) % 8);
		if(shift)
			Information >>= 8 - shift;
		Information &= (1LL << HeaderBitmap[InfoType]) - 1;
	}
	m_arrllInformation[InfoType] = Information;
	return true;
}

void CAudioPacketHeader::showDetails(string prefix)
{
	PRT("%s #-> "
		"PT = %lld "
		"HL = %lld "
		"NT = %lld "
		"VC = %lld "
		"PN = %lld "
		"PL = %lld "
		"RECVDSN = %lld "
		"NPRECVD = %lld "
		"C = %lld "
		"SN = %lld "
		"TS = %lld",
		prefix.c_str(),
		m_arrllInformation[0],
		m_arrllInformation[1],
		m_arrllInformation[2],
		m_arrllInformation[3],
		m_arrllInformation[4],
		m_arrllInformation[5],
		m_arrllInformation[6],
		m_arrllInformation[7],
		m_arrllInformation[8],
		m_arrllInformation[9],
		m_arrllInformation[10]);
}


void CAudioPacketHeader::SetHeaderAllInByteArray(unsigned char* header, int packetType, int nHeaderLength, int networkType, int slotNumber, int packetNumber, int packetLength, int recvSlotNumber,
	int numPacketRecv, int channel, int version, long long timestamp,int iBlockNumber, int nTotalBlocksInThisFrame, int nBlockOffset, int nFrameLength)
{
	//LOGEF("##EN### BuildAndGetHeader ptype %d ntype %d slotnumber %d packetnumber %d plength %d reslnumber %d npacrecv %d channel %d version %d time %lld",
		//	packetType, networkType, slotNumber, packetNumber, packetLength, recvSlotNumber, numPacketRecv, channel, version, timestamp);
		SetInformation(packetType, INF_PACKETTYPE);
		SetInformation(nHeaderLength, INF_HEADERLENGTH);
		SetInformation(packetNumber, INF_PACKETNUMBER);
		SetInformation(slotNumber, INF_SLOTNUMBER);
		SetInformation(packetLength, INF_BLOCK_LENGTH);
		SetInformation(recvSlotNumber, INF_RECVDSLOTNUMBER);
		SetInformation(numPacketRecv, INF_NUMPACKETRECVD);
		SetInformation(version, INF_VERSIONCODE);
		SetInformation(timestamp, INF_TIMESTAMP);
		SetInformation(networkType, INF_NETWORKTYPE);
		SetInformation(channel, INF_CHANNELS);
		SetInformation(iBlockNumber, INF_PACKET_BLOCK_NUMBER);
		SetInformation(nTotalBlocksInThisFrame, INF_TOTAL_PACKET_BLOCKS);
		SetInformation(nBlockOffset, INF_BLOCK_OFFSET);
		SetInformation(nFrameLength, INF_FRAME_LENGTH);

		showDetails("@#BUILD");

		GetHeaderInByteArray(header);	
}

void CAudioPacketHeader::GetHeaderInfoAll(unsigned char* header, int &packetType, int &nHeaderLength, int &networkType, int &slotNumber, int &nFrameNumber, int &nBlockLength, int &recvSlotNumber,
	int &numPacketRecv, int &channel, int &version, long long &timestamp, int &iBlockNumber, int &nNumberOfBlocks, int &iOffsetOfBlock, int &nFrameLength)
{
	CopyHeaderToInformation(header);

	packetType = GetInformation(INF_PACKETTYPE);
	nHeaderLength = GetInformation(INF_HEADERLENGTH);
	networkType = GetInformation(INF_NETWORKTYPE);
	slotNumber = GetInformation(INF_SLOTNUMBER);
	nFrameNumber = GetInformation(INF_PACKETNUMBER);
	nBlockLength = GetInformation(INF_BLOCK_LENGTH);
	recvSlotNumber = GetInformation(INF_RECVDSLOTNUMBER);
	numPacketRecv = GetInformation(INF_NUMPACKETRECVD);
	channel = GetInformation(INF_CHANNELS);
	version = GetInformation(INF_VERSIONCODE);
	timestamp = GetInformation(INF_TIMESTAMP);

	iBlockNumber = GetInformation(INF_PACKET_BLOCK_NUMBER);
	nNumberOfBlocks = GetInformation(INF_TOTAL_PACKET_BLOCKS);
	iOffsetOfBlock = GetInformation(INF_BLOCK_OFFSET);
	nFrameLength = GetInformation(INF_FRAME_LENGTH);

	showDetails("@#PARSE");
}