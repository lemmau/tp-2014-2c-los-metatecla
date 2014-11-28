/*
 * planificador.c
 *
 *  Created on: 06/10/2014
 *      Author: utnso
 */

#include "planificador.h"


void* main_PLANIFICADOR(arg_PLANIFICADOR* parametros)
{
	inicializar_ready_block();
	inicializar_semaforos_colas();

	pthread_t thr_consumidor_new, thr_parca, thr_atencion_CPUs;
	pthread_create(&thr_consumidor_new, NULL, (void*)&poner_new_a_ready, NULL);
	pthread_create(&thr_parca, NULL, (void*)&terminar_TCBs, NULL);
	pthread_create(&thr_atencion_CPUs, NULL, (void*)&atender_solicitudes_pendientes, NULL);

	boot(parametros->syscalls_path);

	//while(1){
	/*
	 * 1 - Mandar a ejecutar TCBs a CPUs que lo hayan requerido
	 * 2 - Atender nuevas solicitudes de CPUs
	 */
		//atender_solicitudes_pendientes();
	//	atender_cpus();

	//}

	pthread_join(thr_consumidor_new, NULL);
	pthread_join(thr_parca, NULL);
	pthread_join(thr_atencion_CPUs, NULL);

return 0;
}

void mostrar_solicitud_cpu(int* sock){
	printf("Tengo la solicitud: %d\n", *sock);
}

void atender_solicitudes_pendientes(){
	while(1){

		sem_wait(&sem_solicitudes);
		pthread_mutex_lock(&mutex_solicitudes);
		int* sockCPU = list_remove(solicitudes_tcb, 0);
		pthread_mutex_unlock(&mutex_solicitudes);
		sem_wait(&sem_ready);
		t_hilo* tcb = obtener_tcb_a_ejecutar();
		printf("obtuve para ejecutar a tcb tid %d\n", tcb->tid);

		mandar_a_ejecutar(tcb, *sockCPU);
	}
}

void atender_cpus(){
	int i;
	fd_set read_cpus;
	FD_ZERO(&read_cpus);

	//Tiempo que se queda bloqueado esperando conexiones. En este caso no se va a esperar.
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	pthread_mutex_lock(&mutex_master_cpus);
	read_cpus = master_cpus;
	pthread_mutex_unlock(&mutex_master_cpus);

	if (select(cpus_fdmax+1, &read_cpus, NULL, NULL, &tv) == -1) {
		perror("select");
		exit(1);
	}
	for(i=0; i<=cpus_fdmax; i++){
		if(FD_ISSET(i, &read_cpus)){
			printf("Procedo a atender a la CPU: %d\n", i);
			handler_cpu(i);
		}
	}
}

/*********************** HILOS DEDICADOS ******************************/

//Este hilo se queda haciendo loop hasta que termine la ejecución
void poner_new_a_ready(){
	while(1){
		t_hilo* tcb = malloc(sizeof(t_hilo));
		sacar_de_new(tcb);
		encolar_en_ready(tcb);
	}
};

/* Este hilo se encarga de:
 * 		- Liberar todos los recursos del hilo
 * 		- Mandar a EXIT otros hilos que deban terminarse
 * 		- Notificar a la consola correspondiente
 */
void terminar_TCBs(){
	while(1){
		t_data_nodo_exit* data;
		data = sacar_de_exit();

		printf("Hay que matar a uno\n");
		switch(data->fin){
		case TERMINAR:
			if(es_padre(data->tcb)==true){
				// Hay que finalizar todos los hijos
				//escribir_consola(data->tcb->pid, "El proceso ha finalizado");
				printf("Y es padre...\n");
				terminar_proceso(data->tcb);
			}else{
				// Se finaliza sólo este hilo
				terminar_hilo(data->tcb);

			};
			break;

		case ABORTAR:
			// Hay que finalizar el proceso completo
			escribir_consola(data->tcb->pid, "Ha ocurrido una excepción");
			terminar_proceso(data->tcb);
			break;
		};
		//free(data->tcb);
		//free(data);
	};
}

/***************** FUNCIONES DEL HILO TERMINAR_TCBS() *****************/

void agregar_a_terminados(int tid){
	int* TID = malloc(sizeof(int));
	*TID = tid;
	pthread_mutex_lock(&mutex_terminados);
	list_add(terminados, TID);
	pthread_mutex_unlock(&mutex_terminados);
}

bool es_padre(t_hilo* tcb){
	tid_de_consola = tcb->tid;
	pthread_mutex_lock(&mutex_consolas);
	t_data_nodo_consolas* data = list_find(consolas, (void*)es_el_tid_consola);
	pthread_mutex_unlock(&mutex_consolas);
	if(data!=NULL){
		return true;
	};
	return false;
}

void escribir_consola(uint32_t pid, char* mensaje){
	int socket_consola = obtener_socket_consola(pid);
	if (socket_consola!=-1){
		imprimir_texto(socket_consola, mensaje);
	}
}

