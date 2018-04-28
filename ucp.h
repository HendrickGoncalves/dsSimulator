typedef struct {
	int          pid;
	unsigned int tempo;	
	char         symbol;
	int          numPreemp;
	int          numContSwitch;
	char         *grid;
	int          comput;
	int          deadline;
}	ucp_t;

ucp_t *ucpNew(int gridSize);
void ucpLoad(ucp_t *ucp, int pid,char symbol, int c,int deadline);
void ucpRun(ucp_t *ucp);
void ucpFree(ucp_t **ucp);


