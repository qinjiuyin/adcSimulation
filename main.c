#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>

#define COMM_PORT "COM2"
#define NOISE_SCALE 0.1  // 噪声强度比例

#define PI 3.14159265358979323846

int UARTInit(void);
int SerialWrites(const HANDLE pCom, const char* pWritebuff, unsigned int Len);
HANDLE hSerial;

int main(void)
{
	const int period = 8192;  // 周期为8192毫秒
	srand(time(NULL));  // 初始化随机数生成器
	if (UARTInit())
		return 1;
	char SendBuff[4] = {0xff,0xff,0xff,0xff};
	while (1) {
		clock_t t = clock();  // 获取当前时钟周期数
		double value = 2047.5 * (1 + sin(2 * PI * t / (double)period));  // 计算正弦波函数的取值
		double noise = (double)rand() / RAND_MAX - 0.5;  // 生成随机噪声
		value += NOISE_SCALE * value * noise;  // 添加噪声
		int output = (int)value;  // 转换为整数
		SendBuff[1] = (output >> 8) & 0xff;
		SendBuff[2] = output & 0xff;
		SerialWrites(hSerial, SendBuff, sizeof(SendBuff));
		//printf("%d\n", output);
		while (clock() < t + (CLOCKS_PER_SEC / 1000));  // 暂停1毫秒，以控制输出速度
	}
	return 0;
}


int UARTInit(void)
{
	DCB dcbSerialParams = { 0 };
	COMMTIMEOUTS timeouts = { 0 };
	hSerial = CreateFile(COMM_PORT, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hSerial == INVALID_HANDLE_VALUE) {
		printf("无法打开串口\n");
		return 1;
	}

	// 设置串口参数
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(hSerial, &dcbSerialParams)) {
		printf("无法获取串口参数\n");
		CloseHandle(hSerial);
		return 1;
	}
	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.Parity = NOPARITY;
	dcbSerialParams.StopBits = ONESTOPBIT;
	if (!SetCommState(hSerial, &dcbSerialParams)) {
		printf("无法设置串口参数\n");
		CloseHandle(hSerial);
		return 1;
	}

	// 设置读超时和写超时
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	if (!SetCommTimeouts(hSerial, &timeouts)) {
		printf("无法设置超时\n");
		CloseHandle(hSerial);
		return 1;
	}
	printf("打开串口成功\n");
	return 0;
}

int SerialWrites(const HANDLE pCom, const char* pWritebuff, unsigned int Len)
{
	// 发送数据
	DWORD bytesWritten;
	if (!WriteFile(hSerial, pWritebuff, Len, &bytesWritten, NULL)) {
		printf("无法发送数据\n");
		CloseHandle(hSerial);
		return 1;
	}
}