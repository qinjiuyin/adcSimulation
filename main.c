#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>

#define COMM_PORT "COM2"
#define NOISE_SCALE 0.1  // ����ǿ�ȱ���

#define PI 3.14159265358979323846

int UARTInit(void);
int SerialWrites(const HANDLE pCom, const char* pWritebuff, unsigned int Len);
HANDLE hSerial;

int main(void)
{
	const int period = 8192;  // ����Ϊ8192����
	srand(time(NULL));  // ��ʼ�������������
	if (UARTInit())
		return 1;
	char SendBuff[4] = {0xff,0xff,0xff,0xff};
	while (1) {
		clock_t t = clock();  // ��ȡ��ǰʱ��������
		double value = 2047.5 * (1 + sin(2 * PI * t / (double)period));  // �������Ҳ�������ȡֵ
		double noise = (double)rand() / RAND_MAX - 0.5;  // �����������
		value += NOISE_SCALE * value * noise;  // �������
		int output = (int)value;  // ת��Ϊ����
		SendBuff[1] = (output >> 8) & 0xff;
		SendBuff[2] = output & 0xff;
		SerialWrites(hSerial, SendBuff, sizeof(SendBuff));
		//printf("%d\n", output);
		while (clock() < t + (CLOCKS_PER_SEC / 1000));  // ��ͣ1���룬�Կ�������ٶ�
	}
	return 0;
}


int UARTInit(void)
{
	DCB dcbSerialParams = { 0 };
	COMMTIMEOUTS timeouts = { 0 };
	hSerial = CreateFile(COMM_PORT, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hSerial == INVALID_HANDLE_VALUE) {
		printf("�޷��򿪴���\n");
		return 1;
	}

	// ���ô��ڲ���
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(hSerial, &dcbSerialParams)) {
		printf("�޷���ȡ���ڲ���\n");
		CloseHandle(hSerial);
		return 1;
	}
	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.Parity = NOPARITY;
	dcbSerialParams.StopBits = ONESTOPBIT;
	if (!SetCommState(hSerial, &dcbSerialParams)) {
		printf("�޷����ô��ڲ���\n");
		CloseHandle(hSerial);
		return 1;
	}

	// ���ö���ʱ��д��ʱ
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	if (!SetCommTimeouts(hSerial, &timeouts)) {
		printf("�޷����ó�ʱ\n");
		CloseHandle(hSerial);
		return 1;
	}
	printf("�򿪴��ڳɹ�\n");
	return 0;
}

int SerialWrites(const HANDLE pCom, const char* pWritebuff, unsigned int Len)
{
	// ��������
	DWORD bytesWritten;
	if (!WriteFile(hSerial, pWritebuff, Len, &bytesWritten, NULL)) {
		printf("�޷���������\n");
		CloseHandle(hSerial);
		return 1;
	}
}