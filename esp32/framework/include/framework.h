/**
 */
#ifndef	__FRAMEWORK_H
#define	__FRAMEWORK_H

#include <stddef.h>
#include <stdint.h> /* READ COMMENT ABOVE. */

#define	IDLE(n)	__asm__ volatile ("waiti " #n)
#define	_RNDUP_REMINDER(n, q)	((((n) + ((q) - 1)) / (q)) * (q))/*(n)*/

#ifdef	__cplusplus

typedef void* HANDLE;

class CThreadFrameBase;
class CThreadInterfaceBase;
class CThreadFrame;

/**
 *
 */
class CUtil {
public:
	///
	static uint32_t	getTickCount();
	///
	static void	dump(const void *a_clpParam, size_t a_nCount);
};

/**
 *
 */
class CSemaphore {
public:
	///
	CSemaphore(bool a_bInitSignal = false);
	///
	virtual ~CSemaphore();
	///
	bool wait(uint32_t a_unTimeout = ~0u);
	///
	void signal();

private:
	HANDLE	m_hResource;
};

/**
 *
 */
class CThreadFrameBase {
public:
	class CResponseMessage;
	class CObserverBase;
	class CMessageTick;
	class CMessageSetTimeout;
	class CRequestMessageTerminete;

public:
	///
	enum {
		ETerminate	= -1,
		ETick		= -2,
		ESetTimeout	= -3,
	};
	///
	enum EXfer {
		EXferRead	= 0,
		EXferWrite,
		EXferSupremum
	};

public:
	/**
	 *
	 */
	class CRequestMessageBase {
	public:
		///
		const int			m_cnRequest;
		///
		CResponseMessage	*m_lpobjResponse;
		///
		CObserverBase		*m_lpobjObserver;
		///
		const int			m_cnLength;
		///
		uint8_t				*m_lpbyContents;
	public:
		///
		CRequestMessageBase(const int a_cnRequest, CResponseMessage *a_lpobjResponse = NULL);
		///
		CRequestMessageBase(const int a_cnRequest, const void *a_clpContents,
				const int a_cnLength, CResponseMessage *a_lpobjResponse = NULL);
		///
		CRequestMessageBase(CRequestMessageBase& a_objMessageBase);
		///
		virtual ~CRequestMessageBase();
		///
		virtual bool isStatic();
	};

	/**
	 *
	 */
	class CRequestMessageTerminete : public CRequestMessageBase {
	public:
		///
		CRequestMessageTerminete();
		///
		~CRequestMessageTerminete();
	};

	/**
	 *
	 */
	class CMessageTick : public CRequestMessageBase {
	public:
		///
		CMessageTick();
	};

	/**
	 *
	 */
	class CRequestMessageSetTimeout : public CRequestMessageBase {
	public:
		///
		CRequestMessageSetTimeout(uint32_t a_unTimeout);
	public:
		uint32_t	m_unTimeout;
	};

	/**
	 *
	 */
	class CObserverBase {
	public:
		///
		virtual void	onPreviousProcess(CThreadInterfaceBase *a_lpobjInterface,
				const int a_cnRequest) = 0;
		///
		virtual void	onCompleted(CThreadInterfaceBase *a_lpobjInterface,
				const int a_cnRequest, const int a_cnResult) = 0;
		///
		virtual void	onCanceled(CThreadInterfaceBase *a_lpobjInterface,
				const int a_cnRequest) = 0;
	};

	/**
	 *
	 */
	class CResponseMessage {
	private:
		///
		HANDLE	m_nXferHandle;
	public:
		///
		CResponseMessage();
		///
		virtual ~CResponseMessage();
		///
		virtual int	reply(void *a_lpParam);
		///
		virtual void	*wait(uint32_t a_unTimeout = ~0u);
		///
		virtual int	replyResult(const int a_cnResult);
		///
		virtual int	waitResult(uint32_t a_unTimeout = ~0u);
		///
		inline HANDLE	getXferHandle(CThreadFrameBase::EXfer/* a_nDirection*/) {return(m_nXferHandle);}
	};

private:
	///
	HANDLE					m_hThread;
	///
	HANDLE					m_nXferHandle;
	///
	CThreadInterfaceBase	*m_lpobjThreadInterface;
	///
	uint32_t				m_unRecvTimeout;
	///
	uint32_t				m_unPrevTickCount;
	///
	uint8_t					*m_byExtra[64];

public:
	///
	void	setThreadInterfaceBase(CThreadInterfaceBase& a_objThreadInterface);
	///
	inline HANDLE	getXferHandle(EXfer/* a_nDirection*/) {return(m_nXferHandle);}

protected:
	///
	CThreadFrameBase(CThreadInterfaceBase& a_objThreadInterface,
			uint32_t a_unRecvTimeout, uint32_t a_unStagesOfStack,
			uint32_t a_unQueues, uint32_t a_unPriority);
	///
	virtual ~CThreadFrameBase();

private:
	///
	inline void	setReceiveTimeout(uint32_t a_unRecvTimeout) {m_unRecvTimeout = a_unRecvTimeout;}
	///
	inline uint32_t	getReceiveTimeout() const {return m_unRecvTimeout;}
	///
	static void	mainRoutine(void *a_lpParam);
	///
	void	*main();
	///
	CRequestMessageBase	*receiveMessage(uint32_t a_unTimeout = ~0u);

public:
	///
	int	join(void*& a_lpParam);
};

