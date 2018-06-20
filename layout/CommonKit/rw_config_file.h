/********************************************************************
* Copyright (c) 2015 �㽭���Ի����˼������޹�˾ 
* GZRobot All rights reserved. 
*
* �� �� ��: RWConfigFile.h
* �� �� ��: zhanghuihui
* ��������: 2015:6:12 10:18
* 
* ժ    Ҫ: һ��ͨ�õĶ�д�����ļ�����
*********************************************************************/
#ifndef RW_CONFIG_FILE_ZHH_20150612_H_
#define RW_CONFIG_FILE_ZHH_20150612_H_

//#include <Singleton.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

#define FILE_SIZE 1024*1024
#define FILE_PATH_LEN 256
#define LINE_MAX_STRING_LEN 1024
#define SECTION_NAME_LEN 32
#define KEY_LEN 32
#define VALUE_LEN 1024
#define VALUE_ERROR -65535
#define Min(a,b) (((a) > (b)) ? (b) : (a))
#define SafeDeleteArray(p)  if ((p)) {delete [] (p); (p) = NULL; }

enum ErrorType
{
    NoError,
    CanNotOpenFile,
    FormatError,
    ParamError,
    NotFound,                  //û���ҵ���ؼ�ֵ
    MemoryError              //�ڴ����
};
enum LineType
{
    TypeBlank,        //���л���ע����
    TypeSection,     //������
    TypeKey,           //�ؼ�����
    TypeValue,        //������мȲ������򡢹ؼ���Ҳ����ע�ͣ���ô��Ϊ����һ���ؼ��ֺ���ļ�ֵ��ֻ�Ƿ�������һ��
    TypeError
};
typedef struct tag_KeyValue //�ؼ��ּ���ֵ�ṹ��
{
    LineType Keytype;         //����¹ؼ�����һ����ʲô���ͣ�Ҳ�п����ǿ��к�ע���У��Լ���ֵ����һ��
    char chInfo[LINE_MAX_STRING_LEN];//������в��ǹؼ����У���ô�ô��ֶα��������Ϣ
    char chKey[KEY_LEN];    //��
    char chValue[VALUE_LEN];  //��ֵ
    tag_KeyValue()
    {
        Keytype = TypeKey;
        memset(chInfo,0,sizeof(char)*LINE_MAX_STRING_LEN);
        memset(chKey,0,sizeof(char)*KEY_LEN);
        memset(chValue,0,sizeof(char)*VALUE_LEN);
    }
}KEY_VALUE;

typedef struct tag_SectionInfo //���ַ�����Ϣ�ṹ��
{
    char chInfo[LINE_MAX_STRING_LEN];  //�ô��ֶα��������Ϣ������������У����¼����������
    std::vector<KEY_VALUE> vecKeyValue;  //���ڱ������Ϣ
    tag_SectionInfo()
    {
        memset(chInfo,0,sizeof(char)*LINE_MAX_STRING_LEN);
        vecKeyValue.clear();
    }
}SECTIONINFO;
class CConfigFile
{
private:
    CConfigFile();
	~CConfigFile();
public:
    static CConfigFile* GetInstance()
    {
        static CConfigFile configFile;
        return &configFile;
    }

public:
	
	bool ReadFile(char* chFileName);

    /********************************************************************
    * ��������: ��ȡ���һ�γ���Ĵ�������
    * �������: ��
    * �������: ��
    * �� �� ֵ: ��������ö��ֵ
    * ע������: 
    * �� �� ��: zhanghuihui
    * ��������: 2015:6:16 19:29
    * ********************************************************************/
    ErrorType CF_GetLastError() const;
	std::string GetValue(const std::string& strSecKeyName,const std::string& strDefVal);
	std::wstring GetValueW(const std::string& strSecKeyName,const std::string& strDefVal);
	std::string GetValueT(const std::string& strSecKeyName,const std::string& strDefVal);
	int GetValueInt(const std::string& strSecKeyName,int nDefVal);
	double GetValueDouble(const std::string& strSecKeyName,double dDefVal);

    /********************************************************************
    * ��������: ���ùؼ��ֵļ�ֵ��������������Ϊ���غ������������õ�������ѡ��ʹ��
    * �������: strSecKeyName:����ӹؼ����ָ�ʽΪ:��/VideoIp/yaan_ip��,strValue:���õļ�ֵ
    * �������: ��
    * �� �� ֵ: true:���óɹ���false:����ʧ��
    * ע������: 
    * �� �� ��: zhanghuihui
    * ��������: 2015:8:21 13:30
    * ********************************************************************/
    bool SetValue(const std::string& strSecKeyName,std::string strValue);
    bool SetValue(const std::string& strSecKeyName,int iValue);
    bool SetValue(const std::string& strSecKeyName,double dValue);
    bool WriteFile();//д�ļ��ӿ����ⲿ���ã������޸�������������ڲ�����д��

private:  //public:
    bool BuildString(char*& pStrBuf);

    /********************************************************************
    * ��������: ���ݹؼ��ֻ�ȡ��ֵ���ַ�����Ϣ���ؼ�����Ҫ�ϸ������¸�ʽ:��/VideoIp/yaan_ip��ǰ���ֶ�Ϊ�������������ֶ�Ϊ�ؼ�����
    * �������: strSecKeyName:�ؼ���
    * �������: ��
    * �� �� ֵ: ��ֵ�ַ���
    * ע������: 
    * �� �� ��: zhanghuihui
    * ��������: 2015:6:16 19:30
    * ********************************************************************/
    std::string GetValue(const std::string& strSecKeyName);