int obtener_socket_consola(uint32_t pid){
	pid_de_consola = pid;
	pthread_mutex_lock(&mutex_consolas);
	t_data_nodo_consolas* data = list_find(consolas, (void*)es_el_pid_consola);
	pthread_mutex_unlock(&mutex_consolas);
	if (data==NULL){
		return -1;
	}
	return data->socket;
}

void terminar_proceso(t_hilo* tcb){
	uint32_t pid = tcb->pid;
	eliminar_ready(pid);
	eliminar_block(pid);
	eliminar_exec(pid);
	sacar_de_consolas(pid);
	liberar_memoria_codigo(tcb);
	terminar_hilo(tcb);
}

void terminar_hilo(t_hilo* tcb){
	liberar_memoria_stack(tcb);
}

void liberar_memoria_stack(t_hilo* tcb){
	//Libero segmento de stack
	t_struct_free* free_segmento = malloc(sizeof(t_struct_free));
	free_segmento->PID = tcb->pid;
	free_segmento->direccion_base = tcb->base_stack;
	printf("Libero el segmento de stack, ubicado en: %d\n", free_segmento->direccion_base);
	socket_enviar(socket_MSP, D_STRUCT_FREE, free_segmento);
	free(free_segmento);
}

void liberar_memoria_codigo(t_hilo* tcb){
	//Libero segmento de codigo
	t_struct_free* free_segmento = malloc(sizeof(t_struct_free));
	free_segmento->PID = tcb->pid;
	free_segmento->direccion_base = tcb->segmento_codigo;
	printf("Libero el segmento de código, ubicado en: %d\n", free_segmento->direccion_base);
	socket_enviar(socket_MSP, D_STRUCT_FREE, free_segmento);
	free(free_segmento);
}

void eliminar_ready(uint32_t pid){
	pid_a_eliminar = pid;
	pthread_mutex_lock(&mutex_ready);
	t_hilo* tcb = list_remove_by_condition(cola_ready, (void*)es_el_pid_ready);
	pthread_mutex_unlock(&mutex_ready);
	while(tcb!=NULL){
		sem_wait(&sem_ready); //FIXME No sé si corresponde ponerlo acá así
		mandar_a_exit(tcb, TERMINAR);
		pthread_mutex_lock(&mutex_ready);
		tcb = list_remove_by_condition(cola_ready, (void*)es_el_pid_ready);
		pthread_mutex_unlock(&mutex_ready);
	};
}

void eliminar_block(uint32_t pid){
	pid_a_eliminar = pid;
	pthread_mutex_lock(&mutex_block);
	t_data_nodo_block* data = list_remove_by_condition(cola_block, (void*)es_el_pid_block);
	pthread_mutex_unlock(&mutex_block);
	while(data!=NULL){
		mandar_a_exit(data->tcb, TERMINAR);
		pthread_mutex_lock(&mutex_block);
		data = list_remove_by_condition(cola_block, (void*)es_el_pid_block);
		pthread_mutex_unlock(&mutex_block);
	};
	free(data);
}

void eliminar_exec(uint32_t pid){
	pid_a_eliminar = pid;
	pthread_mutex_lock(&mutex_exec);
	t_data_nodo_exec* data = list_remove_by_condition(cola_exec, (void*)es_el_pid_exec);
	pthread_mutex_unlock(&mutex_exec);
	while(data!=NULL){
		//TODO decirle a la cpu que aborte su ejecución (¿Eliminar solicitud de atención?)
		mandar_a_exit(data->tcb, TERMINAR);
		pthread_mutex_lock(&mutex_exec);
		data = list_remove_by_condition(cola_exec, (void*)es_el_pid_exec);
		pthread_mutex_unlock(&mutex_exec);
	};
	free(data);
}

bool es_la_solicitud(int* solicitud){
	return(*solicitud == solicitud_a_eliminar);
}

void eliminar_solicitud(int sockCPU){//FIXME Estoy mandando cualquiera
	solicitud_a_eliminar = sockCPU;
	pthread_mutex_lock(&mutex_solicitudes);
	int* solicitud = list_remove_by_condition(solicitudes_tcb, (void*)es_la_solicitud);
	pthread_mutex_unlock(&mutex_solicitudes);
	if(solicitud!=NULL){
		sem_wait(&sem_solicitudes);
		free(solicitud);
	}
}

void sacar_de_consolas(uint32_t pid){
	pid_de_consola = pid;
	pthread_mutex_lock(&mutex_consolas);
	t_data_nodo_consolas* data = list_remove_by_condition(consolas, (void*)es_el_pid_consola);
	pthread_mutex_unlock(&mutex_consolas);
	free(data);
}

/**************************** COLAS ***********************************/

void encolar_en_ready(t_hilo* tcb){
	tcb->cola = READY;
	pthread_mutex_lock(&mutex_ready);
	if (tcb->kernel_mode == true){
		list_add_in_index(cola_ready, 0, (void*)tcb);
	}else{
		list_add(cola_ready, (void*)tcb);
	};
	pthread_mutex_unlock(&mutex_ready);
	sem_post(&sem_ready);

	printf("Se encoló en ready el pid %d con PC %d\n", tcb->pid, tcb->puntero_instruccion);
};


