#pragma once

#include <Windows.h>
#include <stdlib.h>

#define COEXPORT __declspec(dllexport)

#include "win32co_list.h"
#include "win32co_hook.h"
#include "win32co_sysroutine.h"
#include "win32co_error.h"
#include "win32co_queue.h"

#define PERF_TEST

#define ASIO_FILE		1
#define ASIO_NET		2

#define SIGN_OVERLAPPED	0xff981234

#define STORE_FIBER_INSTANCE(_inst_)			(FlsSetValue(Co_FiberInstanceIndex,(PVOID)(_inst_)))
#define RETRIEVE_FIBER_INSTANCE()				((HANDLE)FlsGetValue(Co_FiberInstanceIndex))
#define GET_FIBER_FROM_INSTANCE(_handle_)		(((PCOROUTINE_FIBER_INSTANCE)(_handle_))->FiberRoutine)
#define GET_PARA_FROM_INSTANCE(_handle_)		(((PCOROUTINE_FIBER_INSTANCE)(_handle_))->Parameter)

namespace Win32Coroutine {

	typedef struct _COROUTINE_INSTANCE COROUTINE_INSTANCE, *PCOROUTINE_INSTANCE;

	//重叠对象扩展结构
	typedef struct _COROUTINE_OVERLAPPED_WARPPER {
		DWORD Signature;
		OVERLAPPED Overlapped;
		DWORD BytesTransfered;
		DWORD ErrorCode;
		DWORD AsioType;
		PVOID Fiber;
		HANDLE Handle;
	}COROUTINE_OVERLAPPED_WARPPER, *PCOROUTINE_OVERLAPPED_WARPPER;

	//延时执行对象
	typedef struct _COROUTINE_EXECUTE_DELAY {
		LIST_ENTRY Entry;
		DWORD64 TimeAtLeast;
		LPVOID Fiber;
	}COROUTINE_EXECUTE_DELAY, *PCOROUTINE_EXECUTE_DELAY;

	//消息队列对象
	//由于Queue是
	typedef struct _COROUTINE_MESSAGE_QUEUE {
		SLIST_HEADER QueueHeader;
		SLIST_HEADER PendingQueueHeader;
		SLIST_HEADER WorkerQueueHeader;
		PVOID Fiber;
	}COROUTINE_MESSAGE_QUEUE, *PCOROUTINE_MESSAGE_QUEUE;

	//消息队列节点
	typedef struct _COROUTINE_MESSAGE_NODE {
		SLIST_ENTRY QueueNode;
		PVOID UserBuffer;
		SIZE_T BufferSize;
	}COROUTINE_MESSAGE_NODE, *PCOROUTINE_MESSAGE_NODE;

	//纤程上下文
	typedef struct _COROUTINE_FIBER_INSTANCE {
		LIST_ENTRY Entry;
		PVOID FiberRoutine;
		COROUTINE_MESSAGE_QUEUE InternalMessageQueue;

		LPFIBER_START_ROUTINE UserStanderRoutine;
		LPTHREAD_START_ROUTINE UserCompatRoutine;
		PVOID UserParameter;
	}COROUTINE_FIBER_INSTANCE, *PCOROUTINE_FIBER_INSTANCE;

	//阻塞中的Fiber
	typedef struct _COROUTINE_PENDING_FIBER {
		SLIST_ENTRY Entry;
		LPVOID PendingFiber;
	}COROUTINE_PENDING_FIBER, *PCOROUTINE_PENDING_FIBER;

	//一个协程实例
	struct _COROUTINE_INSTANCE {
		HANDLE Iocp;
		HANDLE ThreadHandle;

		LPVOID HostThread;

		HANDLE ScheduleRoutine;
		
		PVOID InitialRoutine;
		PVOID InitialParameter;

		LIST_ENTRY FiberInstanceList;
		LIST_ENTRY DelayExecutionList;

		BOOLEAN LastFiberFinished;
		HANDLE LastFiber;
	};

	namespace Coroutine {

		/**
		 * 手动进行协程调度
		 */
		COEXPORT
		VOID
			WINAPI
			CoYield(
				BOOLEAN Terminate
			);

		/**
		 * 在执行纤程前域初始化
		 */
		COEXPORT
		VOID
			WINAPI
			CoPreInitialize(
				LPVOID lpParameter
			);

		/**
		 * 创建一个协程
		 * 事实上调试发现，StackSize无论指定多少，只要栈空间不够，系统会
		 * 自动申请新的栈空间，而不会触发异常，所以这个值建议取值和PageSize
		 */
		COEXPORT
		PCOROUTINE_FIBER_INSTANCE
			WINAPI
			CoCreateFiberInstance(
				SIZE_T StackSize,
				LPFIBER_START_ROUTINE StartRoutine,
				LPVOID Parameter
			);

		/**
		 * 创建一个兼容线程ABI的协程
		 */
		COEXPORT
		HANDLE
			WINAPI
			CoInsertCompatRoutine(
				SIZE_T StackSize,
				LPTHREAD_START_ROUTINE StartRoutine,
				LPVOID Parameter,
				PCOROUTINE_INSTANCE Instance
			);

		/**
		 * 创建一个普通的纤程
		 * 为了保证纤程对象能及时的回收，尽量调用这个接口
		 */
		COEXPORT
		HANDLE
			WINAPI
			CoInsertStandardRoutine(
				SIZE_T StackSize,
				LPFIBER_START_ROUTINE StartRoutine,
				LPVOID Parameter,
				PCOROUTINE_INSTANCE Instance
			);

		/**
		 * 添加一个延时执行事件
		 * @param	Fiber			协程
		 * @param	MillionSecond	延时毫秒数
		 * @note	由于协程不是基于时间片调度，这个函数只能延时最小时间，往往可能会比这个时间要长
		 */
		COEXPORT
		BOOLEAN
			WINAPI
			CoDelayExecutionAtLeast(
				PVOID Fiber,
				DWORD MillionSecond
			);

		/**
		 * 手动启动调度
		 */
		COEXPORT
		BOOLEAN
			WINAPI
			CoStartCoroutineManually(
			);

		/**
		 * 创建一个协程
		 * @param[in]	NewThread		是否创建一个新的线程
		 */
		COEXPORT
		HANDLE
			WINAPI
			CoCreateCoroutine(
				SIZE_T StackSize,
				LPFIBER_START_ROUTINE InitRoutine,
				LPVOID Parameter,
				BOOLEAN NewThread
			);

		/**
		 * 初始化协程库
		 */
		COEXPORT
		VOID
			WINAPI
			CoInitialize(
			);

	}

}