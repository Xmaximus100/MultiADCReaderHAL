/*
 * adc_cli.c
 *
 *  Created on: Sep 4, 2025
 *      Author: Celelele
 */

#include "adc_cli.h"


extern ADC_Handler g_adc;
extern ADC_Handler *g_adc_mgr;

/*
 * List out all available commands with its description
 */

AT_StatusTypeDef Cmd_HELP(AT_CtxT *ctx, int argc, const char *argv[]){
	if (argc>0){
		AT_Help(ctx);
		AT_Puts(ctx, "Commands only work when sampling is stopped");
		return AT_OK;
	} else return AT_UNK;
}

/*
 * Setup ADC sampling frequency and number of connected converters.
 * Separate these values with a comma
 */

AT_StatusTypeDef Cmd_ADC_SETUP(AT_CtxT *ctx, int argc, const char *argv[]){
	if (ADC_BusyCheck()) return AT_BUSY;
	if (argc == 3) {
		uint32_t freq;
		uint32_t dev_amount;
		if (!AT_StrToUnsignedInt(argv[1], &freq)) return AT_ARG;
		if (!AT_StrToUnsignedInt(argv[2], &dev_amount)) return AT_ARG;
		if (freq < MAX_SAMPLING_FREQ) {
			LTC2368_ConfigSampling(&g_adc_mgr->clock_handler, freq);
		}
		else return AT_ARG;
		if (dev_amount < MAX_DEVICES) {
			ADC_ManagerInit(g_adc_mgr, (uint8_t)dev_amount);
		}
		else return AT_ARG;
		return AT_OK;
	} else return AT_UNK;
}

AT_StatusTypeDef Cmd_ADC_READ(AT_CtxT *ctx, int argc, const char *argv[]){
	if (ADC_BusyCheck()) return AT_BUSY;
	if (argc == 2) {

	} else return AT_UNK;
}

AT_StatusTypeDef Cmd_ADC_SAMPLING(AT_CtxT *ctx, int argc, const char *argv[]){
	if (ADC_BusyCheck()) return AT_BUSY;
	return AT_UNK;
}

AT_StatusTypeDef Cmd_ADC_START(AT_CtxT *ctx, int argc, const char *argv[]){
	if (ADC_BusyCheck()) return AT_BUSY;
	if (argc == 1){
		if (LTC2368_StartSampling(&g_adc_mgr->clock_handler) == LTC2368_OK) {
			g_adc_mgr->state = true;
			return AT_OK;
		} else return AT_ERROR;
	} else return AT_UNK;
}

AT_StatusTypeDef Cmd_ADC_STOP(AT_CtxT *ctx, int argc, const char *argv[]){
	if (!ADC_BusyCheck()) return AT_BUSY;
	if (argc == 1){
		if (LTC2368_StopSampling(&g_adc_mgr->clock_handler) == LTC2368_OK) {
			g_adc_mgr->state = false;
			return AT_OK;
		} else return AT_ERROR;
	} else return AT_UNK;
}

AT_StatusTypeDef Cmd_ADC_DISPLAY(AT_CtxT *ctx, int argc, const char *argv[]){
	if (ADC_BusyCheck()) return AT_BUSY;
	return AT_UNK;
}

AT_StatusTypeDef Cmd_ADC_FORMAT(AT_CtxT *ctx, int argc, const char *argv[]){
	if (ADC_BusyCheck()) return AT_BUSY;
	return AT_UNK;
//	uint8_t format; 							//display format 0-raw, 1-readable
//	uint32_t refresh_interval; 					//display periods
//	uint32_t display_samples; 					//samples to display at once
//	uint8_t state; 								//adc on/off
}

AT_StatusTypeDef Cmd_ADC_REFRESH(AT_CtxT *ctx, int argc, const char *argv[]){
	if (ADC_BusyCheck()) return AT_BUSY;
	if (argc == 2){
		uint32_t tmp_refresh;
		if (!AT_StrToUnsignedInt(argv[1], (uint32_t*)&tmp_refresh)) return AT_ARG;
		g_adc_mgr->refresh_interval = tmp_refresh;
	} else return AT_UNK;
}

void ADC_CommandInit(void){
	AT_Register("ADC:SETUP", 	Cmd_ADC_SETUP, 	"Setup: number of samples, sampling frequency, number of converters");
	AT_Register("ADC:READ", 	Cmd_ADC_READ, 	"Set requested number of samples or type 0 for continuous mode");
//	AT_Register("ADC:SAMPLING", Cmd_ADC_SAMPLING, help);
	AT_Register("ADC:START", 	Cmd_ADC_START, 	"Start sampling");
	AT_Register("ADC:STOP", 	Cmd_ADC_STOP, 	"Stop sampling");
	AT_Register("ADC:DISPLAY",	Cmd_ADC_DISPLAY,"Display n amount of samples or type 0 to print out whole buffer");
	AT_Register("ADC:FORMAT",	Cmd_ADC_FORMAT,	"Set display format: R or C for raw efficient or clear readable");
	AT_Register("ADC:MODE", 	Cmd_ADC_REFRESH,"Set refresh time of display data in ms");
	AT_Register("HELP", 		Cmd_HELP, 		"");
}
