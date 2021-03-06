// CalcDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ADProLED.h"
#include "CalcDlg.h"


// CCalcDlg dialog

IMPLEMENT_DYNAMIC(CCalcDlg, CDialog)

CCalcDlg::CCalcDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCalcDlg::IDD, pParent)
	, m_csTitleNode(_T("Tổng số LED:"))
	, m_csTitleRED3(_T("Số LED đỏ-3mm:"))
	, m_csTitleRED5(_T("Số LED đỏ-5mm:"))
	, m_csTitleBLUE3(_T("Số LED lam-3mm:"))
	, m_csTitleBLUE5(_T("Số LED lam-5mm:"))
	, m_csTitleGREEN3(_T("Số LED lục-3mm:"))
	, m_csTitleGREEN5(_T("Số LED lục-5mm:"))
	, m_csTitleWHITE3(_T("Số LED bạc-3mm:"))
	, m_csTitleOTHER(_T("Các loại LED khác:"))
	, m_csTitleCPU(_T("CPU điều khiển:"))
	, m_csTitlePOWER(_T("Công suất nguồn:"))
	, m_csTitleCASE(_T("Tiền khung vỏ:"))
	, m_csTitleEXTRA(_T("Các phụ phí:"))
	, m_csTitleTotal(_T("Tổng giá thành:"))	
{
	m_nTotalNode=0;
	m_nTotalRED3=0;
	m_nTotalRED5=0;
	m_nTotalBLUE3=0;
	m_nTotalBLUE5=0;
	m_nTotalGREEN3=0;
	m_nTotalGREEN5=0;
	m_nTotalWHITE3=0;
	m_nTotalOTHER=0;
	m_nTotalCPU=1;
	m_nTotalPOWER=0;
	TCHAR szItem[20];
	_itow_s((int)_gCompPrice[CASE],szItem,20,10);
	m_csTotalCASE=szItem;
	_itow_s((int)_gCompPrice[EXTRA],szItem,20,10);
	m_csTotalEXTRA=szItem;
	m_csTotalPay=(_T("0")); 
}

CCalcDlg::~CCalcDlg()
{
}

void CCalcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TITLE_NODE, m_csTitleNode);
	DDX_Text(pDX, IDC_EDIT_TITLE_RED3, m_csTitleRED3);
	DDX_Text(pDX, IDC_EDIT_TITLE_RED5, m_csTitleRED5);
	DDX_Text(pDX, IDC_EDIT_TITLE_BLUE3, m_csTitleBLUE3);
	DDX_Text(pDX, IDC_EDIT_TITLE_BLUE5, m_csTitleBLUE5);
	DDX_Text(pDX, IDC_EDIT_TITLE_GREEN3, m_csTitleGREEN3);
	DDX_Text(pDX, IDC_EDIT_TITLE_GREEN5, m_csTitleGREEN5);
	DDX_Text(pDX, IDC_EDIT_TITLE_WHITE3, m_csTitleWHITE3);
	DDX_Text(pDX, IDC_EDIT_TITLE_OTHER, m_csTitleOTHER);
	DDX_Text(pDX, IDC_EDIT_TITLE_CPU, m_csTitleCPU);
	DDX_Text(pDX, IDC_EDIT_TITLE_POWER, m_csTitlePOWER);
	DDX_Text(pDX, IDC_EDIT_TITLE_CASE, m_csTitleCASE);
	DDX_Text(pDX, IDC_EDIT_TITLE_EXTRA, m_csTitleEXTRA);

	DDX_Text(pDX, IDC_EDIT_TOTAL_NODE, m_nTotalNode);
	DDX_Text(pDX, IDC_EDIT_TOTAL_RED3, m_nTotalRED3);
	DDX_Text(pDX, IDC_EDIT_TOTAL_RED5, m_nTotalRED5);
	DDX_Text(pDX, IDC_EDIT_TOTAL_BLUE3, m_nTotalBLUE3);
	DDX_Text(pDX, IDC_EDIT_TOTAL_BLUE5, m_nTotalBLUE5);
	DDX_Text(pDX, IDC_EDIT_TOTAL_GREEN3, m_nTotalGREEN3);
	DDX_Text(pDX, IDC_EDIT_TOTAL_GREEN5, m_nTotalGREEN5);
	DDX_Text(pDX, IDC_EDIT_TOTAL_WHITE3, m_nTotalWHITE3);
	DDX_Text(pDX, IDC_EDIT_TOTAL_OTHER, m_nTotalOTHER);
	DDX_Text(pDX, IDC_EDIT_TOTAL_CPU, m_nTotalCPU);
	DDX_Text(pDX, IDC_EDIT_TOTAL_POWER, m_nTotalPOWER);
	DDX_Text(pDX, IDC_EDIT_TOTAL_CASE, m_csTotalCASE);
	DDX_Text(pDX, IDC_EDIT_TOTAL_EXTRA, m_csTotalEXTRA);
	DDX_Text(pDX, IDC_EDIT_TITLE_TOTAL, m_csTitleTotal);
	DDX_Text(pDX, IDC_EDIT_TOTAL_PAY, m_csTotalPay);
}