/**
 *
 */
class CThreadFrame : public CThreadFrameBase {
public:
	/**
	 *
	 */
	class CObserver : public CObserverBase {
	public:
		///
		void	onPreviousProcess(CThreadInterfaceBase *a_lpobjInterface,
				const int a_cnRequest);
		///
		void	onCompleted(CThreadInterfaceBase *a_lpobjInterface,
				const int a_cnRequest, const int a_cnResult);
		///
		void	onCanceled(CThreadInterfaceBase *a_lpobjInterface,
				const int a_cnRequest);
	};

private:
	///
	CThreadFrame(CThreadInterfaceBase& a_objThreadInterface,
			uint32_t a_unRecvTimeout, uint32_t a_unStagesOfStack,
			uint32_t a_unQueues, uint32_t a_unPriority);
	///
	~CThreadFrame();

public:
	///
	static CThreadFrame	*newInstance(
			CThreadInterfaceBase& a_objThreadInterface,
			uint32_t a_unRecvTimeout, uint32_t a_unStagesOfStack,
			uint32_t a_unQueues, uint32_t a_unPriority);
	///
	static void	deleteInstance(CThreadFrame *a_objThreadFrame);
};

/**
 *
 */
class CThreadInterfaceBase {
public:
	/**
	 *
	 */
	class CLock : private CSemaphore {
	public:
		///
		CLock(bool a_bInitSignalStatus = true);
		///
		virtual ~CLock();
		///
		void	lock();
		///
		void	unlock();
	};

private:
	///
	CThreadFrame	*m_lpobjThreadFrame;
	///
	CLock			m_objLock;
	///
	CSemaphore		*m_pobjBlockStartup;
	CSemaphore		*m_pobjBlockPrepareInstance;

protected:
	///
	CThreadFrame::CObserver	*m_lpobjObserver;

protected:
	///
	int	sendMessage(CThreadFrameBase::CRequestMessageBase *a_lpobjMessage);

	///
	int	sendMessage(const int a_cnRequest,
			const void *a_lpParam, const int a_cnLength,
			CThreadFrame::CResponseMessage *a_lpobjResponse,
			CThreadFrameBase::CObserverBase *a_lpobjObserver);

	void	signalPrepare();

	///
	void	waitStartUp();

	CLock	*getLock() const {
		return const_cast<CLock *>(&m_objLock);
	}

public:
	///
	CThreadInterfaceBase(uint32_t a_unRecvTimeout, uint32_t a_unStagesOfStack,
			uint32_t a_unQueues, uint32_t a_unPriority, bool a_bBlocking);
	///
#if 0
	CThreadInterfaceBase(CThreadInterfaceBase& a_objThread, bool a_bBlocking);
#endif
	///
	virtual ~CThreadInterfaceBase();

	void	waitPrepare();
	///
	void	signalStartUp();

	///
	int	terminate();
	///
	int	setTimeout(uint32_t a_unTimeout);
	///
	int	requestSync(const int a_cnRequest,
			const void *a_lpParam, const int a_cnLength,
			CThreadFrame::CObserver *a_lpobjObserver = NULL);
	///
	int	requestAsync(const int a_cnRequest,
			const void *a_lpParam, const int a_cnLength,
			CThreadFrame::CObserver *a_lpobjObserver = NULL);
	///
	void	setObserver(CThreadFrame::CObserver *a_lpobjObserver);
	///
	CThreadFrame::CObserver	*getObserver() const {
		return m_lpobjObserver;
	}

