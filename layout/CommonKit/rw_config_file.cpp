
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "rw_config_file.h"

CConfigFile::CConfigFile()
:m_errorType(NoError)
{
    memset(m_chFileName,0,sizeof(char)*FILE_PATH_LEN);
}

CConfigFile::~CConfigFile()
{
    if(m_fStreamFile.is_open())
    {
        //WriteFile();
        m_fStreamFile.close();
    }
}

bool CConfigFile::ReadFile(char* chFileName)
{  
    m_vecLineStr.clear();
    m_fStreamFile.open(chFileName);//���ļ�       
    if( ! m_fStreamFile.is_open())   
    {  
        m_errorType = CanNotOpenFile;
        return false;  
    }  
    strncpy_s(m_chFileName,sizeof(char)*FILE_PATH_LEN,chFileName,strlen(chFileName));
    char chTemp[LINE_MAX_STRING_LEN] = {0};  
    int iLineNum = 0;
    char* chRemove = NULL;
    while(!m_fStreamFile.eof()) //ѭ����ȡÿһ��   
    {  
        memset(chTemp,0,sizeof(char)*LINE_MAX_STRING_LEN);
        m_fStreamFile.getline(chTemp,LINE_MAX_STRING_LEN); //ÿ�ж�ȡ LINE_MAX_STRING_LEN���ֽ�
        if(iLineNum == 0)//��һ��ȥ��BOMͷ����
        {
            RemoveBomHeader(chTemp,chRemove) ? AnalyzeFile(chRemove) : AnalyzeFile(chTemp);
        }
        else
        {
            AnalyzeFile(chTemp);
        }
        
        ++ iLineNum;
    }  
    return true;
}

void CConfigFile::AddBlankLine(char* chLineStr)
{
    KEY_VALUE keyValue;
    keyValue.Keytype = TypeBlank;
    strncpy_s(keyValue.chInfo,sizeof(char)*LINE_MAX_STRING_LEN,chLineStr,sizeof(char)*LINE_MAX_STRING_LEN);
    if(!m_vecLineStr.empty())
    {
        m_vecLineStr.back().vecKeyValue.push_back(keyValue);
    }
}

void CConfigFile::AddSectionLine(char* chLineStr,int iFrontBlankNum,int iBackBlankNum)
{
    SECTIONINFO secInfo;
    strncpy_s(secInfo.chInfo,sizeof(char)*LINE_MAX_STRING_LEN,chLineStr + iFrontBlankNum + 1,Min(strlen(chLineStr) - iFrontBlankNum - iBackBlankNum - 2,SECTION_NAME_LEN));
    m_vecLineStr.push_back(secInfo);
}

void CConfigFile::AddKeyLine(char* chLineStr,int iFrontBlankNum,int iBackBlankNum)
{
    KEY_VALUE keyValue;
    keyValue.Keytype = TypeKey;
    char* pchKey = new char[KEY_LEN];
    if(pchKey != NULL)
    {
        memset(pchKey,0,sizeof(char)*KEY_LEN);
    }
    char* pchValue = new char[VALUE_LEN];
    if(pchValue != NULL)
    {
        memset(pchValue,0,sizeof(char)*VALUE_LEN);
    }

    if(GetKeyNameAndValue(iFrontBlankNum,iBackBlankNum,chLineStr,pchKey,pchValue))
    {
        strncpy_s(keyValue.chKey,sizeof(char)*KEY_LEN,pchKey,sizeof(char)*KEY_LEN);
        strncpy_s(keyValue.chValue,sizeof(char)*VALUE_LEN,pchValue,sizeof(char)*VALUE_LEN);
        if(!m_vecLineStr.empty())
        {
            m_vecLineStr.back().vecKeyValue.push_back(keyValue);
        }
    }
    SafeDeleteArray(pchKey);
    SafeDeleteArray(pchValue);
}

void CConfigFile::AddValueLine(char* chLineStr,int iFrontBlankNum,int iBackBlankNum)
{
    KEY_VALUE keyValue;
    keyValue.Keytype = TypeValue;
    strncpy_s(keyValue.chValue,sizeof(char)*VALUE_LEN,chLineStr + iFrontBlankNum,Min(strlen(chLineStr) - iFrontBlankNum - iBackBlankNum,VALUE_LEN));
    if(!m_vecLineStr.empty())
    {
        m_vecLineStr.back().vecKeyValue.push_back(keyValue);
    }
}

