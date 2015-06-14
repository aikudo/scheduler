#include <stdint.h>
#include "scheduler.h"

#define TASK_SIZE 32
//priorites are broken into three group
//task[0] -> task[7] is highest
//task[8] -> task[19] is medium
//task[20] -> task[31] is low
#define MEDIUM_PRIO 8
#define LOW_PRIO 20

#define SCHEDULER_PERIOD 100        //call at every of number ticks
#define NULL 0


struct Task_tag{
    CallBack handler;
    uint16_t deadline;
    uint16_t period;     //periodicity of this task
    uint16_t cycle;      //number of times we want to run
    uint16_t jobid;
    //profiling:
    uint16_t start_time;
    uint16_t stop_time;
    //simulation

    uint8_t forever;      //run for ever?
    uint8_t active;      //on/off
    priority_t prio;     //probably won't need this
};

struct Scheduler_tag{
    uint16_t frc;    //free running clock that will be updated each tick
    uint16_t deadline;
    uint16_t jobid;
    Task task[TASK_SIZE];
    uint8_t period;
    uint8_t enable;
    uint8_t activejob;
};

void sch_enable(Scheduler_ptr s) {
    if (!s) while(1);
    s->enable = 1;
}


//There is a bug near the number wrap because:
//For example if a wrapping is at 999 and dead line is 920
//and the window of serving is at every 100.
//
// at 800, not service because 800 < 920
// at 900, not service because 900 < 920
// at 0, not service because 0 < 920
//
// So we are screwed here!
// 
// How do I handle this wrapping for many tasks e.g.
// task1 -> 920, task2 -> 950, task4-> 960?
//
// One solution is to tag the roll over and keep that
// status for two window cycle or 200?
//
// Or remove the scheduler period feature

void sch_tick(Scheduler_ptr s){
    s->frc++; //how do i handle wrapped counter?
    s->period--;
    if(!s->period){
        s->period = SCHEDULER_PERIOD;
        s->enable = 1;
    }
}

//try to execute an task
//if it's a valid task then return the next expected deadline

uint16_t exec(Scheduler_ptr s, uint8_t idx){
    if(s->task[idx].handler == NULL
            || !s->task[idx].active
            || s->task[idx].deadline > s->frc
            || (s->task[idx].cycle == 0 && !s->task[idx].forever)
            ) return 0;

    s->task[idx].start_time = s->frc;
    s->task[idx].handler();
    s->task[idx].stop_time = s->frc;
    s->task[idx].deadline = s->task[idx].period + s->task[idx].stop_time;
    s->task[idx].cycle--;
    return s->task[idx].deadline;
}

void cleartask(Task_ptr t){
    *(t) = (const Task) {0};
}

typedef enum {ZERO_CYCLE, NO_ACTIVE, ALL} killflag_t;

void kill_zombies(Scheduler_ptr s, killflag_t flag){
    for( uint8_t idx = 0;  idx < TASK_SIZE; idx++){
        if(s->task[idx].handler){
            switch (flag) {
                case ZERO_CYCLE: 
                    if (s->task[idx].cycle == 0 && !s->task[idx].forever)
                        cleartask(&s->task[idx]);
                    break;
                case NO_ACTIVE:
                    if (!s->task[idx].active)
                        cleartask(&s->task[idx]);
                    break;
                default:
                        cleartask(&s->task[idx]);
            }
            s->activejob--;
        }
    }
}

void sch_reset(Scheduler_ptr s){
    if (!s) while(1);
    for( uint8_t idx = 0; idx < TASK_SIZE; idx++) cleartask(&s->task[idx]);
    s->frc = 0;
    s->deadline = 0;
    s->jobid = 0;
    s->period = 0;
    s->enable = 0;
    s->activejob = 0;
}


