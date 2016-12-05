
#include "ColorConverter.h"
#include "../VideoEngineController/LogPrinter.h"


#if defined(__ANDROID__) || defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR) || defined(TARGET_OS_WINDOWS_PHONE)
typedef unsigned char byte;
#endif

CColorConverter::CColorConverter(int iVideoHeight, int iVideoWidth) :

m_iVideoHeight(iVideoHeight),
m_iVideoWidth(iVideoWidth),
m_YPlaneLength(m_iVideoHeight*m_iVideoWidth),
m_VPlaneLength(m_YPlaneLength >> 2),
m_UVPlaneMidPoint(m_YPlaneLength + m_VPlaneLength),
m_UVPlaneEnd(m_UVPlaneMidPoint + m_VPlaneLength)

{
	CLogPrinter_Write(CLogPrinter::INFO, "CColorConverter::CColorConverter");

	m_PrevAddValue = 0;
	m_AverageValue = 0;
	m_ThresholdValue = 0;

	for (int i = 0; i < 481; i++)
		for (int j = 0; j < 641; j++)
		{
			m_Multiplication[i][j] = i*j;
		}

	m_pColorConverterMutex.reset(new CLockHandler);

	CLogPrinter_Write(CLogPrinter::DEBUGS, "CColorConverter::CColorConverter Prepared");
}

CColorConverter::~CColorConverter()
{

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


#define getMin(a,b) a<b?a:b

void CColorConverter::mirrorRotateAndConvertNV21ToI420(unsigned char *m_pFrame, unsigned char *pData)
{
	Locker lock(*m_pColorConverterMutex);

	int iWidth = m_iVideoHeight;
	int iHeight = m_iVideoWidth;

	//LOGEG("fahad -->> avgValue= %d, addValue= %d, thresholdVal= %d", m_AverageValue, m_PrevAddValue, m_ThresholdValue);

	int i = 0;
	int totalYValue = 0;
	unsigned char yVal = 0;
	for (int x = iWidth - 1; x >-1; --x)
	{
		for (int y = 0; y <iHeight; ++y)
		{
			yVal =  m_pFrame[m_Multiplication[y][iWidth] + x];
			pData[i] = getMin(yVal + m_PrevAddValue, 255);
			totalYValue += yVal;
			i++;
		}
	}

	m_AverageValue = totalYValue / (iHeight * iWidth);

	if(m_AverageValue < 10)
	{
		m_ThresholdValue = 44;
	}else if(m_AverageValue < 20)
	{
		m_ThresholdValue = 50;
	}else if(m_AverageValue < 30)
	{
		m_ThresholdValue = 60;
	}else if(m_AverageValue < 40)
	{
		m_ThresholdValue = 70;
	}else if(m_AverageValue < 50)
	{
		m_ThresholdValue = 75;
	}else if(m_AverageValue < 60)
	{
		m_ThresholdValue = 80;
	}else if(m_AverageValue < 70)
	{
		m_ThresholdValue = 90;
	}else if(m_AverageValue < 80)
	{
		m_ThresholdValue = 95;
	}else if(m_AverageValue < 90)
	{
		m_ThresholdValue = 100;
	}else if(m_AverageValue < 100)
	{
		m_ThresholdValue = 110;
	}else if(m_AverageValue < 110){
		m_ThresholdValue = 115;
	}else if(m_AverageValue < 120){
		m_ThresholdValue = 125;
	}else{
		m_ThresholdValue = 100;
	}

	m_PrevAddValue = (m_ThresholdValue - m_AverageValue);
	m_PrevAddValue = m_PrevAddValue >> 1;
	if (m_PrevAddValue < 0)
		m_PrevAddValue = 0;


	int halfWidth = iWidth >> 1;
	int halfHeight = iHeight >> 1;
	int dimention = m_Multiplication[iHeight][iWidth];
	int vIndex = dimention + m_Multiplication[halfHeight][halfWidth];

	for (int x = halfWidth - 1; x>-1; --x)
		for (int y = 0; y < halfHeight; ++y)
		{
			int ind = ( m_Multiplication[y][halfWidth] + x) * 2;
			pData[vIndex++] = m_pFrame[dimention + ind];
			pData[i++] = m_pFrame[dimention + ind + 1];
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