void CConfigFile::AnalyzeFile(char* chLineStr)
{
    int iFrontBlankNum = 0;
    int iBackBlankNum = 0;

    LineType type = GetLineType(chLineStr,iFrontBlankNum,iBackBlankNum);
    switch (type)
    {
    case TypeBlank:
        AddBlankLine(chLineStr);
        break;
    case TypeSection:
        AddSectionLine(chLineStr,iFrontBlankNum,iBackBlankNum);
        break;
    case TypeKey:
        AddKeyLine(chLineStr,iFrontBlankNum,iBackBlankNum);
        break;
    case TypeValue:
        AddValueLine(chLineStr,iFrontBlankNum,iBackBlankNum);
        break;
    case TypeError:
        m_errorType = FormatError;
        break;
    default:
        break;
    }
}

bool CConfigFile::IsAnnotate(char* chLineStr)
{
    bool bRet = false;
    for(int i = 0;i < static_cast<int>(strlen(chLineStr)); ++i)
    {
        if(chLineStr[i] == ' ' || chLineStr[i] == '\0') //�����ַ�ǰ��Ŀո�
        {
            continue;
        }
        if(chLineStr[i] == '#') //�����һ�������ľ���#�ţ���ô��Ϊ���ַ���Ϊע����
        {
            bRet = true;
            break;
        }
        else //�����һ���ַ�����#��ô�Ͳ���ע����
        {
            break;
        }
    }
    return bRet;
}

bool CConfigFile::IsBlankLine(char* chLineStr)
{
    bool bRet = true;
    for(int i = 0; i < static_cast<int>(strlen(chLineStr)); ++i)
    {
        if(chLineStr[i] == ' ' || chLineStr[i] == '\0')
        {
            continue;
        }
        else
        {
            bRet = false;   //ֻҪ���к��зǿո�ͽ��������ַ�������Ϊ���ǿ���
            break;
        }
    }
    return bRet;
}

LineType CConfigFile::GetLineType(char* chLineStr,int& iFrontBlankNum,int& iBackBlankNum)
{
    LineType type = TypeBlank;
    iFrontBlankNum = 0;
    iBackBlankNum = 0;
    int iLineStrLen = strlen(chLineStr);
    if(iLineStrLen <= 0)
    {
        type = TypeBlank;
    }
    for(int i = 0; i < iLineStrLen; ++i)
    {
        if(chLineStr[i] == ' ')  //����ǰ��Ŀո�
        {
            ++iFrontBlankNum;//��¼�������ַ�����ͷ�ո����
            continue;
        }
        if(chLineStr[i] == '#')   //��һ�����ֵ���#��˵����ע����
        {
            type = TypeBlank;
            break;
        }
        else if(chLineStr[i] == '[') //��һ�����ֵ���'['���������']'
        {
            bool bSection = false;
            for(int j = iLineStrLen - 1; j > i; --j)
            {
                if(chLineStr[j] == ']') //��������ֵ���']'����˵��������������
                {
                    bSection = true;
                    iBackBlankNum = iLineStrLen - j - 1;
                    break;
                }
                else if(chLineStr[j] != ' ')//��������ֵĲ���']'�Ҳ��ǿո���˵����ʽ����
                {
                    break;
                }
            }
            type = bSection ? TypeSection : TypeError;
            break;
        }
        else                             //��һ�����ֵļȲ���#Ҳ����[����ô���������ң����Ƿ��ǹؼ�����
        {
            bool bKey = false;
            for(int j = iLineStrLen - 1; j > i; --j)
            {
                if(chLineStr[i] == ' ')
                {
                    ++iBackBlankNum;
                    continue;
                }
                if(chLineStr[j] == '=')
                {
                    bKey = true;
                    break;
                }
            }
            type = bKey ? TypeKey : TypeValue;
            break;
        }
    }
    return type;
}