void pop_new(t_hilo* tcb){
	void *nuevo = queue_pop(cola_new);
	*tcb = *(t_hilo*)nuevo;
};

void sacar_de_new(t_hilo* tcb){
	consumir_tcb(pop_new, &sem_new, &mutex_new, tcb);
}

void push_exit(t_data_nodo_exit* data){
	queue_push(cola_exit, (void*)data);
}

void mandar_a_exit(t_hilo* tcb, t_fin fin){
	t_data_nodo_exit* data = malloc(sizeof(t_data_nodo_exit));
	data->fin = fin;
	data->tcb = malloc(sizeof(t_hilo));
	//data->tcb = tcb;
	memcpy(data->tcb, tcb, sizeof(t_hilo));
	free(tcb);
	(data->tcb)->cola = EXIT;
	pthread_mutex_lock(&mutex_exit);
	push_exit(data);
	pthread_mutex_unlock(&mutex_exit);
	printf("Se mando a exit el tid %d\n", (data->tcb)->tid);
	sem_post(&sem_exit);
}

t_hilo* obtener_tcb_a_ejecutar(){
	//sem_wait(&sem_ready);
	pthread_mutex_lock(&mutex_ready);
	t_hilo* tcb = list_remove(cola_ready, 0);
	pthread_mutex_unlock(&mutex_ready);
	return tcb;
};

t_hilo* obtener_tcb_de_cpu(int sock_cpu){
	sockCPU_a_buscar = sock_cpu;
	t_data_nodo_exec* data;
	t_hilo* tcb;
	pthread_mutex_lock(&mutex_exec);
	data = list_remove_by_condition(cola_exec, (void*)es_el_tcbCPU);
	pthread_mutex_unlock(&mutex_exec);
	if (data!=NULL){
		tcb = data->tcb;
	}else{
		//printf("ERROR: No fue encontrado el CPU\n");
		tcb = NULL;
	};
	free(data);
	return tcb;
}

void sacar_de_exec(int sockCPU){
	t_hilo* tcb = obtener_tcb_de_cpu(sockCPU);
	free(tcb);
}

uint32_t obtener_pid_de_cpu(int sock_cpu){
	sockCPU_a_buscar = sock_cpu;
	t_data_nodo_exec* data;
	uint32_t pid;
	pthread_mutex_lock(&mutex_exec);
	data = list_find(cola_exec, (void*)es_el_tcbCPU);
	pthread_mutex_unlock(&mutex_exec);
	if (data!=NULL){
		pid = data->tcb->pid;
	}else{
		//Esto no debería ocurrir nunca (si lo hiciera, qué pid devuelvo?)
		printf("ERROR: No fue encontrado el CPU\n");
	};
	return pid;
}

void agregar_a_exec(int sockCPU, t_hilo* tcb){
	t_data_nodo_exec* data = malloc(sizeof(t_data_nodo_exec));
	data->sock = sockCPU;
	data->tcb = tcb;
	pthread_mutex_lock(&mutex_exec);
	list_add(cola_exec, data);
	pthread_mutex_unlock(&mutex_exec);
	tcb->cola = EXEC;
}

/********************************** BLOQUEAR ***************************************/

//Este bloquear es GENERAL. Para bloquear, usar los particulares.
void bloquear_tcb(t_hilo* tcb, t_evento evento, uint32_t parametro){
	t_data_nodo_block* data = malloc(sizeof(t_data_nodo_block));
	data->tcb = malloc(sizeof(t_hilo));
	memcpy(data->tcb, tcb, sizeof(t_hilo));
	data->evento = evento;
	data->parametro = parametro;
	pthread_mutex_lock(&mutex_block);
	list_add(cola_block, (void*)data);
	pthread_mutex_unlock(&mutex_block);
	tcb->cola = BLOCK;
};

void bloquear_tcbKernel(t_hilo* tcb){
	bloquear_tcb(tcb, TCBKM, 0);
}

void bloquear_tcbJoin(t_hilo* tcb, uint32_t tid){
	bloquear_tcb(tcb, JOIN, tid);
}

void bloquear_tcbSystcall(t_hilo* tcb, uint32_t dir_systcall){
	bloquear_tcb(tcb, SYSTCALL, dir_systcall);
}

void bloquear_tcbSemaforo(t_hilo* tcb, uint32_t sem){
	bloquear_tcb(tcb, SEM, sem);
}


/******************* FUNCIONES BOOLEANAS PARA BUSCAR EN COLAS ***************************/

bool es_el_tcbKernel(t_data_nodo_block* data){
	return (data->evento == TCBKM);
};

bool es_el_tcbSystcall(t_data_nodo_block* data){
	return (data->evento == SYSTCALL) & (data->tcb->tid == tid_a_buscar);
}

bool es_el_tcbBuscado(t_data_nodo_block* data){
	return ((data->parametro == parametro_a_buscar) & (data->evento == evento_a_buscar));
};

bool es_el_tcbCPU(t_data_nodo_exec* data){
	return (data->sock == sockCPU_a_buscar);
}

bool es_el_tid_consola(t_data_nodo_consolas* data){
	return (data->tid == tid_de_consola);
}

