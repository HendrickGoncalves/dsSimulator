#include "ucp.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

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


int waiting = 0;

void resetSimulator(READY_LIST *readyList, char *grid, unsigned int *numPreemp, unsigned int *numCntSw, TASK_PER *taskPer, TASK_APER *taskAper, TASK_PER *auxPerTask, TASK_APER *auxAperTask) {
	
	unsigned int i;

	for(i = 0; i < MAX_PER_TASKS; i++) {
		taskPer[i].c = 0;
		auxPerTask[i].c = 0;
		taskPer[i].p = 0;
		auxPerTask[i].p = 0;
		taskPer[i].d = 0;
		auxPerTask[i].d = 0;
	}

	for(i = 0; i < MAX_PER_TASKS+1; i++) {
		readyList->taskPer[i].c = 0;
		readyList->taskPer[i].p = 0;
		readyList->taskPer[i].d = 0;
	}


	for(i = 0; i < MAX_APER_TASKS; i++) {
		taskAper[i].a = 0;
		auxAperTask[i].a = 0;
		taskAper[i].c = 0;
		auxAperTask[i].c = 0;
	}

	
	for(i = 0; i < MAX_APER_TASKS; i++) {
		readyList->taskAper[i].a = 0;
		readyList->taskAper[i].c = 0;
	}

	for(i = 0; i < MAX_TIME+1; i++) {
		grid[i] = '.';
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

TASK_PER getMostPriorityPeriodicTask(READY_LIST *readyList, unsigned int numPerTasks, TASK_PER *auxPerTask) {
	
	unsigned int i;
	TASK_PER auxTask;

	auxTask.p = MAX_PERIOD;

	//PEGA A TAREFA PERIODICACOM MAIOR PRIORIDADE, OU SEJA, A TAREFA COM O MENOR PERÍODO
	for(i = 1; i < numPerTasks+1; i++) {
		
		if(auxTask.p > auxPerTask[i].p && auxPerTask[i].p > 0 && readyList->taskPer[i].c > 0) {
			
			auxTask.p = auxPerTask[i].p;
			auxTask.symbol = auxPerTask[i].symbol;
			auxTask.pid = auxPerTask[i].pid;
			
			auxTask.d = readyList->taskPer[i].d;
			auxTask.c = readyList->taskPer[i].c;
		}		
	}
	
	if(auxTask.p == MAX_PERIOD) {
	//	auxTask.p = 0;
		auxTask.c = 0;
	}
	
	//printf("DEBUG PERIODO DO MAIS PRIORITARIO: %u\n", auxTask.p);
	return auxTask;

}

TASK_APER getMostPriorityAperiodicTask(READY_LIST *readyList, unsigned int numAperTasks) {
	
	unsigned int i;
	
	if(readyList->taskAper[0].c <= 0) {
		for(i = 0; i < numAperTasks; i++) { //UPDATE FIFO
			if(i < numAperTasks-1){		
				readyList->taskAper[i] = readyList->taskAper[i+1];				
			}						
		}
	}

	return readyList->taskAper[0];
}

void updateAFifoAperiodic(READY_LIST *readyList, unsigned int numAperTasks, TASK_APER *auxAperTask) {
	
	unsigned int i;
	
	for(i = 0; i < numAperTasks; i++) { //UPDATE FIFO
	
		if(i == numAperTasks-1) {
			readyList->taskAper[i].a = MAX_TIME;
			readyList->taskAper[i].c = 0;	
		
		} else {
			readyList->taskAper[i] = readyList->taskAper[i+1];			
			auxAperTask[i] = auxAperTask[i+1];	
		}				
	}	
}

int aperiodicArrival(READY_LIST *readyList, unsigned int numAperTasks, unsigned int time) {

	unsigned int i;

	for(i  = 0; i < numAperTasks; i++) {
		if(readyList->taskAper[i].a <= time && readyList->taskAper[i].c > 0) { //Vefirifica se existe alguma aperiodica chegando no tempo atual
			return 1;
		}
	}

	return 0;
	
}

int checkDsPriority(TASK_PER *auxPerTask, TASK_PER mostPriorityPeriodicTask, unsigned int time) {
	
	if(auxPerTask[0].p < mostPriorityPeriodicTask.p) {
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

int chekAperiodicExecution(READY_LIST *readyList, unsigned int numAperTasks, unsigned int time, TASK_APER *auxAperTask){
	
	if(readyList->taskAper[0].c < auxAperTask[0].c) { //SE A COMPUTAÇÃO DA TAREFA É MENOR DO QUE A COMPUTAÇÃO INICIAL, SIGNIFICA QUE ESTÁ EXECUTANDO E ESTÁ QUERENDO "ENTRAR"
		return 1;
	}

	return 0;
}

int checkAperiodicComputation(READY_LIST *readyList) {

	if(readyList->taskAper[0].c > 0) {
		return 1;
	}

	return 0;
}

TASK scheduler(READY_LIST *readyList, unsigned int time, unsigned int numPerTasks, unsigned int numAperTasks, TASK_APER *auxAperTask, TASK_PER *auxPerTask) {

	TASK_PER mostPriorityPeriodicTask;
	TASK taskToExecute;

	mostPriorityPeriodicTask = getMostPriorityPeriodicTask(readyList, numPerTasks, auxPerTask);	

	if((aperiodicArrival(readyList, numAperTasks, time) || chekAperiodicExecution(readyList, numAperTasks, time, auxAperTask)) && checkAperiodicComputation(readyList)){

		if(checkDsPriority(auxPerTask, mostPriorityPeriodicTask, time)){ //SE O DS É A TAREFA PERIÓDICA MAIS PRIORITÁRIA

			if(checkDsComputation(readyList, time)) {	//E SE O DS AINDA TEM COMPUTAÇÕES PARA FAZER
				taskToExecute.taskPer = readyList->taskPer[0];	//DS
				taskToExecute.taskAper = readyList->taskAper[0];
				taskToExecute.type = aperiodic;
				
			} else if(checkPeriodicComputation(mostPriorityPeriodicTask)) {	//SENÃO, SE A TAREFA PERIODICA MAIS PRIORITARIA AINDA TIVER COMPUTAÇÃO
				taskToExecute.taskPer = mostPriorityPeriodicTask;
				taskToExecute.type = periodic;
			
			} else if(checkAperiodicComputation(readyList)){	//SENÃO EXECUTA A APERIODICA MESMO SEM COMPUTAÇÃO
				taskToExecute.taskPer = readyList->taskPer[0];	//DS
				taskToExecute.taskAper = readyList->taskAper[0];	
				taskToExecute.type = aperiodic;
			
			} else {
				taskToExecute.type = idle;	
			}
			
		} else if(checkPeriodicComputation(mostPriorityPeriodicTask)) {	//SENÃO, SE A TAREFA PERIODICA MAIS PRIORITARIA AINDA TIVER COMPUTAÇÃO
			
			taskToExecute.taskPer = mostPriorityPeriodicTask;
			taskToExecute.type = periodic;
		
		} else { 	//IDLE
			
			taskToExecute.type = idle;
			
		}

	} else if(checkPeriodicComputation(mostPriorityPeriodicTask)) {	//SENÃO, SE A TAREFA PERIODICA MAIS PRIORITARIA AINDA TIVER COMPUTAÇÃO
		
		taskToExecute.taskPer = mostPriorityPeriodicTask;		
		taskToExecute.type = periodic;
	
	} else {	//IDLE

		taskToExecute.type = idle;
	} 

	return taskToExecute;

}

int checkPeriodicPer(READY_LIST *readyList, unsigned int ind){

	if(readyList->taskPer[ind].p > 1) {
		return 1;
	}		

	return 0;
}


//FAZER UPDATE A CADA FINAL DE COMPUTAÇÃO E NAO NO FINAL DO PERÍODO

void updateTasksInformations(unsigned int time, READY_LIST *readyList, TASK_PER *auxPerTask, unsigned int simulationTime, unsigned int numPerTasks) {

	unsigned int i;

	for(i = 0; i < numPerTasks+1; i++){
		if(checkPeriodicPer(readyList, i)) { //SE O PERÍODO AINDA NAO ACABOU, DECREMENTA
			readyList->taskPer[i].p--;	
		
		} else {	//SE ACABOU, COMEÇA NOVO PERÍODO
				if(readyList->taskPer[i].c == 0 && i != 0){
					readyList->taskPer[i].c = auxPerTask[i].c;
					readyList->taskPer[i].p = auxPerTask[i].p;				
					readyList->taskPer[i].d += auxPerTask[i].d; 	
				
				} else if( i == 0) {
					readyList->taskPer[i].c = auxPerTask[i].c;
					readyList->taskPer[i].p = auxPerTask[i].p;				
					readyList->taskPer[i].d += auxPerTask[i].d; 	
				}
		}
	}

}


void runSimulator(unsigned int simulationTime, READY_LIST *readyList, TASK_PER *auxPerTask, TASK_APER *auxAperTask, unsigned int numPerTasks, unsigned int numAperTasks, char *grade, unsigned int *numPreemp, unsigned int *numCntSw) {

	unsigned int time;
	int idleFlag = 0;
	int cntLocalSwitch = 0;
	TASK taskToExecute;
	
	ucp_t *ucp;
	
	ucp = ucpNew(MAX_TIME);

	for(time = 0; time < simulationTime; time++) {	//RELÓGIO CONTANDO
		//printf("TIME: %u --- ", time);
		taskToExecute = scheduler(readyList, time, numPerTasks, numAperTasks, auxAperTask, auxPerTask);		//PEGA A TAREFA QUE SERÁ EXECUTADA
		
		switch(taskToExecute.type){
			case periodic:
						ucpLoad(ucp, taskToExecute.taskPer.pid, taskToExecute.taskPer.symbol, taskToExecute.taskPer.c, taskToExecute.taskPer.d);
						ucpRun(ucp);

						readyList->taskPer[taskToExecute.taskPer.pid].c = ucp->comput;
						*numPreemp = ucp->numPreemp;
						*numCntSw = ucp->numContSwitch;

						if (ucp->grid[ucp->tempo-1] == tolower(ucp->symbol)) {
							cntLocalSwitch++;					
						} 	

						//printf("SYMBOL PERIODIC: %c, computation : %u, DEADLINE : %u, numPreemp: %u, numCntSw: %u\n",taskToExecute.taskPer.symbol, readyList->taskPer[taskToExecute.taskPer.pid].c, readyList->taskPer[taskToExecute.taskPer.pid].d ,*numPreemp, *numCntSw);
				break;
			case aperiodic:
						ucpLoad(ucp, taskToExecute.taskAper.pid, taskToExecute.taskAper.symbol, taskToExecute.taskAper.c, simulationTime);
						ucpRun(ucp);

						readyList->taskAper[0].c = ucp->comput;
						
						if(readyList->taskAper[0].c == 0) { //TERMINOU A EXECUÇÃO DE UM APERIÓDICA, ENTÃO BUSCA A PRÓXIMA
							updateAFifoAperiodic(readyList, numAperTasks, auxAperTask);
						}

						if(readyList->taskPer[0].c > 0){
							readyList->taskPer[0].c--;
						
						} else {
							readyList->taskPer[0].c = 0;
						}

						*numPreemp = ucp->numPreemp;
						*numCntSw = ucp->numContSwitch;	

						//printf("SYMBOL APERIODIC: %c, computation : %u, DEADLINE : %u, numPreemp: %u, numCntSw: %u\n",taskToExecute.taskAper.symbol, readyList->taskAper[0].c, readyList->taskPer[0].d ,*numPreemp, *numCntSw);
				break;
			case idle:
						ucpLoad(ucp, (MAX_PER_TASKS+MAX_APER_TASKS+1), '.', simulationTime, simulationTime);
						ucpRun(ucp);
						
						*numPreemp = ucp->numPreemp;
						*numCntSw = ucp->numContSwitch;	
						
						if(time == simulationTime-1)
							idleFlag = 1;

						//printf("SYMBOL IDLE: %c, numPreemp: %u, numCntSw: %u\n",taskToExecute.taskAper.symbol, *numPreemp, *numCntSw);
				break;
				
			default:
				break;

		}
	
		updateTasksInformations(time, readyList, auxPerTask, simulationTime, numPerTasks);
		
	}

	*numCntSw = *numCntSw + cntLocalSwitch + idleFlag;
	*numPreemp += idleFlag;

	strcpy(grade, ucp->grid);

	ucpFree(&ucp);	

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
    	
    	resetSimulator(&readyList, grade, &numPreemp, &numCntSw, perTasks, aperTasks ,auxPerTask, auxAperTask);

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
			aperTasks[i].pid = (i+auxInd)+1;
			auxAperTask[i] = aperTasks[i];
			fillReadyList(i, &readyList, aperiodic, perTasks[0], aperTasks[i]);
    	}
    

    	runSimulator(sim_time, &readyList, auxPerTask, auxAperTask, numPerTasks, numAperTasks, grade, &numPreemp, &numCntSw);
    	auxInd = 0;

	    /* SAIDA */
	    
	    printf("%s\n", grade);
	    
	    printf("\n");
	    printf("%u %u\n",numPreemp,numCntSw);
	    printf("\n");
  		
  }
  return 0;
}

