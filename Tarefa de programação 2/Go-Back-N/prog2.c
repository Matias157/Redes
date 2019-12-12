#include <stdio.h>

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0    /* change to 1 if you're doing extra credit */
                           /* and write a routine called B_output */
#define TIME_OUT 15.0 //valor do timeout
#define TAM_MAX_JANELA 64 //tamanho da janela

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg {
  char data[20];
  };

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt {
   int seqnum;
   int acknum;
   int checksum;
   char payload[20];
    };

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

int num_seq_esperado_env; //numero de sequencia do pacote enviado por A
int num_seq_esperado_rec; //numero de sequencia esperado para ser recebido por B
int num_seq_inic_janela; //numero de sequencia do primeiro pacote da janela
int num_seq_fim_janela; //numero de sequencia do ultimo pacote da janela
int tam_atual_janela; //tamanho atual da janela
struct pkt janela[TAM_MAX_JANELA]; //janela que contem os pacotes
struct pkt pacote_esperado; //pacote que esta sendo enviado de A para B
struct pkt pacote_para_enviar; //pacote auxiliar que sera enviado de B

int calculaChecksum(packet) //faz o calculo do checksum
	struct pkt packet;
{
    int checksum = 0; //soma inicia em zero
    checksum += packet.seqnum; //adcionamos o numero de sequencia do pacote
    checksum += packet.acknum; //adcionamos o numero de ACK do pacote
    for (int i = 0; i < 20; ++i) 
        checksum += packet.payload[i]; //somamos o tamanho de cada um dos caracteres da mensagem
    return checksum;
}

A_output(message) //chamada pela camanda de aplicacao, passa a mensagem que deve ser enviada ao remetente
	struct msg message;
{
	if(num_seq_esperado_env - num_seq_inic_janela >= TAM_MAX_JANELA) //se não tiver mais espaço na janela
    {
        printf("Janela cheia, descartando a mensagem: %s\n", message.data);
        return;
    }
    printf("Pacote adicionada a janela (seq=%d): %s\n", num_seq_esperado_env, message.data);
	memcpy(janela[num_seq_esperado_env % TAM_MAX_JANELA].payload, message.data, sizeof(message.data)); //copia o conteudo da mensagem da camada de aplicacao para o pacote
	janela[num_seq_esperado_env % TAM_MAX_JANELA].seqnum = num_seq_esperado_env; //define o numero de sequencia do pacote
	janela[num_seq_esperado_env % TAM_MAX_JANELA].checksum = 0; //inicializa o checksum do pacote com zero
	janela[num_seq_esperado_env % TAM_MAX_JANELA].checksum = calculaChecksum(janela[num_seq_esperado_env % TAM_MAX_JANELA]); //calcula o checksum do pacote
	++num_seq_esperado_env; //atualiza o contador do numero de sequencia
	while(num_seq_fim_janela < num_seq_esperado_env && num_seq_fim_janela < num_seq_inic_janela + tam_atual_janela) //vai enviando os pacotes em paralelo enquanto for possivel
    {
        printf("Enviando pacote (seq=%d): %s\n", janela[num_seq_fim_janela % TAM_MAX_JANELA].seqnum, janela[num_seq_fim_janela % TAM_MAX_JANELA].payload);
        tolayer3(0, janela[num_seq_fim_janela % TAM_MAX_JANELA]); //envia o pacote para B
        if (num_seq_inic_janela == num_seq_fim_janela) //se a janela de recepcao estiver cheia
            starttimer(0, TIME_OUT);
        ++num_seq_fim_janela; //atualiza o contador da janela de recepca
    }
}

B_output(message)
	struct msg message;
{
	
}