bool es_el_pid_consola(t_data_nodo_consolas* data){
	return (data->pid == pid_de_consola);
}

bool es_el_pid_ready(t_hilo* tcb){
	return(tcb->pid == pid_a_eliminar);
}

bool es_el_pid_block(t_data_nodo_block* data){
	return(data->tcb->pid == pid_a_eliminar);
}

bool es_el_pid_exec(t_data_nodo_exec* data){
	return(data->tcb->pid == pid_a_eliminar);
}

/******************************* DESBLOQUEAR ***************************************/

// Este desbloquear es GENERAL. Para desbloquear, hay que usar los particulares.
void desbloquear_proceso(t_evento evento, uint32_t parametro){
	t_data_nodo_block *data_desbloqueado;
	parametro_a_buscar = parametro;
	evento_a_buscar = evento;
	pthread_mutex_lock(&mutex_block);
	data_desbloqueado = list_remove_by_condition(cola_block,(void*)es_el_tcbBuscado);
	pthread_mutex_unlock(&mutex_block);
	if (data_desbloqueado != NULL){
		t_hilo * tcb_desbloqueado;// = malloc(sizeof(t_hilo));
		tcb_desbloqueado = data_desbloqueado->tcb;
		encolar_en_ready(tcb_desbloqueado);
		free(data_desbloqueado);
	}
}

t_hilo* desbloquear_tcbKernel(){
	pthread_mutex_lock(&mutex_block);
	t_data_nodo_block* data = list_remove_by_condition(cola_block, (void*)es_el_tcbKernel);
	pthread_mutex_unlock(&mutex_block);
	t_hilo* tcbKM;
	if (data != NULL){
		tcbKM = data->tcb;
	}else{
		tcbKM = NULL;
	}
	free(data);
	return tcbKM;
};

t_hilo* desbloquear_tcbSystcall(uint32_t tid){
	tid_a_buscar = tid;
	pthread_mutex_lock(&mutex_block);
	t_data_nodo_block* data = list_remove_by_condition(cola_block, (void*)es_el_tcbSystcall);
	pthread_mutex_unlock(&mutex_block);
	t_hilo* tcb;
	if (data != NULL){
		tcb = data->tcb;
	}else{
		tcb = NULL;
	}
	free(data);
	return tcb;
}

void desbloquear_por_join(uint32_t tid){
	desbloquear_proceso(JOIN, tid);
}

void desbloquear_por_semaforo(uint32_t sem){
	desbloquear_proceso(SEM, sem);
}

t_data_nodo_block* desbloquear_alguno_por_systcall(){//t_hilo* tcb_kernel){
	pthread_mutex_lock(&mutex_block);
	t_data_nodo_block* data = list_remove_by_condition(cola_block, (void*)esta_por_systcall);
	pthread_mutex_unlock(&mutex_block);
	return data;
}

/********************************* INICIO *******************************************/

void inicializar_ready_block(){
	cola_ready = list_create();
	cola_block = list_create();
	cola_exec = list_create();
	solicitudes_tcb = list_create();
	terminados = list_create();
}

void inicializar_semaforos_colas(){
	pthread_mutex_init(&mutex_ready, NULL);
	pthread_mutex_init(&mutex_block, NULL);
	pthread_mutex_init(&mutex_exec, NULL);
	pthread_mutex_init(&mutex_solicitudes, NULL);
	pthread_mutex_init(&mutex_terminados, NULL);
	sem_init(&sem_ready, 1, 0);
	sem_init(&sem_solicitudes, 1, 0);
};

