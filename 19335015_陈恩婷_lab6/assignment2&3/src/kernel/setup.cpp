#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;

Semaphore semaphore;
Semaphore empty;
Semaphore full;

Semaphore fork[5];

int cheese_burger;

int buffer[5];
//int ID[10] ;
int in = 0 ; int out = 0 ;
int BUFFER_SIZE = 5 ;
int nextProduced = 0 ;
int nextConsumed = 0 ;

void producer(void *arg) {

    int delay = 0;
    int ID = programManager.running->pid;
    while (1) {
        nextProduced++; //Producing Integers
        /* Check to see if Overwriting unread slot */
        //sem_wait(&empty);
        //sem_wait(&mutex);
        empty.P();
        semaphore.P();

        if (buffer[in] != -1) {
            printf("Synchronization Error: Producer %d Just overwrote %d from Slot %d\n", ID, buffer[in], in);
            //exit(0);
        }

        /* Looks like we are OK */
        buffer[in] = nextProduced;
        printf("Producer %d. Put %d in slot %d\n", ID, nextProduced, in);
        in = (in + 1) % BUFFER_SIZE;
        printf("incremented in!\n");

        //sem_post(&mutex);
        //sem_post(&full);
        semaphore.V();
        full.V();
        
        delay = 0x3ffffff;
    	while (delay)
            --delay;
    	// done
    }
}

void consumer (void *arg) {
    int delay = 0;
    int ID = programManager.running->pid;
    while (1) {
        //sem_wait(&full);
        //sem_wait(&mutex);
        full.P();
        semaphore.P();

        nextConsumed = buffer[out];
        /*Check to make sure we did not read from an empty slot*/
        if (nextConsumed == -1) {
            printf("Synch Error: Consumer %d Just Read from empty slot %d\n", ID, out) ;
            //exit(0) ;
        }
        /* We must be OK */
        printf("Consumer %d Just consumed item %d from slot %d\n", ID, nextConsumed, out) ;
        buffer[out] = -1 ;
        out = (out + 1) % BUFFER_SIZE;

        //sem_post(&mutex);
        //sem_post(&empty);
        semaphore.V();
        empty.V();
        
        delay = 0x3ffffff;
    	while (delay)
            --delay;
    	// done
    }
}

void philosopher(void *arg) {
    
    int delay = 0;
    int i = programManager.running->pid;
    
    while(1) { 
    	
    	if ( i == 1 ) { //Philosopher 1 is right-handed
    	
    	    fork[i].P();
    	    printf("Philosopher %d takes fork %d\n", i, i);
    	    
    	    delay = 0xaffffff;
	    while (delay)
	    	--delay;
	    	
    	    fork[i-1].P();
	}
	else{
	
	    fork[i-1].P();
	
	    //3.2 each philosopher takes one fork, use delay to present deadlock
	    printf("Philosopher %d takes fork %d\n", i, i - 1);
	
	    delay = 0xaffffff;
	    while (delay)
	    	--delay;
	
	    fork[i%5].P();	
	}
	
	//printf("fork[i-1]: %d\n", fork[i-1].counter);
	//printf("fork[i%5]: %d\n", fork[i%5].counter);
	    
	//eat
	printf("Philosopher %d is eating\n", i);
	  
        delay = 0xaffffff;
    	while (delay)
            --delay;
    	// done
	
	printf("Philosopher %d has stopped eating\n", i);	
	
	fork[i-1].V();
	fork[i%5].V();
	
	//printf("fork[i-1]: %d\n", fork[i-1].counter);
	//printf("fork[i%5]: %d\n", fork[i%5].counter);
	
	//think
        delay = 0xaffffff;
    	while (delay)
            --delay;
    	// done	
    }    
}

void a_mother(void *arg)
{
    semaphore.P();
    int delay = 0;

    printf("mother: start to make cheese burger, there are %d cheese burger now\n", cheese_burger);
    // make 10 cheese_burger
    cheese_burger += 10;

    printf("mother: oh, I have to hang clothes out.\n");
    // hanging clothes out
    delay = 0xfffffff;
    while (delay)
        --delay;
    // done

    printf("mother: Oh, Jesus! There are %d cheese burgers\n", cheese_burger);
    semaphore.V();
}

void a_naughty_boy(void *arg)
{
    semaphore.P();
    printf("boy   : Look what I found!\n");
    // eat all cheese_burgers out secretly
    cheese_burger -= 10;
    // run away as fast as possible
    semaphore.V();
}

void first_thread(void *arg)
{
    // 第1个线程不可以返回
    stdio.moveCursor(0);
    for (int i = 0; i < 25 * 80; ++i)
    {
        stdio.print(' ');
    }
    stdio.moveCursor(0);

    //cheese_burger = 0;
    semaphore.initialize(1);
    empty.initialize(10);
    full.initialize(0);
    
    //forks[5] = {0};
    for ( int i = 0; i < 5; i++ ){
 	fork[i].initialize(1);  
 	//printf("fork %d: 1\n", fork[i].counter); 
    }
        
    for(int i = 0; i < 5; i++) {
        buffer[i] = -1 ;
    } //initialization
    
    //programManager.executeThread(producer, nullptr, "second thread", 1);
    //programManager.executeThread(consumer, nullptr, "third thread", 1);
    
    for ( int i = 0; i < 5; i++){
    	programManager.executeThread(philosopher, nullptr, "philosopher", 1);
    }

    asm_halt();
}

extern "C" void setup_kernel()
{

    // 中断管理器
    interruptManager.initialize();
    interruptManager.enableTimeInterrupt();
    interruptManager.setTimeInterrupt((void *)asm_time_interrupt_handler);

    // 输出管理器
    stdio.initialize();

    // 进程/线程管理器
    programManager.initialize();

    // 创建第一个线程
    int pid = programManager.executeThread(first_thread, nullptr, "first thread", 1);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }

    ListItem *item = programManager.readyPrograms.front();
    PCB *firstThread = ListItem2PCB(item, tagInGeneralList);
    firstThread->status = RUNNING;
    programManager.readyPrograms.pop_front();
    programManager.running = firstThread;
    asm_switch_thread(0, firstThread);

    asm_halt();
}
