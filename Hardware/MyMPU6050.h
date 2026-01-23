#ifndef __MYMPU6050_H
#define __MYMPU6050_H

void quat2euler(float q0,float q1,float q2,float q3,
	float* phi, float* theta, float* psi);
void MPU6050_Calculation(void);
int MPU6050(void);

#endif