void boot(char* systcalls_path){
	void * structRecibido;
	t_tipoEstructura tipoStruct;
	int respuesta;

	// Usar el mismo socket para la msp del main.c
	int tamanio_codigo;

	//Levanta archivo de syst calls
	FILE* syscalls_file = abrir_archivo(systcalls_path, logger, &mutex_log);
	tamanio_codigo = calcular_tamanio_archivo(syscalls_file);
	char* syscalls_code = leer_archivo(syscalls_file, tamanio_codigo);

	//Mando identificacion kernel
	//uint32_t senial = ES_KERNEL;
	//socket_enviarSignal(socket_MSP, senial);

	//Creo segmento para las syscalls
	t_struct_malloc* crear_seg = malloc(sizeof(t_struct_malloc));
	crear_seg->PID = 0; //Pid del tcb kernel
	crear_seg->tamano_segmento = tamanio_codigo;
	int resultado = socket_enviar(socket_MSP, D_STRUCT_MALC, crear_seg);
	if(resultado != 1){
		printf("No se pudo crear segmento de codigo para las syscalls\n");
	}
	free(crear_seg);

	//Recibo direccion del nuevo segmento de codigo
	socket_recibir(socket_MSP, &tipoStruct, &structRecibido);
	if(tipoStruct == D_STRUCT_NUMERO ){
		respuesta = ((t_struct_numero *) structRecibido)->numero;
		if (respuesta != -1){
			direccion_codigo_syscalls = ((t_struct_numero *) structRecibido)->numero;
			printf("Recibi direccion %d de codigo de syscalls\n", direccion_codigo_syscalls);
		}else{
			printf("No hay espacio suficiente en memoria para el código de las systcalls\n");
		}
	} else {
		printf("No se recibio la direccion del segmento de codigo de las syscalls\n");
	}


	//Creo segmento de stack para las syscalls
	crear_seg = malloc(sizeof(t_struct_malloc));
	crear_seg->PID = 0; //Pid del tcb kernel
	crear_seg->tamano_segmento = tamanio_stack;
	resultado = socket_enviar(socket_MSP, D_STRUCT_MALC, crear_seg);
	if(resultado != 1){
		printf("No se pudo crear segmento de stack para las syscalls\n");
	}
	free(crear_seg);

	//Recibo direccion del nuevo segmento de stack
	socket_recibir(socket_MSP, &tipoStruct, &structRecibido);
	if(tipoStruct == D_STRUCT_NUMERO ){
		respuesta = ((t_struct_numero *) structRecibido)->numero;
		if (respuesta != -1){
			direccion_stack_syscalls = ((t_struct_numero *) structRecibido)->numero;
			printf("Se recibio la direcc del stack %d\n", direccion_stack_syscalls);
		}else{
			printf("No hay espacio suficiente en memoria para el stack del TCB Kernel\n");
		}
	} else {
		printf("No se recibio la direccion del segmento de stack de las syscalls\n");
	}


	//Manda codigo de syscalls a la MSP
	t_struct_env_bytes* paquete_syscalls = malloc(sizeof(t_struct_env_bytes));
	paquete_syscalls->buffer = malloc(tamanio_codigo);
	memcpy(paquete_syscalls->buffer, syscalls_code, tamanio_codigo);
	paquete_syscalls->tamanio = tamanio_codigo;
	paquete_syscalls->base = direccion_codigo_syscalls;
	paquete_syscalls->PID = 0; //Pid del tcb kernel

	socket_enviar(socket_MSP, D_STRUCT_ENV_BYTES, paquete_syscalls);//sockMSP
	free(paquete_syscalls->buffer);
	free(paquete_syscalls);
	free(structRecibido);

	socket_recibir(socket_MSP, &tipoStruct, &structRecibido);//sockMSP
	if (tipoStruct != D_STRUCT_NUMERO) {
		if(((t_struct_numero*) structRecibido)->numero == 0){
			printf("Se escribio correctamente en memoria\n");
		}
	}

	free(structRecibido);

	//Crea hilo en modo kernel y lo encola en bloqueados
	t_hilo *tcb_kernel = crear_TCB(0, direccion_codigo_syscalls, direccion_stack_syscalls, tamanio_codigo);
	tcb_kernel->kernel_mode = true;
	bloquear_tcbKernel(tcb_kernel);
}


/*************************** SYSCALLS *******************************************/

void copiar_tcb(t_hilo* original, t_hilo* copia){
	t_list* lista_hilos= list_create();
	list_add(lista_hilos, original);
	list_add(lista_hilos, copia);

	pthread_mutex_lock(&mutex_log);
	hilos(lista_hilos);
	pthread_mutex_unlock(&mutex_log);

	list_destroy(lista_hilos);

	copia->tid = original->tid;
	copia->pid = original->pid;
	int i;
	for(i=0; i<=4; i++){
		copia->registros[i] = original->registros[i];
	}
};

char* nombre_syscall(uint32_t dir_systcall){
	switch(dir_systcall){
	case 0:
		return "MALC";
		break;
	case 28:
		return "FREE";
		break;
	case 48:
		return "INNN";
		break;
	case 64:
		return "INNC";
		break;
	case 76:
		return "OUTN";
		break;
	case 108:
		return "OUTC";
		break;
	case 132:
		return "CREA";
		break;
	case 164:
		return "JOIN";
		break;
	case 176:
		return "BLOK";
		break;
	case 244:
		return "WAKE";
		break;
	case 312:
		return "SETSEM";
		break;
	default:
		return "No se encontro la instruccion protegida";

	}
}

void atender_systcall(t_hilo* tcb, uint32_t dir_systcall){
	printf("Procedo a atender la systcall del TCB de tid: %d\n", tcb->tid);

	pthread_mutex_lock(&mutex_log);
	instruccion_protegida(nombre_syscall(dir_systcall), tcb);
	pthread_mutex_unlock(&mutex_log);

	t_hilo* tcb_kernel = desbloquear_tcbKernel();
	bloquear_tcbSystcall(tcb, dir_systcall);
	if (tcb_kernel != NULL){
		printf("Desbloqueé el TCB de kernel y lo mando a ready, tenía el tid: %d\n", tcb_kernel->tid);
		copiar_tcb(tcb, tcb_kernel);
		tcb_kernel->puntero_instruccion = dir_systcall;
		//tcb_kernel->segmento_codigo = direccion_codigo_syscalls; //FIXME: FIJARSE QUE ESTO ESTE BIEN
		//tcb_kernel->base_stack = direccion_stack_syscalls;
		//tcb_kernel->pid = 0; //Pid de kernel
		encolar_en_ready(tcb_kernel);
	}
	// Avisarle a la cpu que pida otro proceso para ejecutar? No. Lo hace sola.
};

