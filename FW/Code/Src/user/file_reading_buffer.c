/*
 * file_reading_buffer.c
 *
 *  Created on: 17.07.2018
 *      Author: Michal Kowalik
 */

#include "user/file_reading_buffer.h"

/**
 * pFile - should be opened
 */
FileReadingBuffer_Status_TypeDef FileReadingBuffer_init(FileReadingBuffer_TypeDef* pSelf, FileSystemWrapper_File_TypeDef* pFile){

	if ((pSelf == NULL) || (pFile == NULL)){
		return FileReadingBuffer_Status_NullPointerError;
	}

	if (pSelf->state != FileReadingBuffer_State_UnInitialized){
		return FileReadingBuffer_Status_Error;
	}

	if (pFile == NULL){
		return FileReadingBuffer_Status_Error;
	}

	pSelf->pFile		= pFile;
	pSelf->iterator		= 0;
	pSelf->bytesLeft	= 0;

	pSelf->state = FileReadingBuffer_State_Initialized;

	return FileReadingBuffer_Status_OK;
}

static FileReadingBuffer_Status_TypeDef FileReadingBuffer_checkIfEmpty(FileReadingBuffer_TypeDef* pSelf){

	if (pSelf == NULL){
		return FileReadingBuffer_Status_NullPointerError;
	}

	if (pSelf->bytesLeft == 0){

		FileSystemWrapper_Status_TypeDef status = FileSystemWrapper_readData(pSelf->pFile, pSelf->buffer, FILE_READING_BUFFER_SIZE, &(pSelf->bytesLeft));

		if (pSelf->bytesLeft == 0){
			return FileReadingBuffer_Status_EndOfFile;
		}
		if (status != FileSystemWrapper_Status_OK){
			return FileReadingBuffer_Status_Error;
		}

	}

	return FileReadingBuffer_Status_OK;
}

FileReadingBuffer_Status_TypeDef FileReadingBuffer_readUInt8(FileReadingBuffer_TypeDef* pSelf, uint8_t* pReturnValue){

	if ((pSelf == NULL) || (pReturnValue == NULL)){
		return FileReadingBuffer_Status_NullPointerError;
	}

	if (pSelf->state != FileReadingBuffer_State_Initialized){
		return FileReadingBuffer_Status_UnInitializedError;
	}

	FileReadingBuffer_Status_TypeDef status;
	if ((status = FileReadingBuffer_checkIfEmpty(pSelf)) != FileReadingBuffer_Status_OK){
		return status;
	}

	*pReturnValue = pSelf->buffer[(pSelf->iterator)%FILE_READING_BUFFER_SIZE];
	pSelf->iterator++;
	pSelf->bytesLeft--;

	return FileReadingBuffer_Status_OK;

}

FileReadingBuffer_Status_TypeDef FileReadingBuffer_readUInt16(FileReadingBuffer_TypeDef* pSelf, uint16_t* pReturnValue){

	if ((pSelf == NULL) || (pReturnValue == NULL)){
		return FileReadingBuffer_Status_NullPointerError;
	}

	if (pSelf->state != FileReadingBuffer_State_Initialized){
		return FileReadingBuffer_Status_UnInitializedError;
	}

	uint8_t	tempValue = 0;
	FileReadingBuffer_Status_TypeDef status;

	*pReturnValue = 0;

	if ((status = FileReadingBuffer_readUInt8(pSelf, &tempValue)) != FileReadingBuffer_Status_OK){
		return status;
	}

	*pReturnValue |= tempValue;

	if ((status = FileReadingBuffer_readUInt8(pSelf, &tempValue)) != FileReadingBuffer_Status_OK){
		return status;
	}

	*pReturnValue |= (((uint16_t)tempValue) << 8);

	return FileReadingBuffer_Status_OK;
}

FileReadingBuffer_Status_TypeDef FileReadingBuffer_readUInt32(FileReadingBuffer_TypeDef* pSelf, uint32_t* pReturnValue){

	if ((pSelf == NULL) || (pReturnValue == NULL)){
		return FileReadingBuffer_Status_NullPointerError;
	}

	if (pSelf->state != FileReadingBuffer_State_Initialized){
		return FileReadingBuffer_Status_UnInitializedError;
	}

	uint16_t	tempValue = 0;
	FileReadingBuffer_Status_TypeDef status;

	*pReturnValue = 0;

	if ((status = FileReadingBuffer_readUInt16(pSelf, &tempValue)) != FileReadingBuffer_Status_OK){
		return status;
	}

	*pReturnValue |= tempValue;

	if ((status = FileReadingBuffer_readUInt16(pSelf, &tempValue)) != FileReadingBuffer_Status_OK){
		return status;
	}

	*pReturnValue |= (((uint32_t)tempValue) << 16);

	return FileReadingBuffer_Status_OK;
}

