/*
 * can_receiver.c
 *
 *  Created on: 03.06.2017
 *      Author: Michal Kowalik
 */

#include "user/can_receiver.h"
#include "main.h"

//< ----- Private functions prototypes ----- >//

CANReceiver_Status_TypeDef	CANReceiver_RxCallback(CANReceiver_TypeDef* pSelf, CANData_TypeDef* pData);
void						CANReceiver_RxCallbackWrapper(CANData_TypeDef* pData, void* pVoidSelf);

CANReceiver_Status_TypeDef	CANReceiver_ErrorCallback(CANReceiver_TypeDef* pSelf, CANTransceiverDriver_ErrorCode_TypeDef errorcode);
void						CANReceiver_ErrorCallbackWrapper(CANTransceiverDriver_ErrorCode_TypeDef errorcode, void* pVoidSelf);

//< ----- Public functions ----- >//

CANReceiver_Status_TypeDef CANReceiver_init(CANReceiver_TypeDef* pSelf, Config_TypeDef* pConfig, CANTransceiverDriver_TypeDef* pCanTransceiverHandler, MSTimerDriver_TypeDef* pMsTimerDriverHandler){

	if ((pSelf == NULL) || (pConfig == NULL) || (pCanTransceiverHandler == NULL) || (pMsTimerDriverHandler == NULL)){
		return CANReceiver_Status_NullPointerError;
	}

	pSelf->state					= CANReceiver_State_NotInitialised;
	pSelf->pConfig					= pConfig;
	pSelf->pCanTransceiverHandler	= pCanTransceiverHandler;
	pSelf->pMsTimerDriverHandler	= pMsTimerDriverHandler;

	if (FIFOQueue_init(&(pSelf->framesFIFO), pSelf->aReceiverQueueBuffer, sizeof(CANData_TypeDef), CAN_MSG_QUEUE_SIZE) != FIFO_Status_OK){ //TODO czy alignment nie popusje sizeof
		return CANReceiver_Status_FIFOError;
	}

	for (uint16_t i=0; i<CAN_MSG_QUEUE_SIZE; i++){
		pSelf->aReceiverQueueBuffer[i] = (CANData_TypeDef){0};
	}

	uint16_t aFilterIDsTab[pConfig->numOfFrames];

	for (uint16_t i=0; i<pConfig->numOfFrames; i++){
		aFilterIDsTab[i] = pConfig->canFrames[i].ID;
	}

	if (CANTransceiverDriver_configFiltering(pSelf->pCanTransceiverHandler, aFilterIDsTab, pConfig->numOfFrames) != CANTransceiverDriver_Status_OK){
		return CANReceiver_Status_CANTransceiverDriverError;
	}

	if (CANTransceiverDriver_registerReceiveCallbackToCall(pSelf->pCanTransceiverHandler, CANReceiver_RxCallbackWrapper, (void*) pSelf) != CANTransceiverDriver_Status_OK){
		return CANReceiver_Status_CANTransceiverDriverError;
	}

	if (CANTransceiverDriver_registerErrorCallbackToCall(pSelf->pCanTransceiverHandler, CANReceiver_ErrorCallbackWrapper, (void*) pSelf) != CANTransceiverDriver_Status_OK){
		return CANReceiver_Status_CANTransceiverDriverError;
	}

	pSelf->state = CANReceiver_State_Initialised;

	return CANReceiver_Status_OK;

}

CANReceiver_Status_TypeDef CANReceiver_start(CANReceiver_TypeDef* pSelf){

	if (pSelf == NULL){
		return CANReceiver_Status_NullPointerError;
	}

	if (pSelf->state == CANReceiver_State_NotInitialised){
		return CANReceiver_Status_NotInitialisedError;
	}

	if (pSelf->state == CANReceiver_State_Running){
		return CANReceiver_Status_AlreadyRunningError;
	}

	if (CANTransceiverDriver_start(pSelf->pCanTransceiverHandler) != CANTransceiverDriver_Status_OK){
		return CANReceiver_Status_CANTransceiverDriverError;
	}

	pSelf->state = CANReceiver_State_Running;

	return CANReceiver_Status_OK;
}

