

#ifndef _libmediasrv_callback_h_
#define _libmediasrv_callback_h_ 


class  CCallbackEvent :
	public CObjEvent
{

public:
	typedef enum tagCallbackType
	{
		CallbackUnknown,
		CallbackWillPublish,
		CallbackPublished,
		CallbackPublishClosed,

		CallbackWillPlay,
		CallbackPlayClosed,

		CallbackRecordFinished,
	}CallbackType;

	_CPP_UTIL_DYNAMIC(CCallbackEvent)
	_CPP_UTIL_CLASSNAME(CCallbackEvent);
	_CPP_UTIL_QUERYOBJ(CObjEvent);


public:
	CCallbackEvent();
	virtual ~CCallbackEvent();
	virtual int OnExecute(CObj *pThreadContext = NULL);

	void SetCtx(CCtx *ctx);
	BOOL Post();




	CallbackType m_type;
	
private:
	CCtx *m_ctx;
	CMediasrvCallback *m_callback;
};


class  CCallbackProcessorCtx :
	public CObj
{


	_CPP_UTIL_DYNAMIC(CCallbackProcessorCtx);
	_CPP_UTIL_CLASSNAME(CCallbackProcessorCtx);
	_CPP_UTIL_QUERYOBJ(CObj);
public:
	CCallbackProcessorCtx();
	virtual ~CCallbackProcessorCtx();

	CMediasrvCallback *m_callback;
};

class  CCallbackProcessor :
	public CObj,
	public CObjEventQueue
{


	_CPP_UTIL_DYNAMIC(CCallbackProcessor);
	_CPP_UTIL_CLASSNAME(CCallbackProcessor);
	_CPP_UTIL_QUERYOBJ(CObj);
public:
	CCallbackProcessor(int nThreads = 1, int nStackSize = 0, int nWaitMilliSecond = -1);
	virtual ~CCallbackProcessor();
public:
	virtual CObj * CreateThreadContext(const CObj *threadObj);
	virtual void OnQueueEmpty(CObj *pThreadContext, const CObj *threadObj);
};



















#endif