A_input(packet) //chamada pela camada de rede, quando um pacote chega para a camada de transporte
	struct pkt packet;
{
	if (packet.checksum != calculaChecksum(packet)) //se o checksum estiver incorreto
    {
        printf("Pacote corrompido, descartando\n");
        return;
    }
    if (packet.acknum < num_seq_inic_janela) //se o numero de ACK nao for menor que o inicio da jenela de recepcao
    {
        printf("Recebeu NACK (ACK=%d), descartando\n", packet.acknum);
        return;
    }
    printf("Recebeu ACK (ACK=%d)\n", packet.acknum);
    num_seq_inic_janela = packet.acknum + 1; //atualiza o inicio da janela de recepcao
    if (num_seq_inic_janela == num_seq_fim_janela) //se a janela de recepcao estiver cheia
    {
        stoptimer(0); //para o timer
        printf("Timer parado\n");
        while(num_seq_fim_janela < num_seq_esperado_env && num_seq_fim_janela < num_seq_inic_janela + tam_atual_janela) //vai enviando os pacotes em paralelo enquanto for possivel
		{
			printf("Enviando pacote (seq=%d): %s\n", janela[num_seq_fim_janela % TAM_MAX_JANELA].seqnum, janela[num_seq_fim_janela % TAM_MAX_JANELA].payload);
			tolayer3(0, janela[num_seq_fim_janela % TAM_MAX_JANELA]); //envia o pacote para B
			if (num_seq_inic_janela == num_seq_fim_janela) //se a janela de recepcao estiver cheia
				starttimer(0, TIME_OUT);
			++num_seq_fim_janela; //atualiza o contador da janela de recepca
		}
    }
    else
    {
        starttimer(0, TIME_OUT); //timer eh iniciado
        printf("Timer + %f\n", TIME_OUT);
    }
}

