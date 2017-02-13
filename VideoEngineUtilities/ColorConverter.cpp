
#include "ColorConverter.h"
#include "../VideoEngineController/LogPrinter.h"
#include <math.h>

#if defined(__ANDROID__) || defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR) || defined(TARGET_OS_WINDOWS_PHONE)
typedef unsigned char byte;
#endif

CColorConverter::CColorConverter(int iVideoHeight, int iVideoWidth) :

m_iVideoHeight(iVideoHeight),
m_iVideoWidth(iVideoWidth),
m_YPlaneLength(m_iVideoHeight*m_iVideoWidth),
m_VPlaneLength(m_YPlaneLength >> 2),
m_UVPlaneMidPoint(m_YPlaneLength + m_VPlaneLength),
m_UVPlaneEnd(m_UVPlaneMidPoint + m_VPlaneLength),
m_bMergingSmallFrameEnabled(false)

{
	CLogPrinter_Write(CLogPrinter::INFO, "CColorConverter::CColorConverter");

	m_PrevAddValue = 0;
	m_AverageValue = 0;
	m_ThresholdValue = 0;
    
    m_iSmallFrameHeight = iVideoHeight/3;
    m_iSmallFrameWidth = iVideoWidth/3;
    if(m_iSmallFrameHeight%2) m_iSmallFrameHeight--;
    if(m_iSmallFrameWidth%2) m_iSmallFrameWidth--;
    
    m_iDeviceHeight = -1;
    m_iDeviceWidth = -1;
    
	m_VideoBeautificationer = new CVideoBeautificationer(iVideoHeight, iVideoWidth);

	for (int i = 0; i < 481; i++)
		for (int j = 0; j < 641; j++)
		{
			m_Multiplication[i][j] = i*j;
		}

	m_pColorConverterMutex.reset(new CLockHandler);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CColorConverter::CColorConverter Prepared");


	//LOGE("fahad -->> CColorConverter::ConvertRGB32ToRGB24  inside constructor");


}

CColorConverter::~CColorConverter()
{
	if (NULL != m_VideoBeautificationer)
	{
		delete m_VideoBeautificationer;
		m_VideoBeautificationer = NULL;
	}
}

void CColorConverter::SetHeightWidth(int iVideoHeight, int iVideoWidth)
{
	Locker lock(*m_pColorConverterMutex);

	m_iVideoHeight = iVideoHeight;
	m_iVideoWidth = iVideoWidth;
	m_YPlaneLength = m_iVideoHeight*m_iVideoWidth;
	m_VPlaneLength = m_YPlaneLength >> 2;
	m_UVPlaneMidPoint = m_YPlaneLength + m_VPlaneLength;
	m_UVPlaneEnd = m_UVPlaneMidPoint + m_VPlaneLength;

}

void CColorConverter::SetDeviceHeightWidth(int iVideoHeight, int iVideoWidth)
{
	Locker lock(*m_pColorConverterMutex);

	m_iDeviceHeight = iVideoHeight;
	m_iDeviceWidth = iVideoWidth;

	m_VideoBeautificationer->SetDeviceHeightWidth(iVideoHeight, iVideoWidth);
}

