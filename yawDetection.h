/*
 * yawDectection.h
 *
 *  Created on: 28/05/2018
 *      Author: tjl54
 */

#ifndef YAWDETECTION_H_
#define YAWDETECTION_H_

void changeYaw(int32_t changeValue);

//*************************************************************************
// Yaw Port & Pin Configuration
//*************************************************************************
void initYaw(void);

//*********************************************************
// Yaw Reference initialization
//*********************************************************
void initRef(void);

// Return calculated yaw value
int getYaw(void);

// Return boolean value storing state of yaw ref
int checkYawRef(void);

#endif /* YAWDETECTION_H_ */
