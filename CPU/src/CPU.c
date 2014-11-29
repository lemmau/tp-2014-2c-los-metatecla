/*
 ============================================================================
 Name        : CPU.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "libs/funcionesCPU.h"

#include "commons/bitarray.h"
#include <string.h>
#include <math.h>

t_hilo* tcb;
char* PATH;
t_config* config_cpu;
t_config_cpu config_struct_cpu;

void * structRecibido;
t_tipoEstructura tipo_struct;
void* datos_recibidos;
int resultado;

int sockKernel;
int sockMSP;

int main(int argc, char** argv) {

	PATH = argv[1];
	inicializar_configuracion();

	tcb = malloc(sizeof(t_hilo));

	//Abrir conexion de sockets con Kernel y MSP.
	sockKernel = socket_crearYConectarCliente(config_struct_cpu.ip_kernel, config_struct_cpu.puerto_kernel);

	if(sockKernel == -1){
			printf("No se pudo conectar a Kernel\n");
		} else {
			printf("Me conecte al Kernel. Socket %d\n", sockKernel);
			t_struct_numero* es_cpu = malloc(sizeof(t_struct_numero));
			es_cpu->numero = ES_CPU;
			socket_enviar(sockKernel, D_STRUCT_NUMERO, es_cpu);
			free(es_cpu);
		}


	sockMSP = socket_crearYConectarCliente(config_struct_cpu.ip_msp, config_struct_cpu.puerto_msp);

	if(sockMSP == -1){
		printf("No se pudo conectar a MSP\n");
	} else {
		printf("Numero de socket de MSP es %d\n", sockMSP);
		t_struct_numero* es_cpu = malloc(sizeof(t_struct_numero));
		es_cpu->numero = ES_CPU;
		socket_enviar(sockMSP, D_STRUCT_NUMERO, es_cpu);
		free(es_cpu);
	}


	//Recibir quantum del Kernel
	socket_recibir(sockKernel, &tipo_struct, &structRecibido);
	if(tipo_struct == D_STRUCT_NUMERO){
		quantum = ((t_struct_numero *) structRecibido)->numero;
		printf("Recibi quantum de %d\n", quantum);
	}

	//socket a Kernel pidiendo tcb
	t_struct_numero* pedir_tcb = malloc(sizeof(t_struct_numero));
	pedir_tcb->numero = D_STRUCT_PEDIR_TCB;
	resultado = socket_enviar(sockKernel, D_STRUCT_NUMERO, pedir_tcb);
	if(resultado != 1){
		printf("No se pudo pedir TCB\n");
	} else {
		//socket de Kernel con tcb
		int j =socket_recibir(sockKernel, &tipo_struct, &structRecibido);
		if(j != 1){
			printf("No se recibio tcb\n");
			printf("Me llego d_struct = %d\n", tipo_struct);
			} else {
				printf("Se recibio tcb\n");
				copiar_structRecibido_a_tcb(tcb, structRecibido);
			}

	}

	int bytecode[4];// = malloc(sizeof(uint32_t));

	cantidad_lineas_ejecutadas = 0;
	copiar_tcb_a_registros();
	registros_cpu.I = tcb->pid;
	comienzo_ejecucion(tcb,quantum);


	while(1){
	if(registros_cpu.K == false){ //SI NO ES MODO KERNEL
		if(terminoEjecucion == true){
			terminar_y_pedir_tcb(tcb);

		} else {
			if(cantidad_lineas_ejecutadas == quantum){
				printf("cant lineas = quantum\n");
				//socket a kernel con tcb
				copiar_registros_a_tcb();
				t_struct_tcb* tcb_enviar = malloc(sizeof(t_struct_tcb));
				copiar_tcb_a_structTcb(tcb, tcb_enviar);
				resultado = socket_enviar(sockKernel, D_STRUCT_TCB_QUANTUM, tcb_enviar);
				controlar_envio(resultado, D_STRUCT_TCB_QUANTUM);

				fin_ejecucion();

				terminar_y_pedir_tcb(tcb);

			} else{
				cantidad_lineas_ejecutadas++;
				registros_cpu.I = tcb->pid;
				ejecutar_otra_linea(sockMSP, tcb, bytecode);
			}
		}
	}

	if(registros_cpu.K == true){ //SI ES MODO KERNEL
		if(terminoEjecucion == true){
			terminar_y_pedir_tcb(tcb);
		} else{
			registros_cpu.I = 0;
			ejecutar_otra_linea(sockMSP,tcb,bytecode);
			}
		}

	//esperar_retardo();

	}

	return EXIT_SUCCESS;
}

void ejecutar_otra_linea(int sockMSP,t_hilo* tcb, int bytecode[4]) {
	/*uint32_t senial = ES_CPU;
	socket_enviarSignal(sockMSP, senial);*/

	//socket a MSP con PC
	t_struct_sol_bytes* datos_solicitados = malloc(sizeof(t_struct_sol_bytes));
	uint32_t direccionMSP = sumar_desplazamiento(registros_cpu.M, registros_cpu.P);
	datos_solicitados->base = direccionMSP;
	datos_solicitados->PID = registros_cpu.I;
	datos_solicitados->tamanio = 4; //tamaño bytecode
	int resultado = socket_enviar(sockMSP, D_STRUCT_SOL_BYTES, datos_solicitados);
	controlar_envio(resultado, D_STRUCT_SOL_BYTES);
	socket_recibir(sockMSP, &tipo_struct, &structRecibido);
	controlar_struct_recibido(tipo_struct, D_STRUCT_RESPUESTA_MSP);
	memcpy(bytecode, ((t_struct_respuesta_msp*) structRecibido)->buffer, ((t_struct_respuesta_msp*) structRecibido)->tamano_buffer);
	incrementar_pc(4);
	free(datos_solicitados);
	ejecutarLinea(bytecode);
}

t_struct_numero* terminar_y_pedir_tcb(t_hilo* tcb) {
	//socket a Kernel pidiendo tcb
	t_struct_numero* pedir_tcb = malloc(sizeof(t_struct_numero));
	pedir_tcb->numero = D_STRUCT_PEDIR_TCB;
	int resultado = socket_enviar(sockKernel, D_STRUCT_NUMERO, pedir_tcb);
	if (resultado != 1) {
		printf("No se pudo pedir TCB\n");
	}
	//socket de Kernel con tcb
	socket_recibir(sockKernel, &tipo_struct, &structRecibido);
	copiar_structRecibido_a_tcb(tcb, structRecibido);
	cantidad_lineas_ejecutadas = 0;
	terminoEjecucion = false;
	free(pedir_tcb);
	comienzo_ejecucion(tcb, quantum);
	copiar_tcb_a_registros();
	return pedir_tcb;
}


