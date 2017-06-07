#include "EchoCancellerProvider.h"

#include "WebRTCEchoCanceller.h"
#include "SpeexEchoCanceller.h"

namespace MediaSDK
{

	SmartPointer<EchoCancellerInterface> EchoCancellerProvider::GetEchoCanceller(EchoCancelerType echoCancellerType, bool isLiveRunning)
	{
		EchoCancellerInterface* pInstance = nullptr;
		LOG18("##TT 15");
		switch (echoCancellerType)
		{
		case WebRTC_ECM:
			pInstance = new WebRTCEchoCanceller(isLiveRunning);
			break;

		case Speex_ECM:
			pInstance = new SpeexEchoCanceller();
			break;

		default:
			pInstance = nullptr;
		}

		return SmartPointer<EchoCancellerInterface>(pInstance);
	}

} //namespace MediaSDK
