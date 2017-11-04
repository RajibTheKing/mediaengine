
#ifndef AUDIODUMP_H
#define AUDIODUMP_H
#include <string>
#include "MediaLogger.h"

namespace MediaSDK
{
	class CAudioDumper
	{
	public:
		CAudioDumper(std::string fileName, bool enable);
		~CAudioDumper();
		void WriteDump(void* audioData, int typeSize, int dataSize, bool enable);

	private:
		FILE *dumpfile = nullptr;
	};
} //namespace MediaSDK
#endif // !AUDIODUMP_H