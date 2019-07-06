#ifndef __SENSOR_INI_FILE__
#define __SENSOR_INI_FILE__

#include "DTCCM2_SDK\dtccm2.h"

class SensorIniFile
{
public:
    CString        m_strFile;
    SensorTab      m_tab;

public:
    SensorIniFile(CString strFile)
    {
        m_strFile = strFile;
        memset(&m_tab, 0x00, sizeof(SensorTab));

        m_tab.ParaList = new USHORT [8192*4];   //(USHORT*) malloc(8192*sizeof(USHORT));
        m_tab.SleepParaList = new USHORT [2048];   //(USHORT*) malloc(1024*2);
        m_tab.AF_InitParaList = new USHORT [8192];
        m_tab.AF_AutoParaList = new USHORT [2048];
        m_tab.AF_FarParaList = new USHORT [2048];
        m_tab.AF_NearParaList = new USHORT [2048];
        m_tab.Exposure_ParaList = new USHORT [2048];
        m_tab.Gain_ParaList = new USHORT [2048];
    }

    ~SensorIniFile()
    {
        delete [] m_tab.ParaList;
        delete [] m_tab.SleepParaList;
        delete [] m_tab.AF_InitParaList;
        delete [] m_tab.AF_AutoParaList;
        delete [] m_tab.AF_FarParaList;
        delete [] m_tab.AF_NearParaList;
        delete [] m_tab.Exposure_ParaList;
        delete [] m_tab.Gain_ParaList;
    }


    CString sIniPathName()
    {
        TCHAR sFilename[_MAX_PATH];
        TCHAR sDrive[_MAX_DRIVE];
        TCHAR sDir[_MAX_DIR];
        TCHAR sFname[_MAX_FNAME];
        TCHAR sExt[_MAX_EXT];
        GetModuleFileName(AfxGetInstanceHandle(), sFilename, _MAX_PATH);
        _tsplitpath(sFilename, sDrive, sDir, sFname, sExt);

        CString rVal(CString(sDrive) + CString(sDir));

        if ( rVal.Right(1) != _T('\\') )
            rVal += _T("\\"); 

        return rVal;
    }

    CString sIniFileName()
    {
        if (!m_strFile.GetLength())
        {
            m_strFile = sIniPathName()+"hsset.ini";
        }
        return m_strFile;
    }

    BOOL WriteIniData(CString sSection, CString sSectionKey, int nValue)
    {
        CString  sValue;
        sValue.Format("%d",nValue);
        return WritePrivateProfileString(sSection,sSectionKey,sValue,sIniFileName());
    }

    //UpdateCode --------------------------
    BOOL WriteIniData(CString sSection, CString sSectionKey, CString nValue)
    {
        return WritePrivateProfileString(sSection,sSectionKey,nValue,sIniFileName());
    }
    //--------------------------

    int ReadIniData(CString sSection,CString sSectionKey,int nDefault)
    {
        return GetPrivateProfileInt(sSection,sSectionKey,nDefault,sIniFileName());
    }

    //2010 09 07 added...
    long ReadIniDataHex(CString sSection,CString sSectionKey,long nDefault)
    {
        CString sTemp;
        GetPrivateProfileString(sSection,sSectionKey,"", sTemp.GetBuffer(MAX_PATH), MAX_PATH,sIniFileName());
        sTemp.ReleaseBuffer();
        char* pAddr = sTemp.GetBuffer(MAX_PATH); 
        long x = sTemp.IsEmpty() ? nDefault : strtoul( pAddr, NULL, 16); 
        sTemp.ReleaseBuffer();
        return x;
    }

    //2010 09 07 added...
    BOOL WriteIniDataHex(CString sSection, CString sSectionKey, long nValue)
    {
        CString sTmp;
        sTmp.Format("%x", nValue);
        return WritePrivateProfileString(sSection,sSectionKey,sTmp,sIniFileName());
    }

    BOOL WriteIniString(CString sSection, CString sSectionKey, CString sValue)
    {
        return WritePrivateProfileString(sSection,sSectionKey,sValue,sIniFileName());
    }

    CString ReadIniString(CString sSection,CString sSectionKey,CString sDefault)
    {
        CString sTemp;
        GetPrivateProfileString(sSection,sSectionKey,sDefault,sTemp.GetBuffer(MAX_PATH), MAX_PATH,sIniFileName());
        sTemp.ReleaseBuffer();
        return sTemp;
    }

