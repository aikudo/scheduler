#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

int stuff(int stuffhere);

typedef void (*CallBack) (void);
typedef enum {HIGH, MEDIUM, LOW} priority_t;
typedef struct Task_tag Task, *Task_ptr;
typedef struct Scheduler_tag Scheduler, *Scheduler_ptr;

//public functions
void sch_enable(Scheduler_ptr s);
void sch_tick(Scheduler_ptr s);
void sch_reset(Scheduler_ptr s);
void sch_process(Scheduler_ptr s);

//returns 0 if fails otherwise jobid
uint16_t sch_add(Scheduler_ptr s,
        CallBack fn,
        uint16_t period,
        uint16_t cycle,
        uint8_t forever,
        priority_t prio) ;

//returns 0 if fails otherwise jobid
uint16_t sch_remove(Scheduler_ptr s, uint16_t jobid);

//To pause/resume a job,
//returns 0 if fails otherwise jobid
uint16_t sch_run(Scheduler_ptr s, uint16_t jobid);
uint16_t sch_stop(Scheduler_ptr s, uint16_t jobid);


#endif //__SCHEDULER_H__