bool esta_por_systcall(t_data_nodo_block* data){
	return (data->evento == SYSTCALL);
}

void atender_otra_systcall_si_hay(){
	t_data_nodo_block* data_otro_tcb = desbloquear_alguno_por_systcall();
	if (data_otro_tcb != NULL){
		atender_systcall(data_otro_tcb->tcb, data_otro_tcb->parametro);
	};
}

void retornar_de_systcall(t_hilo* tcb_kernel, t_fin fin){
	t_hilo* tcb = desbloquear_tcbSystcall(tcb_kernel->tid);
	int i;

	if (tcb!=NULL){
		printf("TCB que vuelve de syscall tiene PID: %d y PC: %d\n", tcb->pid, tcb->puntero_instruccion);
		switch(fin){
		case TERMINAR:
			for(i=0; i<=4; i++){
				tcb->registros[i] = tcb_kernel->registros[i];
			}
			encolar_en_ready(tcb);
			break;

		case ABORTAR:
			mandar_a_exit(tcb, fin);
		}
	}
	bloquear_tcbKernel(tcb_kernel);

	atender_otra_systcall_si_hay();//tcb_kernel);
}

uint32_t crear_nuevo_hilo(t_hilo* tcb_padre, int pc){
	void * structRecibido;
	t_tipoEstructura tipoStruct;
	uint32_t dir_stack_hijo;
	int respuesta;
	void* codigo_stack_padre;
	int tamanio_stack_padre;

	// Pido a la MSP un nuevo segmento de stack para el hilo hijo
	t_struct_malloc* crear_seg = malloc(sizeof(t_struct_malloc));
	crear_seg->PID = tcb_padre->pid;
	crear_seg->tamano_segmento = tamanio_stack;
	int resultado = socket_enviar(socket_MSP, D_STRUCT_MALC, crear_seg);
	if(resultado != 1){
		printf("No se pudo crear segmento de codigo\n");
		return 0;
	}
	free(crear_seg);

	//Recibo dirección del nuevo segmento de stack del hijo
	socket_recibir(socket_MSP, &tipoStruct, &structRecibido);
	if(tipoStruct == D_STRUCT_NUMERO ){
		respuesta = ((t_struct_numero *) structRecibido)->numero;
		if (respuesta != -1){
			dir_stack_hijo = ((t_struct_numero *) structRecibido)->numero;
		}else{
			printf("No hay espacio suficiente en memoria para el stack del nuevo hijo\n");
			return 0;
		}
	} else {
		printf("No se recibio la direccion del segmento de codigo del proceso\n");
		return 0;
	}

	//Pido el código de stack del proceso padre
	t_struct_sol_bytes* solicitud = malloc(sizeof(t_struct_sol_bytes));
	solicitud->PID = tcb_padre->pid;
	solicitud->base = tcb_padre->base_stack;
	solicitud->tamanio = tcb_padre->cursor_stack;
	socket_enviar(socket_MSP, D_STRUCT_SOL_BYTES, solicitud);
	free(solicitud);

	//Recibo el código de stack del proceso padre
	socket_recibir(socket_MSP, &tipoStruct, &structRecibido);
	if(tipoStruct == D_STRUCT_RESPUESTA_MSP ){
		tamanio_stack_padre = ((t_struct_respuesta_msp*)structRecibido)->tamano_buffer;
		codigo_stack_padre = malloc(tamanio_stack_padre);
		memcpy(codigo_stack_padre, ((t_struct_respuesta_msp*)structRecibido)->buffer, tamanio_stack_padre);
	} else {
		return 0;
	}

	//Escribo el código de stack del padre en el del hijo
	t_struct_env_bytes* paquete_codigo = malloc(sizeof(t_struct_env_bytes));
	paquete_codigo->buffer = malloc(tamanio_stack_padre);
	memcpy(paquete_codigo->buffer, codigo_stack_padre, tamanio_stack_padre);
	paquete_codigo->tamanio = tamanio_stack_padre;
	paquete_codigo->base = dir_stack_hijo;
	paquete_codigo->PID = tcb_padre->pid;

	socket_enviar(socket_MSP, D_STRUCT_ENV_BYTES, paquete_codigo);
	free(paquete_codigo->buffer);
	free(paquete_codigo);
	free(structRecibido);

	socket_recibir(socket_MSP, &tipoStruct, &structRecibido);
	if (tipoStruct != D_STRUCT_NUMERO) {
		if(((t_struct_numero*) structRecibido)->numero == 0){
			printf("Se escribió correctamente en memoria\n");
		}else{
			printf("Hubo un problema al copiar el stack del padre al hijo\n");
			return 0;
		}
	}

	//Creo el TCB hijo, copio los datos del padre, y lo mando a READY
	t_hilo* tcb_hijo = crear_TCB(tcb_padre->pid, tcb_padre->segmento_codigo, dir_stack_hijo, tcb_padre->segmento_codigo_size);
	tcb_hijo->puntero_instruccion = pc;
	tcb_hijo->cursor_stack = tcb_padre->cursor_stack;
	int i;
	for(i=0; i<=4; i++){
		tcb_hijo->registros[i] = tcb_padre->registros[i];
	}
	encolar_en_ready(tcb_hijo);
	return tcb_hijo->tid;
}