    int CheckIniPreferent()
    {
        int iOverlayFlag = 0;
        int iFoundSensor = 0;
        int iSensor[10];
        int iSensorTotal = 0;
        int temp = 0;
        CString  sSensornum;

        memset(iSensor, 0, 10); 

        iSensorTotal = ReadIniData("PREFERENTIAL","SensorTotal", -1);
        if (iSensorTotal == -1)
        {
            iSensorTotal = 1;
        }
        iOverlayFlag = ReadIniData("PREFERENTIAL","OverlayFlag", -1);
        if (iOverlayFlag == -1)
        {
            iOverlayFlag = 0;
        }

        for (int m=0; m<10; m++)
        {

            sSensornum.Format("%d", m);
            sSensornum = "Sensornum" + sSensornum;
            iSensor[m] =ReadIniData("PREFERENTIAL",sSensornum, -1);
        }
        //delete the Section "PREFERENTIAL"
        WritePrivateProfileString("PREFERENTIAL",NULL,NULL,sIniFileName());
        WriteIniData("PREFERENTIAL", "SensorTotal", iSensorTotal);
        WriteIniData("PREFERENTIAL", "OverlayFlag", iOverlayFlag);


        for(int n=0; n<10; n++)
        {
            if (iSensor[n] == 2 )
            {
                temp = iSensor[0];
                iSensor[0] = iSensor[n];
                iSensor[n] = temp;

            }
        }

        for(int p=0; p<10; p++)
        {
            if (iSensor[p] != -1 )
            {
                sSensornum.Format("%d", iFoundSensor);
                sSensornum = "Sensornum" + sSensornum;
                WriteIniData("PREFERENTIAL", sSensornum, iSensor[p]);
                iFoundSensor++;
            } 

        }
        return iFoundSensor;

    }


    BOOL LoadFile(CString filename)
    {
        CStdioFile file;
        if (!file.Open((filename), CFile::modeRead | CFile::typeText))
        {
            return FALSE;
        }
        CString szLine = _T("");
        int addr =0, reg=0, value=0;
        BYTE i2cmode = 0;

        USHORT ParaListSize = 0;
        USHORT SleepParaListSize = 0;
        USHORT AF_InitParaListSize = 0;
        USHORT AF_AutoParaListSize = 0;
        USHORT AF_FarParaListSize = 0;
        USHORT AF_NearParaListSize = 0;
        USHORT Exposure_ParaListSize = 0;
        USHORT Gain_ParaListSize = 0;

        USHORT *pParaList = m_tab.ParaList;
        USHORT *pSleepParaList = m_tab.SleepParaList;
        USHORT *pAF_InitParaList = m_tab.AF_InitParaList;
        USHORT *pAF_AutoParaList = m_tab.AF_AutoParaList;
        USHORT *pAF_FarParaList = m_tab.AF_FarParaList;
        USHORT *pAF_NearParaList = m_tab.AF_NearParaList;
        USHORT *pExposure_ParaList = m_tab.Exposure_ParaList;
        USHORT *pGain_ParaList =  m_tab.Gain_ParaList;


        CString sReg, sVal;
        CString strTmp[10];
        int tmp = 0;
        strTmp[0] = "[ParaList]";
        strTmp[1] = "[SleepParaList]";
        strTmp[2] = "[AF_InitParaList]";
        strTmp[3] = "[AF_AutoParaList]";
        strTmp[4] = "[AF_FarParaList]";
        strTmp[5] = "[AF_NearParaList]";
        strTmp[6] = "[Exposure_ParaList]";
        strTmp[7] = "[Gain_ParaList]";

        for(int i = 0; i <10; i++)
        {
            strTmp[i].MakeLower();
            strTmp[i].TrimLeft();
            strTmp[i].TrimRight();	
        }
        int state = -1;
        while(file.ReadString(szLine))
        {
            CString Textout;
            //寻找注释符号或者']',如果有这样的，只取前面的，
            tmp = szLine.FindOneOf("//"); 
            if ( tmp == 0)
            {
                continue;
            }
            else if (tmp > 0)
            {
                szLine = szLine.Left(tmp);
            }
            tmp = szLine.FindOneOf("]"); 
            if ( tmp == 0)
            {
                continue;
            }
            else if (tmp > 0)
            {
                szLine = szLine.Left(tmp+1);
            }
            szLine.MakeLower();
            szLine.TrimLeft();
            szLine.TrimRight();		

            if (szLine == strTmp[0]) 
            {
                state = 0;
                ParaListSize = 0;
                continue;
            }
            else if (szLine == strTmp[1])
            {
                state = 1;
                SleepParaListSize = 0;
                continue;
            }
            else if (szLine == strTmp[2])
            {
                state = 2;
                AF_InitParaListSize = 0;
                continue;
            }
            else if (szLine == strTmp[3])
            {
                state = 3;
                AF_AutoParaListSize = 0;
                continue;
            }
            else if (szLine == strTmp[4])
            {
                state = 4;
                AF_FarParaListSize = 0;
                continue;
            }
            else if (szLine == strTmp[5])
            {
                state = 5;
                AF_NearParaListSize = 0;
                continue;
            }
            else if (szLine == strTmp[6])
            {
                state = 6;
                Exposure_ParaListSize = 0;
                continue;
            }
            else if (szLine == strTmp[7])
            {
                state = 7;
                Gain_ParaListSize = 0;
                continue;
            }

            if (szLine.IsEmpty())
                continue;
            if (szLine.Left(1) == ",")
                continue;
            if (szLine.Left(1) == ";")
                continue;
            if (szLine.Left(1) == "/")
                continue;

            if (szLine.Left(1) == "[")
            {
                state = -1;
                continue;
            }


            AfxExtractSubString(sReg, szLine, 0, ',');
            AfxExtractSubString(sVal, szLine, 1, ',');
            sReg.TrimLeft();   
            sReg.TrimRight();
            sVal.TrimRight();  
            sVal.TrimLeft();

            if (!sscanf(sReg, "0x%x", &reg)) //读取键值对数据	
                sscanf(sReg, "%d", &reg);

            if (!sscanf(sVal, "0x%x", &value)) //读取键值对数据	
                sscanf(sVal, "%d", &value);

            if (state == 0)
            {
                *(pParaList+ParaListSize) = reg;
                *(pParaList+ParaListSize+1) = value;
                ParaListSize += 2;
            }
            else if (state == 1)
            {
                *(pSleepParaList+SleepParaListSize) = reg;
                *(pSleepParaList+SleepParaListSize+1) = value;
                SleepParaListSize += 2;			
            }
            else if (state == 2)
            {
                *(pAF_InitParaList+AF_InitParaListSize) = reg;
                *(pAF_InitParaList+AF_InitParaListSize+1) = value;
                AF_InitParaListSize += 2;			
            }
            else if (state == 3)
            {
                *(pAF_AutoParaList+AF_AutoParaListSize) = reg;
                *(pAF_AutoParaList+AF_AutoParaListSize+1) = value;
                AF_AutoParaListSize += 2;			
            }
            else if (state == 4)
            {
                *(pAF_FarParaList+AF_FarParaListSize) = reg;
                *(pAF_FarParaList+AF_FarParaListSize+1) = value;
                AF_FarParaListSize += 2;			
            }
            else if (state == 5)
            {
                *(pAF_NearParaList+AF_NearParaListSize) = reg;
                *(pAF_NearParaList+AF_NearParaListSize+1) = value;
                AF_NearParaListSize += 2;			
            }
            else if (state == 6)
            {
                *(pExposure_ParaList+Exposure_ParaListSize) = reg;
                *(pExposure_ParaList+Exposure_ParaListSize+1) = value;
                Exposure_ParaListSize += 2;			
            }
            else if (state == 7)
            {
                *(pGain_ParaList+Gain_ParaListSize) = reg;
                *(pGain_ParaList+Gain_ParaListSize+1) = value;
                Gain_ParaListSize += 2;			
            }

        }
        file.Close();

        m_tab.ParaListSize = ParaListSize;
        m_tab.SleepParaListSize = SleepParaListSize;
        m_tab.AF_InitParaListSize = AF_InitParaListSize;
        m_tab.AF_AutoParaListSize = AF_AutoParaListSize;
        m_tab.AF_FarParaListSize = AF_FarParaListSize;
        m_tab.AF_NearParaListSize = AF_NearParaListSize;
        m_tab.Exposure_ParaListSize = Exposure_ParaListSize;
        m_tab.Gain_ParaListSize = Gain_ParaListSize;

        return TRUE;
    }