bool CConfigFile::GetKeyNameAndValue(int iFrontBlankNum,int iBackBlankNum,char* chText,char*& chName,char*& chValue)
{
    char* p = strchr(chText,'=');
    if(p == NULL)
    {
        return false;
    }
    for(int i = 0;i < static_cast<int>(strlen(chText) - strlen(p)); ++i)
    {
        if(chText[iFrontBlankNum + i] != ' ')
        {
            chName[i] = chText[iFrontBlankNum + i];
        }
        else
        {
            break;
        }
    }
    int iBlankNum = 0;
    int i = 1;
    while(p[i++] == ' ' && i < static_cast<int>(strlen(p)) - iBackBlankNum)
    {
        ++iBlankNum;
    }
    strncpy_s(chValue,sizeof(char)*VALUE_LEN,p + iBlankNum + 1,Min(strlen(p) - iBackBlankNum - 1,VALUE_LEN));
    return true;
}

ErrorType CConfigFile::CF_GetLastError() const
{
    return m_errorType;
}

std::string CConfigFile::GetValue(const std::string& strSecKeyName,const std::string& strDefVal)
{
	std::string strRet = GetValue(strSecKeyName) ;
	if( NoError != CF_GetLastError() )
		strRet = strDefVal ;
	
	return strRet ;
}

std::string CConfigFile::GetValue(const std::string& strSecKeyName)
{
    std::string strValue = "";
    if(strSecKeyName.at(0) != '/')
    {
        m_errorType = ParamError;
        return strValue;
    }
    std::string::size_type posLast = strSecKeyName.rfind("/");
    if(posLast < 0 || posLast > strSecKeyName.length())
    {
        m_errorType = ParamError;
        return strValue;
    }
    std::string strSectionName = strSecKeyName.substr(1,posLast - 1);
    std::string strKeyName = strSecKeyName.substr(posLast + 1);
    bool bFinded = false;
    
    for(int i = 0; i < static_cast<int>(m_vecLineStr.size()); ++i)
    {
        if(strSectionName.compare(m_vecLineStr.at(i).chInfo) == 0)
        {
            std::vector<KEY_VALUE> vecKeyValue = m_vecLineStr.at(i).vecKeyValue;
            for(int j = 0; j < static_cast<int>(vecKeyValue.size()); ++j)
            {
                if(vecKeyValue.at(j).Keytype == TypeKey && strKeyName.compare(vecKeyValue.at(j).chKey) == 0)
                {
                    strValue = vecKeyValue.at(j).chValue;
                    bFinded = true;
                    break;
                }
            }
        }
        if(bFinded)
        {
            break;
        }
    }
    m_errorType = bFinded ? NoError : NotFound;

    return strValue;
}

int CConfigFile::GetValueInt(const std::string& strSecKeyName,int nDefVal)
{
	int nRet = GetValueInt( strSecKeyName ) ;
	if( NoError != CF_GetLastError() )
		nRet = nDefVal ;

	return nRet ;
}

int CConfigFile::GetValueInt(const std::string& strSecKeyName)
{
    std::string strValue = GetValue(strSecKeyName);
    if(!strValue.empty())
    {
        return atoi(strValue.c_str());
    }
    
    return VALUE_ERROR;
}

bool CConfigFile::RemoveBomHeader(char* chLineStr,char*& chRemove)
{
    bool bRet = false;
    printf("%x,%x,%x\n",chLineStr[0],chLineStr[1],chLineStr[2]);
    if(chLineStr[0] == 0xFFFFFFEF && chLineStr[1] == 0xFFFFFFBB && chLineStr[2] == 0xFFFFFFBF)//ƥ��ǰ�����ֽ��Ƿ���BOM�ֽ�
    {
        chRemove = chLineStr + 3;//�����BOM�ֽڣ����˵��������ֽ�
        bRet = true;
    }
    return bRet;
}

std::wstring CConfigFile::GetValueW(const std::string& strSecKeyName,const std::string& strDefVal)
{
	std::wstring strRetW = GetValueW( strSecKeyName ) ;
	if( NoError == CF_GetLastError() )
		return strRetW  ;
	
	return StringToWstring(strDefVal) ;
}

