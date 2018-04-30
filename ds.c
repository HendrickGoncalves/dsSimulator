#include "ucp.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAX_PER_TASKS 13
#define MAX_APER_TASKS 13
#define MAX_TIME 100000
#define MAX_PERIOD 100000
#define MAX_ARRIVE 100000

enum TYPE_TASK {
	periodic,
	aperiodic
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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* ----------------------------------------------------------	functions ------------------------------------------	*/


/* ----------------------------------------------------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void resetSimulator(READY_LIST *readyList, char *grid, unsigned int *numPreemp, unsigned int *numCntSw) {
	
	unsigned int i;

	for(i = 0; i < MAX_PER_TASKS+1; i++) {
		readyList->taskPer[i].c = 0;
		readyList->taskPer[i].p = 0;
		readyList->taskPer[i].d = 0;
	}
	
	for(i = 0; i < MAX_APER_TASKS; i++) {
		readyList->taskAper[i].a = 0;
		readyList->taskAper[i].c = 0;
	}

	for(i = 0; i < MAX_TIME+1; i++) {
		grid[i] = '\0';
	}

	*numPreemp = 0;
	*numCntSw = 0;
		
}


void fillReadyList(unsigned int ind, READY_LIST *readyList, int type, TASK_PER taskPer, TASK_APER taskAper) {
	
	switch(type) {
		case periodic:
					readyList->taskPer[ind] = taskPer;
			break;
		case aperiodic:
					readyList->taskAper[ind] = taskAper;
			break;
		default:
			break;

	}

}

TASK_PER getMostPriorityPeriodicTask(READY_LIST *readyList, unsigned int numPerTasks) {
	
	unsigned int i;
	TASK_PER auxTask;

	auxTask.p = MAX_PERIOD;

	//PEGA A TAREFA PERIODICACOM MAIOR PRIORIDADE, OU SEJA, A TAREFA COM O MENOR PERÍODO
	for(i = 1; i < numPerTasks+1; i++) {
		if(auxTask.p > (readyList->taskPer[i].p) && (readyList->taskPer[i].p) > 0 && readyList->taskPer[i].c > 1) {
			auxTask = readyList->taskPer[i];
		}		
	}

	return auxTask;

}

int aperiodicArrival(READY_LIST *readyList, unsigned int numAperTasks, unsigned int time) {

	unsigned int i;

	for(i  = 0; i < numAperTasks; i++) {
		if(readyList->taskAper[i].a == time) { //Vefirifica se existe alguma aperiodica chegando no tempo atual
			return 1;
		}
	}

	return 0;
	
}

int checkDsPriority(READY_LIST *readyList, TASK_PER mostPriorityPeriodicTask, unsigned int time) {

	if(readyList->taskPer[0].p < mostPriorityPeriodicTask.p) {
		return 1;
	}

	return 0;

}

int checkDsComputation(READY_LIST *readyList, unsigned int time) {

	if(readyList->taskPer[0].c > 0) {
		return 1;
	}
	
	return 0;

}

int checkPeriodicComputation(TASK_PER mostPriorityPeriodicTask) {

	if(mostPriorityPeriodicTask.c > 0) {
		return 1;
	}	

	return 0;
}


TASK scheduler(READY_LIST *readyList, unsigned int time, unsigned int numPerTasks, unsigned int numAperTasks) {

	TASK_PER mostPriorityPeriodicTask;
	TASK taskToExecute;

	mostPriorityPeriodicTask = getMostPriorityPeriodicTask(readyList, numPerTasks);
	
	if(aperiodicArrival(readyList, numAperTasks, time)){ //SE ESTÁ CHEGANDO UMA APERIÓDICA NO TEMPO ATUAL
		
		if(checkDsPriority(readyList, mostPriorityPeriodicTask, time)){ //SE O DS É A TAREFA PERIÓDICA MAIS PRIORITÁRIA
		
			if(checkDsComputation(readyList, time)) {	//E SE O DS AINDA TEM COMPUTAÇÕES PARA FAZER

				taskToExecute.taskPer = readyList->taskPer[0];	//DS
				taskToExecute.taskAper = readyList->taskAper[0]; //PRIMEIRA APERIODICA DA FILA  
				taskToExecute.type = aperiodic;
				
			} else if(checkPeriodicComputation(mostPriorityPeriodicTask)) {	//SENÃO, SE A TAREFA PERIODICA MAIS PRIORITARIA AINDA TIVER COMPUTAÇÃO

				taskToExecute.taskPer = mostPriorityPeriodicTask;
				taskToExecute.type = periodic;
			}
		} else if(checkPeriodicComputation(mostPriorityPeriodicTask)) {	//SENÃO, SE A TAREFA PERIODICA MAIS PRIORITARIA AINDA TIVER COMPUTAÇÃO

			taskToExecute.taskPer = mostPriorityPeriodicTask;
			taskToExecute.type = periodic;
		}

	} else if(checkPeriodicComputation(mostPriorityPeriodicTask)) {	//SENÃO, SE A TAREFA PERIODICA MAIS PRIORITARIA AINDA TIVER COMPUTAÇÃO

		taskToExecute.taskPer = mostPriorityPeriodicTask;
		taskToExecute.type = periodic;
	}

	return taskToExecute;

}

int checkPeriodicPer(READY_LIST *readyList, unsigned int ind){

	if(readyList->taskPer[ind].p > 1) {
		return 1;
	}		

	return 0;
}

void updateTasksInformations(unsigned int time, READY_LIST *readyList, TASK_PER *auxPerTask, unsigned int simulationTime, unsigned int numPerTasks) {

	unsigned int i;

	for(i = 0; i < simulationTime; i++){
		if(checkPeriodicPer(readyList, i)) { //SE O PERÍODO AINDA NAO ACABOU, DECREMENTA
			readyList->taskPer[i].p--;	
			
		} else {	//SE ACABOU, COMEÇA NOVO PERÍODO
				readyList->taskPer[i] = auxPerTask[i];
		}
	}
}


void runSimulator(unsigned int simulationTime, READY_LIST *readyList, TASK_PER *auxPerTask, TASK_APER *auxAperTask, unsigned int numPerTasks, unsigned int numAperTasks, char *grid, unsigned int *numPreemp, unsigned int *numCntSw) {

	unsigned int time;

	TASK taskToExecute;
	
	ucp_t *ucp;
	
	ucp = ucpNew(MAX_TIME);

	for(time = 0; time < simulationTime; time++) {	//RELÓGIO CONTANDO
		taskToExecute = scheduler(readyList, time, numPerTasks, numAperTasks);		//PEGA A TAREFA QUE SERÁ EXECUTADA
		
		//ucp = ucpNew(MAX_TIME);

		switch(taskToExecute.type){
			case periodic:
						ucpLoad(ucp, taskToExecute.taskPer.pid, taskToExecute.taskPer.symbol, taskToExecute.taskPer.c, taskToExecute.taskPer.d);
						ucpRun(ucp);

						readyList->taskPer[taskToExecute.taskPer.pid].c = ucp->comput;
						grid[time] = ucp->symbol;
						*numPreemp = ucp->numPreemp;
						*numCntSw = ucp->numContSwitch;
						printf("GRID: %c, numPreemp: %u, numCntSw: %u\n",grid[time], *numPreemp, *numCntSw);						
				break;
			case aperiodic:
						ucpLoad(ucp, taskToExecute.taskAper.pid, taskToExecute.taskAper.symbol, taskToExecute.taskPer.c, taskToExecute.taskPer.d);
						ucpRun(ucp);
						
						readyList->taskAper[taskToExecute.taskAper.pid].c = ucp->comput;
						readyList->taskPer[taskToExecute.taskPer.pid].c--;
						grid[time] = ucp->symbol;
						*numPreemp = ucp->numPreemp;
						*numCntSw = ucp->numContSwitch;	

						printf("GRID: %c, numPreemp: %u, numCntSw: %u\n",grid[time], *numPreemp, *numCntSw);
				break;

			default:
				break;

		}
				
		updateTasksInformations(time, readyList, auxPerTask, simulationTime, numPerTasks);

	}

}

int main() {
		
	unsigned int i;  
	unsigned int sim_time;	
	unsigned int numPerTasks;
	unsigned int numAperTasks;
	int auxInd = 0;
	
	TASK_PER ds;
	TASK_PER perTasks[MAX_PER_TASKS];
	TASK_APER aperTasks[MAX_APER_TASKS];
	
	TASK_PER auxPerTask[MAX_PER_TASKS+1];
	TASK_APER auxAperTask[MAX_APER_TASKS];

	READY_LIST readyList;
  
	char grade[MAX_TIME+1];
	unsigned int numPreemp, numCntSw;
	
 	 while (1) {
 	   
    	/* LEITURA */
   		scanf("%u%u%u",&sim_time,&numPerTasks,&numAperTasks);

