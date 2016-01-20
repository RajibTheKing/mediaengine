#include "VideoDecoder.h"
#include "CommonElementsBucket.h"
#include "DefinedDataTypes.h"
#include "LogPrinter.h"
#include "Tools.h"
#include "codec_api.h"

CVideoDecoder::CVideoDecoder(CCommonElementsBucket* sharedObject, CDecodingBuffer *decodingBuffer) :

m_pCommonElementsBucket(sharedObject),
m_pDecodingBuffer(decodingBuffer),
m_pSVCVideoDecoder(NULL)

{
	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoDecoder::CVideoDecoder video decoder created");
}

CVideoDecoder::~CVideoDecoder()
{
    if(NULL != m_pSVCVideoDecoder)
    {
        m_pSVCVideoDecoder->Uninitialize();
        
        m_pSVCVideoDecoder = NULL;
    }
}

int CVideoDecoder::CreateVideoDecoder()
{
	CLogPrinter_Write(CLogPrinter::INFO, "CVideoDecoder::CreateVideoDecoder");

	long iRet = WelsCreateDecoder(&m_pSVCVideoDecoder);
	if (iRet != 0 || NULL == m_pSVCVideoDecoder){
		CLogPrinter_Write(CLogPrinter::INFO, "Unable to create OpenH264 decoder");
		return -1;
	}

	SDecodingParam decParam = { 0 };
	decParam.sVideoProperty.size = sizeof(decParam.sVideoProperty);
	decParam.eOutputColorFormat = videoFormatI420;
	decParam.uiTargetDqLayer = (uint8_t)-1;
	decParam.eEcActiveIdc = ERROR_CON_FRAME_COPY;
	decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;

	iRet = m_pSVCVideoDecoder->Initialize(&decParam);
	if (iRet != 0){
		CLogPrinter_Write(CLogPrinter::INFO, "Unable to initialize OpenH264 decoder");
		return -1;
	}

	int flag = 0;
#define SET_DECODER_OPTION(key, value) \
    flag = value; \
    if(m_pSVCVideoDecoder->SetOption(key, &flag) != 0){ \
        cout << "Error in setting option " << #key << " to OpenH264 decoder\n"; \
			    }

	SET_DECODER_OPTION(DECODER_OPTION_DATAFORMAT, videoFormatI420);
	SET_DECODER_OPTION(DECODER_OPTION_END_OF_STREAM, 0);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CVideoDecoder::CreateVideoDecoder open h264 video decoder initialized");

	return 1;
}

int CVideoDecoder::Decode(unsigned char *in_data, unsigned int in_size, unsigned char *out_data, int &iVideoHeight, int &iVideoWidth)
{
	//CLogPrinter_Write(CLogPrinter::INFO, "CVideoDecoder::Decode called");
	if (!m_pSVCVideoDecoder){
		cout << "OpenH264 decoder NULL!\n";
		return 0;
	}
	//CLogPrinter_Write(CLogPrinter::INFO, "CVideoDecoder::Decode started");

	int strides[2] = { 0 };
	unsigned char *outputPlanes[3] = { NULL };
	DECODING_STATE decodingState = m_pSVCVideoDecoder->DecodeFrame(in_data, in_size, outputPlanes, strides, iVideoWidth, iVideoHeight);

	//CLogPrinter_Write(CLogPrinter::INFO, "CVideoDecoder::Decode OpenH264 decoding of length " + Tools::IntegertoStringConvert(in_size) + " returned " + Tools::IntegertoStringConvert(decodingState) + ", stride = " + Tools::IntegertoStringConvert(strides[0]) + " " + Tools::IntegertoStringConvert(strides[1]) + ", width = " + Tools::IntegertoStringConvert(iVideoWidth) + ", height = " + Tools::IntegertoStringConvert(iVideoHeight));

	if (decodingState != 0){
		//cout << "OpenH264 decoding failed " << (int)decodingState;
		
        CLogPrinter_WriteSpecific(CLogPrinter::DEBUGS, "CVideoDecoder::Decode OpenH264 Decoding FAILED");
		return 0;
	}
	//CLogPrinter_Write(CLogPrinter::INFO, "CVideoDecoder::Decode decodingState 0");
	int retsize = 0;
	for (int plane = 0; plane < 3; plane++){
		
		unsigned char *from = outputPlanes[plane];
		int stride = strides[plane ? 1 : 0];
		
		// Copy one row at a time, skip the extra portion in the stride 
		
		for (int row = 0; row < iVideoHeight >> (plane ? 1 : 0); row++){
			size_t w_count = iVideoWidth >> (plane ? 1 : 0);

			memcpy(out_data + retsize, from, w_count);
			retsize += w_count;
			from += stride;
		}
	}
	//CLogPrinter_Write(CLogPrinter::INFO, "CVideoDecoder::Decoding successful, retsize = " + Tools::IntegertoStringConvert(retsize));

	return retsize;
}




