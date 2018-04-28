#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ucp.h"


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
} TASK_PER;

typedef struct {
  unsigned int a;
  unsigned int c;
	int pid;
} TASK_APER;

typedef struct {
	TASK_APER taskAper;
	TASK_PER taskPer;
	int type;
} TASK;

typedef struct {

	unsigned int time;
	TASK_APER taskAper[MAX_APER_TASKS];
	
} aperiodicEvent;

typedef struct {

	unsigned int time;
	TASK_PER taskPer[MAX_PER_TASKS+1];
	
} periodicEvent;  ///FIFO

typedef struct {
	TASK_APER taskAper[MAX_APER_TASKS];
	TASK_PER taskPer[MAX_PER_TASKS+1];

} READY_LIST;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* ----------------------------------------------------------	functions ------------------------------------------	*/

void resetEventsList(periodicEvent *perEventsList, aperiodicEvent *aperEventsList);

void fillEventsList(unsigned int time, int type, periodicEvent *perEventsList, aperiodicEvent *aperEventsList,unsigned int simulationTime, TASK_PER task_per, TASK_APER task_aper, int ind);

void runDeferrableServer(periodicEvent *perEventsList, aperiodicEvent *aperEventsList, unsigned int simulationTime, READY_LIST readyList);

TASK scheduler(READY_LIST readyList, unsigned int time, periodicEvent *perEventsList)

/* ----------------------------------------------------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void resetEventsList(periodicEvent *perEventsList, aperiodicEvent *aperEventsList) {
	unsigned int i;
	unsigned int j;

	for(i = 0; i < MAX_TIME; i++) {
		for(j = 0; j < MAX_PER_TASKS+1; j++) {
			perEventsList[i].taskPer[j].c = 0;
			perEventsList[i].taskPer[j].p = 0;
			perEventsList[i].taskPer[j].d = 0;
		}
	}

	for(i = 0; i < MAX_TIME; i++) {
		for(j = 0; j < MAX_APER_TASKS; j++) {
			aperEventsList[i].taskAper[j].a = 0;
			aperEventsList[i].taskAper[j].c = 0;
		}
	}	
}

void fillEventsList(unsigned int time, int type, periodicEvent *perEventsList, aperiodicEvent *aperEventsList,unsigned int simulationTime, TASK_PER task_per, TASK_APER task_aper, int ind) {

	unsigned int i;
	
	switch(type) {
	
		case periodic: 
				for(i = 0; i < simulationTime; i++){
					perEventsList[i].taskPer[ind] = task_per; 
				}
				/*
				for(i = 0; i < simulationTime; i += time){
					perEventsList[i].taskPer[ind] = task_per; 
				}*/

			break;
		case aperiodic: 
				//Time = tempo de chegada
				aperEventsList[time].taskAper[ind] = task_aper; 	
				
			break;
		default:
			break;

	}			

}

void updateReadyList(unsigned int time, periodicEvent *perEventsList, aperiodicEvent *aperEventsList, READY_LIST readyList) {
	
	unsigned int i;

	//PREENCHE LISTA DE PRONTOS COM OS EVENTOS QUE ESTAO CHEGANDO NO TEMPO ATUAL
	for(i = 0; i < MAX_PER_TASKS+1; i++) {	
		readyList.taskPer[i] = perEventsList[time].taskPer[i];
	}

	
	for(i = 0; i < MAX_APER_TASKS; i++) {
		readyList.taskAper[i] = aperEventsList[time].taskAper[i];
	}

}

TASK_PER getMostPriorityPeriodicTask(readyList) {
	
	unsigned int i;
	TASK_PER auxTask;

	auxTask.p = MAX_PERIOD;

	//PEGA A TAREFA PERIODICACOM MAIOR PRIORIDADE, OU SEJA, A TAREFA COM O MENOR PERÍODO
	for(i = 1; i < MAX_PER_TASKS+1; i++) {
		if(auxTask.p > readyList.taskPer[i].p && readyList.taskPer[i].p > 0 ) {
			auxTask = readyList.taskPer[i];
		}		
	}

	return auxTask;

}

int aperiodicArrival(readyList) {

	unsigned int i;
	TASK_APER auxTask;

	for(i  = 0; i < MAX_APER_TASKS; i++) {
		if(readyList.taskAper[i].a > 0) { //Vefirifica se existe alguma aperiodica chegando no tempo atual
			return 1;
		}
	}

	return 0;
	
}

int checkDsPriority(periodicEvent *perEventsList, TASK_PER mostPriorityPeriodicTask, unsigned int time) {

	if(perEventsList[time].taskPer[0].p < mostPriorityPeriodicTask.p) {
		return 1;
	}

	return 0;

}