	///
	virtual int	onInitialize();
	///
	virtual int	onTerminate();
	///
	virtual int	onTimeout();
	///
	virtual int	onMessage(const int a_cnRequest,
			const void *a_lpParam, const int a_cnLength);
	///
	virtual int	onJudgeStatus(const int a_cnRequest,
			const void *a_lpParam, const int a_cnLength);
};

/**
 *
 */
class CHandlerBase : public CThreadInterfaceBase {
public:
	/**
	 *
	 */
	class CExecBase {
	public:
		///
		CExecBase() {}
		///
		virtual ~CExecBase() {}
		///
		virtual int doExec(const void *a_lpParam, const int a_cnLength) = 0;
	};

protected:
	typedef CExecBase *pExecBase_t;
	/**
	 *
	 */
	class CExecMap {
	private:
		///
		const int	m_cnRequest;
		///
		int			m_nExecItems;
		///
		pExecBase_t	*m_lpobjExecArray;

	public:
		///
		CExecMap(const int a_cnRequest);
		///
		virtual ~CExecMap();
		///
		int	add(CExecBase *a_lpobjExec);
		///
		int	remove(CExecBase *a_lpobjExec);
		///
		int	broadCast(const void *a_lpParam, const int a_cnLength);
		///
		inline int	getRequest() const {return(m_cnRequest);}
		///
		inline int	getNumOfItems() const {return(m_nExecItems);}
		///
		inline const CExecBase	*getElement(const int a_nIndex) const {
			if (a_nIndex >= getNumOfItems()) {
				return(NULL);
			}
			return(m_lpobjExecArray[a_nIndex]);
		}
	};
	typedef CExecMap *pExecMap_t;

public:
	///
	CHandlerBase(uint32_t a_unRecvTimeout, uint32_t a_unStagesOfStack,
			uint32_t a_unQueues, uint32_t a_unPriority,
			CThreadFrame::CObserver *a_lpobjObserver, bool a_bBlocking = false);
	///
	virtual ~CHandlerBase();
	///
	int	registerExec(const int a_cnRequest, CExecBase *a_lpobjExec);
	///
	int	deregisterExec(const int a_cnRequest, CExecBase *a_lpobjExec);

protected:
	///
	int	onMessage(const int a_cnRequest,
			const void *a_lpParam, const int a_cnLength);
	///
	CExecMap	*searchExecMap(const int a_cnRequest);

private:
	///
	CExecMap	**m_lpobjExecMap;
	///
	int			m_nExecMapItems;
};

/**
 *
 */
class CInterruptRequest : public CThreadFrameBase::CRequestMessageBase {
public:
	/**
	 *
	 */
	class CRequest {
	public:
		///
		uint32_t	m_unLength;
		///
		uint8_t		*m_pbyData;
	};

public:
	///
	CInterruptRequest(const uint32_t a_cunRequestNo,
			const uint32_t a_cunDataLength);
	///
	virtual ~CInterruptRequest();
	///
	virtual bool isStatic();
	///
	uint8_t *getData() {
		return m_objRequest.m_pbyData;
	}
	///
	uint32_t getMaxLength() {
		return m_cunMaxLength;
	}
	///
	void setLength(const uint32_t a_cunLength) {
		m_objRequest.m_unLength = a_cunLength;
	}
	///
	void setDataAddress(void *a_pParam) {
		m_objRequest.m_pbyData = reinterpret_cast<uint8_t *>(a_pParam);
	}
	///
	void setUseState() {
		m_bUsed = true;
	}
	///
	void setFreeState() {
		m_bUsed = false;
	}
	///
	bool isUsed() const {
		return m_bUsed;
	}
	///
	const CRequest *getRequest() const {
		return &m_objRequest;
	}

private:
	///
	CRequest		m_objRequest;
	///
	const uint32_t	m_cunMaxLength;
	///
	bool			m_bUsed;
};
///
typedef CInterruptRequest *pInterruptRequest_t;

/**
 *
 */
class CInterruptRequestManager {
public:
	///
	CInterruptRequestManager(const uint32_t a_cunRequestNo,
			const uint32_t a_cunDataLength,
			const uint32_t a_cnInterruptRequests);
	///
	virtual ~CInterruptRequestManager();

	///
	CInterruptRequest*& getFree();
	///
	void release(CInterruptRequest::CRequest *a_pobjRequest);

private:
	///
	pInterruptRequest_t	*m_pobjIntReqs;
	///
	const uint32_t		m_cnIntReqs;
};

