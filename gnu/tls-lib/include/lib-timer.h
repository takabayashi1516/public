//
//
//
#ifndef __LIB_TIMER_H
#define __LIB_TIMER_H


#include <lib-event.h>


#ifdef __cplusplus


/**
*/
class CTimer : public CHandle {
public:
	enum ECycle {
		EOneShot,
		ECyclic
	};

private:
	EBool m_bActive;

public:
	CTimer(CIOEventDispatcher& a_objDispatcher);
	virtual ~CTimer();

	RESULT get(unsigned& a_unUntilUs);
	RESULT set(unsigned a_unTimeoutUs, ECycle a_nCycle = EOneShot);
	RESULT clear();

	virtual RESULT onInput(CIOEventDispatcher& a_objDispatcher);

protected:
	virtual void onTimeout(EBool a_nActive);
};


#endif /* __cplusplus */


#endif /* __LIB_TIMER_H */
