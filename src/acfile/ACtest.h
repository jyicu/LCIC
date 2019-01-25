// ACtest.h: interface for the CACtest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACTEST_H__C072D22E_FCA8_4176_B86F_16A494C7A334__INCLUDED_)
#define AFX_ACTEST_H__C072D22E_FCA8_4176_B86F_16A494C7A334__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CACtest  
{
public:
	CACtest();
	virtual ~CACtest();

	void set_prob_0(double x);
	void runtest();

private:
	double m_dprob_0;

};

#endif // !defined(AFX_ACTEST_H__C072D22E_FCA8_4176_B86F_16A494C7A334__INCLUDED_)
