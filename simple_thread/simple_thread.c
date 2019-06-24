#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <inttypes.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <asm/ptrace.h>

// this comes from thread.S
extern void *simple_thread(void *arg);

static int thread_enter(void *entry_point, void *stack_addr, size_t stack_size,
			struct user_pt_regs *ctx)
{
	pthread_attr_t attr;
	pthread_t thread;
	void *res = NULL;

	int ret = pthread_attr_init(&attr);
	if (ret) {
		printf("%s pthread_addr_init failed\n", __func__);
	}

	ret = pthread_attr_setstack(&attr, stack_addr, stack_size);
	if (ret) {
		printf("%s pthread_attr_setstack failed\n", __func__);
		goto out;
	}

	ret = pthread_create(&thread, &attr, entry_point, ctx);
	if (ret) {
		printf("%s pthread_create failed\n", __func__);
		goto out;
	}

	ret = pthread_join(thread, &res);
	if (ret) {
		printf("%s pthread_join failed\n", __func__);
	}

out:
	ret = pthread_attr_destroy(&attr);
	if (ret) {
		printf("%s pthread_attr_destroy failed\n", __func__);
	}

	if (res)
		free(res);

	return ret;
}

int main(void)
{
	const size_t page_size = (size_t)sysconf(_SC_PAGESIZE);

	void *entry_point = simple_thread;
	const size_t stack_size = PTHREAD_STACK_MIN;

	void *stack = mmap(NULL, stack_size, PROT_READ | PROT_WRITE,
			   MAP_STACK | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (stack == MAP_FAILED) {
		printf("%s: mmap failed, ret = %d\n", __func__, -errno);
		return -errno;
	}

	const uint64_t test_pattern_a = 0x4242424242424242;
	const uint64_t test_pattern_b = 0x4343434343434343;

	struct user_pt_regs ctx;

	ctx.regs[0] = test_pattern_a;
	ctx.regs[1] = test_pattern_b;

	printf("Before x0 = %#" PRIx64 "\n", (uint64_t)ctx.regs[0]);
	printf("x1 = %#" PRIx64 "\n", (uint64_t)ctx.regs[1]);

	const int ret = thread_enter(entry_point, stack, stack_size, &ctx);
	if (ret) {
		printf("%s: thread_enter failed, ret = %d\n", __func__, ret);
	}

	printf("After x0 = %#" PRIx64 "\n", (uint64_t)ctx.regs[0]);
	printf("got x1 = %#" PRIx64 "\n", (uint64_t)ctx.regs[1]);

	if (ctx.regs[0] - 1 != test_pattern_a) {
		printf("%s test pattern A mismatch\n", __func__);
	}

	if (ctx.regs[1] + 1 != test_pattern_b) {
		printf("%s test pattern B mismatch\n", __func__);
	}

	if (stack)
		munmap(stack, stack_size);
}
