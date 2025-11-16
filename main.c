#include <stdio.h>
#include <direct.h>
#include <string.h>
#include <stdlib.h>
#define MAX_RECORDS 100

typedef struct
{
	char date[11]; //2025-11-14
	int startHour; //취침 (시)
	int startMin; //취침 (분)
	int endHour;//기상 (시)
	int endMin; //기상 (분)
	int sleepMinutes;
} Sleepdata;

Sleepdata records[MAX_RECORDS]; //모든 기록을 저장할 배열
int recordCount = 0; //현재 기록수 

void loadData() //데이터 불러오기
{
	FILE* fp = fopen("sleep_data.csv", "r");
	if (fp == NULL)
	{
		printf("기존 데이터 파일이 없습니다. 새로 생성합니다.\n");
		return;
	}

	Sleepdata temp;

	while (fscanf(fp, "%10[^,],%d:%d,%d:%d,%d\n", temp.date, &temp.startHour, &temp.startMin, &temp.endHour, &temp.endMin, &temp.sleepMinutes) == 6)
	{
		printf("불러온 데이터 : %s %02d:%02d ~ %02d:%02d (%d분)\n", temp.date, temp.startHour, temp.startMin, temp.endHour, temp.endMin, temp.sleepMinutes);
		
		if (recordCount < MAX_RECORDS)
			records[recordCount++] = temp;
	}
	fclose(fp); 
} 

void saveData() //데이터 저장하기
{
	FILE* fp = fopen("sleep_data.csv", "w");
	if (fp == NULL)
	{
		printf("파일 저장 실패!\n");
		return;
	}

	for (int i = 0; i < recordCount; i++)
	{
		fprintf(fp, "%s,%02d:%02d,%02d:%02d,%d\n",
			records[i].date,
			records[i].startHour, records[i].startMin,
			records[i].endHour, records[i].endMin,
			records[i].sleepMinutes);
	}
	fclose(fp);
}

void viewRecords() //기록 보기
{
	if (recordCount == 0)
	{
		printf("기록이 없습니다.\n");
		return;
	}
	printf("\n=== 수면 기록 ===\n");
	for (int i = 0; i < recordCount; i++)
	{
		printf("%d. %s %02d:%02d ~ %02d:%02d (%d분)\n",
			i + 1,
			records[i].date,
			records[i].startHour, records[i].startMin,
			records[i].endHour, records[i].endMin,
			records[i].sleepMinutes);
	}
	printf("================\n");
}

void addRecord() //기록 추가
{
	if (recordCount >= MAX_RECORDS)
	{
		printf("더 이상 기록을 추가할 수 없습니다.\n");
		return;
	}
	Sleepdata newRec;

	printf("날짜 입력 (YYYY-MM-DD): ");
	scanf("%10s", newRec.date);

	// 취침 시간 입력
	printf("취침 시간 입력 (시 분): ");
	scanf("%d %d", &newRec.startHour, &newRec.startMin);

	// 기상 시간 입력
	printf("기상 시간 입력 (시 분): ");
	scanf("%d %d", &newRec.endHour, &newRec.endMin);

	// 수면 시간 계산
	int startTotal = newRec.startHour * 60 + newRec.startMin;
	int endTotal = newRec.endHour * 60 + newRec.endMin;
	if (endTotal < startTotal) // 다음날 기상 시
		endTotal += 24 * 60;

	newRec.sleepMinutes = endTotal - startTotal;

	// 배열에 저장
	records[recordCount++] = newRec;

	// CSV 파일 저장
	saveData();

	printf("기록이 추가되었습니다.\n");
}

void showMenu() //메뉴 출력
{
	printf("\n==== 수면 관리 프로그램 ====\n");
	printf("1. 기록 보기\n");
	printf("2. 기록 추가\n");
	printf("3. 종료\n");
	printf("===========================\n");
	printf("선택: ");
}



int main()
{	
	loadData(); //기존 데이터 불러오기

	int choice;
	while (1)
	{
		showMenu();
		scanf("%d", &choice);

		switch (choice)
		{
		case 1:
			viewRecords();
			break;
		case 2:
			addRecord();
			break;
		case 3:
			printf("프로그램을 종료합니다.\n");
			return 0;
		default:
			printf("잘못된 선택입니다. 다시 입력하세요.\n");
		}
	}

	return 0;
}