#include "syscall.h"
#include "interrupt.h"
#include "stdlib.h"
#include "asm_utils.h"
#include "os_modules.h"
#include "stdio.h"
#include "malloc.h"
#include "os_type.h"
#include "program.h"

int system_call_table[MAX_SYSTEM_CALL];

SystemService::SystemService() {
    initialize();
}

void SystemService::initialize()
{
    memset((char *)system_call_table, 0, sizeof(int) * MAX_SYSTEM_CALL);
    // 代码段选择子默认是DPL=0的平坦模式代码段选择子，DPL=3，否则用户态程序无法使用该中断描述符
    interruptManager.setInterruptDescriptor(0x80, (uint32)asm_system_call_handler, 3);
}

bool SystemService::setSystemCall(int index, int function)
{
    system_call_table[index] = function;
    return true;
}

int write(const char *str) {
    return asm_system_call(1, (int)str);
}

int syscall_write(const char *str) {
    return stdio.print(str);
}

int fork() {
    return asm_system_call(2);
}

int syscall_fork() {
    return programManager.fork();
}

void exit(int ret) {
    asm_system_call(3, ret);
}

void syscall_exit(int ret) {
    programManager.exit(ret);
}

int wait(int *retval) {
    return asm_system_call(4, (int)retval);
}

int syscall_wait(int *retval) {
    return programManager.wait(retval);
}

void move_cursor(int i, int j) {
    asm_system_call(5, i, j);
}

void syscall_move_cursor(int i, int j) {
    stdio.moveCursor(i, j);
}

void * malloc(size_t num_bytes){
    return (void *)asm_system_call(6, (int)num_bytes);
}

void * syscall_malloc(size_t num_bytes){
    PCB *pcb = programManager.running;
    if (pcb->pageDirectoryAddress)
    {
        return pcb->mallocManager.malloc(num_bytes);
    }
    return kernelMallocManager.malloc(num_bytes);
}

void free( void * ptr ){
    asm_system_call(7, (int)ptr);
}

void syscall_free(void * ptr){
    PCB *pcb = programManager.running;
    if (pcb->pageDirectoryAddress)
    {
        return pcb->mallocManager.free(ptr);
    }
    return kernelMallocManager.free(ptr);
}