    BOOL LoadIniFile()
    {
        m_tab.width    = ReadIniData("Sensor","width",0);
        m_tab.height   = ReadIniData("Sensor","height",0);
        m_tab.type     = ReadIniData("Sensor","type",2);

        m_tab.port = ReadIniData("Sensor", "port", 0);
        m_tab.pin = ReadIniData("Sensor", "pin", 0);

        m_tab.SlaveID  = ReadIniData("Sensor", "SlaveID", 0);
        m_tab.mode     = ReadIniData("Sensor", "mode", 0);
        m_tab.FlagReg  = ReadIniData("Sensor", "FlagReg", 0);
        m_tab.FlagMask = ReadIniData("Sensor", "FlagMask", 0xff);
        m_tab.FlagData = ReadIniData("Sensor", "FlagData", 0);

        m_tab.FlagReg1  = ReadIniData("Sensor", "FlagReg1", 0);
        m_tab.FlagMask1 = ReadIniData("Sensor", "FlagMask1", 0x0);
        m_tab.FlagData1 = ReadIniData("Sensor", "FlagData1", 0);

        m_tab.outformat= ReadIniData("Sensor", "outformat", 0x00);
        m_tab.mclk     = ReadIniData("Sensor", "mclk", 0x01);

        m_tab.avdd     = ReadIniData("Sensor", "avdd", 0x00);
        m_tab.dovdd     = ReadIniData("Sensor", "dovdd", 0x00);
        m_tab.dvdd     = ReadIniData("Sensor", "dvdd", 0x00);


        LoadFile(m_strFile);
        if ( (m_tab.width==0)       ||
            (m_tab.height==0)      ||
            (m_tab.ParaList==NULL) ||
            (m_tab.ParaListSize==0)	  )
        {
            return FALSE;
        }

        return TRUE;
    }
};



#endif