int CColorConverter::ConvertI420ToNV21(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{
	Locker lock(*m_pColorConverterMutex);

	int i, j, k;

	int YPlaneLength = iVideoHeight*iVideoWidth;
	int VPlaneLength = YPlaneLength >> 2;
	int UVPlaneMidPoint = YPlaneLength + VPlaneLength;
	int UVPlaneEnd = UVPlaneMidPoint + VPlaneLength;

	memcpy(m_pUPlane, convertingData + YPlaneLength, VPlaneLength);

	for (i = YPlaneLength, j = 0, k = UVPlaneMidPoint; i < UVPlaneEnd; i += 2, j++, k++)
	{
		convertingData[i] = convertingData[k];
		convertingData[i + 1] = m_pUPlane[j];
	}

	return UVPlaneEnd;
}

int CColorConverter::ConvertNV21ToI420(unsigned char *convertingData)
{
	Locker lock(*m_pColorConverterMutex);

	int i, j, k;

	for (i = m_YPlaneLength, j = 0, k = i; i < m_UVPlaneEnd; i += 2, j++, k++)
	{
		m_pVPlane[j] = convertingData[i];
		convertingData[k] = convertingData[i + 1];
	}

	memcpy(convertingData + m_UVPlaneMidPoint, m_pVPlane, m_VPlaneLength);

	return m_UVPlaneEnd;
}

int CColorConverter::ConvertYUY2ToI420( unsigned char * input, unsigned char * output )
{
	Locker lock(*m_pColorConverterMutex);

	int pixels = m_YPlaneLength;
	int macropixels = pixels >> 1;

	long mpx_per_row = m_iVideoWidth >> 1;

	for (int i = 0, ci = 0; i < macropixels; i++)
	{
		output[i << 1] = input[i << 2];
		output[(i << 1) + 1] = input[(i << 2) + 2];

		long row_number = i / mpx_per_row;
		if (row_number % 2 == 0)
		{
			output[pixels + ci] = input[(i << 2) + 1];
			output[pixels + (pixels >> 2) + ci] = input[(i << 2) + 3];
			ci++;
		}
	}

	return m_YPlaneLength* 3 / 2;
}

int CColorConverter::ConvertI420ToNV12(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{
	Locker lock(*m_pColorConverterMutex);

	int i, j, k;

	int YPlaneLength = iVideoHeight*iVideoWidth;
	int VPlaneLength = YPlaneLength >> 2;
	int UVPlaneMidPoint = YPlaneLength + VPlaneLength;
	int UVPlaneEnd = UVPlaneMidPoint + VPlaneLength;

	memcpy(m_pUPlane, convertingData + YPlaneLength, VPlaneLength);

	for (i = YPlaneLength, j = 0, k = UVPlaneMidPoint; i < UVPlaneEnd; i += 2, j++, k++)
	{
		convertingData[i] = m_pUPlane[j];
		convertingData[i + 1] = convertingData[k];
	}

	return UVPlaneEnd;
}

int CColorConverter::ConvertI420ToYV12(unsigned char *convertingData, int iVideoHeight, int iVideoWidth)
{
	Locker lock(*m_pColorConverterMutex);

	int i, j, k;

	int YPlaneLength = iVideoHeight*iVideoWidth;
	int VPlaneLength = YPlaneLength >> 2;
	int UPlaneLength = YPlaneLength >> 2;
	int UVPlaneMidPoint = YPlaneLength + VPlaneLength;
	int UVPlaneEnd = UVPlaneMidPoint + VPlaneLength;

	memcpy(m_pTempPlane, convertingData + YPlaneLength, UPlaneLength);
	memcpy(convertingData + YPlaneLength, convertingData + YPlaneLength + UPlaneLength, VPlaneLength);
	memcpy(convertingData + YPlaneLength + UPlaneLength, m_pTempPlane, UPlaneLength);

	return UVPlaneEnd;
}

int CColorConverter::ConvertNV12ToI420(unsigned char *convertingData)
{
	Locker lock(*m_pColorConverterMutex);

	int i, j, k;

	for (i = m_YPlaneLength, j = 0, k = i; i < m_UVPlaneEnd; i += 2, j++, k++)
	{
		m_pVPlane[j] = convertingData[i + 1];
		convertingData[k] = convertingData[i];
	}

	memcpy(convertingData + m_UVPlaneMidPoint, m_pVPlane, m_VPlaneLength);

	return m_UVPlaneEnd;
}

/*
void CColorConverter::mirrorRotateAndConvertNV21ToI420(unsigned char *pData)
{
	int iWidth = m_iVideoHeight;
	int iHeight = m_iVideoWidth;

	int i = 0;
	memcpy(m_pFrame, pData, iWidth*iHeight * 3 / 2);

	//Y
	for (int x = 0; x < iWidth; x++)
	{
		for (int y = 0; y < iHeight; y++)
		{
			pData[i] = m_pFrame[y*iWidth + x];
			i++;
		}
	}

	int halfWidth = iWidth / 2;
	int halfHeight = iHeight / 2;
	int dimention = iHeight*iWidth;
	int vIndex = dimention + halfHeight*halfWidth;

	for (int x = 0; x < halfWidth; x++)
		for (int y = 0; y < halfHeight; y++)
		{
			int ind = y*halfWidth + x;
			pData[i++] = m_pFrame[dimention + ind * 2 + 1];           //U
			pData[vIndex++] = m_pFrame[dimention + ind * 2];    //V
		}
}
*/


void CColorConverter::mirrorRotateAndConvertNV21ToI420(unsigned char *m_pFrame, unsigned char *pData)
{
	//LOGE("fahad -->> avgValue= %d, addValue= %d, thresholdVal= %d, m_iVideoHeight=%d, m_iVideoWidth = %d", m_AverageValue, m_PrevAddValue, m_ThresholdValue,m_iVideoHeight, m_iVideoWidth);

	Locker lock(*m_pColorConverterMutex);

	int iWidth = m_iVideoHeight;
	int iHeight = m_iVideoWidth;

	int i = 0;
	int totalYValue = 0;

	for (int x = iWidth - 1; x >-1; --x)
	{
		for (int y = 0; y <iHeight; ++y)
		{

			pData[i] = m_pFrame[m_Multiplication[y][iWidth] + x];

			totalYValue += (pData[i] & 0xFF);
			m_VideoBeautificationer->MakePixelBright(&pData[i]);
			i++;
		}
	}

	int m_AverageValue = totalYValue / m_Multiplication[iHeight][iWidth];

	m_VideoBeautificationer->SetBrighteningValue(m_AverageValue , 10/*int brightnessPrecision*/);

	int halfWidth = iWidth >> 1;
	int halfHeight = iHeight >> 1;
	int dimention = m_Multiplication[iHeight][iWidth];
	int vIndex = dimention + m_Multiplication[halfHeight][halfWidth];

	for (int x = halfWidth - 1; x>-1; --x)
	{
		for (int y = 0; y < halfHeight; ++y)
		{
			int ind = ( m_Multiplication[y][halfWidth] + x) * 2;
			pData[vIndex++] = m_pFrame[dimention + ind];
			pData[i++] = m_pFrame[dimention + ind + 1];
		}
	}

}

void CColorConverter::NegativeRotateAndConvertNV12ToI420(unsigned char *m_pFrame, unsigned char *pData)
{
    Locker lock(*m_pColorConverterMutex);
    
    int iWidth = m_iVideoHeight;
    int iHeight = m_iVideoWidth;
    
    int i = iWidth * iHeight - iHeight;
    
    for(int y=iWidth-1;y>=0; y--)
    {
        int temp = i;
        for(int x = 0; x<iHeight;x++)
        {
            pData[temp++] = m_pFrame[x*iWidth + y];
        }
        
        i-=iHeight;
    }
    
    
    int halfWidth = iWidth / 2;
    int halfHeight = iHeight / 2;
    int dimention = m_Multiplication[iHeight][iWidth];
    int vIndex = dimention + m_Multiplication[halfHeight][halfWidth];
    
    i = dimention+ halfWidth*halfHeight - halfHeight;
    vIndex = dimention + halfHeight*halfWidth + halfWidth*halfHeight - halfHeight;
    
    for(int y=halfWidth-1;y>=0; y--)
    {
        int temp = i;
        int temp2 = vIndex;
        for(int x = 0; x<halfHeight;x++)
        {
            int ind = (x*halfWidth + y) << 1;
            
            pData[temp2++] = m_pFrame[dimention + ind + 1];
            pData[temp++] = m_pFrame[dimention + ind];
        }
        i-=halfHeight;
        vIndex-=halfHeight;
    }
    
}


void CColorConverter::mirrorRotateAndConvertNV12ToI420(unsigned char *m_pFrame, unsigned char *pData)
{
	Locker lock(*m_pColorConverterMutex);

	int iWidth = m_iVideoHeight;
	int iHeight = m_iVideoWidth;

	int i = 0;

	for (int x = iWidth - 1; x >-1; --x)
	{
		for (int y = 0; y < iHeight; ++y)
		{
			pData[i] = m_pFrame[m_Multiplication[y][iWidth] + x];
			i++;
		}
	}

	int halfWidth = iWidth / 2;
	int halfHeight = iHeight / 2;
	int dimention = m_Multiplication[iHeight][iWidth];
	int vIndex = dimention + m_Multiplication[halfHeight][halfWidth];

	for (int x = halfWidth - 1; x>-1; --x)
		for (int y = 0; y < halfHeight; ++y)
		{
			int ind = (m_Multiplication[y][halfWidth] + x) << 1 ;
			pData[vIndex++] = m_pFrame[dimention + ind + 1];
			pData[i++] = m_pFrame[dimention + ind];
		}

}


int CColorConverter::mirrorI420_XDirection(unsigned char *inData, unsigned char *outData, int iHeight, int iWidth)
{
    int halfHeight = iHeight>>1;
    int iTotalHeight = iHeight + halfHeight;
    int indx = 0;
    for (int y = 0; y < iTotalHeight; ++y)
    {
        for (int x = iWidth - 1; x > -1; --x)
        {
            outData[indx++] = inData[y*iWidth + x];
            
        }
    }
    return indx;
}

void CColorConverter::mirrorAndConvertNV12ToI420(unsigned char *m_pFrame, unsigned char *pData)
{
	int iWidth = m_iVideoWidth;
	int iHeight =  m_iVideoHeight;

	int i = 0;
	for (int y = 0; y < iHeight; ++y)
	{
		for (int x = iWidth - 1; x > -1; --x)
		{
			pData[i] = m_pFrame[m_Multiplication[y][iWidth] + x];
			i++;
		}
	}

	int halfWidth = iWidth >> 1;
	int halfHeight = iHeight >> 1;
	int dimention = m_Multiplication[iHeight][iWidth];
	int vIndex = dimention + iWidth;
	int nYUV12Height = iHeight + halfHeight;

	for (int y = iHeight; y < nYUV12Height; ++y)
		for (int x = halfWidth - 1; x > -1; --x)
		{
			int ind = m_Multiplication[y][iWidth] + (x << 1);
			pData[vIndex++] = m_pFrame[ind + 1];
			pData[i++] = m_pFrame[ind];
		}

}

void CColorConverter::mirrorRotateAndConvertNV21ToI420ForBackCam(unsigned char *m_pFrame, unsigned char *pData)
{
	Locker lock(*m_pColorConverterMutex);

	int iWidth = m_iVideoHeight;
	int iHeight = m_iVideoWidth;

	int i = 0;

	int totalYValue = 0;

	for (int x = 0; x < iWidth; x++)
	{
		for (int y = iHeight - 1; y > -1; --y)
		{
			pData[i] = m_pFrame[m_Multiplication[y][iWidth] + x];

			totalYValue += (pData[i] & 0xFF);
			m_VideoBeautificationer->MakePixelBright(&pData[i]);

			i++;
		}
	}

	int m_AverageValue = totalYValue / m_Multiplication[iHeight][iWidth];

	m_VideoBeautificationer->SetBrighteningValue(m_AverageValue , 10/*int brightnessPrecision*/);

	int halfWidth = iWidth / 2;
	int halfHeight = iHeight / 2;
	int dimention = m_Multiplication[iHeight][iWidth];
	int vIndex = dimention + m_Multiplication[halfHeight][halfWidth];

//	for (int x = halfWidth - 1; x>-1; --x)
	for (int x = 0; x < halfWidth; x++)
		for (int y = halfHeight - 1; y > -1; --y)
		{
			int ind = ( m_Multiplication[y][halfWidth] + x) * 2;
			pData[vIndex++] = m_pFrame[dimention + ind];
			pData[i++] = m_pFrame[dimention + ind + 1];
		}

}

void CColorConverter::mirrorRotateAndConvertNV12ToI420ForBackCam(unsigned char *m_pFrame, unsigned char *pData)
{
	Locker lock(*m_pColorConverterMutex);

	int iWidth = m_iVideoHeight;
	int iHeight = m_iVideoWidth;

	int i = 0;

	//	for (int x = iWidth - 1; x >-1; --x)
	for (int x = 0; x < iWidth; x++)
	{
		for (int y = iHeight - 1; y > -1; --y)
		{
			pData[i] = m_pFrame[m_Multiplication[y][iWidth] + x];
			i++;
		}
	}

	int halfWidth = iWidth / 2;
	int halfHeight = iHeight / 2;
	int dimention = m_Multiplication[iHeight][iWidth];
	int vIndex = dimention + m_Multiplication[halfHeight][halfWidth];

	//	for (int x = halfWidth - 1; x>-1; --x)
	for (int x = 0; x < halfWidth; x++)
		for (int y = halfHeight - 1; y > -1; --y)
		{
			int ind = (m_Multiplication[y][halfWidth] + x) * 2;
			pData[vIndex++] = m_pFrame[dimention + ind + 1];
			pData[i++] = m_pFrame[dimention + ind];
		}

}

int CColorConverter::ConverterYUV420ToRGB24(unsigned char * pYUVs, unsigned char * pRGBs, int height, int width)
{
	Locker lock(*m_pColorConverterMutex);

	int yIndex = 0;
	int uIndex = width * height;
	int vIndex = (width * height * 5) / 4;


	for (int r = 0; r < height; r++)
	{
		byte* pRGB = pRGBs + r * width * 3;

		if (r % 2 != 0)
		{
			uIndex -= (width >> 1);
			vIndex -= (width >> 1);
		}
		for (int c = 0; c < width; c += 2)
		{
			int C1 = pYUVs[yIndex++] - 16;
			int C2 = pYUVs[yIndex++] - 16;


			int D = pYUVs[vIndex++] - 128;
			int E = pYUVs[uIndex++] - 128;

			int R1 = (298 * C1 + 409 * E + 128) >> 8;
			int G1 = (298 * C1 - 100 * D - 208 * E + 128) >> 8;
			int B1 = (298 * C1 + 516 * D + 128) >> 8;

			int R2 = (298 * C2 + 409 * E + 128) >> 8;
			int G2 = (298 * C2 - 100 * D - 208 * E + 128) >> 8;
			int B2 = (298 * C2 + 516 * D + 128) >> 8;


			pRGB[0] = (byte)(R1 < 0 ? 0 : R1 > 255 ? 255 : R1);
			pRGB[1] = (byte)(G1 < 0 ? 0 : G1 > 255 ? 255 : G1);
			pRGB[2] = (byte)(B1 < 0 ? 0 : B1 > 255 ? 255 : B1);

			pRGB[3] = (byte)(R2 < 0 ? 0 : R2 > 255 ? 255 : R2);
			pRGB[4] = (byte)(G2 < 0 ? 0 : G2 > 255 ? 255 : G2);
			pRGB[5] = (byte)(B2 < 0 ? 0 : B2 > 255 ? 255 : B2);


			pRGB += 6;

		}
	}
	return width * height * 3;
}

void CColorConverter::mirrorYUVI420(unsigned char *pFrame, unsigned char *pData, int iHeight, int iWidth)
{
	Locker lock(*m_pColorConverterMutex);

	int yLen = m_Multiplication[iHeight][iWidth];;
	int uvLen = yLen >> 2;
	int vStartIndex = yLen + uvLen;
	int vEndIndex = (yLen * 3) >> 1;

	for(int i=0; i<iHeight;i++)
	{
		int k = iWidth-1;
		for(int j=0; j <iWidth; j++)
		{
			pData[i*iWidth +k] = pFrame[i*iWidth+j];
			k--;
		}

	}


	int uIndex = vStartIndex-1;
	int smallHeight = iHeight >> 1;
	int smallWidth = iWidth >> 1;

	for(int i=0; i<smallHeight;i++)
	{
		int k = smallWidth -1;
		for(int j=0; j <smallWidth; j++)
		{
			pData[yLen + i*smallWidth +k] = pFrame[yLen + i*smallWidth+j];
			k--;
		}

	}

	for(int i=0; i<smallHeight;i++)
	{
		int k = smallWidth - 1;
		for(int j=0; j <smallWidth; j++)
		{
			pData[vStartIndex+i*smallWidth +k] = pFrame[vStartIndex+i*smallWidth+j];
			k--;
		}

	}
}


static unsigned char clip[896];

static void InitClip() 
{
	memset(clip, 0, 320);
	for (int i = 0; i<256; ++i) clip[i + 320] = i;
	memset(clip + 320 + 256, 255, 320);
}

static inline unsigned char Clip(int x)
{
	return clip[320 + ((x + 0x8000) >> 16)];
}

int CColorConverter::ConvertRGB24ToI420(unsigned char* lpIndata, unsigned char* lpOutdata)
{
	Locker lock(*m_pColorConverterMutex);

	static bool bInit = false;

	if (!bInit)
	{
		bInit = true;
		InitClip();
	}

	const int cyb = int(0.114 * 219 / 255 * 65536 + 0.5);
	const int cyg = int(0.587 * 219 / 255 * 65536 + 0.5);
	const int cyr = int(0.299 * 219 / 255 * 65536 + 0.5);

	unsigned char* py = lpOutdata;
	unsigned char* pu = lpOutdata + m_iVideoWidth * m_iVideoHeight;
	unsigned char* pv = pu + m_iVideoWidth*m_iVideoHeight / 4;

	for (int row = 0; row < m_iVideoHeight; ++row)
	{
		unsigned char* rgb = lpIndata + m_iVideoWidth * 3 * (m_iVideoHeight - 1 - row);
		for (int col = 0; col < m_iVideoWidth; col += 2)
		{
			// y1 and y2 can't overflow
			int y1 = (cyb*rgb[0] + cyg*rgb[1] + cyr*rgb[2] + 0x108000) >> 16;
			*py++ = y1;
			int y2 = (cyb*rgb[3] + cyg*rgb[4] + cyr*rgb[5] + 0x108000) >> 16;
			*py++ = y2;

			if ((row & 1) == 0)
			{
				int scaled_y = (y1 + y2 - 32) * int(255.0 / 219.0 * 32768 + 0.5);
				int b_y = ((rgb[0] + rgb[3]) << 15) - scaled_y;
				unsigned char u = *pu++ = Clip((b_y >> 10) * int(1 / 2.018 * 1024 + 0.5) + 0x800000);  // u
				int r_y = ((rgb[2] + rgb[5]) << 15) - scaled_y;
				unsigned char v = *pv++ = Clip((r_y >> 10) * int(1 / 1.596 * 1024 + 0.5) + 0x800000);  // v
			}
			rgb += 6;
		}
	}

	return m_iVideoHeight * m_iVideoWidth * 3 / 2;
}


/*
int CColorConverter::ConvertRGB24ToI420(unsigned char *input, unsigned char *output)
{
	if (m_bClipInitialization == false)
	{
		m_bClipInitialization = true;
		memset(m_pClip, 0, 320);
		for (int i = 0; i < 256; ++i) m_pClip[i + 320] = i;
		memset(m_pClip + 320 + 256, 255, 320);

		cyb = int(0.114 * 219 / 255 * 65536 + 0.5);
		cyg = int(0.587 * 219 / 255 * 65536 + 0.5);
		cyr = int(0.299 * 219 / 255 * 65536 + 0.5);

	}

	

	int py = 0;
	int pu = m_iVideoWidth*m_iVideoHeight;
	int pv = pu + m_iVideoWidth*m_iVideoHeight / 4;

	for (int row = 0; row < m_iVideoHeight; ++row)
	{
		unsigned char *rgb = input + m_iVideoWidth * 3 * (m_iVideoHeight - 1 - row);
		
		

		for (int col = 0; col < m_iVideoWidth; col += 2)
		{
			// y1 and y2 can't overflow
			int y1 = (cyb*rgb[0] + cyg*rgb[1] + cyr*rgb[2] + 0x108000) >> 16;
			output[py++] = (unsigned char)y1;
			int y2 = (cyb*rgb[3] + cyg*rgb[4] + cyr*rgb[5] + 0x108000) >> 16;
			output[py++] = (unsigned char)y2;

			if ((row & 1) == 0)
			{
				int scaled_y = (y1 + y2 - 32) * int(255.0 / 219.0 * 32768 + 0.5);
					
				int b_y = ((rgb[0] + rgb[3]) << 15) - scaled_y;
				int x1 = (b_y >> 10) * int(1 / 2.018 * 1024 + 0.5) + 0x800000;
				unsigned char u = output[pu++] = m_pClip[320 + ((x1 + 0x8000) >> 16)];  // u


				int r_y = ((rgb[2] + rgb[5]) << 15) - scaled_y;
				int x2 = (r_y >> 10) * int(1 / 1.596 * 1024 + 0.5) + 0x800000;
				unsigned char v = output[pv++] = m_pClip[320 + ((x2 + 0x8000) >> 16)];  // v
			}
			rgb += 6;
		}
		
	}

	return m_iVideoHeight * m_iVideoWidth * 3 / 2;
}
*/

int CColorConverter::ConvertRGB32ToRGB24(unsigned char *input, int iHeight, int iWidth, unsigned char *output)
{
	Locker lock(*m_pColorConverterMutex);

    int in_len = iHeight * iWidth * 4;
    
    int indx = 0;
    for(int i=0;i<in_len; i+=4)
    {
        output[indx++] = input[i];
        output[indx++] = input[i+1];
        output[indx++] = input[i+2];
    }
    
    return indx;
}


int CColorConverter::DownScaleYUVNV12_YUVNV21_AverageNotApplied(byte* pData, int &iHeight, int &iWidth, byte* outputData)
{
    
    int YPlaneLength = iHeight*iWidth;
    int indx = 0;
    
    for(int i=0;i<iHeight;i+=4)
    {
        for(int j=0;j<iWidth;j+=2)
        {
            outputData[indx++] = pData[i*iWidth + j];
        }
        
        for(int j=0;j<iWidth;j+=2)
        {
            outputData[indx++] = pData[(i+1)*iWidth + j];
        }
    }
    
    byte*p = pData+YPlaneLength;
    
    for(int i=0;i<iHeight/2;i+=2)
    {
        for(int j=0;j<iWidth;j+=4)
        {
            outputData[indx++] = p[i*iWidth + j];
            outputData[indx++] = p[i*iWidth + j+1];
        }
    }
    
    //cout<<"CurrentLen = "<<indx<<endl;
    
    iHeight = iHeight>>1;
    iWidth = iWidth>>1;
    
    
    return indx;
    
}

int CColorConverter::DownScaleYUVNV12_YUVNV21_AverageVersion1(byte* pData, int &iHeight, int &iWidth, byte* outputData)
{
    
    int YPlaneLength = iHeight*iWidth;
    int indx = 0;
    
    for(int i=0;i<iHeight;i+=4)
    {
        for(int j=0;j<iWidth;j+=2)
        {
            //outputData[indx++] = pData[i*iWidth + j];
            
            int w,x,y,z;
            w = pData[i*iWidth + j];
            x = pData[i*iWidth + j+2];
            y = pData[(i+2)*iWidth + j];
            z = pData[(i+2)*iWidth + j+2];
            int avg = (w+x+y+z)/4;
            outputData[indx++] = (byte)avg;
            
        }
        
        for(int j=0;j<iWidth;j+=2)
        {
            int I = i+1;
            
            int w,x,y,z;
            w = pData[I*iWidth + j];
            x = pData[I*iWidth + j+2];
            y = pData[(I+2)*iWidth + j];
            z = pData[(I+2)*iWidth + j+2];
            int avg = (w+x+y+z)/4;
            outputData[indx++] = (byte)avg;
        }
    }
    
    byte*p = pData+YPlaneLength;
    for(int i=0;i<iHeight/2;i+=2)
    {
        for(int j=0;j<iWidth;j+=4)
        {
            int w,x,y,z, J, avg;
            
            
            w = p[i*iWidth + j];
            x = p[i*iWidth + j+2];
            y = p[(i+1)*iWidth + j];
            z = p[(i+1)*iWidth + j+2];
            avg = (w+x+y+z)/4;
            outputData[indx++] = (byte)avg;
            //outputData[indx++] = p[i*iWidth + j];
            
            J = j+1;
            w = p[i*iWidth + J];
            x = p[i*iWidth + J+2];
            y = p[(i+1)*iWidth + J];
            z = p[(i+1)*iWidth + J+2];
            avg = (w+x+y+z)/4;
            outputData[indx++] = (byte)avg;
            //outputData[indx++] = p[i*iWidth + j+1];
        }
    }
    
    cout<<"CurrentLen = "<<indx<<endl;
    
    iHeight = iHeight>>1;
    iWidth = iWidth>>1;
    
    
    return indx;
    
}


int CColorConverter::DownScaleYUVNV12_YUVNV21_AverageVersion2(byte* pData, int &iHeight, int &iWidth, byte* outputData)
{
    
    int YPlaneLength = iHeight*iWidth;
    int indx = 0;
    
    for(int i=0;i<iHeight;i+=4)
    {
        for(int j=0;j<iWidth;j+=2)
        {
            //outputData[indx++] = pData[i*iWidth + j];
            int w,x,y,z;
            if(j%2==0)
            {
                w = pData[i*iWidth + j];
                x = pData[i*iWidth + j+1];
                y = pData[(i+1)*iWidth + j];
                z = pData[(i+1)*iWidth + j+1];
                int avg = (w+x+y+z)/4;
                outputData[indx++] = (byte)avg;
            }
            else
            {
                w = pData[i*iWidth + j+1];
                x = pData[i*iWidth + j+2];
                y = pData[(i+1)*iWidth + j+1];
                z = pData[(i+1)*iWidth + j+2];
                int avg = (w+x+y+z)/4;
                outputData[indx++] = (byte)avg;
            }
        }
        
        for(int j=0;j<iWidth;j+=2)
        {
            int I = i+1;
            
            int w,x,y,z;
            if(j%2==0)
            {
                w = pData[(I+1)*iWidth + j];
                x = pData[(I+1)*iWidth + j+1];
                y = pData[(I+2)*iWidth + j];
                z = pData[(I+2)*iWidth + j+1];
                int avg = (w+x+y+z)/4;
                outputData[indx++] = (byte)avg;
            }
            else
            {
                w = pData[(I+1)*iWidth + j+1];
                x = pData[(I+1)*iWidth + j+2];
                y = pData[(I+2)*iWidth + j+1];
                z = pData[(I+2)*iWidth + j+2];
                int avg = (w+x+y+z)/4;
                outputData[indx++] = (byte)avg;
            }
        }
    }
    
    byte*p = pData+YPlaneLength;
    for(int i=0;i<iHeight/2;i+=2)
    {
        for(int j=0;j<iWidth;j+=4)
        {
            int w,x,y,z, J, avg;
            
            
            w = p[i*iWidth + j];
            x = p[i*iWidth + j+2];
            y = p[(i+1)*iWidth + j];
            z = p[(i+1)*iWidth + j+2];
            avg = (w+x+y+z)/4;
            outputData[indx++] = (byte)avg;
            
            J = j+1;
            w = p[i*iWidth + J];
            x = p[i*iWidth + J+2];
            y = p[(i+1)*iWidth + J];
            z = p[(i+1)*iWidth + J+2];
            avg = (w+x+y+z)/4;
            outputData[indx++] = (byte)avg;
        }
    }
    
    //cout<<"CurrentLen = "<<indx<<endl;
    
    iHeight = iHeight>>1;
    iWidth = iWidth>>1;
    
    
    return indx;
    
}

int CColorConverter::DownScaleYUV420_EvenVersion(unsigned char* pData, int &iHeight, int &iWidth, unsigned char* outputData)
{
	//cout<<"inHeight,inWidth = "<<iHeight<<", "<<iWidth<<endl;
	int YPlaneLength = iHeight*iWidth;
	int UPlaneLength = YPlaneLength >> 2;


	int indx = 0;

	int iNewHeight = iHeight >> 1;
	int iNewWidth = iWidth >> 1;

	if (iNewHeight % 2 != 0) iNewHeight = iNewHeight - 1;
	if (iNewWidth % 2 != 0) iNewWidth = iNewWidth - 1;

	iNewHeight = iNewHeight * 2;
	iNewWidth = iNewWidth * 2;

	//cout<<"Now iHeight = "<<iHeight<<", now iWidth = "<<iWidth<<endl;

	for (int i = 0; i<iNewHeight; i += 2)
	{
		for (int j = 0; j<iNewWidth; j += 2)
		{
			int w, x, y, z;
			w = pData[i*iWidth + j];
			x = pData[i*iWidth + j + 1];
			y = pData[(i + 1)*iWidth + j];
			z = pData[(i + 1)*iWidth + j + 1];
			int avg = (w + x + y + z) / 4;
			outputData[indx++] = (byte)avg;

		}
	}




	byte *p = pData + YPlaneLength;
	byte *q = pData + YPlaneLength + UPlaneLength;
	int uIndex = indx;
	int vIndex = indx + iNewHeight * iNewWidth;


	for (int i = 0; i<iNewHeight / 2; i += 2)
	{
		for (int j = 0; j<iNewWidth; j += 2)
		{
			int w, x, y, z, J, avg;


			w = p[i*iWidth + j];
			x = p[i*iWidth + j + 1];
			y = p[(i + 1)*iWidth + j];
			z = p[(i + 1)*iWidth + j + 1];
			avg = (w + x + y + z) / 4;
			outputData[uIndex++] = (byte)avg;


			w = q[i*iWidth + j];
			x = q[i*iWidth + j + 1];
			y = q[(i + 1)*iWidth + j];
			z = q[(i + 1)*iWidth + j + 1];
			avg = (w + x + y + z) / 4;
			outputData[vIndex++] = (byte)avg;
		}
	}

	iHeight = iNewHeight >> 1;
	iWidth = iNewWidth >> 1;

	return iHeight * iWidth * 3 / 2;

}

//Date: 07-January-2017
//Receive YUV420 Data, and address of Height and Width, and Diff. Example: Diff = 3 means video will become 1/3 of original video.

int CColorConverter::DownScaleYUV420_Dynamic(unsigned char* pData, int &iHeight, int &iWidth, unsigned char* outputData, int diff)
{
    //cout<<"inHeight,inWidth = "<<iHeight<<", "<<iWidth<<endl;
    int YPlaneLength = iHeight*iWidth;
    int UPlaneLength = YPlaneLength >> 2;
    
    
    int indx = 0;
    int H, W;
    
    int iNewHeight = iHeight/diff;
    int iNewWidth = iWidth/diff;
    
    H =  iHeight - iHeight%diff;
    W = iWidth - iWidth % diff;
    
    if(iNewHeight%2!=0)
    {
        iNewHeight--;
        H = iNewHeight * diff;
    }
    
    if(iNewWidth%2!=0)
    {
        iNewWidth--;
        W = iNewWidth * diff;
    }
    
    
    
   // printf("iNewHeight, iNewWidth --> %d, %d\n", iNewHeight, iNewWidth);
    
    int avg;
    
    for(int i=0; i<H; i+=diff)
    {
        for(int j=0; j<W; j+=diff)
        {
            int sum = 0;
            for(int k=i; k<(i+diff); k++)
            {
                for(int l=j; l<(j+diff); l++)
                {
                    sum+=pData[k*iWidth + l];
                }
            }
            avg = sum/(diff*diff);
            outputData[indx++] = (byte)avg;
            
        }
    }
    
    //printf("index = %d\n", indx);
    
    
    
    
    byte *p = pData + YPlaneLength;
    byte *q = pData + YPlaneLength + UPlaneLength;
    int uIndex = indx;
    int vIndex = indx + (iNewHeight * iNewWidth)/4;
    
    int halfH = H>>1, halfW = W>>1;
    int www = iWidth>>1;
    
    for(int i=0;i<halfH;i+=diff)
    {
        for(int j=0;j<halfW;j+=diff)
        {
            int sum1 = 0, sum2 = 0;
            
            for(int k=i; k<(i+diff); k++)
            {
                for(int l=j; l<(j+diff); l++)
                {
                    sum1+=p[k*www + l];
                    sum2+=q[k*www + l];
                }
            }
            
            avg = sum1/(diff*diff);
            outputData[uIndex++] = (byte)avg;
            
            avg = sum2/(diff*diff);
            outputData[vIndex++] = (byte)avg;
        }
    }
    
    //printf("uIndex, vIndex = %d, %d\n", uIndex, vIndex);
    
    //cout<<"CurrentLen = "<<indx<<endl;
    
    iHeight = iNewHeight;
    iWidth = iNewWidth;
    
    return iHeight * iWidth * 3 / 2;
    
}


int CColorConverter::CreateFrameBorder(unsigned char* pData, int iHeight, int iWidth, int Y, int U, int V)
{
    int iTotal = iHeight * iWidth;
    
    for(int i=0;i<iHeight;i++) //Traverse through each row
    {
        int rowIndx = i*iWidth;
        pData[rowIndx + 0] = Y;
        //pData[rowIndx + 1] = Y;
        pData[rowIndx + iWidth-1] = Y;
       // pData[rowIndx + iWidth-2] = Y;
        
        pData[getUIndex(iHeight, iWidth, i, 0, iTotal)] = U;
        //pData[getUIndex(iHeight, iWidth, i, 1, iTotal)] = U;
        pData[getUIndex(iHeight, iWidth, i, iWidth-1, iTotal)] = U;
        //pData[getUIndex(iHeight, iWidth, i, iWidth-2, iTotal)] = U;
        
        pData[getVIndex(iHeight, iWidth, i, 0, iTotal)] = V;
        //pData[getVIndex(iHeight, iWidth, i, 1, iTotal)] = V;
        pData[getVIndex(iHeight, iWidth, i, iWidth -1, iTotal)] = V;
        //pData[getVIndex(iHeight, iWidth, i, iWidth-2, iTotal)] = V;
        
    }
    
    for(int j=0;j<iWidth;j++) //Traverse through each Column
    {
        int colIndx = j;
        pData[0 + colIndx] = Y;
        //pData[iWidth + colIndx] = Y;
        pData[(iHeight-1)*iWidth + colIndx] = Y;
        //pData[(iHeight-2)*iWidth + colIndx] = Y;
        
        pData[getUIndex(iHeight, iWidth,  0, colIndx, iTotal)] = U;
        //pData[getUIndex(iHeight, iWidth,  1, colIndx, iTotal)] = U;
        pData[getUIndex(iHeight, iWidth,  (iHeight-1), colIndx, iTotal)] = U;
        //pData[getUIndex(iHeight, iWidth,  (iHeight-2), colIndx, iTotal)] = U;
        
        pData[getVIndex(iHeight, iWidth,  0, colIndx, iTotal)] = V;
        //pData[getVIndex(iHeight, iWidth,  1, colIndx, iTotal)] = V;
        pData[getVIndex(iHeight, iWidth,  (iHeight-1), colIndx, iTotal)] = V;
        //pData[getVIndex(iHeight, iWidth,  (iHeight-2), colIndx, iTotal)] = V;
        
        
    }
    
    return iHeight * iWidth * 3 / 2;
}

void CColorConverter::SetSmallFrame(unsigned char * smallFrame, int iHeight, int iWidth, int nLength)
{
	Locker lock(*m_pColorConverterMutex);

	//int iLen = DownScaleYUV420_EvenVersion(smallFrame, iHeight, iWidth, m_pSmallFrame);
	//memcpy(smallFrame, m_pSmallFrame, iLen);
	//iLen = DownScaleYUV420_EvenVersion(smallFrame, iHeight, iWidth, m_pSmallFrame);
    
    int iLen = DownScaleYUV420_Dynamic(smallFrame, iHeight, iWidth, m_pSmallFrame, 3 /*Making 1/3 rd of original Frame*/);
    m_iSmallFrameHeight = iHeight;
    m_iSmallFrameWidth = iWidth;

	m_iSmallFrameSize = iHeight * iWidth * 3 / 2;
    
    iLen = CreateFrameBorder(m_pSmallFrame, iHeight, iWidth, 0, 128, 128); // [Y:0, U:128, V:128] = Black

	m_bMergingSmallFrameEnabled = true;
 
	//memcpy(m_pSmallFrame, smallFrame, nLength);
}

//This Function will return UIndex based on YUV_420 Data
int CColorConverter::getUIndex(int h, int w, int yVertical, int xHorizontal, int& total)
{
    return (yVertical>>1) * (w>>1) + (xHorizontal>>1) + total; //total = h * w
}

//This Function will return VIndex based on YUV_420 Data
int CColorConverter::getVIndex(int h, int w, int yVertical, int xHorizontal, int& total)
{
    return (yVertical>>1) * (w>>1) + (xHorizontal>>1) + total + (total>>2); //total = h * w
}

//Date: 28-December-2016
//Constraits:
// i) Receive Big YUV420 Data  and  Small YUV420 Data, Finally Output Merged YUV420 Data
// ii) iPosX is Distance in Horizontal Direction. Value must be in Range iPosX >= 0 and iPosX < (BigDataWidth - SmallDataWidth)
// ii) iPosY is Distance in Vertical Direction. Value must be in Range iPosY >= 0 and iPosY < (BigDataHeight - SmallDataHeight)

int CColorConverter::Merge_Two_Video(unsigned char *pInData1, int iPosX, int iPosY, int iVideoHeight, int iVideoWidth)
{
	Locker lock(*m_pColorConverterMutex);

	if (m_bMergingSmallFrameEnabled == false)
		return 0;

	int h1 = iVideoHeight;
	int w1 = iVideoWidth;
	int h2 = /* m_iVideoHeight >> 2 */ m_iSmallFrameHeight;
	int w2 = /* m_iVideoWidth >> 2 */ m_iSmallFrameWidth;

    int iLen1 = h1 * w1 * 3 / 2;
    int iLen2 = h2 * w2 * 3 / 2;
    
    int total1 = h1 * w1, total2 = h2 * w2;
    
    for(int i=iPosY;i<(iPosY+h2);i++)
    {
        for(int j=iPosX;j<(iPosX+w2);j++)
        {
            int ii = i-iPosY;
            int jj = j-iPosX;
            int now1 = i*w1 + j;
            int now2 = ii*w2 + jj;
            
			pInData1[now1] = m_pSmallFrame[now2];
			pInData1[getUIndex(h1, w1, i, j, total1)] = m_pSmallFrame[getUIndex(h2, w2, ii, jj, total2)];
			pInData1[getVIndex(h1, w1, i, j, total1)] = m_pSmallFrame[getVIndex(h2, w2, ii, jj, total2)];
        }
    }
    
    return iLen1;
}

int CColorConverter::CropWithAspectRatio_YUVNV12_YUVNV21(unsigned char* pData, int inHeight, int inWidth, int screenHeight, int screenWidth, unsigned char* outputData, int &outHeight, int &outWidth)
{
    //cout<<"inHeight,inWidth = "<<iHeight<<", "<<iWidth<<endl;

	if (screenHeight == -1 || screenWidth == -1)
		return 0;

    int YPlaneLength = inHeight*inWidth;
    int UPlaneLength = YPlaneLength >> 2;
    
    float aspectRatio_Screen, aspectRatio_VideoData;
    int indx = 0;
    int newHeight, newWidth, diff;
    
    aspectRatio_Screen = screenHeight * 1.0 / screenWidth;
    aspectRatio_VideoData = inHeight * 1.0 / inWidth;
    
    
    //cout<<"Screen_Ratio = "<<aspectRatio_Screen<<", Video_Ratio = "<<aspectRatio_VideoData<<endl;
    
    if(fabs(aspectRatio_Screen - aspectRatio_VideoData) < 0.00001)
    {
        //Do Nothing
        newHeight = inHeight;
        newWidth = inWidth;
        memcpy(outputData, pData, inHeight*inWidth*3/2);
        
    }
    else if(aspectRatio_Screen > aspectRatio_VideoData)
    {
        //We have to delete columns [reduce Width]
        newWidth = floor(inHeight / aspectRatio_Screen);
        
        int target = floor(inWidth * 0.85);
        
        if(newWidth < target)
        {
            newWidth = target;
        }
        
        newWidth = newWidth - newWidth % 16;
        newHeight = inHeight;
        diff = inWidth - newWidth;
        
        
        Crop_YUVNV12_YUVNV21(pData, inHeight, inWidth, diff/2,diff/2,0,0, outputData, newHeight, newWidth);
        cout<<"First Block, Deleting Columns"<<endl;
        
    }
    else
    {
        //We have to delete rows [Reduce Height]
        
        newHeight = floor(inWidth * aspectRatio_Screen);
        newHeight = newHeight - newHeight%2;
        newWidth = inWidth;
        diff = inHeight - newHeight;
        
        Crop_YUVNV12_YUVNV21(pData, inHeight, inWidth, 0,0,diff,0, outputData, newHeight, newWidth);
        cout<<"Second Block, Deleting Rows working"<<endl;
    }
    
    
    
    outHeight = newHeight;
    outWidth = newWidth;
    
    return outHeight*outWidth*3/2;
    
}

int CColorConverter::Crop_YUVNV12_YUVNV21(unsigned char* pData, int inHeight, int inWidth, int startXDiff, int endXDiff, int startYDiff, int endYDiff, unsigned char* outputData, int &outHeight, int &outWidth)
{
    //cout<<"inHeight,inWidth = "<<iHeight<<", "<<iWidth<<endl;
    int YPlaneLength = inHeight*inWidth;
    int UPlaneLength = YPlaneLength >> 2;
    int indx = 0;
    
    for(int i=startYDiff; i<(inHeight-endYDiff); i++)
    {
        for(int j=startXDiff; j<(inWidth-endXDiff); j++)
        {
            outputData[indx++] = pData[i*inWidth + j];
        }
    }
    
    
    byte *p = pData + YPlaneLength;
    int uIndex = indx;
    int vIndex = indx + 1;
    
    
    int halfH = inHeight>>1, halfW = inWidth>>1;
    
    for(int i=startYDiff/2; i<(halfH-endYDiff/2); i++)
    {
        for(int j=startXDiff; j<(inWidth-endXDiff); j+=2)
        {
            outputData[uIndex] = p[i*inWidth + j];
            outputData[vIndex] = p[i*inWidth + j + 1];
            uIndex+=2;
            vIndex+=2;
        }
    }
    
    outHeight = inHeight - startYDiff - endYDiff;
    outWidth = inWidth - startXDiff - endXDiff;
    printf("Now, First Block, H:W -->%d,%d  Indx = %d, uIndex = %d, vIndex = %d\n", outHeight, outWidth, indx, uIndex, vIndex);
    return outHeight*outWidth*3/2;
    
}


int CColorConverter::GetWidth()
{
	Locker lock(*m_pColorConverterMutex);

	return m_iVideoWidth;
}
int CColorConverter::GetHeight()
{
	Locker lock(*m_pColorConverterMutex);

	return m_iVideoHeight;
}

int CColorConverter::GetSmallFrameWidth()
{
    Locker lock(*m_pColorConverterMutex);
    
    return m_iSmallFrameWidth;
}
int CColorConverter::GetSmallFrameHeight()
{
    Locker lock(*m_pColorConverterMutex);
    
    return m_iSmallFrameHeight;
}

int CColorConverter::GetScreenHeight()
{
    Locker lock(*m_pColorConverterMutex);
    return m_iDeviceHeight;
}

int CColorConverter::GetScreenWidth()
{
    Locker lock(*m_pColorConverterMutex);
    return m_iDeviceWidth;
}

void CColorConverter::ClearSmallScreen()
{
	Locker lock(*m_pColorConverterMutex);

	m_bMergingSmallFrameEnabled = false;
}

bool CColorConverter::GetSmallFrameStatus()
{
	Locker lock(*m_pColorConverterMutex);

	return m_bMergingSmallFrameEnabled;
}

void CColorConverter::GetSmallFrame(unsigned char *pSmallFrame)
{
	Locker lock(*m_pColorConverterMutex);

	memcpy(pSmallFrame, m_pSmallFrame, m_iSmallFrameSize);
}