std::wstring CConfigFile::StringToWstring(const std::string& strSrc)
{
	std::string strValue = strSrc ;
	int iUTF8Size = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)strValue.c_str(), -1, NULL, 0);//��CP_UTF8��ʽ����ת����������CP_ACP

	if(iUTF8Size <= 0) 
	{
		return std::wstring(L"");
	}
	wchar_t *pwszDst = new wchar_t[iUTF8Size+1];
	if( NULL == pwszDst) 
	{
		return std::wstring(L"");
	}
	memset(pwszDst,0,sizeof(wchar_t)*(iUTF8Size + 1));
	MultiByteToWideChar(CP_UTF8, 0,(LPCSTR)strValue.c_str(), -1, (LPWSTR)pwszDst, iUTF8Size);
	pwszDst[iUTF8Size] = 0;

	std::wstring wcharString(pwszDst);
	SafeDeleteArray(pwszDst);

	return wcharString;
}

std::wstring CConfigFile::GetValueW(const std::string& strSecKeyName)
{
    std::string strValue = GetValue(strSecKeyName);
    return StringToWstring(strValue) ;
}

std::string CConfigFile::WstringToString(const std::wstring& wstrSrc)
{
    //��ʱ�Ŀ��ַ���ʽΪUTF8��ʽ�����ת��string�����Ҫ��CP_ACP����ת��������ת��ȥ�Ķ��ֽڻ�������
    int iTextLen = WideCharToMultiByte(CP_ACP,0,(LPWSTR)wstrSrc.c_str(),-1,NULL,0,NULL,NULL);
    char* pchDst = new char[iTextLen + 1];
    if(pchDst == NULL)
    {
        return std::string("");
    }
    memset(pchDst,0,sizeof(char)*(iTextLen + 1));
    WideCharToMultiByte(CP_ACP,0,(LPWSTR)wstrSrc.c_str(),-1,pchDst,iTextLen,NULL,NULL);
    pchDst[iTextLen] = 0;

    std::string strDst(pchDst);
    SafeDeleteArray(pchDst);
    
    return strDst;
}

std::string CConfigFile::GetValueT(const std::string& strSecKeyName,const std::string& strDefVal)
{
	std::string strRet = GetValueT( strSecKeyName ) ;
	if( NoError != CF_GetLastError() )
		strRet = strDefVal ;

	return strRet ;
}

std::string CConfigFile::GetValueT(const std::string& strSecKeyName)
{
    std::wstring wsValue = GetValueW(strSecKeyName);
    return WstringToString(wsValue);
}

double CConfigFile::GetValueDouble(const std::string& strSecKeyName,double dDefVal)
{
	double dRet = GetValueDouble( strSecKeyName ) ;
	if( NoError != CF_GetLastError() )
		dRet = dDefVal ;

	return dRet ;
}

double CConfigFile::GetValueDouble(const std::string& strSecKeyName)
{
    std::string strValue = GetValue(strSecKeyName);
    return atof(strValue.c_str());
}

void CConfigFile::CloseFile()
{
    if(m_fStreamFile.is_open())
    {
        m_fStreamFile.close();
    }
    
}

bool CConfigFile::SetValue(const std::string& strSecKeyName,std::string strValue)
{
    if(strSecKeyName.at(0) != '/')
    {
        m_errorType = ParamError;
        return false;
    }
    std::string::size_type posLast = strSecKeyName.rfind("/");
    if(posLast < 0 || posLast > strSecKeyName.length())
    {
        m_errorType = ParamError;
        return false;
    }
    std::string strSectionName = strSecKeyName.substr(1,posLast - 1);
    std::string strKeyName = strSecKeyName.substr(posLast + 1);

    bool bFinded = false;
    for(int i = 0; i < static_cast<int>(m_vecLineStr.size()); ++i)
    {
        if(strSectionName.compare(m_vecLineStr.at(i).chInfo) == 0)
        {
            std::vector<KEY_VALUE> vecKeyValue = m_vecLineStr.at(i).vecKeyValue;
            for(int j = 0; j < static_cast<int>(vecKeyValue.size()); ++j)
            {
                if(vecKeyValue.at(j).Keytype == TypeKey && strKeyName.compare(vecKeyValue.at(j).chKey) == 0)
                {
                    strncpy_s(m_vecLineStr.at(i).vecKeyValue.at(j).chValue,sizeof(char)*VALUE_LEN,strValue.c_str(),strValue.length());
                    bFinded = true;
                }
            }
        }
    }
    m_errorType = bFinded ? NoError : NotFound;
    if(!bFinded)
    {
        return false;
    }
    return WriteFile();//�޸��������д���ļ�
    //return true;
}