    	if (sim_time==0 && numPerTasks==0 && numAperTasks==0) {
       		break;
		}
    	
    	resetSimulator(&readyList, grade, &numPreemp, &numCntSw);

    	scanf("%u%u%u",&ds.c,&ds.p,&ds.d);
    	ds.pid = 0;
		auxPerTask[0] = ds;
		fillReadyList(0, &readyList, periodic, ds, aperTasks[0]);

   		for (i=1;i<numPerTasks+1;i++) {
        	scanf("%u%u%u",&perTasks[i].c,&perTasks[i].p,&perTasks[i].d);
			perTasks[i].symbol = 'A' + (i-1);
			perTasks[i].pid = i;
			auxPerTask[i] = perTasks[i];			
    		fillReadyList(i, &readyList, periodic, perTasks[i], aperTasks[0]);
    		auxInd++;
    	}


    	for (i=0;i<numAperTasks;i++) {
        	scanf("%u%u",&aperTasks[i].a,&aperTasks[i].c);
			aperTasks[i].symbol = 'A' + (i+auxInd);
			aperTasks[i].pid = i;
			auxAperTask[i] = aperTasks[i];
			fillReadyList(i, &readyList, aperiodic, perTasks[0], aperTasks[i]);
    	}
    

    	runSimulator(sim_time, &readyList, auxPerTask, auxAperTask, numPerTasks, numAperTasks, grade, &numPreemp, &numCntSw);
	    
	    //strcpy(grade,"AAAABCBBBBAADAABBB..");

	    /* SAIDA */
	    /*
	    for(i = 0; i < MAX_TIME+1; i++){
	    	printf("%c",grade[i]);
	    }*/
	    printf("\n");
	    printf("%u %u\n",numPreemp,numCntSw);
	    printf("\n");
  		
  }
  return 0;
}