    /********************************************************************
    * ��������: ���ݹؼ��ֻ�ȡ��ֵ���ַ���(���ַ�)�����ڼ�ֵ���������ģ����ļ���UTF-8���룬��Ҫת���ɿ��ַ�UTF8��������
    * �������: strSecKeyName:�ؼ���
    * �������: ��
    * �� �� ֵ: ��ֵ�ַ���(���ַ�)
    * ע������: �����ֵ�к������ģ��ҷ���ֵϣ����ȡwstring��ʹ�ô˽ӿڻ�ȡ��ֵ
    * �� �� ��: zhanghuihui
    * ��������: 2015:6:16 19:30
    * ********************************************************************/
    std::wstring GetValueW(const std::string& strSecKeyName);

    /********************************************************************
    * ��������: ���ݹؼ��ֻ�ȡ��ֵ���ַ���(���ֽ�)����char�ַ�ת����UTF-8���룬��ת��ANSI��ʽ�����Ա�������
    * �������: strSecKeyName:�ؼ���
    * �������: ��
    * �� �� ֵ: ��ֵ�ַ���ANSI���ֽ�
    * ע������: �����ֵ�к������ģ��ҷ���ֵϣ����ȡstring��ʹ�ô˽ӿڻ�ȡ��ֵ
    * �� �� ��: zhanghuihui
    * ��������: 2015:6:17 10:03
    * ********************************************************************/
    std::string GetValueT(const std::string& strSecKeyName);

    /********************************************************************
    * ��������: ���ݹؼ��ֻ�ȡ��ֵ�����֣�
    * �������: strSecKeyName:�ؼ���
    * �������: ��
    * �� �� ֵ: ��ֵ
    * ע������: �����ֵΪ���֣�����ֱ�ӵ��ô˽ӿڷ���int������
    * �� �� ��: zhanghuihui
    * ��������: 2015:6:16 19:30
    * ********************************************************************/
    int GetValueInt(const std::string& strSecKeyName);

    /********************************************************************
    * ��������: ���ݹؼ��ֻ�ȡ��ֵ��double�����֣�
    * �������: strSecKeyName:�ؼ���
    * �������: ��
    * �� �� ֵ: ��ֵ
    * ע������: �����ֵΪdouble�����֣�����ֱ�ӵ��ô˽ӿڷ���double������
    * �� �� ��: zhanghuihui
    * ��������: 2015:6:16 19:30
    * ********************************************************************/
    double GetValueDouble(const std::string& strSecKeyName);

    void CloseFile();
private:
    /********************************************************************
    * ��������: �ж�һ���ַ����Ƿ���ע����
    * �������: chLineStr::�����ַ���
    * �������: ��
    * �� �� ֵ: true:ע���У�false:��ע����
    * ע������: 
    * �� �� ��: zhanghuihui
    * ��������: 2015:6:12 11:10
    * ********************************************************************/
    bool IsAnnotate(char* chLineStr);

    /********************************************************************
    * ��������: �ж��Ƿ��ǿ���
    * �������: chLineStr::�����ַ���
    * �������: ��
    * �� �� ֵ: true:���У�false:�ǿ���
    * ע������: 
    * �� �� ��: zhanghuihui
    * ��������: 2015:6:12 14:07
    * ********************************************************************/
    bool IsBlankLine(char* chLineStr);

    /********************************************************************
    * ��������: �ж�һ���ַ��������Ǽ���ע�ͣ����ǿ�������
    * �������: chLineStr::�����ַ���
    * �������: iFrontBlankNum:�ַ���ǰ��ո����,iBackBlankNum:�ַ�����β�ո����
    * �� �� ֵ: ������
    * ע������: 
    * �� �� ��: zhanghuihui
    * ��������: 2015:6:12 14:23
    * ********************************************************************/
    LineType GetLineType(char* chLineStr,int& iFrontBlankNum,int& iBackBlankNum);

    bool GetKeyNameAndValue(int iFrontBlankNum,int iBackBlankNum,char* chText,char*& chName,char*& chValue);

    /********************************************************************
    * ��������: ���ļ�ʱUTF-8��BOMͷ�����ʽ��ʱ���ļ��Ŀ�ͷ���EF BB BF�����ֽڵı�־���������룬�˺�������ȥ��BOMͷ
    * �������: chLineStr:�ı�ָ��
    * �������: chRemove:�޸ĺ���ı�ָ��
    * �� �� ֵ: true:�ı�����BOMͷ��ȥ���ɹ���false:�ı�������BOMͷ
    * ע������: 
    * �� �� ��: zhanghuihui
    * ��������: 2015:6:15 15:48
    * ********************************************************************/
    bool RemoveBomHeader(char* chLineStr,char*& chRemove);

    void AddBlankLine(char* chLineStr);
    void AddSectionLine(char* chLineStr,int iFrontBlankNum,int iBackBlankNum);
    void AddKeyLine(char* chLineStr,int iFrontBlankNum,int iBackBlankNum);
    void AddValueLine(char* chLineStr,int iFrontBlankNum,int iBackBlankNum);

    void AnalyzeFile(char* chLineStr);
    std::string WstringToString(const std::wstring& wstrSrc);
	std::wstring StringToWstring(const std::string& strSrc);


private:
    fstream m_fStreamFile;
    std::vector<SECTIONINFO> m_vecLineStr;
    ErrorType m_errorType;
    char m_chFileName[FILE_PATH_LEN];
};

#define CFGFILE_INSTANCE CConfigFile::GetInstance()
#endif