CANReceiver_Status_TypeDef CANReceiver_pullLastFrame(CANReceiver_TypeDef* pSelf, CANData_TypeDef* pRetMsg){

	if ((pSelf == NULL) || (pRetMsg == NULL)){
		return CANReceiver_Status_NullPointerError;
	}

	if (pSelf->state != CANReceiver_State_Running){
		return CANReceiver_Status_NotRunningError;
	}

	FIFO_Status_TypeDef fifoStatus = FIFO_Status_OK;

	fifoStatus = FIFOQueue_dequeue(&(pSelf->framesFIFO), pRetMsg);

	switch(fifoStatus){
		case FIFO_Status_OK:
			return CANReceiver_Status_OK;
		case FIFO_Status_Empty:
			return CANReceiver_Status_Empty;
		case FIFO_Status_Full:
			return CANReceiver_Status_FullFIFOError;
		case FIFO_Status_DequeueInProgressError:
		case FIFO_Status_UnInitializedError:
		case FIFO_Status_Error:
		default:
			return CANReceiver_Status_FIFOError;
	}

	return CANReceiver_Status_OK;
}

CANReceiver_Status_TypeDef CANReceiver_reset(CANReceiver_TypeDef* pSelf){

	if (pSelf == NULL){
		return CANReceiver_Status_NullPointerError;
	}

	if (pSelf->state == CANReceiver_State_NotInitialised){
		return CANReceiver_Status_NotInitialisedError;
	}

	if (FIFOQueue_clear(&(pSelf->framesFIFO)) != FIFO_Status_OK){
		return CANReceiver_Status_FIFOError;
	}

	return CANReceiver_Status_OK;
}

//< ----- Callback functions ----- >//

CANReceiver_Status_TypeDef CANReceiver_RxCallback(CANReceiver_TypeDef* pSelf, CANData_TypeDef* pData){

	if (MSTimerDriver_getMSTime(pSelf->pMsTimerDriverHandler, &pData->msTime) != MSTimerDriver_Status_OK){ //TODO trzeba tu wykorzystac ten czas z CANa
		return CANReceiver_Status_MSTimerError;
	}

	FIFO_Status_TypeDef fifoStatus;
	if ((fifoStatus = FIFOQueue_enqueue(&(pSelf->framesFIFO), pData)) != FIFO_Status_OK){
		return CANReceiver_Status_FIFOError;	//TODO moze jak sie nie zmiesci do kolejki, to nie Error tylko jakas sytuacja wyjatkowa???
	}

	return CANReceiver_Status_OK;

}

void CANReceiver_RxCallbackWrapper(CANData_TypeDef* pData, void* pVoidSelf){
	CANReceiver_Status_TypeDef ret = CANReceiver_Status_OK;
	if((ret = CANReceiver_RxCallback((CANReceiver_TypeDef*) pVoidSelf, pData)) != CANReceiver_Status_OK){
		Error_Handler();
	}

}

CANReceiver_Status_TypeDef CANReceiver_ErrorCallback(CANReceiver_TypeDef* pSelf, CANTransceiverDriver_ErrorCode_TypeDef errorcode){

	uint32_t msTime;
	if (MSTimerDriver_getMSTime(pSelf->pMsTimerDriverHandler, &msTime) != MSTimerDriver_Status_OK){ //TODO trzeba tu wykorzystac ten czas z CANa
		return CANReceiver_Status_MSTimerError;
	}

	if (errorcode != CANTransceiverDriver_ErrorCode_None){

		//TODO

	}

	return CANReceiver_Status_OK;

}

void CANReceiver_ErrorCallbackWrapper(CANTransceiverDriver_ErrorCode_TypeDef errorcode, void* pVoidSelf){

	if (CANReceiver_ErrorCallback((CANReceiver_TypeDef*) pVoidSelf, errorcode) != CANReceiver_Status_OK){
		Error_Handler();
	}

}
