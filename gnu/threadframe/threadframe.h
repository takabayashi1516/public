


#ifndef	__THREADFRAME_H
#define	__THREADFRAME_H


/**
 *
 */
struct tag_threadframe_callbacks {
	/// 
	int	(*notify_initialize)(void *handle);
	/// 
	int	(*notify_terminate)(void *handle);
	/// 
	int	(*notify_tick)(void *handle);
	/// 
	int	(*notify_message)(void *handle, const int req,
			const void *param, const int length);
	/// 
	int	(*notify_judgestatus)(void *handle, const int req,
			const void *param, const int length);
};

/**
 *
 */
struct tag_threadframe_observer {
	/// 
	void	(*proc_start)(void *handle, const int req);
	/// 
	void	(*proc_finished)(void *handle, const int req, const int result);
	/// 
	void	(*proc_canceled)(void *handle, const int req);
};

/**
 *
 */
struct tag_threadframe_action {
	/// 
	void	*handle;
	/// 
	int		request;
	/// 
	int		(*do_action)(void *handle, const void *param, const int length,
			struct tag_threadframe_action *action);
};


#ifdef	__cplusplus
extern "C" {
#endif	// __cplusplus
void	*cre_threadframe(struct tag_threadframe_callbacks *callbacks,
		struct tag_threadframe_observer *observer, struct timeval *timeout);
void	*del_threadframe(void *handle);
void	*cre_threadhandler(struct tag_threadframe_callbacks *callbacks,
		struct tag_threadframe_observer *observer, struct timeval *timeout);
void	*del_threadhandler(void *handle);
int	term_threadframe(void *handle);
int	req_sync_threadframe(void *handle, const int req,
		const void *param, const int length);
int	req_async_threadframe(void *handle, const int req,
		const void *param, const int length);
int	register_action(void *handle, struct tag_threadframe_action *action);
int	deregister_action(void *handle, struct tag_threadframe_action *action);

#ifdef	__cplusplus
}
#endif	// __cplusplus


#ifdef	__cplusplus


class CThreadFrameBase;
class CThreadInterfaceBase;
class CThreadFrame;


/**
 *
 */
class CUtil {
public:
	/// 
	static int	read(const int a_cnFd, void *a_lpParam, size_t a_nCount);
	/// 
	static int	write(const int a_cnFd,
			const void *a_clpParam, size_t a_nCount);
private:
	/// 
	static void	dump(const void *a_clpParam, size_t a_nCount);
};


/**
 *
 */
class CThreadFrameBase {
protected:
	class CReplyMessageBase;

public:
	class CObserverBase;
	class CMessageTick;
	class CRequestMessageTerminete;

public:
	enum {
		ETerminate	= -1,
		ETick		= -2,
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
		CReplyMessageBase	*m_lpobjReplyMessage;
		/// 
		CObserverBase		*m_lpobjObserver;
		/// 
		const int			m_cnLength;
	public:
		/// 
		CRequestMessageBase();
		/// 
		CRequestMessageBase(const int a_cnRequest);
		/// 
		CRequestMessageBase(const int a_cnRequest, const int a_cnLength);
		/// 
		CRequestMessageBase(CRequestMessageBase& a_objMessageBase);
		/// 
		virtual ~CRequestMessageBase();
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
	class CObserverBase {
	public:
		/// 
		virtual void	onStart(const int a_cnRequest) = 0;
		/// 
		virtual void	onFinished(const int a_cnRequest,
				const int a_cnResult) = 0;
		/// 
		virtual void	onCanceled(const int a_cnRequest) = 0;
	};

protected:
	/**
	 *
	 */
	class CReplyMessageBase {
	public:
		/// 
		CReplyMessageBase();
		/// 
		virtual ~CReplyMessageBase();
		/// 
		virtual int	reply(const int a_cnResult) = 0;
		/// 
		virtual int	wait() = 0;
	};

private:
	/// 
	pthread_t				m_objPThread;
	/// 
	int						m_nArrayPipeFd[2];

