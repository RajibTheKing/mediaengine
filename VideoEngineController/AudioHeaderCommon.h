#ifndef AUDIO_HEADER_COMMON_H
#define AUDIO_HEADER_COMMON_H


#include "AudioPacketHeader.h"


namespace MediaSDK
{

	class AudioHeaderCommon : public AudioPacketHeader
	{
	public:

		AudioHeaderCommon();
		AudioHeaderCommon(unsigned int * Information);
		AudioHeaderCommon(unsigned char *Header);

		~AudioHeaderCommon();

		void CopyHeaderToInformation(unsigned char *Header);
		int GetHeaderInByteArray(unsigned char* data);

		int GetHeaderSize();

		void SetInformation(long long Information, int InfoType);
		long long GetInformation(int InfoType);

		long long GetFieldCapacity(int InfoType);

		bool IsPacketTypeSupported(unsigned int PacketType);
		bool IsPacketTypeSupported();

		void ShowDetails(char prefix[]);

		bool PutInformationToArray(int InfoType);

	protected:
		int CopyInformationToHeader(unsigned int * Information);

	private:
		void InitHeaderBitMap();

	};

} //namespace MediaSDK

#endif