BEGIN_MESSAGE_MAP(CCalcDlg, CDialog)
	ON_EN_CHANGE(IDC_EDIT_TOTAL_CASE, &CCalcDlg::OnEnChangeEditTotalCase)
	ON_EN_CHANGE(IDC_EDIT_TOTAL_EXTRA, &CCalcDlg::OnEnChangeEditTotalExtra)
	ON_EN_CHANGE(IDC_EDIT_TOTAL_POWER, &CCalcDlg::OnEnChangeEditTotalPower)
	ON_EN_CHANGE(IDC_EDIT_TOTAL_CPU, &CCalcDlg::OnEnChangeEditTotalCpu)
	ON_EN_CHANGE(IDC_EDIT_TOTAL_OTHER, &CCalcDlg::OnEnChangeEditTotalOther)
	ON_EN_CHANGE(IDC_EDIT_TOTAL_WHITE3, &CCalcDlg::OnEnChangeEditTotalWhite3)
	ON_EN_CHANGE(IDC_EDIT_TOTAL_GREEN5, &CCalcDlg::OnEnChangeEditTotalGreen5)
	ON_EN_CHANGE(IDC_EDIT_TOTAL_GREEN3, &CCalcDlg::OnEnChangeEditTotalGreen3)
	ON_EN_CHANGE(IDC_EDIT_TOTAL_BLUE5, &CCalcDlg::OnEnChangeEditTotalBlue5)
	ON_EN_CHANGE(IDC_EDIT_TOTAL_BLUE3, &CCalcDlg::OnEnChangeEditTotalBlue3)
	ON_EN_CHANGE(IDC_EDIT_TOTAL_RED5, &CCalcDlg::OnEnChangeEditTotalRed5)
	ON_EN_CHANGE(IDC_EDIT_TOTAL_RED3, &CCalcDlg::OnEnChangeEditTotalRed3)
	ON_EN_CHANGE(IDC_EDIT_TOTAL_NODE, &CCalcDlg::OnEnChangeEditTotalNode)
END_MESSAGE_MAP()


// CCalcDlg message handlers

BOOL CCalcDlg::OnInitDialog()
{
	this->Calc();
	CDialog::OnInitDialog();		
	return TRUE;
}

BOOL CCalcDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::DestroyWindow();
}

void CCalcDlg::Calc(void)
{
	double total = 0;
	
	m_nTotalNode = m_nTotalRED3+m_nTotalRED5;
	m_nTotalNode += m_nTotalBLUE3+m_nTotalBLUE5;
	m_nTotalNode += m_nTotalGREEN3+m_nTotalGREEN5;
	m_nTotalNode += m_nTotalWHITE3+m_nTotalOTHER;
	m_nTotalPOWER = m_nTotalNode/10;	// (20mA x 5V) = 0.1W per LED

	total += m_nTotalRED3*_gCompPrice[RED_3];
	total += m_nTotalRED5*_gCompPrice[RED_5];
	total += m_nTotalBLUE3*_gCompPrice[BLUE_3];
	total += m_nTotalBLUE5*_gCompPrice[BLUE_5];
	total += m_nTotalGREEN3*_gCompPrice[GREEN_3];
	total += m_nTotalGREEN5*_gCompPrice[GREEN_5];
	total += m_nTotalWHITE3*_gCompPrice[WHITE_3];
	total += m_nTotalOTHER*_gCompPrice[OTHER];
	total += m_nTotalCPU*_gCompPrice[CPU];
	total += m_nTotalPOWER*_gCompPrice[POWER];
	total += _wtof(m_csTotalCASE.GetBuffer());
	total += _wtof(m_csTotalEXTRA.GetBuffer());
	m_csTotalPay.Format(_T("%#10.0f đồng"),total);
	
}

void CCalcDlg::OnEnChangeEditTotalCase()
{
	this->UpdateData(TRUE);
	this->Calc();
	this->UpdateData(FALSE);
}

void CCalcDlg::OnEnChangeEditTotalExtra()
{
	this->UpdateData(TRUE);
	this->Calc();
	this->UpdateData(FALSE);
}

void CCalcDlg::OnEnChangeEditTotalPower()
{
	this->UpdateData(TRUE);
	this->Calc();
	this->UpdateData(FALSE);
}

void CCalcDlg::OnEnChangeEditTotalCpu()
{
	this->UpdateData(TRUE);
	this->Calc();
	this->UpdateData(FALSE);
}

void CCalcDlg::OnEnChangeEditTotalOther()
{
	this->UpdateData(TRUE);
	this->Calc();
	this->UpdateData(FALSE);
}

void CCalcDlg::OnEnChangeEditTotalWhite3()
{
	this->UpdateData(TRUE);
	this->Calc();
	this->UpdateData(FALSE);
}

void CCalcDlg::OnEnChangeEditTotalGreen5()
{
	this->UpdateData(TRUE);
	this->Calc();
	this->UpdateData(FALSE);
}

void CCalcDlg::OnEnChangeEditTotalGreen3()
{
	this->UpdateData(TRUE);
	this->Calc();
	this->UpdateData(FALSE);
}

void CCalcDlg::OnEnChangeEditTotalBlue5()
{
	this->UpdateData(TRUE);
	this->Calc();
	this->UpdateData(FALSE);
}

void CCalcDlg::OnEnChangeEditTotalBlue3()
{
	this->UpdateData(TRUE);
	this->Calc();
	this->UpdateData(FALSE);
}

void CCalcDlg::OnEnChangeEditTotalRed5()
{
	this->UpdateData(TRUE);
	this->Calc();
	this->UpdateData(FALSE);
}

void CCalcDlg::OnEnChangeEditTotalRed3()
{
	this->UpdateData(TRUE);
	this->Calc();
	this->UpdateData(FALSE);
}

void CCalcDlg::OnEnChangeEditTotalNode()
{
	this->UpdateData(TRUE);
	this->Calc();
	this->UpdateData(FALSE);
}