	/// 
	CThreadInterfaceBase	*m_lpobjThreadInterface;
	/// 
	struct timeval			*m_lpobjRecvTimeout;
	/// 
	struct timeval			m_objRecvTimeout;

public:
	/// 
	void	setThreadInterfaceBase(CThreadInterfaceBase& a_objThreadInterface);
	/// 
	inline int	*getArrayPipeFd() {return(m_nArrayPipeFd);}

protected:
	/// 
	CThreadFrameBase(CThreadInterfaceBase& a_objThreadInterface,
			struct timeval *a_lpobjRecvTimeout);
	/// 
	virtual ~CThreadFrameBase();

private:
	/// 
	static void	*mainRoutine(void *a_lpParam);
	/// 
	void	*main();
	/// 
	CRequestMessageBase	*receiveMessage(struct timeval *a_lpobjTimeout = NULL);

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
	class CReplyMessage : public CReplyMessageBase {
	private:
		/// 
		int	m_nArrayPipeFd[2];
	public:
		/// 
		CReplyMessage();
		/// 
		~CReplyMessage();
		/// 
		int	reply(const int a_cnResult);
		/// 
		int	wait();
	};

	/**
	 *
	 */
	class CObserver : public CObserverBase {
	public:
		/// 
		void	onStart(const int a_cnRequest);
		/// 
		void	onFinished(const int a_cnRequest, const int a_cnResult);
		/// 
		void	onCanceled(const int a_cnRequest);
	};

private:
	/// 
	CThreadFrame(CThreadInterfaceBase& a_objThreadInterface,
			struct timeval *a_lpobjRecvTimeout);
	/// 
	~CThreadFrame();

public:
	/// 
	static CThreadFrame	*newInstance(
			CThreadInterfaceBase& a_objThreadInterface,
			struct timeval *a_lpobjRecvTimeout);
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
	class CLock {
	private:
		/// 
		pthread_mutex_t	m_objMutex;

	public:
		/// 
		CLock();
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

protected:
	/// 
	CThreadFrame::CObserver	*m_lpobjObserver;

protected:
	/// 
	int	sendMessage(CThreadFrameBase::CRequestMessageBase& a_objMessage);

	/// 
	int	sendMessage(const int a_cnRequest,
			const void *a_lpParam, const int a_cnLength,
					CThreadFrame::CReplyMessage *a_lpobjReplyMessage,
							CThreadFrameBase::CObserverBase *a_lpobjObserver);

public:
	/// 
	CThreadInterfaceBase(struct timeval *a_lpobjRecvTimeout);
	/// 
	CThreadInterfaceBase(CThreadInterfaceBase& a_objThread);
	/// 
	virtual ~CThreadInterfaceBase();

	/// 
	int	terminate();
	/// 
	int	requestSync(const int a_cnRequest,
			const void *a_lpParam, const int a_cnLength);
	/// 
	int	requestAsync(const int a_cnRequest,
			const void *a_lpParam, const int a_cnLength);
	/// 
	void	setObserver(CThreadFrame::CObserver *a_lpobjObserver);

	/// 
	virtual int	onInitialize();
	/// 
	virtual int	onTerminate();
	/// 
	virtual int	onTick();
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
class CThreadInterface4C : public CThreadInterfaceBase {
private:
	struct tag_threadframe_callbacks&	m_objCallbacks;
	struct tag_threadframe_observer		*m_lpObserver;

