/*
 * planificador.c
 *
 *  Created on: 06/10/2014
 *      Author: utnso
 */

#include "planificador.h"

void encolar_en_ready(t_hilo* tcb){
	pthread_mutex_lock(mutex_ready);
	if (tcb->kernel_mode == 1){
		list_add_in_index(cola_ready, 0, (void*)tcb);
	}else{
		list_add(cola_ready, (void*)tcb);
	};
	pthread_mutex_unlock(mutex_ready);
	tcb->cola = READY;
};

t_hilo* obtener_tcb_a_ejecutar(){
	return (t_hilo*) list_remove(cola_ready, 0);
};

//tid y recurso pueden ser -1 dependiendo del tipo de evento
void bloquear_tcb(t_hilo* tcb, t_evento evento, uint32_t tid, uint32_t recurso){
	t_data_nodo_block* data = malloc(sizeof(t_data_nodo_block));
	data->tcb = tcb;
	data->evento = evento;
	data->tid = tid;
	data->recurso = recurso;
	list_add(cola_block, (void*)data);
	tcb->cola = BLOCK;
};

void copiar_tcb(t_hilo* original, t_hilo* copia){
	copia->tid = original->tid;
	copia->pid = original->pid;
	copia->registros = original->registros;
};

bool es_el_tcbkernel(t_data_nodo_block* data){
	return (data->evento == TCBKM);
};

t_hilo* desbloquear_tcbkernel(){
	return list_remove_by_condition(cola_block, es_el_tcbkernel);
};

void atender_systcall(t_hilo* tcb, uint32_t dir_systcall){
	t_hilo* tcb_kernel = desbloquear_tcbkernel();
	bloquear_tcb(tcb, SYSTCALL, NULL, NULL);
	if (*tcb_kernel != NULL){
		copiar_tcb(tcb, tcb_kernel);
		tcb_kernel->puntero_instruccion = dir_systcall;
		encolar_en_ready(tcb_kernel);
	}
	//TODO Avisarle a la cpu que pida otro proceso para ejecutar
};

// TODO Desbloquear un proceso. Depende de la cantidad de colas de bloqueados que haya.

void inicializar_ready_block(){
	cola_ready = list_create();
	cola_block = list_create();
}

void pop_new(t_hilo* tcb){
	void* nuevo = queue_pop(cola_new);
	*tcb = *nuevo;
};

//Este hilo se queda haciendo loop hasta que termine la ejecución
void poner_new_a_ready(){
	while(1){
		t_hilo* tcb;
		consumir_tcb(pop_new, sem_new, mutex_new, tcb);
		encolar_en_ready(tcb);
	}
};

void inicializar_semaforo_ready(){
	pthread_mutex_init(mutex_ready, NULL);
};

void boot(char* systcalls_path){
	uint32_t dir_codigo;
	uint32_t dir_stack;
	int tamanio_codigo;
	//TODO levantar archivo de syst calls
	//tamanio_codigo =
	//TODO mandárselo a la MSP
	//dir_codigo =
	//dir_stack =
	t_hilo* tcb_kernel = crear_TCB(obtener_pid(), dir_codigo, dir_stack, tamanio_codigo);
	tcb_kernel->kernel_mode = true;
	bloquear_tcb(tcb_kernel, TCBKM, -1,-1);
}

void* main_PLANIFICADOR(void* parametros)
{
	inicializar_ready_block();
	inicializar_semaforo_ready();
	pthread_t thr_consumidor_new;
	pthread_create(&thr_consumidor_new, NULL, poner_new_a_ready, NULL);

	boot(parametros->syscalls_path);
	// TODO: Boot

	pthread_join(thr_consumidor_new, NULL);

return 0;
}
