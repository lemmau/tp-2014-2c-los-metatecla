/*
 * auxiliares.c
 *
 *  Created on: 12/09/2014
 *      Author: utnso
 */

#include "auxiliares.h"

void copiar_tcb_a_registros(){
	registros_cpu.I = tcb->pid;
	registros_cpu.K = tcb->kernel_mode;
	registros_cpu.M = tcb->segmento_codigo;
	registros_cpu.P = tcb->puntero_instruccion;
	registros_cpu.S = tcb->cursor_stack;
	registros_cpu.X = tcb->base_stack;
	registros_cpu.registros_programacion[0] = tcb->registros[0];
	registros_cpu.registros_programacion[1] = tcb->registros[1];
	registros_cpu.registros_programacion[2] = tcb->registros[2];
	registros_cpu.registros_programacion[3] = tcb->registros[3];
	registros_cpu.registros_programacion[4] = tcb->registros[4];
}

void copiar_registros_a_tcb(){
	tcb->pid = registros_cpu.I;
	tcb->kernel_mode = registros_cpu.K;
	tcb->segmento_codigo = registros_cpu.M;
	tcb->puntero_instruccion = registros_cpu.P;
	tcb->cursor_stack = registros_cpu.S;
	tcb->base_stack = registros_cpu.X;
	tcb->registros[0] = registros_cpu.registros_programacion[0];
	tcb->registros[1] = registros_cpu.registros_programacion[1];
	tcb->registros[2] = registros_cpu.registros_programacion[2];
	tcb->registros[3] = registros_cpu.registros_programacion[3];
	tcb->registros[4] = registros_cpu.registros_programacion[4];
}

void obtener_registro(char* parametros, int posicion, char* registro){
	memcpy(registro, parametros + posicion, sizeof(char));
}

void obtener_numero(char* parametros, int posicion, int32_t* numero, char* aux){
	memcpy(aux,parametros,sizeof(int32_t));
	int num = atoi(aux);
	memcpy(numero,&num,sizeof(int32_t));
}

void obtener_direccion(char* parametros, int posicion, uint32_t* direccion, char* aux){
	memcpy(aux,parametros,sizeof(uint32_t));
	int num = atoi(aux);
	memcpy(direccion,&num,sizeof(uint32_t));}

int elegirRegistro(char registro){
	int posicion_registros;
	switch(registro){
	case 'A':
		posicion_registros = 0;
		break;
	case 'B':
		posicion_registros = 1;
		break;
	case 'C':
		posicion_registros = 2;
		break;
	case 'D':
		posicion_registros = 3;
		break;
	case 'E':
		posicion_registros = 4;
		break;
	}
	return posicion_registros;
}

int convertirAString(int* byte){
	char* bytecode = byte;
	int bytecodeLetras;

	if(string_equals_ignore_case(bytecode,"LOAD")){
		bytecodeLetras = LOAD;
	}

	if(string_equals_ignore_case(bytecode,"GETM")){
			bytecodeLetras = GETM;
		}

	if(string_equals_ignore_case(bytecode,"SETM")){
			bytecodeLetras = SETM;
		}

	if(string_equals_ignore_case(bytecode,"MOVR")){
			bytecodeLetras = MOVR;
		}

	if(string_equals_ignore_case(bytecode,"ADDR")){
				bytecodeLetras = ADDR;
			}

	if(string_equals_ignore_case(bytecode,"SUBR")){
			bytecodeLetras = SUBR;
		}

	if(string_equals_ignore_case(bytecode,"MULR")){
			bytecodeLetras = MULR;
		}

	if(string_equals_ignore_case(bytecode,"MODR")){
			bytecodeLetras = MODR;
		}

	if(string_equals_ignore_case(bytecode,"DIVR")){
			bytecodeLetras = DIVR;
		}

	if(string_equals_ignore_case(bytecode,"INCR")){
			bytecodeLetras = INCR;
		}

	if(string_equals_ignore_case(bytecode,"DECR")){
			bytecodeLetras = DECR;
		}

	if(string_equals_ignore_case(bytecode,"COMP")){
			bytecodeLetras = COMP;
		}

	if(string_equals_ignore_case(bytecode,"CGEQ")){
			bytecodeLetras = CGEQ;
		}

	if(string_equals_ignore_case(bytecode,"CLEQ")){
			bytecodeLetras = CLEQ;
		}

	if(string_equals_ignore_case(bytecode,"GOTO")){
			bytecodeLetras = GOTO;
		}

	if(string_equals_ignore_case(bytecode,"JMPZ")){
			bytecodeLetras = JMPZ;
		}

	if(string_equals_ignore_case(bytecode,"JPNZ")){
			bytecodeLetras = JPNZ;
		}

	if(string_equals_ignore_case(bytecode,"INTE")){
			bytecodeLetras = INTE;
		}

	if(string_equals_ignore_case(bytecode,"SHIF")){
			bytecodeLetras = SHIF;
		}

	if(string_equals_ignore_case(bytecode,"NOPP")){
			bytecodeLetras = NOPP;
		}

	if(string_equals_ignore_case(bytecode,"PUSH")){
			bytecodeLetras = PUSH;
		}

	if(string_equals_ignore_case(bytecode,"TAKE")){
			bytecodeLetras = TAKE;
		}

	if(string_equals_ignore_case(bytecode,"XXXX")){
			bytecodeLetras = XXXX;
		}

	if(string_equals_ignore_case(bytecode,"MALC")){
			bytecodeLetras = MALC;
		}

	if(string_equals_ignore_case(bytecode,"FREE")){
			bytecodeLetras = FREE;
		}

	if(string_equals_ignore_case(bytecode,"INNN")){
			bytecodeLetras = INNN;
		}

	if(string_equals_ignore_case(bytecode,"INNC")){
			bytecodeLetras = INNC;
		}

	if(string_equals_ignore_case(bytecode,"OUTN")){
			bytecodeLetras = OUTN;
		}

	if(string_equals_ignore_case(bytecode,"OUTC")){
			bytecodeLetras = OUTC;
		}

	if(string_equals_ignore_case(bytecode,"CREA")){
			bytecodeLetras = CREA;
		}

	if(string_equals_ignore_case(bytecode,"JOIN")){
			bytecodeLetras = JOIN;
		}

	if(string_equals_ignore_case(bytecode,"BLOK")){
			bytecodeLetras = BLOK;
		}

	if(string_equals_ignore_case(bytecode,"WAKE")){
			bytecodeLetras = WAKE;
		}

	return bytecodeLetras;
}