bool CConfigFile::SetValue(const std::string& strSecKeyName,int iValue)
{
    char chValue[VALUE_LEN] = {0};
    sprintf_s(chValue,sizeof(char)*VALUE_LEN,"%d",iValue);
    std::string strValue(chValue);
    return SetValue(strSecKeyName,strValue);
}

bool CConfigFile::SetValue(const std::string& strSecKeyName,double dValue)
{
    char chValue[VALUE_LEN] = {0};
    sprintf_s(chValue,sizeof(char)*VALUE_LEN,"%.4f",dValue);
    std::string strValue(chValue);
    return SetValue(strSecKeyName,strValue);
}

bool CConfigFile::WriteFile()
{
    if(!m_fStreamFile.is_open())
    {
        m_errorType = CanNotOpenFile;
        return false;
    }
    char* pChStrBuf = NULL;
    if(!BuildString(pChStrBuf))
    {
        return false;
    }
	pChStrBuf[strlen(pChStrBuf)] = 0;

	//m_fStreamFile.seekg(0, ios::end);
	//int size = m_fStreamFile.tellg();//�Ȼ�ȡָ������λ�ã����ļ����ܴ�С

    m_fStreamFile.clear();//����������������ļ��򿪵������д�룬��Ȼд��ʧ��
    m_fStreamFile.seekp(0,ios::beg);//���ļ���ͷ��ʼд
	int bufSize = strlen(pChStrBuf);
	m_fStreamFile.write(pChStrBuf, bufSize);

    SafeDeleteArray(pChStrBuf);
    return true;
}

bool CConfigFile::BuildString(char*& pStrBuf)
{
    pStrBuf = new char[FILE_SIZE];
    if(pStrBuf == NULL)
    {
        m_errorType = MemoryError;
        SafeDeleteArray(pStrBuf);
        return false;
    }
    memset(pStrBuf,0,FILE_SIZE);
    int iSectionNum = static_cast<int>(m_vecLineStr.size());
    for(int i = 0; i < iSectionNum; ++i)
    {
        char strSec[SECTION_NAME_LEN + 2] = {0};
        sprintf_s(strSec,SECTION_NAME_LEN+2,"[%s]\n",m_vecLineStr.at(i).chInfo);//��֯�����и�ʽ�ַ���
        strcat_s(pStrBuf,FILE_SIZE,strSec);

        std::vector<KEY_VALUE> vecKeyValue = m_vecLineStr.at(i).vecKeyValue;
        int iKeyNum = static_cast<int>(vecKeyValue.size());
        for(int j = 0; j < iKeyNum; ++j)
        {
            char strTemp[LINE_MAX_STRING_LEN] = {0};
            if(vecKeyValue.at(j).Keytype == TypeKey) //��֯�ؼ������ַ���
            {
                if(i == iSectionNum - 1 && j == iKeyNum - 1)//���һ�в��ټӻ���
                {
                    sprintf_s(strTemp,LINE_MAX_STRING_LEN*sizeof(char),"%s = %s\n",vecKeyValue.at(j).chKey,vecKeyValue.at(j).chValue);
                }
                else
                {
                    sprintf_s(strTemp,LINE_MAX_STRING_LEN*sizeof(char),"%s = %s\n",vecKeyValue.at(j).chKey,vecKeyValue.at(j).chValue);
                }
                strcat_s(pStrBuf,FILE_SIZE,strTemp);
            }
            else //�����Ŀ��У�ע���У�����һ�еļ�ֵ�У���ͳһ����
            {
                if(i == iSectionNum - 1 && j == iKeyNum - 1)//���һ�в��ټӻ���
                {
                    sprintf_s(strTemp,LINE_MAX_STRING_LEN*sizeof(char),"%s\n",vecKeyValue.at(j).chInfo);
                }
                else
                {
                    sprintf_s(strTemp,LINE_MAX_STRING_LEN*sizeof(char),"%s\n",vecKeyValue.at(j).chInfo);
                }
                strcat_s(pStrBuf,FILE_SIZE,strTemp);
            }
        }
    }
    return true;
}