/******************************** HANDLER CPU *****************************************/

void mandar_a_ejecutar(t_hilo* tcb, int sockCPU){
	t_struct_tcb* paquete_tcb = malloc(sizeof(t_struct_tcb));
	copiar_tcb_a_structTcb(tcb, paquete_tcb);
	socket_enviar(sockCPU, D_STRUCT_TCB, paquete_tcb);
	printf("Mandé a ejecutar el tcb de pid: %d a la CPU: %d\n", paquete_tcb->pid, sockCPU);
	free(paquete_tcb);
	agregar_a_exec(sockCPU, tcb);
}

void agregar_solicitud(int* sockCPU){
	pthread_mutex_lock(&mutex_solicitudes);
	list_add(solicitudes_tcb, sockCPU);
	pthread_mutex_unlock(&mutex_solicitudes);
	sem_post(&sem_solicitudes);
}

void handler_numeros_cpu(int32_t numero_cpu, int sockCPU){
	t_hilo* tcb = malloc(sizeof(t_hilo));
	uint32_t pid;

	switch(numero_cpu){
	case D_STRUCT_INNN:
		//pido numero por consola
		pid = obtener_pid_de_cpu(sockCPU);
		int socket_consola = obtener_socket_consola(pid);
		t_struct_numero* innn = malloc(sizeof(t_struct_numero));
		innn->numero = D_STRUCT_INNN;
		socket_enviar(socket_consola, D_STRUCT_NUMERO, innn);

		break;
	case D_STRUCT_ABORT:

		tcb = obtener_tcb_de_cpu(sockCPU);

		//Verifico si es el de Kernel
		if(tcb->kernel_mode == true){
			//TODO Consultar: si es el de Kernel, aborto sólo el proceso que hizo a la systcall?
			retornar_de_systcall(tcb, ABORTAR);
		}else{
			mandar_a_exit(tcb, ABORTAR);
		}

		break;

	case D_STRUCT_PEDIR_TCB:
			printf("me pidieron TCB\n");
			printf("Agrego a las solicitudes a la CPU %d\n", sockCPU);
			int* solicitud = malloc(sizeof(int));
			*solicitud = sockCPU;
			agregar_solicitud(solicitud);
			break;
	}
}

