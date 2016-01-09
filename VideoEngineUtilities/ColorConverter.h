
#ifndef _COLOR_CONVERTER_H_
#define _COLOR_CONVERTER_H_

#include "AudioVideoEngineDefinitions.h"

#if _MSC_VER > 1000
#pragma once
#endif

#define MAX_FRAME_HEIGHT 640
#define MAX_FRAME_WIDTH 480

#include <string>

class CColorConverter
{

public:

	CColorConverter(int iVideoHeight, int iVideoWidth);
	~CColorConverter();

	int ConvertI420ToNV21(unsigned char *convertingData, int iVideoHeight, int iVideoWidth);
	int ConvertI420ToNV12(unsigned char *convertingData, int iVideoHeight, int iVideoWidth);
	int ConvertNV12ToI420(unsigned char *convertingData);
	int ConvertNV21ToI420(unsigned char *convertingData);
	void mirrorRotateAndConvertNV21ToI420(unsigned char *m_pFrame, unsigned char *pData);
	void mirrorRotateAndConvertNV21ToI420ForBackCam(unsigned char *m_pFrame, unsigned char *pData);

private:

	int m_iVideoHeight;
	int m_iVideoWidth;
	int m_YPlaneLength;
	int m_VPlaneLength;
	int m_UVPlaneMidPoint;
	int m_UVPlaneEnd;

	unsigned char m_pVPlane[(MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH) >> 2];
	unsigned char m_pUPlane[(MAX_FRAME_HEIGHT * MAX_FRAME_WIDTH) >> 2];

	int m_Multiplication[481][641];
};

#endif 
