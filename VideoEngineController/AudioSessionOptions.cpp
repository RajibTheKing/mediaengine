#include "AudioSessionOptions.h"
#include "InterfaceOfAudioVideoEngine.h"



AudioSessionOptions::AudioSessionOptions()
{
	ResetOptions();
}


void AudioSessionOptions::ResetOptions()
{
	headerType = HEADER_COMMON;

	encoderType = Disable_Encoder;
	decoderType = Disable_Decoder;

	noiseReducerType = Disable_ANR;
	echoCancelerType = Disable_ECM;
	gainType = Disable_Gain;

	adaptEncoderBitrate = false;
	adaptEncoderComplexity = false;
	adaptDecoderBitrate = false;
	adaptDecoderComplexity = false;

	enableBufferData = false;
	enableMuxing = false;
	enablePacketization = false;
}


AudioEntityRoleType AudioSessionOptions::GetActionType(int serviceType, int entityType)
{
	if (serviceType == SERVICE_TYPE_CALL || serviceType == SERVICE_TYPE_SELF_CALL)
	{
		return EntityInCall;
	}
	else if (SERVICE_TYPE_CHANNEL == serviceType)
	{
		return EntityChannel;
	}
	else if (SERVICE_TYPE_LIVE_STREAM == serviceType || SERVICE_TYPE_SELF_STREAM == serviceType)
	{
		if (ENTITY_TYPE_PUBLISHER == entityType)
		{
			return EntityPublisher;
		}
		else if (ENTITY_TYPE_PUBLISHER_CALLER == entityType)
		{
			return EntityPublisherInCall;
		}
		else if (ENTITY_TYPE_VIEWER == entityType)
		{
			return EntityViewer;
		}
		else if (ENTITY_TYPE_VIEWER_CALLEE == entityType)
		{
			return EntityViewerInCall;
		}
		else
		{
			return EntityNone;
		}
	}
	else
	{
		return EntityNone;
	}
	
}



void AudioSessionOptions::SetOptions(int serviceType, int entityType)
{
	AudioEntityRoleType actionType = GetActionType(serviceType, entityType);

	switch (actionType)
	{
	case EntityInCall:
		SetOptionsForCall();
		break;

	case EntityChannel:
		SetOptionsForChannel();
		break;

	case EntityPublisher:
		SetOptionsForPublisher();
		break;

	case EntityPublisherInCall:
		SetOptionsForPublisherInCall();
		break;

	case EntityViewer:
		SetOptionsForViewer();
		break;

	case EntityViewerInCall:
		SetOptionsForViewerInCall();
		break;

	default:
		ResetOptions();
		break;
	}
}


void AudioSessionOptions::SetOptionsForCall()
{
	headerType = HEADER_COMMON;

	encoderType = Opus_Encoder;
	decoderType = Opus_Decoder;

	noiseReducerType = Disable_ANR;
	echoCancelerType = WebRTC_ECM;
	gainType = WebRTC_AGC;

	adaptEncoderBitrate = true;
	adaptEncoderComplexity = true;
	adaptDecoderBitrate = true;
	adaptDecoderComplexity = true;

	enableBufferData = false;
	enableMuxing = false;
	enablePacketization = false;
}


void AudioSessionOptions::SetOptionsForChannel()
{
	headerType = HEADER_COMMON;

	encoderType = Disable_Encoder;
	decoderType = AAC_Decoder;

	noiseReducerType = Disable_ANR;
	echoCancelerType = Disable_ECM;
	gainType = Disable_Gain;

	adaptEncoderBitrate = false;
	adaptEncoderComplexity = false;
	adaptDecoderBitrate = false;
	adaptDecoderComplexity = false;

	enableBufferData = false;
	enableMuxing = false;
	enablePacketization = false;
}

void AudioSessionOptions::SetOptionsForPublisher()
{
	headerType = HEADER_COMMON;

	encoderType = Disable_Encoder;
	decoderType = Disable_Decoder;

	noiseReducerType = Disable_ANR;
	echoCancelerType = Disable_ECM;
	gainType = Disable_Gain;

	adaptEncoderBitrate = false;
	adaptEncoderComplexity = false;
	adaptDecoderBitrate = false;
	adaptDecoderComplexity = false;

	enableBufferData = false;
	enableMuxing = false;
	enablePacketization = false;
}


void AudioSessionOptions::SetOptionsForPublisherInCall()
{
	headerType = HEADER_COMMON;

	encoderType = Disable_Encoder;
	decoderType = Disable_Decoder;

	noiseReducerType = Disable_ANR;
	echoCancelerType = Disable_ECM;
	gainType = Disable_Gain;

	adaptEncoderBitrate = false;
	adaptEncoderComplexity = false;
	adaptDecoderBitrate = false;
	adaptDecoderComplexity = false;

	enableBufferData = false;
	enableMuxing = false;
	enablePacketization = false;
}


void AudioSessionOptions::SetOptionsForViewer()
{
	headerType = HEADER_COMMON;

	encoderType = Disable_Encoder;
	decoderType = Disable_Decoder;

	noiseReducerType = Disable_ANR;
	echoCancelerType = Disable_ECM;
	gainType = Disable_Gain;

	adaptEncoderBitrate = false;
	adaptEncoderComplexity = false;
	adaptDecoderBitrate = false;
	adaptDecoderComplexity = false;

	enableBufferData = false;
	enableMuxing = false;
	enablePacketization = false;
}


void AudioSessionOptions::SetOptionsForViewerInCall()
{
	headerType = HEADER_COMMON;

	encoderType = Disable_Encoder;
	decoderType = Disable_Decoder;

	noiseReducerType = Disable_ANR;
	echoCancelerType = Disable_ECM;
	gainType = Disable_Gain;

	adaptEncoderBitrate = false;
	adaptEncoderComplexity = false;
	adaptDecoderBitrate = false;
	adaptDecoderComplexity = false;

	enableBufferData = false;
	enableMuxing = false;
	enablePacketization = false;
}



