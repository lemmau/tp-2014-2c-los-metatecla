/*
 * functions.h
 *
 *  Created on: 07/10/2014
 *      Author: utnso
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

	#include "../estructuras_auxiliares.h"

	t_header crearHeader(uint8_t tipoEstructura, uint16_t lengthDatos);
	char * crearDataConHeader(uint8_t tipoEstructura, int length);
	t_stream * serialize(int tipoEstructura, void * estructuraOrigen);

	t_stream * serializeStruct_numero(t_struct_numero * estructuraOrigen);
	t_stream * serializeStruct_direccion(t_struct_direccion * estructuraOrigen);
	t_stream * serializeStruct_char(t_struct_char * estructuraOrigen);
	t_stream * serializeStruct_string(t_struct_string * estructuraOrigen);

	t_header despaquetizarHeader(char * header);
	void * deserialize(uint8_t tipoEstructura, char * dataPaquete, uint16_t length);

	t_struct_numero * deserializeStruct_numero(char * dataPaquete, uint16_t length);
	t_struct_direccion * deserializeStruct_direccion(char * dataPaquete, uint16_t length);
	t_struct_char * deserializeStruct_char(char * dataPaquete, uint16_t length);
	t_struct_string * deserializeStruct_string(char * dataPaquete, uint16_t length);



#endif /* FUNCTIONS_H_ */