	/**
	 *
	 */
	class CObserver : public CThreadFrame::CObserver {
	private:
		CThreadInterface4C				*m_lpobjThreadInterface;
		struct tag_threadframe_observer	*m_lpObserver;
	public:
		/// 
		CObserver(CThreadInterface4C *a_lpobjThreadInterface,
				struct tag_threadframe_observer *a_lpObserver);
		/// 
		void	onStart(const int a_cnRequest);
		/// 
		void	onFinished(const int a_cnRequest, const int a_cnResult);
		/// 
		void	onCanceled(const int a_cnRequest);
	};

public:
	/// 
	CThreadInterface4C(struct tag_threadframe_callbacks& a_objCallbacks,
			struct tag_threadframe_observer *a_lpObserver,
			struct timeval *a_lpobjRecvTimeout);
	/// 
	~CThreadInterface4C();
	/// 
	int	onInitialize();
	/// 
	int	onTerminate();
	/// 
	int	onTick();
	/// 
	int	onMessage(const int a_cnRequest,
			const void *a_lpParam, const int a_cnLength);
	/// 
	int	onJudgeStatus(const int a_cnRequest,
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
	class CActionBase {
	public:
		/// 
		virtual int doAction(const void *a_lpParam, const int a_cnLength) = 0;
	};

protected:
	typedef CActionBase *tag_LPACTIONBASE;
	/**
	 *
	 */
	class CActionMap {
	private:
		/// 
		const int			m_cnRequest;
		/// 
		int					m_nActionItems;
		/// 
		tag_LPACTIONBASE	*m_lpobjActionArray;
		/// 
		CLock				m_objLock;

	public:
		/// 
		CActionMap(const int a_cnRequest);
		/// 
		virtual ~CActionMap();
		/// 
		int	add(CActionBase *a_lpobjAction);
		/// 
		int	remove(CActionBase *a_lpobjAction);
		/// 
		int	broadCast(const void *a_lpParam, const int a_cnLength);
		/// 
		inline const int	getRequest() const {return(m_cnRequest);}
		/// 
		inline const int	getNumOfItems() const {return(m_nActionItems);}
		/// 
		inline const CActionBase	*getElement(const int a_nIndex) const {
			if (a_nIndex >= getNumOfItems()) {
				return(NULL);
			}
			return(m_lpobjActionArray[a_nIndex]);
		}
	};
	typedef CActionMap *tag_LPACTIONMAP;

public:
	/// 
	CHandlerBase(struct timeval *a_lpobjRecvTimeout,
			CThreadFrame::CObserver *a_lpobjObserver);
	/// 
	~CHandlerBase();
	/// 
	int	registerAction(const int a_cnRequest, CActionBase *a_lpobjAction);
	/// 
	int	deregisterAction(const int a_cnRequest, CActionBase *a_lpobjAction);

protected:
	/// 
	int	onMessage(const int a_cnRequest,
			const void *a_lpParam, const int a_cnLength);
	/// 
	CActionMap	*searchActionMap(const int a_cnRequest);

private:
	/// 
	CActionMap	**m_lpobjActionMap;
	/// 
	int			m_nActionMapItems;
	/// 
//	CLock		m_objLock;
};

/**
 *
 */
class CHandler4C : public CHandlerBase {
private:
	/**
	 *
	 */
	class CObserver : public CThreadFrame::CObserver {
	private:
		/// 
		CHandler4C						*m_lpobjOwner;
		/// 
		struct tag_threadframe_observer	*m_lpObserver;

	public:
		/// 
		CObserver(CHandler4C *a_lpobjOwner,
				struct tag_threadframe_observer *a_lpObserver);
		/// 
		void	onStart(const int a_cnRequest);
		/// 
		void	onFinished(const int a_cnRequest, const int a_cnResult);
		/// 
		void	onCanceled(const int a_cnRequest);
	};

public:
	/**
	 *
	 */
	class CAction : public CHandlerBase::CActionBase {
	private:
		/// 
		CHandler4C						*m_lpobjOwner;
		/// 
		struct tag_threadframe_action	*m_lpobjAction;
	public:
		/// 
		CAction(CHandler4C *a_lpobjOwner,
				struct tag_threadframe_action *a_lpobjAction);
		/// 
		int doAction(const void *a_lpParam, const int a_cnLength);
		/// 
		inline struct tag_threadframe_action	*getCallback() const {
			return(m_lpobjAction);
		}
	};

public:
	/// 
	CHandler4C(struct tag_threadframe_callbacks& a_objCallbacks,
			struct tag_threadframe_observer *a_lpObserver,
					struct timeval *a_lpobjRecvTimeout);

	/// 
	int	onInitialize();
	/// 
	int	onTerminate();
	/// 
	int	onTick();
	/// 
	int	onJudgeStatus(const int a_cnRequest,
			const void *a_lpParam, const int a_cnLength);

	/// 
	int	registerAction4C(struct tag_threadframe_action *a_lpobjAction);
	/// 
	int	deregisterAction4C(struct tag_threadframe_action *a_lpobjAction);

private:
	/// 
	struct tag_threadframe_callbacks&	m_objCallbacks;
};

#endif	// __cplusplus


#endif	// __THREADFRAME_H



