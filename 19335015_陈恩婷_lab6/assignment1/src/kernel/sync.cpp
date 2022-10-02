#include "sync.h"
#include "asm_utils.h"
#include "stdio.h"
#include "os_modules.h"

SpinLock::SpinLock()
{
    initialize();
}

void SpinLock::initialize()
{
    bolt = 0;
}

void SpinLock::lock()
{
    asm_test_and_set(&bolt);
    //uint32 key = 1;

    //do
    //{
      //  asm_atomic_exchange(&key, &bolt);
        //printf("pid: %d\n", programManager.running->pid);
    //;} while (key);
}

void SpinLock::unlock()
{
    asm_exit_critical_section(&bolt);
    //bolt = 0;
}