/**
 *
 */
class CRingBuffer {
public:
	///
	CRingBuffer(const uint32_t a_cunSize);
	///
	virtual ~CRingBuffer();
	///
	bool push(uint8_t *a_pbyData, uint32_t a_unLength);
	///
	bool pull(uint8_t *a_pbyBuffer, uint32_t a_unRequireLength, uint32_t *a_punProvidedLength);
	///
	uint32_t pullWait(uint8_t *a_pbyBuffer, uint32_t a_unLength, uint32_t a_unTimeout);
	///
	bool getData(uint8_t*& a_pbyBuffer, uint32_t& a_unLength);

	///
	uint8_t *getBaseAddress() const {
		return const_cast<uint8_t *>(m_cpbyBaseAddr);
	}
	///
	uint32_t getMaxSize() const {
		return m_cunSize;
	}
	///
	uint8_t *getEndAddress() const {
		return const_cast<uint8_t *>(&(getBaseAddress()[getMaxSize()]));
	}

protected:
	///
	virtual void onRequireLock() = 0;
	///
	virtual void onRequireUnLock() = 0;

private:
	///
	void lock();
	///
	void unLock();
	///
	void notify();
	///
	bool wait(uint32_t a_unTimeout);
	///
	uint32_t getFreeLength();
	///
	uint32_t getValidLength();
	///
	uint8_t *getWriteAddress() const {
		return m_pbyWrAddr;
	}
	///
	uint8_t *getReadAddress() const {
		return m_pbyRdAddr;
	}
	///
	void setWriteAddress(uint8_t *a_pbyUpdate) {
		m_pbyWrAddr = a_pbyUpdate;
	}
	///
	void setReadAddress(uint8_t *a_pbyUpdate) {
		m_pbyRdAddr = a_pbyUpdate;
	}

private:
	///
	CThreadInterfaceBase::CLock	*m_pobjLock;
	///
	uint8_t						*m_pbyWrAddr;
	///
	uint8_t						*m_pbyRdAddr;
	///
	const uint8_t				*m_cpbyBaseAddr;
	///
	const uint32_t				m_cunSize;
	///
	CSemaphore					*m_pobjSemaphore;
};

/**
 *
 */
class CTripleBuffer {
public:
	/**
	 */
	enum {
		ENumOfBuffer = 3
	};
	/**
	 */
	enum EQuadState {
		EQuadStateInvalid = 0,
		EQuadStateValid,
		EQuadStateNextUse,
		EQuadStateInUse,
		EQuadStateSupremum
	};
	/**
	 */
	class CQuadStateBuffer {
	public:
		CQuadStateBuffer(const uint32_t a_cunSize);
		virtual ~CQuadStateBuffer();
		bool update(uint8_t *a_pbyBuffer);
		bool preSelect();
		bool select();
		bool deselect();
		EQuadState getState() const;
		uint8_t *getAddress() const;
	private:
		const uint32_t	m_cunSize;
		EQuadState		m_nState;
		uint8_t			*m_pbyBuffer;
	};

public:
	///
	CTripleBuffer(const uint32_t a_cunSize);
	///
	virtual ~CTripleBuffer();
	///
	void push(uint8_t *a_pbyData);
	/*
	      | 000 | 100 | 200 | 210 | 300 | 310 | 320 | 321
	------|-----|-----|-----|-----|-----|-----|-----|----
	push  | 100 | 100 | 210 | 210 | 310 | 310 | 321 | 321
	update| NG  | 300 | 300 | 320 | 300 | 300 | 300 | 320
	*/
	///
	uint8_t *update();

protected:
	///
	virtual void onRequireLock();
	///
	virtual void onRequireUnLock();
	///
	uint32_t getNumOfBuffer() const;
	///
	CQuadStateBuffer *getQuadStateBuffer(uint32_t a_unIndex);

private:
	///
	void lock();
	///
	void unlock();
	///
	void sort();
	///
	static int sortCompare(const void *a_pParam1, const void *a_pParam2);

private:
	///
	CThreadInterfaceBase::CLock	*m_pobjLock;
	///
	CQuadStateBuffer			*m_pobjQuadStateBuffer[ENumOfBuffer];
};

/**
 *
 */
class CContext {
public:
	static bool inInterrupt();
};

#endif	// __cplusplus


#endif	// __FRAMEWORK_H