A_timerinterrupt() //chamada quando o timer de A acaba
{
	for (int i = num_seq_inic_janela; i < num_seq_fim_janela; ++i) //enquanto existirem pacotes na janela 
    {
        printf("retransmitido pacote (seq=%d): %s\n", janela[i % TAM_MAX_JANELA].seqnum, janela[i % TAM_MAX_JANELA].payload);
        tolayer3(0, janela[i % TAM_MAX_JANELA]); //retransmite o pacote
    }
    starttimer(0, TIME_OUT); //reinicia o timer
    printf("Timer + %f\n", TIME_OUT);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
A_init()
{
	num_seq_inic_janela = 1; //numero de sequencia do inicio janela de recepcao eh 1 
    num_seq_fim_janela = 1; //numero de sequencia do fim janela de recepcao eh 1
    tam_atual_janela = 8; //tamanho da janela de recepcao
    num_seq_esperado_env = 1; //numero de sequencia inicial eh 1
}

/* called from layer 3, when a packet arrives for layer 4 at B*/
B_input(packet)
	struct pkt packet;
{
	if(packet.checksum != calculaChecksum(packet)){ //se o checksum estiver incorreto
		printf("Checksum incorreto, enviando NACK (ACK=%d)\n", pacote_para_enviar.acknum);
		tolayer3(1, pacote_para_enviar); //envia o NACK
		return;
	}
	if(packet.seqnum != num_seq_esperado_rec){ //caso o pacote recebido nao seja o esperado
		printf("Pacote nao esperado, enviando NACK (ACK=%d)\n", pacote_para_enviar.acknum);
		tolayer3(1, pacote_para_enviar); //envia o NACK
		return;
	}

	printf("Pacote recebido (seq=%d): %s\n", packet.seqnum, packet.payload);
	tolayer5(1, packet.payload); //envia a mensagem

	printf("ACK enviado (seq=%d): %s\n", packet.seqnum, packet.payload);
	pacote_para_enviar.acknum = num_seq_esperado_rec; //atualiza o ack do pacote auxiliar
	pacote_para_enviar.checksum = calculaChecksum(pacote_para_enviar); //atualiza o checksum do pacote auxiliar
	tolayer3(1, pacote_para_enviar); //encia o ACK para A
	
	++num_seq_esperado_rec; //atualiza o numero de sequencia do pacote esperado
}

/* called when B's timer goes off */
B_timerinterrupt()
{

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
B_init()
{
	num_seq_esperado_rec = 1; //numero de sequencia inicial esperado eh 1
	pacote_para_enviar.seqnum = -1; //numero de sequencia inicial do pacote auxiliar eh -1
	pacote_para_enviar.acknum = 0; //numero de ack inicial do pacote auxiliar eh 0
	memset(pacote_para_enviar.payload, 0, 20); //mensagem inicial do pacote auxiliar eh 0
	pacote_para_enviar.checksum = calculaChecksum(pacote_para_enviar); //calcula checksum do pacote auxiliar
}

/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
  - emulates the tranmission and delivery (possibly with bit-level corruption
    and packet loss) of packets across the layer 3/4 interface
  - handles the starting/stopping of a timer, and generates timer
    interrupts (resulting in calling students timer handler).
  - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event {
	float evtime;           /* event time */
	int evtype;             /* event type code */
	int eventity;           /* entity where event occurs */
	struct pkt *pktptr;     /* ptr to packet (if any) assoc w/ this event */
	struct event *prev;
	struct event *next;
};
struct event *evlist = NULL;   /* the event list */

/* possible events: */
#define  TIMER_INTERRUPT 0
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2

#define  OFF             0
#define  ON              1
#define   A    0
#define   B    1



int TRACE = 1;             /* for my debugging */
int nsim = 0;              /* number of messages from 5 to 4 so far */
int nsimmax = 0;           /* number of msgs to generate, then stop */
float time = 0.000;
float lossprob;            /* probability that a packet is dropped  */
float corruptprob;         /* probability that one bit is packet is flipped */
float lambda;              /* arrival rate of messages from layer 5 */
int   ntolayer3;           /* number sent into layer 3 */
int   nlost;               /* number lost in media */
int ncorrupt;              /* number corrupted by media*/

main()
{
	struct event *eventptr;
	struct msg  msg2give;
	struct pkt  pkt2give;

	int i, j;
	char c;

	init();
	A_init();
	B_init();

	while (1) {
		eventptr = evlist;            /* get next event to simulate */
		if (eventptr == NULL)
			goto terminate;
		evlist = evlist->next;        /* remove this event from event list */
		if (evlist != NULL)
			evlist->prev = NULL;
		if (TRACE >= 2) {
			printf("\nEVENT time: %f,", eventptr->evtime);
			printf("  type: %d", eventptr->evtype);
			if (eventptr->evtype == 0)
				printf(", timerinterrupt  ");
			else if (eventptr->evtype == 1)
				printf(", fromlayer5 ");
			else
				printf(", fromlayer3 ");
			printf(" entity: %d\n", eventptr->eventity);
		}
		time = eventptr->evtime;        /* update time to next event time */
		if (nsim == nsimmax)
			break;                        /* all done with simulation */
		if (eventptr->evtype == FROM_LAYER5 ) {
			generate_next_arrival();   /* set up future arrival */
			/* fill in msg to give with string of same letter */
			j = nsim % 26;
			for (i = 0; i < 20; i++)
				msg2give.data[i] = 97 + j;
			if (TRACE > 2) {
				printf("          MAINLOOP: data given to student: ");
				for (i = 0; i < 20; i++)
					printf("%c", msg2give.data[i]);
				printf("\n");
			}
			nsim++;
			if (eventptr->eventity == A)
				A_output(msg2give);
			else
				B_output(msg2give);
		}
		else if (eventptr->evtype ==  FROM_LAYER3) {
			pkt2give.seqnum = eventptr->pktptr->seqnum;
			pkt2give.acknum = eventptr->pktptr->acknum;
			pkt2give.checksum = eventptr->pktptr->checksum;
			for (i = 0; i < 20; i++)
				pkt2give.payload[i] = eventptr->pktptr->payload[i];
			if (eventptr->eventity == A)     /* deliver packet by calling */
				A_input(pkt2give);            /* appropriate entity */
			else
				B_input(pkt2give);
			free(eventptr->pktptr);          /* free the memory for packet */
		}
		else if (eventptr->evtype ==  TIMER_INTERRUPT) {
			if (eventptr->eventity == A)
				A_timerinterrupt();
			else
				B_timerinterrupt();
		}
		else  {
			printf("INTERNAL PANIC: unknown event type \n");
		}
		free(eventptr);
	}

terminate:
	printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n", time, nsim);
}



init()                         /* initialize the simulator */
{
	int i;
	float sum, avg;
	float jimsrand();


	printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
	printf("Enter the number of messages to simulate: ");
	scanf("%d", &nsimmax);
	printf("Enter  packet loss probability [enter 0.0 for no loss]:");
	scanf("%f", &lossprob);
	printf("Enter packet corruption probability [0.0 for no corruption]:");
	scanf("%f", &corruptprob);
	printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
	scanf("%f", &lambda);
	printf("Enter TRACE:");
	scanf("%d", &TRACE);

	srand(9999);              /* init random number generator */
	sum = 0.0;                /* test random number generator for students */
	for (i = 0; i < 1000; i++)
		sum = sum + jimsrand(); /* jimsrand() should be uniform in [0,1] */
	avg = sum / 1000.0;
	if (avg < 0.25 || avg > 0.75) {
		printf("It is likely that random number generation on your machine\n" );
		printf("is different from what this emulator expects.  Please take\n");
		printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
		exit(0);
	}

	ntolayer3 = 0;
	nlost = 0;
	ncorrupt = 0;

	time = 0.0;                  /* initialize time to 0.0 */
	generate_next_arrival();     /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand()
{
	double mmm = 2147483647;   /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
	float x;                   /* individual students may need to change mmm */
	x = rand() / mmm;          /* x should be uniform in [0,1] */
	return (x);
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

generate_next_arrival()
{
	double x, log(), ceil();
	struct event *evptr;
	char *malloc();
	float ttime;
	int tempint;

	if (TRACE > 2)
		printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

	x = lambda * jimsrand() * 2; /* x is uniform on [0,2*lambda] */
	/* having mean of lambda        */
	evptr = (struct event *)malloc(sizeof(struct event));
	evptr->evtime =  time + x;
	evptr->evtype =  FROM_LAYER5;
	if (BIDIRECTIONAL && (jimsrand() > 0.5) )
		evptr->eventity = B;
	else
		evptr->eventity = A;
	insertevent(evptr);
}


insertevent(p)
struct event *p;
{
	struct event *q, *qold;

	if (TRACE > 2) {
		printf("            INSERTEVENT: time is %lf\n", time);
		printf("            INSERTEVENT: future time will be %lf\n", p->evtime);
	}
	q = evlist;     /* q points to header of list in which p struct inserted */
	if (q == NULL) { /* list is empty */
		evlist = p;
		p->next = NULL;
		p->prev = NULL;
	}
	else {
		for (qold = q; q != NULL && p->evtime > q->evtime; q = q->next)
			qold = q;
		if (q == NULL) { /* end of list */
			qold->next = p;
			p->prev = qold;
			p->next = NULL;
		}
		else if (q == evlist) { /* front of list */
			p->next = evlist;
			p->prev = NULL;
			p->next->prev = p;
			evlist = p;
		}
		else {     /* middle of list */
			p->next = q;
			p->prev = q->prev;
			q->prev->next = p;
			q->prev = p;
		}
	}
}

printevlist()
{
	struct event *q;
	int i;
	printf("--------------\nEvent List Follows:\n");
	for (q = evlist; q != NULL; q = q->next) {
		printf("Event time: %f, type: %d entity: %d\n", q->evtime, q->evtype, q->eventity);
	}
	printf("--------------\n");
}



/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
stoptimer(AorB)
int AorB;  /* A or B is trying to stop timer */
{
	struct event *q, *qold;

	if (TRACE > 2)
		printf("          STOP TIMER: stopping timer at %f\n", time);
	/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
	for (q = evlist; q != NULL ; q = q->next)
		if ( (q->evtype == TIMER_INTERRUPT  && q->eventity == AorB) ) {
			/* remove this event */
			if (q->next == NULL && q->prev == NULL)
				evlist = NULL;       /* remove first and only event on list */
			else if (q->next == NULL) /* end of list - there is one in front */
				q->prev->next = NULL;
			else if (q == evlist) { /* front of list - there must be event after */
				q->next->prev = NULL;
				evlist = q->next;
			}
			else {     /* middle of list */
				q->next->prev = q->prev;
				q->prev->next =  q->next;
			}
			free(q);
			return;
		}
	printf("Warning: unable to cancel your timer. It wasn't running.\n");
}


starttimer(AorB, increment)
int AorB;  /* A or B is trying to stop timer */
float increment;
{

	struct event *q;
	struct event *evptr;
	char *malloc();

	if (TRACE > 2)
		printf("          START TIMER: starting timer at %f\n", time);
	/* be nice: check to see if timer is already started, if so, then  warn */
	/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
	for (q = evlist; q != NULL ; q = q->next)
		if ( (q->evtype == TIMER_INTERRUPT  && q->eventity == AorB) ) {
			printf("Warning: attempt to start a timer that is already started\n");
			return;
		}

	/* create future event for when timer goes off */
	evptr = (struct event *)malloc(sizeof(struct event));
	evptr->evtime =  time + increment;
	evptr->evtype =  TIMER_INTERRUPT;
	evptr->eventity = AorB;
	insertevent(evptr);
}


/************************** TOLAYER3 ***************/
tolayer3(AorB, packet)
int AorB;  /* A or B is trying to stop timer */
struct pkt packet;
{
	struct pkt *mypktptr;
	struct event *evptr, *q;
	char *malloc();
	float lastime, x, jimsrand();
	int i;


	ntolayer3++;

	/* simulate losses: */
	if (jimsrand() < lossprob)  {
		nlost++;
		if (TRACE > 0)
			printf("          TOLAYER3: packet being lost\n");
		return;
	}

	/* make a copy of the packet student just gave me since he/she may decide */
	/* to do something with the packet after we return back to him/her */
	mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
	mypktptr->seqnum = packet.seqnum;
	mypktptr->acknum = packet.acknum;
	mypktptr->checksum = packet.checksum;
	for (i = 0; i < 20; i++)
		mypktptr->payload[i] = packet.payload[i];
	if (TRACE > 2)  {
		printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
		       mypktptr->acknum,  mypktptr->checksum);
		for (i = 0; i < 20; i++)
			printf("%c", mypktptr->payload[i]);
		printf("\n");
	}

	/* create future event for arrival of packet at the other side */
	evptr = (struct event *)malloc(sizeof(struct event));
	evptr->evtype =  FROM_LAYER3;   /* packet will pop out from layer3 */
	evptr->eventity = (AorB + 1) % 2; /* event occurs at other entity */
	evptr->pktptr = mypktptr;       /* save ptr to my copy of packet */
	/* finally, compute the arrival time of packet at the other end.
	   medium can not reorder, so make sure packet arrives between 1 and 10
	   time units after the latest arrival time of packets
	   currently in the medium on their way to the destination */
	lastime = time;
	/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
	for (q = evlist; q != NULL ; q = q->next)
		if ( (q->evtype == FROM_LAYER3  && q->eventity == evptr->eventity) )
			lastime = q->evtime;
	evptr->evtime =  lastime + 1 + 9 * jimsrand();



	/* simulate corruption: */
	if (jimsrand() < corruptprob)  {
		ncorrupt++;
		if ( (x = jimsrand()) < .75)
			mypktptr->payload[0] = 'Z'; /* corrupt payload */
		else if (x < .875)
			mypktptr->seqnum = 999999;
		else
			mypktptr->acknum = 999999;
		if (TRACE > 0)
			printf("          TOLAYER3: packet being corrupted\n");
	}

	if (TRACE > 2)
		printf("          TOLAYER3: scheduling arrival on other side\n");
	insertevent(evptr);
}

tolayer5(AorB, datasent)
int AorB;
char datasent[20];
{
	int i;
	if (TRACE > 2) {
		printf("          TOLAYER5: data received: ");
		for (i = 0; i < 20; i++)
			printf("%c", datasent[i]);
		printf("\n");
	}

}
