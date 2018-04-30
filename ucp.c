/* ucp.c - Roland Teodorowitsch <roland.teodorowitsch@gmail.com> - 24 abr. 2018 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "ucp.h"

ucp_t *ucpNew(int gridSize)
{
	ucp_t *res;

	res = (ucp_t *)malloc(sizeof(ucp_t));
	if	(res==NULL)
		return res;
	res->pid = -1;
    res->tempo = 0;
	res->symbol = ' ';
	res->numPreemp = 0;
	res->numContSwitch = 0;
	res->grid = (char *)malloc(gridSize+1);
	if	(res->grid==NULL) {
		free((void *)res);
		return NULL;
	}
	res->grid[0] = '\0';
	res->comput = 0;
	return res;
}

void ucpLoad(ucp_t *ucp, int pid,char symbol, int c,int deadline)
{
//printf("UCP/LOAD: %d - %c - %d [ucp->comput=%d]\n",pid,symbol,c,ucp->comput);
	ucp->symbol = symbol;	
        ucp->deadline = deadline;
	if	(ucp->pid==-1) {
		ucp->pid = pid;
		ucp->comput = c;
	}
	else	{
		if	(pid != ucp->pid) {
			ucp->pid = pid;
			ucp->numContSwitch++;
			if	(ucp->comput>0)
				ucp->numPreemp++;
			ucp->comput = c;
		}
		else
			ucp->comput = c;
	}	
}

void ucpRun(ucp_t *ucp)
{
	if	(ucp!=NULL && ucp->pid!=-1) {
        if(ucp->deadline!=-1 && ucp->tempo>=ucp->deadline)
			ucp->grid[ucp->tempo] = tolower(ucp->symbol);
		else
			ucp->grid[ucp->tempo] = ucp->symbol;
        ucp->tempo++;
		ucp->grid[ucp->tempo] = '\0';
		ucp->comput--;
//printf("UCP/RUN:  %s %u %u\n",ucp->grid,ucp->numPreemp,ucp->numContSwitch);
	}
	else {
		ucp->grid[ucp->tempo] = '_';
        ucp->tempo++;
		ucp->grid[ucp->tempo] = '\0';
	}
}

void ucpFree(ucp_t **ucp)
{
	ucp_t *aux;

	aux = *ucp;
	if	(aux != NULL) {
		if	(aux->grid != NULL) {
			free((void *)aux->grid);
			aux->grid = NULL;
		}
		free((void *)aux);
		*ucp = NULL;
	}
}


