#include "asm_utils.h"
#include "interrupt.h"
#include "memory.h"
#include "program.h"
#include "shell.h"
#include "stdio.h"
#include "sync.h"
#include "syscall.h"
#include "thread.h"
#include "tss.h"
#include "malloc.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;
// 内存管理器
MemoryManager memoryManager;
// 系统调用
SystemService systemService;
// Task State Segment
TSS tss;
//malloc
MallocManager kernelMallocManager;

int syscall_0(int first, int second, int third, int forth, int fifth) {
    printf("systerm call 0: %d, %d, %d, %d, %d\n",
           first, second, third, forth, fifth);
    return first + second + third + forth + fifth;
}

void first_process() {
    printf("start process\n");
    char *q = (char*)malloc(100 * sizeof(char));
    free(q);
}

void first_thread(void *arg) {
    programManager.executeProcess((const char *)first_process, 1);
    asm_halt();
}

extern "C" void setup_kernel() {
    // 中断管理器
    interruptManager.initialize();
    interruptManager.enableTimeInterrupt();
    interruptManager.setTimeInterrupt((void *)asm_time_interrupt_handler);

    // 输出管理器
    stdio.initialize();

    // 进程/线程管理器
    programManager.initialize();

    // 初始化系统调用
    systemService.initialize();
    // 设置0号系统调用
    systemService.setSystemCall(0, (int)syscall_0);
    // 设置1号系统调用
    systemService.setSystemCall(1, (int)syscall_write);
    // 设置2号系统调用
    systemService.setSystemCall(2, (int)syscall_fork);
    // 设置3号系统调用
    systemService.setSystemCall(3, (int)syscall_exit);
    // 设置4号系统调用
    systemService.setSystemCall(4, (int)syscall_wait);
    // 设置5号系统调用
    systemService.setSystemCall(5, (int)syscall_move_cursor);
    // set system call 6
    systemService.setSystemCall(6, (int)syscall_malloc);
    // set system call 7
    systemService.setSystemCall(7, (int)syscall_free);

    // 内存管理器
    memoryManager.initialize();

    // mallocManager
    kernelMallocManager.initialize();

    // 创建第一个线程
    int pid = programManager.executeThread(first_thread, nullptr, "first thread", 1);
    if (pid == -1) {
        printf("can not execute thread\n");
        asm_halt();
    }

    ListItem *item = programManager.readyPrograms.front();
    PCB *firstThread = ListItem2PCB(item, tagInGeneralList);
    firstThread->status = ProgramStatus::RUNNING;
    programManager.readyPrograms.pop_front();
    programManager.running = firstThread;
    asm_switch_thread(0, firstThread);

    asm_halt();
}
