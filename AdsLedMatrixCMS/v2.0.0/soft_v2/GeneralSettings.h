// GeneralSettings.h: interface for the CGeneralSettings class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GENERALSETTINGS_H__58A996C3_BA8F_4AC4_8353_D0BFBBE2DBAD__INCLUDED_)
#define AFX_GENERALSETTINGS_H__58A996C3_BA8F_4AC4_8353_D0BFBBE2DBAD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGeneralSettings : public CObject  
{
public:
	int m_nCommPort;
	int m_nColumn;
	int m_nRow;
	int m_nScale;
	CString m_csFontPath;
	CString m_csFontText;	
	CString m_csFontBkGnd;
	BOOL m_bSaveAfterLoaded;
	CGeneralSettings();
	virtual ~CGeneralSettings();

};

#endif // !defined(AFX_GENERALSETTINGS_H__58A996C3_BA8F_4AC4_8353_D0BFBBE2DBAD__INCLUDED_)
