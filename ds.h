
#define MAX_PER_TASKS 13
#define MAX_APER_TASKS 13
#define MAX_TIME 100000
#define MAX_PERIOD 100000
#define MAX_ARRIVE 100000

enum TYPE_TASK {
	periodic,
	aperiodic,
	idle
};

typedef struct {
  unsigned int c;
  unsigned int p;
  unsigned int d;
  int pid;
  char symbol;
} TASK_PER;

typedef struct {
  unsigned int a;
  unsigned int c;
  int pid;
  char symbol;
} TASK_APER;

typedef struct {
	TASK_APER taskAper;
	TASK_PER taskPer;
	int type;
} TASK;

typedef struct {
	TASK_APER taskAper[MAX_APER_TASKS];
	TASK_PER taskPer[MAX_PER_TASKS+1];

} READY_LIST;


void resetSimulator(READY_LIST *readyList, char *grid, unsigned int *numPreemp, unsigned int *numCntSw, TASK_PER *taskPer, TASK_APER *taskAper, TASK_PER *auxPerTask, TASK_APER *auxAperTask);

void fillReadyList(unsigned int ind, READY_LIST *readyList, int type, TASK_PER taskPer, TASK_APER taskAper);

TASK_PER getMostPriorityPeriodicTask(READY_LIST *readyList, unsigned int numPerTasks, TASK_PER *auxPerTask);

TASK_APER getMostPriorityAperiodicTask(READY_LIST *readyList, unsigned int numAperTasks);

void updateAFifoAperiodic(READY_LIST *readyList, unsigned int numAperTasks, TASK_APER *auxAperTask);

int aperiodicArrival(READY_LIST *readyList, unsigned int numAperTasks, unsigned int time);

int checkDsPriority(TASK_PER *auxPerTask, TASK_PER mostPriorityPeriodicTask, unsigned int time);

int checkDsComputation(READY_LIST *readyList, unsigned int time);

int checkPeriodicComputation(TASK_PER mostPriorityPeriodicTask);

int chekAperiodicExecution(READY_LIST *readyList, unsigned int numAperTasks, unsigned int time, TASK_APER *auxAperTask);

int checkAperiodicComputation(READY_LIST *readyList);

TASK scheduler(READY_LIST *readyList, unsigned int time, unsigned int numPerTasks, unsigned int numAperTasks, TASK_APER *auxAperTask, TASK_PER *auxPerTask);

int checkPeriodicPer(READY_LIST *readyList, unsigned int ind);

void updateTasksInformations(unsigned int time, READY_LIST *readyList, TASK_PER *auxPerTask, unsigned int simulationTime, unsigned int numPerTasks);

void runSimulator(unsigned int simulationTime, READY_LIST *readyList, TASK_PER *auxPerTask, TASK_APER *auxAperTask, unsigned int numPerTasks, unsigned int numAperTasks, char *grade, unsigned int *numPreemp, unsigned int *numCntSw);



