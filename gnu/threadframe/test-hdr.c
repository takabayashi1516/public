


#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<sys/socket.h>
#include	<sys/un.h>

#include	"threadframe.h"


static int	s_nTerminateResult = 0;

static int	notify_initialize(void *handle)
{
	printf("%s: l(%4d): %s(): handle:=0x%08lx.\n",
			__FILE__, __LINE__, __func__, (unsigned long)handle);
	return(0);
}

static int	notify_terminate(void *handle)
{
	printf("%s: l(%4d): %s(): handle:=0x%08lx.\n",
			__FILE__, __LINE__, __func__, (unsigned long)handle);
	return(s_nTerminateResult);
}

static int	notify_tick(void *handle)
{
	printf("%s: l(%4d): %s(): handle:=0x%08lx.\n",
			__FILE__, __LINE__, __func__, (unsigned long)handle);
	return(0);
}

static int notify_judgestatus(void *handle, const int req,
		const void *param, const int length)
{
	printf("%s: l(%4d): %s(): handle:=0x%08lx.\n",
			__FILE__, __LINE__, __func__, (unsigned long)handle);
	return(0);
}

static struct tag_threadframe_callbacks	s_threadframe_callbacks = {
	.notify_initialize	= notify_initialize,
	.notify_terminate	= notify_terminate,
	.notify_tick		= notify_tick,
	.notify_judgestatus	= notify_judgestatus,
};

static void	proc_start(void *handle, const int req)
{
	printf("%s: l(%4d): %s(): handle:=0x%08lx: req:=%d.\n",
			__FILE__, __LINE__, __func__, (unsigned long)handle, req);
}

static void	proc_finished(void *handle, const int req, const int result)
{
	printf("%s: l(%4d): %s(): handle:=0x%08lx: req:=%d: result:=%d.\n",
			__FILE__, __LINE__, __func__, (unsigned long)handle, req, result);
}

static void	proc_canceled(void *handle, const int req)
{
	printf("%s: l(%4d): %s(): handle:=0x%08lx: req:=%d.\n",
			__FILE__, __LINE__, __func__, (unsigned long)handle, req);
}

static struct tag_threadframe_observer	s_threadframe_observer = {
	.proc_start		= proc_start,
	.proc_finished	= proc_finished,
	.proc_canceled	= proc_canceled,
};

static int	do_action(void *handle, const void *param, const int length,
		struct tag_threadframe_action *action)
{
	printf("%s: l(%4d): %s(): handle:=0x%08lx, action:=0x%08lx, action->handle:=0x%08lx, req:=%d.\n",
			__FILE__, __LINE__, __func__, (unsigned long)handle,
					(unsigned long)action, (unsigned long)action->handle,
							action->request);
	return(0);
}

static struct tag_threadframe_action s_debug_action[] = {
	{
		.do_action	= do_action,
		.request	= 100,
	},
	{
		.do_action	= do_action,
		.request	= 100,
	},
	{
		.do_action	= do_action,
		.request	= 200,
	},
	{
		.do_action	= do_action,
		.request	= 300,
	},
	{
		.do_action	= do_action,
		.request	= 10000,
	},
	{
		.do_action	= do_action,
		.request	= 10000,
	},
	{
		.do_action	= do_action,
		.request	= 20000,
	},
	{
		.do_action	= do_action,
		.request	= 30000,
	},
};


int	main(int argc, char *argv[])
{
	int result = 0;
	int req = 0;
	void *handle = NULL;
	char strbuff[1024];

	struct timeval *ptime = NULL;
	struct timeval tim;

	int i;

	if (argc > 1) {
		tim.tv_sec	= atoi(argv[1]);
		tim.tv_usec	= 0;
		ptime = &tim;
	}
	if (argc > 2) {
		tim.tv_usec	= atoi(argv[2]);
	}

	handle = cre_threadhandler(&s_threadframe_callbacks,
			&s_threadframe_observer, ptime);
	printf("%s: l(%4d): %s(): handle:=0x%08lx.\n",
			__FILE__, __LINE__, __func__, (unsigned long)handle);

	for (i = 0; i < sizeof(s_debug_action) / sizeof(s_debug_action[0]); i++) {
		result = register_action(handle, &s_debug_action[i]);
		printf("%s: l(%4d): %s():=%d, request:=%d, handle:=0x%08lx.\n",
				__FILE__, __LINE__, __func__, result, s_debug_action[i].request,
						(unsigned long)s_debug_action[i].handle);
	}

	while (1)	{
		printf("press any decimal digit...\n");
		scanf("%d", &req);
		if (req == -1)	{
			printf("press any decimal digit(terminate result)...\n");
			scanf("%d", &s_nTerminateResult);
			result = term_threadframe(handle);
			printf("%s: l(%4d): %s(): result:=%d.\n",
					__FILE__, __LINE__, __func__, result);
			if (!result)	{
				break;
			}
			continue;
		}

		printf("press any string...\n");
		scanf("%s", strbuff);

		if (req > 1000) {
			result = req_async_threadframe(handle,
					req, strbuff, strlen(strbuff));
			printf("%s: l(%4d): %s(): req:=%d, result:=%d (%s).\n",
					__FILE__, __LINE__, __func__, req, result, strbuff);
		} else {
			result = req_sync_threadframe(handle,
					req, strbuff, strlen(strbuff));
			printf("%s: l(%4d): %s(): req:=%d, result:=%d (%s).\n",
					__FILE__, __LINE__, __func__, req, result, strbuff);
		}
	}

	for (i = 0; i < sizeof(s_debug_action) / sizeof(s_debug_action[0]); i++) {
		result = deregister_action(handle, &s_debug_action[i]);
		printf("%s: l(%4d): %s():=%d, request:=%d, handle:=0x%08lx.\n",
				__FILE__, __LINE__, __func__, result, s_debug_action[i].request,
						(unsigned long)s_debug_action[i].handle);
	}

	handle = del_threadhandler(handle);
	printf("%s: l(%4d): %s(): handle:=0x%08lx.\n",
			__FILE__, __LINE__, __func__, (unsigned long)handle);

	return(result);
}



