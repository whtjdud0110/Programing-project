#include <stdio.h>
#include <direct.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

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

int validHour(int h) //시 검사
{
	return (h >= 0 && h <= 23);
}
int validMinute(int m) //분 검사
{
	return (m >= 0 && m <= 59);
}

int validDate(const char* date) //날짜 검사
{
	if (strlen(date) != 10) return 0;
	if (date[4] != '-' || date[7] != '-') return 0;
	return 1;
}

Sleepdata records[MAX_RECORDS]; //모든 기록을 저장할 배열
int recordCount = 0; //현재 기록수 
int dailyGoalMinutes = 0;

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

int calcScore(int sleepMinutes)
{
	int diff = abs(sleepMinutes - dailyGoalMinutes);
	int score = 100 - (diff / 30) * 10;
	if (score < 0) score = 0;
	return score;
}

void alertIfNeeded(int sleepMinutes)
{
	if (sleepMinutes < dailyGoalMinutes *0.5)
	{
		printf("\a[경고] 수면 시간이 매우 부족합니다!\n");
	}
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

int calcDeficit(int startIndex, int endIndex)
{
	int totalMinutes = 0;
	for (int i = startIndex; i <= endIndex && i < recordCount; i++)
		totalMinutes += records[i].sleepMinutes;
	int expected = (endIndex - startIndex + 1) * dailyGoalMinutes;
	return expected - totalMinutes;
}

void printMaxMin()
{
	if (recordCount == 0) return;
	int maxIdx = 0, minIdx = 0;
	for (int i = 1; i < recordCount; i++)
	{
		if (records[i].sleepMinutes > records[maxIdx].sleepMinutes) maxIdx = i;
		if (records[i].sleepMinutes < records[minIdx].sleepMinutes) minIdx = i;
	}
	printf("최장 수면: %s %d분\n", records[maxIdx].date, records[maxIdx].sleepMinutes);
	printf("최단 수면: %s %d분\n", records[minIdx].date, records[minIdx].sleepMinutes);
}

double calcAverage(int startIndex, int endIndex)
{
	if (startIndex > endIndex || startIndex >= recordCount) return 0.0;
	int sum = 0, count = 0;
	for (int i = startIndex; i <= endIndex && i < recordCount; i++)
	{
		sum += records[i].sleepMinutes;
		count++;
	}
	if (count == 0) return 0.0;
	return (double)sum / count;
}

void addRecord() //기록 추가
{
	if (recordCount >= MAX_RECORDS)
	{
		printf("더 이상 기록을 추가할 수 없습니다.\n");
		return;
	}

	Sleepdata newRec;
	printf("날짜 입력 (YYYY-MM-DD, 비워두면 오늘 자동 입력): ");
	scanf("%10s", newRec.date);

	if (!validDate(newRec.date))
	{
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		sprintf(newRec.date, "%04d-%02d-%02d", tm.tm_year+1900, tm.tm_mon + 1, tm.tm_mday);
	}

	do
	{
		printf("취침 시간 입력 (시 분): ");
		scanf("%d %d", &newRec.startHour, &newRec.startMin);
	} while (!validHour(newRec.startHour) || !validMinute(newRec.startMin));

	do
	{
		printf("기상 시간 입력 (시 분): ");
		scanf("%d %d", &newRec.endHour, &newRec.endMin);
	} while (!validHour(newRec.endHour) || !validMinute(newRec.endMin));


	// 수면 시간 계산
	int startTotal = newRec.startHour * 60 + newRec.startMin;
	int endTotal = newRec.endHour * 60 + newRec.endMin;
	if (endTotal < startTotal) // 다음날 기상 시
		endTotal += 24 * 60;
	newRec.sleepMinutes = endTotal - startTotal;

	alertIfNeeded(newRec.sleepMinutes);

	records[recordCount++] = newRec;
	saveData();

	printf("기록이 추가되었습니다. 수면 점수: %d\n", calcScore(newRec.sleepMinutes));
}

void showMenu() //메뉴 출력
{
	printf("\n==== 수면 관리 프로그램 ====\n");
	printf("1. 기록 보기\n");
	printf("2. 기록 추가\n");
	printf("3. 주간/월간 통계\n");
	printf("4. 종료\n");
	printf("===========================\n");
	printf("선택: ");
}

void showStats()
{
	if (recordCount == 0)
	{
		printf("기록이 없습니다.\n");
		return;
	}

	printMaxMin();

	int startWeek = recordCount >= 7 ? recordCount - 7 : 0;
	double weeklyAvg = calcAverage(startWeek, recordCount - 1);
	int deficitWeek = calcDeficit(startWeek, recordCount - 1);
	printf("주간 평균 수면: %.2f분, 목표 대비 부족: %d분\n", weeklyAvg, deficitWeek);
	if (deficitWeek > 0) printf("\a[경고] 주간 목표 수면 미달!\n");

	int startMonth = recordCount >= 30 ? recordCount - 30 : 0;
	double monthlyAvg = calcAverage(startMonth, recordCount - 1);
	int deficitMonth = calcDeficit(startMonth, recordCount - 1);
	printf("월간 평균 수면: %.2f분, 목표 대비 부족 : %d분\n", monthlyAvg, deficitMonth);
	if (deficitMonth > 0)printf("\a[경고] 월간 목표 수면 미달!\n");
}

int main()
{	
	printf("하루 목표 수면시간 입력(시간): ");
	double goalHour;
	scanf("%lf", &goalHour);
	dailyGoalMinutes = (int)(goalHour) * 60;

	loadData(); //기존 데이터 불러오기

	int choice;
	while (1)
	{
		showMenu();
		scanf("%d", &choice);
		while (getchar() != '\n');

		switch (choice)
		{
		case 1:
			viewRecords();
			break;
		case 2:
			addRecord();
			break;
		case 3:
			showStats();
			break;
		case 4:
			printf("프로그램을 종료합니다.\n");
			return 0;
		default:
			printf("잘못된 선택입니다. 다시 입력하세요.\n");
		}
	}

	return 0;
}