FileReadingBuffer_Status_TypeDef FileReadingBuffer_readUInt64(FileReadingBuffer_TypeDef* pSelf, uint64_t* pReturnValue){

	if ((pSelf == NULL) || (pReturnValue == NULL)){
		return FileReadingBuffer_Status_NullPointerError;
	}

	if (pSelf->state != FileReadingBuffer_State_Initialized){
		return FileReadingBuffer_Status_UnInitializedError;
	}

	uint32_t	tempValue = 0;
	FileReadingBuffer_Status_TypeDef status;

	*pReturnValue = 0;

	if ((status = FileReadingBuffer_readUInt32(pSelf, &tempValue)) != FileReadingBuffer_Status_OK){
		return status;
	}

	*pReturnValue |= tempValue;

	if ((status = FileReadingBuffer_readUInt32(pSelf, &tempValue)) != FileReadingBuffer_Status_OK){
		return status;
	}

	*pReturnValue |= (((uint64_t)tempValue) << 32);

	return FileReadingBuffer_Status_OK;
}

FileReadingBuffer_Status_TypeDef FileReadingBuffer_readInt8(FileReadingBuffer_TypeDef* pSelf, int8_t* pReturnValue){

	if ((pSelf == NULL) || (pReturnValue == NULL)){
		return FileReadingBuffer_Status_NullPointerError;
	}

	if (pSelf->state != FileReadingBuffer_State_Initialized){
		return FileReadingBuffer_Status_UnInitializedError;
	}

	FileReadingBuffer_Status_TypeDef status;
	if ((status = FileReadingBuffer_checkIfEmpty(pSelf)) != FileReadingBuffer_Status_OK){
		return status;
	}

	*pReturnValue = *((int8_t*)&(pSelf->buffer[(pSelf->iterator)%FILE_READING_BUFFER_SIZE])); //< reinterpret_cast like
	pSelf->iterator++;
	pSelf->bytesLeft--;

	return FileReadingBuffer_Status_OK;
}

FileReadingBuffer_Status_TypeDef FileReadingBuffer_readChar(FileReadingBuffer_TypeDef* pSelf, char* pReturnValue){

	if ((pSelf == NULL) || (pReturnValue == NULL)){
		return FileReadingBuffer_Status_NullPointerError;
	}

	if (pSelf->state != FileReadingBuffer_State_Initialized){
		return FileReadingBuffer_Status_UnInitializedError;
	}

	return FileReadingBuffer_readUInt8(pSelf, (uint8_t*) pReturnValue);

}

FileReadingBuffer_Status_TypeDef FileReadingBuffer_readString(FileReadingBuffer_TypeDef* pSelf, char* pReturnString, uint16_t length){

	if ((pSelf == NULL) || (pReturnString == NULL)){
		return FileReadingBuffer_Status_NullPointerError;
	}

	if (pSelf->state != FileReadingBuffer_State_Initialized){
		return FileReadingBuffer_Status_UnInitializedError;
	}

	FileReadingBuffer_Status_TypeDef status;

	for(uint32_t i = 0; i < length; i++){

		if ((status = FileReadingBuffer_readChar(pSelf, pReturnString+i)) != FileReadingBuffer_Status_OK){
			return status;
		}
	}

	return FileReadingBuffer_Status_OK;

}

FileReadingBuffer_Status_TypeDef FileReadingBuffer_skipBytes(FileReadingBuffer_TypeDef* pSelf, uint16_t length){

	if (pSelf == NULL){
		return FileReadingBuffer_Status_NullPointerError;
	}

	if (pSelf->state != FileReadingBuffer_State_Initialized){
		return FileReadingBuffer_Status_UnInitializedError;
	}

	FileReadingBuffer_Status_TypeDef	status;
	uint8_t								buffer;

	for(uint16_t i = 0; i < length; i++){

		if ((status = FileReadingBuffer_readUInt8(pSelf, &buffer)) != FileReadingBuffer_Status_OK){
			return status;
		}
	}

	return FileReadingBuffer_Status_OK;

}