int checkDsComputation(periodicEvent *perEventsList, unsigned int time) {

	if(perEventsList[time].taskPer[0].c > 0) {
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


TASK scheduler(READY_LIST readyList, unsigned int time, periodicEvent *perEventsList) {

	unsigned int i;
	TASK_PER mostPriorityPeriodicTask;
	TASK_APER mostPriorityAperiodicTask;
	TASK taskToExecute;

	mostPriorityPeriodicTask = getMostPriorityPeriodicTask(readyList);
	
	if(aperiodicArrival(readyList)){ //SE ESTÁ CHEGANDO UMA APERIÓDICA NO TEMPO ATUAL
		
		if(checkDsPriority(perEventsList, mostPriorityPeriodicTask, time)){ //SE O DS É A TAREFA PERIÓDICA MAIS PRIORITÁRIA
		
			if(checkDsComputation(perEventsList, time)) {	//E SE O DS AINDA TEM COMPUTAÇÕES PARA FAZER

				taskToExecute.taskPer = perEventsList[time].taskPer[0];	//DS
				taskToExecute.taskAper = aperEventsList[time].taskAper[0]; //PRIMEIRA APERIODICA DA FILA  
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

int checkPeriodicPer(eventsList *perEventsList, unsigned int time){
	unsigned int i;

	
	if(perEventsList[0].taskPer[time].p > 0) {
		return 1;
	}		

	return 0;
}

void updateTasksInformations(unsigned int time, periodicEvent *perEventsList, READY_LIST readyList, TASK_PER *auxPerTask) {

	unsigned int i;

	for(i = 0; t < MAX_PER_TASKS+1; i++){
		
		if(checkPeriodicPer(perEventsList, i)) { //SE O PERÍODO AINDA NAO ACABOU, DECREMENTA
			perEventsList[0].taskPer[i].p--;	
		
		} else {	//SE ACABOU, COMEÇA NOVO PERÍODO
			perEventsList[0].taskPer[i] = auxPerTask[i];
		}
	}
}


void runSimulator(periodicEvent *perEventsList, aperiodicEvent *aperEventsList, unsigned int simulationTime, READY_LIST readyList, TASK_PER *auxPerTask, TASK_APER *auxAperTask) {

	unsigned int ind = 0;
	unsigned int time;

	periodicEvent per_event;
	aperiodicEvent aper_event;
	TASK taskToExecute;
	
	ucp_t *ucp;

	ucp = ucpNew(MAX_TIME);

	for(time = 0; time < simulationTime; time++) {
		
		updateReadyList(time, perEventsList, aperEventsList, readyList);	
		taskToExecute = scheduler(readyList, time, perEventsList);		//PEGA A TAREFA QUE SERÁ EXECUTADA
		
		//ucp = ucpNew(MAX_TIME);

		switch(taskToExecute.type){
			case periodic:
						ucpLoad(ucp, taskToExecute.taskPer.pid, ('A' + taskToExecute.taskPer.pid), taskToExecute.taskPer.c, taskToExecute.taskPer.d);
						ucpRun(ucp);

						perEventsList[0].taskPer[time].c = ucp.comput;
				break;
			case aperiodic:
						ucpLoad(ucp, taskToExecute.taskAper.pid, ('A' + taskToExecute.taskAper.pid), taskToExecute.taskPer.c, taskToExecute.taskPer.d);
						ucpRun(ucp);

						aperEventsList[time].taskAper[0].c = ucp.comput;
						(perEventsList[time].taskPer[0].c)--;
						
				break;

			default:
				break;

		}
				
		updateTasksInformations(time, perEventsList, readyList, auxPerTask);

	}

}

int main() {
		
	unsigned int i;  
	unsigned int sim_time;	
	unsigned int numPerTasks;
	unsigned int numAperTasks;
	unsigned int indAux = 1;
	
	TASK_PER ds;
	TASK_PER perTasks[MAX_PER_TASKS];
	TASK_APER aperTasks[MAX_APER_TASKS];
	
	TASK_PER auxPerTask[MAX_PER_TASKS];
	TASK_APER auxAperTask[MAX_APER_TASKS];

	READY_LIST readyList;
  
	char grade[MAX_TIME+1];
	unsigned int numPreemp, numCntSw;

	aperiodicEvent aperEventsList[MAX_TIME];
	periodicEvent perEventsList[MAX_TIME];	
	
 	 while (1) {
 	   
    	/* LEITURA */
   		scanf("%u%u%u",&sim_time,&numPerTasks,&numAperTasks);

		//eventsList = (Event *)malloc(sim_time*sizeof(Event));

    	if (sim_time==0 && numPerTasks==0 && numAperTasks==0) {
       		break;
		}
    	
    	resetEventsList(perEventsList, aperEventsList);

    	scanf("%u%u%u",&ds.c,&ds.p,&ds.d);
    	perTasks[0].pid = 0;
		auxPerTask[0] = ds;
    	fillEventsList(ds.p, periodic, perEventsList, aperEventsList, sim_time, ds, aperTasks[0], 0);


   		for (i=1;i<numPerTasks;++i) {
        	scanf("%u%u%u",&perTasks[i].c,&perTasks[i].p,&perTasks[i].d);
			perTasks[i].pid = i;
			auxPerTask[i] = perTasks[i];
			fillEventsList(perTasks[i].p, periodic, perEventsList, aperEventsList, sim_time, perTasks[i], aperTasks[0], i);
    		indAux++;
    	}

    	for (i=0;i<numAperTasks;++i) {
        	scanf("%u%u",&aperTasks[i].a,&aperTasks[i].c);
			aperTasks[i].pid = i;
			auxAperTask[i] = aperTasks[i];
			fillEventsList(aperTasks[i].a, aperiodic, perEventsList, aperEventsList, sim_time, perTasks[0], aperTasks[i], i);
    	}
    

    	runSimulator(perEventsList, aperEventsList, sim_time, readyList,a auxPerTask, auxAperTask);

		//fillEventList();
		
		//scheduler();
	    /* SIMULACAO */
	    /*numPreemp = 0;
	    numCntSw = 0;
	    grade[0] = '\0';*/
	    /* ... */
	    
	    /* provisorio (apenas para gerar uma saida) */
	    /*numPreemp = 4;
	    numCntSw = 9;
	    strcpy(grade,"AAAABCBBBBAADAABBB..");/*

	    /* SAIDA */
	    /*
	    printf("%s\n",grade);
	    printf("%u %u\n",numPreemp,numCntSw);
	    printf("\n");
  		*/
  }
  return 0;
}

