/*
 * funcionesCPU.c
 *
 *  Created on: 07/09/2014
 *      Author: utnso
 */

#include "funcionesCPU.h"

int retardo;

void leer_configuracion(){
	config_cpu = config_create(PATH);
	char* direccion_ip_kernel;
	char* direccion_ip_msp;
	int puerto_kernel, puerto_msp, ret;

	direccion_ip_kernel = config_get_string_value(config_cpu, "IP_KERNEL");
	puerto_kernel = config_get_int_value(config_cpu,"PUERTO_KERNEL");
	log_info(logger,"La direccion IP del kernel es:%s",direccion_ip_kernel);
	log_info(logger,"El puerto del kernel es:%d",puerto_kernel);

	config_struct_cpu.ip_kernel = direccion_ip_kernel;
	config_struct_cpu.puerto_kernel = puerto_kernel;

	direccion_ip_msp = config_get_string_value(config_cpu, "IP_MSP");
	puerto_msp = config_get_int_value(config_cpu,"PUERTO_MSP");
	log_info(logger,"La direccion IP de la MSP es:%s",direccion_ip_msp);
	log_info(logger,"El puerto de la MSP es:%d",puerto_msp);


	config_struct_cpu.ip_msp = direccion_ip_msp;
	config_struct_cpu.puerto_msp = puerto_msp;

	ret = config_get_int_value(config_cpu,"RETARDO");
	log_info(logger,"El retardo es:%d",ret);
	retardo = ret;

	config_struct_cpu.retardo = retardo;

}

void inicializar_configuracion(){
	logger = malloc(sizeof(t_log));
	inicializar_panel(CPU,"/home/utnso/tp-2014-2c-los-metatecla/");
	struct stat config_file;
	int control = lstat(PATH,&config_file);
	if(control == -1){
		log_error(logger, "No existe el archivo de configuracion");
	} else{
		leer_configuracion();
	}

}