void handler_cpu(int sockCPU){
	t_hilo* tcb = malloc(sizeof(t_hilo));
	uint32_t tid_llamador;
	uint32_t tid_a_esperar;
	uint32_t tid_padre;
	uint32_t tid_hijo;
	uint32_t direccion_syscall;
	int32_t id_semaforo;
	int32_t numero_cpu;
	//int32_t numero_consola;
	//uint32_t maximo_caracteres;
	//char* cadena_consola;
	int pid;
	int socket_consola;

	t_tipoEstructura tipoRecibido;
	t_tipoEstructura tipoRecibido2;
	void* structRecibido;
	void* structRecibido2;

	t_struct_numero* paquete_crea;

	if(socket_recibir(sockCPU, &tipoRecibido, &structRecibido)==-1){
		//La CPU cerró la conexión
		t_hilo* tcb = obtener_tcb_de_cpu(sockCPU);
		if (tcb!=NULL){
			if (tcb->kernel_mode == 0){
				mandar_a_exit(tcb, ABORTAR);
			}else{
				retornar_de_systcall(tcb, ABORTAR);
			}
		}
		eliminar_solicitud(sockCPU);
		pthread_mutex_lock(&mutex_cpus_conectadas);
		pthread_mutex_lock(&mutex_log);
		desconexion_cpu(sockCPU);
		pthread_mutex_unlock(&mutex_log);
		pthread_mutex_unlock(&mutex_cpus_conectadas);

		close(sockCPU);
		//pthread_mutex_lock(&mutex_master_cpus);
		FD_CLR(sockCPU, &master_cpus);
		//pthread_mutex_unlock(&mutex_master_cpus);
	}else{

	//TODO: PONER LOGS!
	switch(tipoRecibido){
	case D_STRUCT_INTE:
		//Recibo la direccion
		direccion_syscall = ((t_struct_direccion*) structRecibido)->numero;
		//otro socket para el tcb
		socket_recibir(sockCPU, &tipoRecibido2, &structRecibido2);

		printf("Estoy en INTE y ya me llegó el tcb\n");

		if(tipoRecibido2 == D_STRUCT_TCB){
			copiar_structRecibido_a_tcb(tcb, structRecibido2);
		} else {
			printf("No llegó el TCB para la operacion INTE\n");
		}

		sacar_de_exec(sockCPU);
		atender_systcall(tcb, direccion_syscall);


		break;
	case D_STRUCT_NUMERO:

		numero_cpu = ((t_struct_numero*) structRecibido)->numero;
		handler_numeros_cpu(numero_cpu, sockCPU);
		break;
	case D_STRUCT_INNC:

		//Pido string por consola con el maximo de caracteres recibido
		//maximo_caracteres = ((t_struct_numero *) structRecibido)->numero;
		pid = obtener_pid_de_cpu(sockCPU);
		socket_consola = obtener_socket_consola(pid);
		socket_enviar(socket_consola, tipoRecibido, structRecibido);

		break;
	case D_STRUCT_TCB_CREA:

		//copiar_structRecibido_a_tcb(tcb, structRecibido);
		tid_padre = ((t_struct_numero*) structRecibido)->numero;
		tcb = desbloquear_tcbSystcall(tid_padre);

		//RECIBO PC
		socket_recibir(sockCPU, &tipoRecibido2, &structRecibido2);
		int pc = ((t_struct_numero*) structRecibido2)->numero;
		tid_hijo = crear_nuevo_hilo(tcb, pc);

		if(tid_hijo != 0){
			paquete_crea = malloc(sizeof(t_struct_numero));
			paquete_crea->numero = tid_hijo;
			socket_enviar(sockCPU, D_STRUCT_NUMERO, paquete_crea);
			free(paquete_crea);
			bloquear_tcbSystcall(tcb, 0); //VALOR HARDCODEADO, PERO DEBERÍA FUNCIONAR
		}else{
			mandar_a_exit(tcb, ABORTAR);
		}

		break;
	case D_STRUCT_JOIN:
		tid_llamador = ((t_struct_join*) structRecibido)->tid_llamador;
		tid_a_esperar = ((t_struct_join*) structRecibido)->tid_a_esperar;

		int tid_buscado = tid_a_esperar;
		bool esta_terminado(int* tid){
			return *tid == tid_buscado;
		}
		printf("Tids del join: %d y %d\n", tid_a_esperar, tid_llamador);

		pthread_mutex_lock(&mutex_terminados);
		bool termino = list_any_satisfy(terminados, (void*)esta_terminado);
		pthread_mutex_unlock(&mutex_terminados);

		tcb = desbloquear_tcbSystcall(tid_llamador);

		if(!termino){
			bloquear_tcbJoin(tcb, tid_a_esperar);
		}else{
			encolar_en_ready(tcb);
		}
		break;
	case D_STRUCT_BLOCK:

		//Recibo id semaforo
		id_semaforo = ((t_struct_numero*) structRecibido)->numero;

		//otro socket para el TID
		socket_recibir(sockCPU, &tipoRecibido2, &structRecibido2);

		if(tipoRecibido2 == D_STRUCT_NUMERO){
			//copiar_structRecibido_a_tcb(tcb, structRecibido);
			int tid_a_bloquear = ((t_struct_numero *) structRecibido2)->numero;
			tcb = desbloquear_tcbSystcall(tid_a_bloquear);
			bloquear_tcbSemaforo(tcb, id_semaforo);
			//obtener_tcb_de_cpu(sockCPU);
		} else {
			printf("No se recibio el TID para la operacion BLOCK\n");
		}

		break;
	case D_STRUCT_WAKE:
		//Recibo id semaforo
		id_semaforo = ((t_struct_numero*) structRecibido)->numero;
		desbloquear_por_semaforo(id_semaforo);

		break;
	case D_STRUCT_TCB_QUANTUM:

		copiar_structRecibido_a_tcb(tcb, structRecibido);
		//Lo saco de EXEC
		sacar_de_exec(sockCPU);
		encolar_en_ready(tcb);

		break;
	case D_STRUCT_TCB:

		copiar_structRecibido_a_tcb(tcb, structRecibido);
		//Lo saco de EXEC
		sacar_de_exec(sockCPU);
		printf("Me devolvieron el TCB de PID %d porque finalizó. El reg B vale %d\n", tcb->pid, tcb->registros[1]);

		//Verifico si es el de Kernel
		if(tcb->kernel_mode == true){
			printf("Me devolvieron el TCB del kernel, con el tid: %d\n", tcb->tid);
			retornar_de_systcall(tcb, TERMINAR);
		}else{
			//terminar tcb
			agregar_a_terminados(tcb->tid);
			mandar_a_exit(tcb, TERMINAR);
			desbloquear_por_join(tcb->tid);
		}
		break;

	case D_STRUCT_OUTN:
		//Mandar ese numero a consola para ser mostrado
		pid = obtener_pid_de_cpu(sockCPU);
		socket_consola = obtener_socket_consola(pid);
		socket_enviar(socket_consola, tipoRecibido, structRecibido);
		break;

	case D_STRUCT_OUTC:
		//Mandar esa cadena a consola para ser mostrada
		pid = obtener_pid_de_cpu(sockCPU);
		socket_consola = obtener_socket_consola(pid);
		socket_enviar(socket_consola, tipoRecibido, structRecibido);

		socket_recibir(sockCPU, &tipoRecibido2, &structRecibido2);
		socket_enviar(socket_consola, tipoRecibido2, structRecibido2);
		break;
	}//ACÁ TERMINA EL SWITCH
	//free(structRecibido);
	}//ACÁ TERMINA EL ELSE DEL IF
}