void sch_process(Scheduler_ptr s){
    if (!s) while(1);
    if (!s->enable) return;
    if (!s->activejob) return;
    if (s->deadline > s->frc) return; // no job has a deadline yet.

    uint16_t next_deadline = 0xffffu;

    while(s->deadline < s->frc){
        for( uint8_t idx = 0;  idx < TASK_SIZE; idx++){
            uint16_t deadline = exec(s, idx);
            if(deadline){
                next_deadline = (next_deadline < deadline) ?
                    next_deadline : deadline;
            }
        }
        s->deadline = next_deadline;
        kill_zombies(s, ZERO_CYCLE);
    }

    s->enable = 0;
}

void insert(Scheduler_ptr s,
        uint8_t idx,
        CallBack fn,
        uint16_t period,
        uint16_t cycle,
        uint8_t forever,
        uint16_t jobid,
        priority_t prio
        ) {

    s->task[idx].handler = fn;
    s->task[idx].deadline = s->frc + period;
    s->task[idx].period = period;
    s->task[idx].cycle = cycle;
    s->task[idx].jobid = jobid;
    s->task[idx].forever = forever;
    s->task[idx].active = 1;
    s->task[idx].prio = prio;
}

uint16_t sch_add(Scheduler_ptr s,
        CallBack fn,
        uint16_t period,
        uint16_t cycle,
        uint8_t forever,
        priority_t prio) {
    if(!s) while(1);                            //trap bad scheduler
    if(!fn) return 0;                           //empty callback
    if(s->activejob > TASK_SIZE) return 0;      //queue is full

    s->jobid++;
    s->activejob++;
    uint8_t idx;
    
    switch(prio) {
        case HIGH:
            //sweep from 0-> end
            for(idx = 0; idx < TASK_SIZE; idx++){
                if(s->task[idx].handler == NULL){
                    insert(s, idx, fn, period, cycle, forever, s->jobid, prio);
                    break;
                }
            }
            break;

        case MEDIUM:
            //sweep from med -> end; then 0-> med
            for(idx = MEDIUM_PRIO; idx < TASK_SIZE; idx++){
                if(s->task[idx].handler == NULL){
                    insert(s, idx, fn, period, cycle, forever, s->jobid, prio);
                    break;
                }
            }

            //from 0 -> MED
            if(idx == TASK_SIZE){
                for(idx = 0; idx < MEDIUM_PRIO; idx++){
                    if(s->task[idx].handler == NULL){
                        insert(s, idx, fn, period, cycle, forever, s->jobid, prio);
                        break;
                    }
                }
            }
            break;

        case LOW:
            // sweep from low -> end; then 0 -> low
            for(idx = LOW_PRIO; idx < TASK_SIZE; idx++){
                if(s->task[idx].handler == NULL){
                    insert(s, idx, fn, period, cycle, forever, s->jobid, prio);
                    break;
                }
            }

            //from 0 -> LOW
            if(idx == TASK_SIZE){
                for(idx = 0; idx < LOW_PRIO; idx++){
                    if(s->task[idx].handler == NULL){
                        insert(s, idx, fn, period, cycle, forever, s->jobid, prio);
                        break;
                    }
                }
            }
            break;

        default:
            break;
    }
    return s->jobid;
}

Task_ptr find(Scheduler_ptr s, uint16_t jobid){
    if (!s) while(1);
    for( uint8_t idx = 0;  idx < TASK_SIZE; idx++){
        if (s->task[idx].jobid == jobid){
            return ( & (s->task[idx]));
        }
    }
    return  0;
}

uint16_t sch_run(Scheduler_ptr s, uint16_t jobid){
    if (!s) while(1);
    Task_ptr job  = find(s, jobid);
    if (job) {
        job->active = 1;
        return jobid;
    }
    return 0;
}

uint16_t sch_stop(Scheduler_ptr s, uint16_t jobid){
    if (!s) while(1);
    Task_ptr job  = find(s, jobid);
    if (job) {
        job->active = 0;
        return jobid;
    }
    return 0;
}

uint16_t sch_remove(Scheduler_ptr s, uint16_t jobid){
    if (!s) while(1);
    Task_ptr job  = find(s, jobid);
    if (job) {
        job->active = 0;
        job->handler = 0;
        s->activejob--;
        return jobid;
    }
    return 0;
}


int stuff(int stuffhere) {
    return (stuffhere * 32);
}

