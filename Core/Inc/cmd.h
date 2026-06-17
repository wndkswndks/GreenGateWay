#ifndef CMD_H
#define CMD_H

/*  			include start  			*/
#include "common.h"
/*  			include end  			*/



/*  			define start  			*/
#define RX_BUFF_SIZE	30
   // ========================================================================================
        // 1. P 계열 순수 고정 길이 명령어 패킷 총 크기 (헤더 18 + 바디 + CRC 2)
        // ========================================================================================


#define   TOTAL_LEN_PDUH5     43   // [5] ★복구: 저장자료 요청 (18 + 바디 10 + 10 + 2)
#define   TOTAL_LEN_PFST7     24   // [7] 미전송자료 전송시간 변경 요청 (18 + 바디 4 + 2) [cite: 921]
#define   TOTAL_LEN_PSEP8     36   // [8] 비밀번호 변경 요청 (18 + 바디 16(암호화) + 2) [cite: 952, 954]
#define   TOTAL_LEN_PTIM9     32   // [9] 서버시간 응답 (18 + 바디 12 + 2) [cite: 983]
#define   TOTAL_LEN_PUPG10    164  // [10] GW 업그레이드 요청 (18 + 바디 131(암호화) + 2) [cite: 1006, 1013]
#define   TOTAL_LEN_PVER11    20   // [11] 버전정보 요청 (바디 없음: 18 + 0 + 2) [cite: 1045]
#define   TOTAL_LEN_PSET12    32   // [12] GW 시간 변경 요청 (18 + 바디 12 + 2) [cite: 1070]
#define   TOTAL_LEN_PFCC13    30   // [13] 시설코드 변경 요청 (18 + 바디 5 + 5 + 2) [cite: 1097]
#define   MIN_LEN_PAST14(N)   (24*N + 22)//   // [14] ★추가: 측정범위 변경 (헤더 18 + 항목수 2 + 반복 블록 24*1 + CRC 2)
#define   TOTAL_LEN_PFCR15    20   // [15] 관계정보 조회 요청 (바디 없음: 18 + 0 + 2) [cite: 1142]
#define   MIN_LEN_PFRS16(N)   (10*N + 22)//    // [16] ★추가: 관계정보 변경 (헤더 18 + 관계수 2 + 반복 블록 10*1 + CRC 2)
#define   TOTAL_LEN_PRSI17    36   // [17] 통신서버IP 변경 요청 (18 + 바디 16(암호화) + 2) [cite: 1188, 1192]
#define   TOTAL_LEN_PDAT18     21   // [18] 자료전송모드 변경 요청 (18 + 바디 1 + 2) [cite: 1214]
#define   TOTAL_LEN_PODT19    26   // [19] 유예시간 설정 변경 요청 (18 + 바디 3 + 3 + 2) [cite: 1237]
#define   TOTAL_LEN_PCN2_20    20   // [20] 설정정보 요청 (바디 없음: 18 + 0 + 2) [cite: 1264]
#define   TOTAL_LEN_PRBT22    20   // [22] GW 재기동 요청 (바디 없음: 18 + 0 + 2) [cite: 1330]

#define   TOTAL_LEN_TDAH1(N)    	 (35+15*N)
#define   TOTAL_LEN_TOFH2(N)    	 (34+8*N)
#define   TOTAL_LEN_TDDH3(N)    	 (42+21*N)
#define   TOTAL_LEN_TFDH4(N)   	     (35+15*N)
#define   TOTAL_LEN_TDUH5(N)    	 (35+15*N)
#define   TOTAL_LEN_TNOH6(N)    	 (35+8*N)
#define   TOTAL_LEN_TTIM9    	 	 20
#define   TOTAL_LEN_TUPG10   	 	 126
#define   TOTAL_LEN_TVER11   	 	 126
#define   TOTAL_LEN_TFCR15(N)   	 (22+10*N)
#define   TOTAL_LEN_TFCR16(N)   	 (22+10*N)
#define   TOTAL_LEN_TCN2_20(N)   	 (155+24*N)

//YYMMDDhhmm
#define YY(T) (((T) / 100000000) % 100)
#define MM(T) (((T) / 1000000) % 100)
#define DD(T) (((T) / 10000) % 100)
#define hh(T) (((T) / 100) % 100)
#define mm(T) ((T) % 100)





//서버 rx 파싱용 ==========================


#define  MIN_COMM_2		0
#define  MAX_COMM_2		9999999
#define  MIN_COMM_3		0
#define  MAX_COMM_3		9


#define  MIN_5_PDUH_6		2606030155
#define  MAX_5_PDUH_6		4206030155
#define  MIN_5_PDUH_7		2606030155
#define  MAX_5_PDUH_7		4206030155

#define  MIN_7_PFST_5		0
#define  MAX_7_PFST_5		9999

#define  MIN_8_PSEP_5		0
#define  MAX_8_PSEP_5		4200000000


#define  MIN_9_PTIM_5_1		260603
#define  MAX_9_PTIM_5_1		360603
#define  MIN_9_PTIM_5_2		0
#define  MAX_9_PTIM_5_2		240000

#define  MIN_10_PUPG_5		0
#define  MAX_10_PUPG_5		9
#define  MIN_10_PUPG_7		0
#define  MAX_10_PUPG_7		99999


#define  MIN_12_PSET_5_1		260603
#define  MAX_12_PSET_5_1		360603
#define  MIN_12_PSET_5_2		0
#define  MAX_12_PSET_5_2		240000

#define  MIN_14_PAST_5		0
#define  MAX_14_PAST_5		99
#define  MIN_14_PAST_8		0.0
#define  MAX_14_PAST_8		999.99
#define  MIN_14_PAST_9		0
#define  MAX_14_PAST_9		999.99
#define  MIN_14_PAST_10		0
#define  MAX_14_PAST_10		999.99

#define  MIN_16_PFRS_5		0
#define  MAX_16_PFRS_5		99

#define  MIN_18_PDAT_5		0
#define  MAX_18_PDAT_5		2

#define  MIN_19_PODT_5		0
#define  MAX_19_PODT_5		999
#define  MIN_19_PODT_6		0
#define  MAX_19_PODT_6		999
//==========================


#define CMD_TDAH	"TDAH"
#define CMD_TOFH	"TOFH"
#define CMD_TDDH	"TDDH"
#define CMD_TFDH	"TFDH"
#define CMD_TDUH	"TDUH"
#define CMD_TNOH	"TNOH"
#define CMD_TTIM	"TTIM"
#define CMD_TUPG	"TUPG"
#define CMD_TVER	"TVER"
#define CMD_TFCR	"TFCR"
#define CMD_TCN2	"TCN2"

#define	CMD_PDUH	"PDUH"
#define	CMD_PFST	"PFST"
#define	CMD_PSEP	"PSEP"
#define	CMD_PTIM	"PTIM"
#define	CMD_PUPG	"PUPG"
#define	CMD_PVER	"PVER"
#define	CMD_PSET	"PSET"
#define	CMD_PFCC	"PFCC"
#define	CMD_PAST	"PAST"
#define	CMD_PFCR	"PFCR"
#define	CMD_PFRS	"PFRS"
#define	CMD_PRSI	"PRSI"
#define	CMD_PDAT	"PDAT"
#define	CMD_PODT	"PODT"
#define	CMD_PCN2	"PCN2"
#define	CMD_PRBT	"PRBT"

#define	TXMODE_HAF	 "HAF"
#define	TXMODE_FIV	  "FIV"
#define	TXMODE_ALL 	"ALL"

#define	TXMODE_HAF_NUM      0
#define	TXMODE_FIV_NUM	    1
#define	TXMODE_ALL_NUM 	    2

#define	MSG_ACK      0x06
#define	MSG_NAK      0x15
#define	MSG_EOT      0x04


/*  			define end  			*/


/*  			enum start  			*/
typedef enum
{
	IDX_RX_CMD = 0,
	IDX_RX_DATA = 1,
} UART_E;

typedef enum
{
	LEN_COMM_1_4 = 4,
	LEN_COMM_2_7 = 7,
	LEN_COMM_3_3 = 3,
	LEN_COMM_4_4 = 4,
	LEN_COMM_5_3 = 3,

	LEN_TDAH1_6_10 = 10,
	LEN_TDAH1_7_2 = 2,
	LEN_TDAH1_8n_5 = 5,
	LEN_TDAH1_9n_1 = 1,
	LEN_TDAH1_10n_6 = 6,
	LEN_TDAH1_11n_1 = 1,
	LEN_TDAH1_12n_1 = 1,
	LEN_TDAH1_13n_1 = 1,


	LEN_TOFH2_6_8 = 8,
	LEN_TOFH2_7_3 = 3,
	LEN_TOFH2_8n_4 = 4,

	LEN_TDDH3_6_8 = 8,
	LEN_TDDH3_7_3 = 3,
	LEN_TDDH3_8_3 = 3,
	LEN_TDDH3_9_3 = 3,
	LEN_TDDH3_10_2 = 2,
	LEN_TDDH3_11n_5 = 5,
	LEN_TDDH3_12n_1 = 1,
	LEN_TDDH3_13n_3 = 3,
	LEN_TDDH3_14n_3 = 3,
	LEN_TDDH3_15n_3 = 3,
	LEN_TDDH3_16n_3 = 3,
	LEN_TDDH3_17n_3 = 3,

	LEN_TFDH4_6_10 = 10,
	LEN_TFDH4_7_2 = 2,
	LEN_TFDH4_8n_5 = 5,
	LEN_TFDH4_9n_1 = 1,
	LEN_TFDH4_10n_6 = 6,
	LEN_TFDH4_11n_1 = 1,
	LEN_TFDH4_12n_1 = 1,
	LEN_TFDH4_13n_1 = 1,

	LEN_TDUH5_6_10 = 10,
	LEN_TDUH5_7_2 = 2,
	LEN_TDUH5_8n_5 = 5,
	LEN_TDUH5_9n_1 = 1,
	LEN_TDUH5_10n_6 = 6,
	LEN_TDUH5_11n_1 = 1,
	LEN_TDUH5_12n_1 = 1,
	LEN_TDUH5_13n_1 = 1,

    LEN_PDUH5_6_10 = 10,
    LEN_PDUH5_7_10 = 10,


	LEN_TNOH6_6_10 = 10,
	LEN_TNOH6_7_2 = 2,
	LEN_TNOH6_8n_5 = 5,
	LEN_TNOH6_9n_1 = 1,
	LEN_TNOH6_10n_1 = 1,
	LEN_TNOH6_11n_1 = 1,

	LEN_PFST7_5_4 = 4,

	LEN_PSEP8_5_16_RAW = 16,
    LEN_PSEP8_5_10 = 10,

	LEN_PTIM9_5_1_6 = 6,
    LEN_PTIM9_5_2_6 = 6,

    LEN_PUPG10_5_144_RAW = 144,
    LEN_PUPG10_5_131 = 131,

	LEN_PUPG10_5_1 = 1,
	LEN_PUPG10_6_40 = 40,
	LEN_PUPG10_7_5 = 5,
	LEN_PUPG10_8_50 = 50,
	LEN_PUPG10_9_10 = 10,
	LEN_PUPG10_10_10 = 10,
	LEN_PUPG10_11_15 = 15,

	LEN_TUPG10_5_16 = 16,
	LEN_TUPG10_6_16 = 16,
	LEN_TUPG10_7_2 = 2,
	LEN_TUPG10_8_20 = 20,
	LEN_TUPG10_9_20 = 20,
	LEN_TUPG10_10_32 = 32,

	LEN_TVER11_5_16 = 16,
	LEN_TVER11_6_16 = 16,
	LEN_TVER11_7_2 = 2,
	LEN_TVER11_8_20 = 20,
	LEN_TVER11_9_20 = 20,
	LEN_TVER11_10_32 = 32,

	LEN_PSET12_5_1_6 = 6,
    LEN_PSET12_5_2_6 = 6,

	LEN_PFCC13_5_5 = 5,
	LEN_PFCC13_6_5 = 5,

	LEN_PAST14_5_2 = 2,
	LEN_PAST14_6n_5 = 5,
	LEN_PAST14_7n_1 = 1,
	LEN_PAST14_8n_6 = 6,
	LEN_PAST14_9n_6 = 6,
	LEN_PAST14_10n_6 = 6,

	LEN_TFCR15_5_2 = 2,
	LEN_TFCR15_6n_5 = 5,
	LEN_TFCR15_7n_5 = 5,

	LEN_PFRS16_5_2 = 2,
	LEN_PFRS16_6n_5 = 5,
	LEN_PFRS16_7n_5 = 5,

    LEN_PRSI17_5_16_RAW = 16,
	LEN_PRSI17_5_15 = 15,

	LEN_PDAT18_5_1 = 1,

	LEN_PODT19_5_3 = 3,
	LEN_PODT19_6_3 = 3,

	LEN_TCN2_21_5_16 = 16,
	LEN_TCN2_21_6_16 = 16,
	LEN_TCN2_21_7_2 = 2,
	LEN_TCN2_21_8_20 = 20,
	LEN_TCN2_21_9_20 = 20,
	LEN_TCN2_21_10_32 = 32,
	LEN_TCN2_21_11_16_RAW = 16,
	LEN_TCN2_21_11_10 = 10,
	LEN_TCN2_21_12_4 = 4,
	LEN_TCN2_21_13_1 = 1,
	LEN_TCN2_21_14_3 = 3,
	LEN_TCN2_21_15_3 = 3,
	LEN_TCN2_21_16_2 = 2,
	LEN_TCN2_21_17_5n = 5,
	LEN_TCN2_21_18_1n = 1,
	LEN_TCN2_21_19_6n = 6,
	LEN_TCN2_21_20_6n = 6,
	LEN_TCN2_21_21_6n = 6,
// ==========================================
// ========================================================================================
	// 공통 헤더 (Header) 및 바디 시작점
	// ========================================================================================
	IDX_COMM_1 = 0, 	// 명령어 (4)
	IDX_COMM_2 = 4, 	// 사업장코드 (7)
	IDX_COMM_3 = 11,	// 굴뚝코드 (3)
	IDX_COMM_4 = 14,	// 전체길이 (4)
	IDX_COMM_5 = 18,	// 자료구분 (3) - 자료구분이 있는 전문의 바디 시작점

	// ========================================================================================
	// [1] 측정자료 전송 (TDAH) - 가변 구조
	// ========================================================================================
	IDX_TDAH1_6 = 21,	// 측정시간 (10)
	IDX_TDAH1_7 = 31,	// 항목수 (2)
	IDX_TDAH1_8n = 33,	// 시설 코드 (5)
	IDX_TDAH1_9n = 38,	// 항목 코드 (1)
	IDX_TDAH1_10n = 39, // 측정값 (6)
	IDX_TDAH1_11n = 45, // 자료상태 (1)
	IDX_TDAH1_12n = 46, // 가동상태 (1)
	IDX_TDAH1_13n = 47, // 방지시설 정상여부 (1)
	IDX_TDAH1_OUT = 48, // 가변 바디 종료점
	IDX_TDAH1_CYCLE = IDX_TDAH1_OUT - IDX_TDAH1_8n, // 15 (루프 보폭)

	// ========================================================================================
	// [2] 전원단절구간자료 전송 (TOFH) - 가변 구조
	// ========================================================================================
	IDX_TOFH2_6 = 21,	// 전원단절 기준일자 (8)
	IDX_TOFH2_7 = 29,	// 전원단절 건수 (3)
	IDX_TOFH2_8n = 32,	// 시분 (4)
	IDX_TOFH2_OUT = 36, // 가변 바디 종료점
	IDX_TOFH2_CYCLE = IDX_TOFH2_OUT - IDX_TOFH2_8n, // 4

	// ========================================================================================
	// [3] 일일 마감자료 전송 (TDDH) - 가변 구조
	// ========================================================================================
	IDX_TDDH3_6 = 21,	// 마감 기준 일 (8)
	IDX_TDDH3_7 = 29,	// 일 자료 건수 (3)
	IDX_TDDH3_8 = 32,	// TDAH 건수 (3)
	IDX_TDDH3_9 = 35,	// TOFH 건수 (3)
	IDX_TDDH3_10 = 38,	// 항목 수 (2)
	IDX_TDDH3_11n = 40, // 시설 코드 (5)
	IDX_TDDH3_12n = 45, // 항목 코드 (1)
	IDX_TDDH3_13n = 46, // 정상 건수 (3)
	IDX_TDDH3_14n = 49, // 비정상 범위 건수 (3)
	IDX_TDDH3_15n = 52, // 통신 불량 건수 (3)
	IDX_TDDH3_16n = 55, // 전원 단절 건수 (3)
	IDX_TDDH3_17n = 58, // 점검 중 건수 (3)
	IDX_TDDH3_OUT = 61, // 가변 바디 종료점
	IDX_TDDH3_CYCLE = IDX_TDDH3_OUT - IDX_TDDH3_11n, // 21

	// ========================================================================================
	// [4] 미전송자료 전송 (TFDH) - 가변 구조
	// ========================================================================================
	IDX_TFDH4_6 = 21,	// 측정일시 (10)
	IDX_TFDH4_7 = 31,	// 항목 수 (2)
	IDX_TFDH4_8n = 33,	// 시설 코드 (5)
	IDX_TFDH4_9n = 38,	// 항목 코드 (1)
	IDX_TFDH4_10n = 39, // 측정값 (6)
	IDX_TFDH4_11n = 45, // 자료 상태 (1)
	IDX_TFDH4_12n = 46, // 가동 상태 (1)
	IDX_TFDH4_13n = 47, // 배출시설 정상여부 (1)
	IDX_TFDH4_OUT = 48, // 가변 바디 종료점
	IDX_TFDH4_CYCLE = IDX_TFDH4_OUT - IDX_TFDH4_8n, // 15

	// ========================================================================================
	// [5] 저장자료 요청/응답 (PDUH / TDUH)
	// ========================================================================================
	// ■ 요청 (PDUH) - 고정 길이 구조
	IDX_PDUH5_6 = 21,	 // 시작 일시 (10)
	IDX_PDUH5_7 = 31,	 // 끝 일시 (10)
	IDX_PDUH5_CRC = 41,  // ★ CRC 시작 인덱스 고정

	// ■ 응답 (TDUH) - 가변 구조
	IDX_TDUH5_6 = 21,	// 측정 일시 (10)
	IDX_TDUH5_7 = 31,	// 항목 수 (2)
	IDX_TDUH5_8n = 33,	// 시설 코드 (5)
	IDX_TDUH5_9n = 38,	// 항목 코드 (1)
	IDX_TDUH5_10n = 39, // 측정값 (6)
	IDX_TDUH5_11n = 45, // 자료 상태 (1)
	IDX_TDUH5_12n = 46, // 가동 상태 (1)
	IDX_TDUH5_13n = 47, // 배출시설 정상여부 (1)
	IDX_TDUH5_OUT = 48, // 가변 바디 종료점
	IDX_TDUH5_CYCLE = IDX_TDUH5_OUT - IDX_TDUH5_8n, // 15

	// ========================================================================================
	// [6] 5분자료 전송대상 정보 (TNOH) - 가변 구조
	// ========================================================================================
	IDX_TNOH6_6 = 21,	// 자료시간 (10)
	IDX_TNOH6_7 = 31,	// 항목 갯수 (2)
	IDX_TNOH6_8n = 33,	// 시설코드 (5)
	IDX_TNOH6_9n = 38,	// 항목코드 (1)
	IDX_TNOH6_10n = 39, // 가동상태 (1)
	IDX_TNOH6_11n = 40, // 방지시설 정상여부 코드 (1)
	IDX_TNOH6_OUT = 41, // 가변 바디 종료점
	IDX_TNOH6_CYCLE = IDX_TNOH6_OUT - IDX_TNOH6_8n, // 8

	// ========================================================================================
	// [7] 미전송자료 전송시간 변경 요청 (PFST) - 고정 길이 구조
	// ========================================================================================
	IDX_PFST7_5 = 18,	 // 미전송자료 전송시간 (4)
	IDX_PFST7_CRC = 22,  // ★ CRC 시작 인덱스 고정

	// ========================================================================================
	// [8] 비밀번호 변경 요청 (PSEP) - 고정 길이 구조
	// ========================================================================================
	IDX_PSEP8_5 = 18,	 // 암호화된 비밀번호 (16)
	IDX_PSEP8_CRC = 34,  // ★ CRC 시작 인덱스 고정

	// ========================================================================================
	// [9] 서버시간 조회 요청/응답 (TTIM / PTIM) - 고정 길이 구조
	// ========================================================================================
	// ■ 요청 (TTIM) : 바디 없음
	IDX_TTIM9_CRC = 18,  // ★ 헤더 직후 바로 CRC 시작

	// ■ 응답 (PTIM)
	IDX_PTIM9_5_1 = 18,	 // 서버시간 (6) 내가 쪼갬
	IDX_PTIM9_5_2 = 24,	 // 서버시간 (6) 내가 쪼갬
	IDX_PTIM9_CRC = 30,  // ★ CRC 시작 인덱스 고정

	// ========================================================================================
	// [10] 게이트웨이 업그레이드 요청/결과 전송 (PUPG / TUPG) - 고정 길이 구조
	// ========================================================================================
	// ■ 요청 (PUPG) *평문 규격 기준 고정 길이
    IDX_PUPG10_5_RAW = 18,   //

	IDX_PUPG10_5 = 0,	 // FTP 타입 (1)
	IDX_PUPG10_6 = 1,	 // FTP IP/Domain (40)
	IDX_PUPG10_7 = 41,	 // FTP Port (5)
	IDX_PUPG10_8 = 46,	 // 경로 (50)
	IDX_PUPG10_9 = 96,  // FTP ID (10)
	IDX_PUPG10_10 = 106, // FTP PWD (10)
	IDX_PUPG10_11 = 116, // 통신서버 IP (15)
	IDX_PUPG10_CRC = 162, // ★ CRC 시작 인덱스 고정 +13 = 144

	// ■ 결과 전송 (TUPG)
	IDX_TUPG10_5 = 18,	 // 통신 서버 IP (16)
	IDX_TUPG10_6 = 34,	 // GW IP (16)
	IDX_TUPG10_7 = 50,	 // 제조사 코드 (2)
	IDX_TUPG10_8 = 52,	 // GW 모델 (20)
	IDX_TUPG10_9 = 72,	 // 펌웨어 버전 (20)
	IDX_TUPG10_10 = 92,  // 해쉬 코드 (32)
	IDX_TUPG10_CRC = 124, // ★ CRC 시작 인덱스 고정

	// ========================================================================================
	// [11] 버전정보 요청/전송 (PVER / TVER) - 고정 길이 구조
	// ========================================================================================
	// ■ 요청 (PVER) : 바디 없음
	IDX_PVER11_CRC = 18, // ★ 헤더 직후 바로 CRC 시작

	// ■ 응답 (TVER)
	IDX_TVER11_5 = 18,	 // 통신 서버 IP (16)
	IDX_TVER11_6 = 34,	 // GW IP (16)
	IDX_TVER11_7 = 50,	 // 제조사 코드 (2)
	IDX_TVER11_8 = 52,	 // GW 모델 (20)
	IDX_TVER11_9 = 72,	 // 펌웨어 버전 (20)
	IDX_TVER11_10 = 92,  // 해쉬 코드 (32)
	IDX_TVER11_CRC = 124, // ★ CRC 시작 인덱스 고정

	// ========================================================================================
	// [12] 게이트웨이 시간 변경 요청 (PSET) - 고정 길이 구조
	// ========================================================================================
	IDX_PSET12_5_1 = 18,	 // 서버시간 (6)내가쪼갬
	IDX_PSET12_5_2 = 24,	 // 서버시간 (6)내가쪼갬
	IDX_PSET12_CRC = 30, // ★ CRC 시작 인덱스 고정

	// ========================================================================================
	// [13] 시설코드 변경 요청 (PFCC) - 고정 길이 구조
	// ========================================================================================
	IDX_PFCC13_5 = 18,	 // 이전 시설코드 (5)
	IDX_PFCC13_6 = 23,	 // 변경 시설코드 (5)
	IDX_PFCC13_CRC = 28, // ★ CRC 시작 인덱스 고정

	// ========================================================================================
	// [14] 측정범위 변경 요청 (PAST) - 가변 구조
	// ========================================================================================
	IDX_PAST14_5 = 18,	 // 항목수 (2)
	IDX_PAST14_6n = 20,  // 시설 코드 (5)
	IDX_PAST14_7n = 25,  // 항목 코드 (1)
	IDX_PAST14_8n = 26,  // 측정범위 최소값 (6)
	IDX_PAST14_9n = 32,  // 측정범위 최대값 (6)
	IDX_PAST14_10n = 38, // 측정범위 기준값 (6)
	IDX_PAST14_OUT = 44, // 가변 바디 종료점
	IDX_PAST14_CYCLE = IDX_PAST14_OUT - IDX_PAST14_6n, // 24

	// ========================================================================================
	// [15 / 16] 관계정보 요청/응답 및 변경 (TFCR / PFRS 공통)
	// ========================================================================================
	// ■ 요청 (PFCR) : 바디 없음
	IDX_PFCR15_CRC = 18, // ★ 헤더 직후 바로 CRC 시작

	// ■ 응답 (TFCR) 및 변경 요청 (PFRS) - 가변 구조
	IDX_TFCR15_5 = 18,	 // 관계 정보 수 (2)
	IDX_TFCR15_6n = 20,  // 배출시설코드 (5)
	IDX_TFCR15_7n = 25,  // 방지시설코드 (5)
	IDX_TFCR15_OUT = 30, // 가변 바디 종료점
	IDX_TFCR15_CYCLE = IDX_TFCR15_OUT - IDX_TFCR15_6n, // 10

	// ========================================================================================
// ========================================================================================
	// [16] 방지시설 정상여부 관계정보 변경 요청 (PFRS) - 가변 구조
	// ========================================================================================
	IDX_PFRS16_5 = 18,	 // 관계 정보 수 (2) [cite: 1167]
	IDX_PFRS16_6n = 20,  // 배출시설코드 (5) [cite: 1167]
	IDX_PFRS16_7n = 25,  // 방지시설코드 (5) [cite: 1167]
	IDX_PFRS16_OUT = 30, // 가변 바디 종료점
	IDX_PFRS16_CYCLE = IDX_PFRS16_OUT - IDX_PFRS16_6n, // 10 (루프 보폭)
	// [17] 통신서버IP 변경 요청 (PRSI) - 고정 길이 구조
	// ========================================================================================
	IDX_PRSI17_5 = 18,	 // 암호화된 통신서버 IP (16)
	IDX_PRSI17_CRC = 33, // ★ CRC 시작 인덱스 고정

	// ========================================================================================
	// [18] 자료전송모드 변경 요청 (PDAT) - 고정 길이 구조
	// ========================================================================================
	IDX_PDAT18_5 = 18,	 // 전송모드 (1)
	IDX_PDAT18_CRC = 19, // ★ CRC 시작 인덱스 고정

	// ========================================================================================
	// [19] 유예시간 설정 변경 요청 (PODT) - 고정 길이 구조
	// ========================================================================================
	IDX_PODT19_5 = 18,	 // 배출시설 가동유예시간 (3)
	IDX_PODT19_6 = 21,	 // 방지시설 정지유예시간 (3)
	IDX_PODT19_CRC = 24, // ★ CRC 시작 인덱스 고정

	// ========================================================================================
	// [20 / 21] 게이트웨이 설정정보 요청/응답 (PCN2 / TCN2)
	// ========================================================================================
	// ■ 요청 (PCN2) : 바디 없음
	IDX_PCN220_CRC = 18, // ★ 헤더 직후 바로 CRC 시작

	// ■ 응답/전송 (TCN2) - 가변 구조 (마지막에 항목별 범위 반복)
	IDX_TCN2_21_5 = 18,   // 통신서버 IP (16)
	IDX_TCN2_21_6 = 34,   // GW IP (16)
	IDX_TCN2_21_7 = 50,   // 제조사 코드 (2)
	IDX_TCN2_21_8 = 52,   // GW모델 (20)
	IDX_TCN2_21_9 = 72,   // 펌웨어 버전 (20)
	IDX_TCN2_21_10 = 92,  // 해쉬코드 (32)
	IDX_TCN2_21_11 = 124, // 비밀번호 (16)
	IDX_TCN2_21_12 = 140, // 미전송자료 전송시간 (4)
	IDX_TCN2_21_13 = 144, // 자료전송 모드 (1)
	IDX_TCN2_21_14 = 145, // 배출시설 가동유예 시간 (3)
	IDX_TCN2_21_15 = 148, // 방지시설 중지유예 시간 (3)
	IDX_TCN2_21_16 = 151, // 항목수 (2)
	IDX_TCN2_21_17n = 153, // 시설 코드 (5)
	IDX_TCN2_21_18n = 158, // 항목 코드 (1)
	IDX_TCN2_21_19n = 159, // 측정범위 최소값 (6)
	IDX_TCN2_21_20n = 165, // 측정범위 최대값 (6)
	IDX_TCN2_21_21n = 171, // 측정범위 기준값 (6)
	IDX_TCN2_21_OUT = 177, // 가변 바디 종료점
	IDX_TCN2_21_CYCLE = IDX_TCN2_21_OUT - IDX_TCN2_21_17n, // 24

	// ========================================================================================
	// [22] GW 재기동 요청 (PRBT) - 바디 없음
	// ========================================================================================
	IDX_PRBT22_CRC = 18, // ★ 헤더 직후 바로 CRC 시작








        // ========================================================================================
        // 2. P 계열 가변 길이 명령어 패킷 최소 크기 (항목수/관계수 n = 1일 때의 최소 바이트)
        // ========================================================================================


    /*
    ========================================================================================
      [1] ~ [22] 명령어별 패킷 총 길이 공식 리스트 (N = 항목 수 또는 관계 수)
    ========================================================================================

    1)  TDAH    35+15N      1)
    2)  TOFH    34+8N       2)
    3)  TDDH    42+21N      3)
    4)  TFDH    35+15N      4)
    5)  TDUH    35+15N      5)  PDUH    43
    6)  TNOH    35+8N       6)
    7)                      7)  PFST    24
    8)                      8)  PSEP    36
    9)  TTIM    20          9)  PTIM    32
    10) TUPG    126         10) PUPG    151
    11) TVER    126         11) PVER    20
    12)                     12) PSET    32
    13)                     13) PFCC    30
    14)                     14) PAST    22+24N
    15) TFCR    22+10N      15) PFCR    20
    16) TFCR    22+10N      16) PFRS    22+10N
    17)                     17) PRSI    36
    18)                     18) PDAT    21
    19)                     19) PODT    26
    20) TCN2    155+24N     20) PCN2    20

    22)                     22) PRBT    20
    */

 //   ========================================================================================
// RX DEBUG============
    VIEW_ADD_1 = 1,
    VIEW_ADD_2 = 2,
    VIEW_ADD_3 = 3,
    VIEW_ADD_4 = 4,
    VIEW_ADD_5 = 5,
    VIEW_ADD_6 = 6,
    VIEW_ADD_7 = 7,
    VIEW_ADD_8 = 8,
    VIEW_ADD_9 = 9,
    VIEW_ADD_10 = 10,
    VIEW_ADD_11 = 11,

    MAX_COMM_1 = 0,
    MIN_COMM_1 = 0,
//ID===================
    ID_TDAH_1  = 1,
    ID_TOFH_2  = 2,
    ID_TDDH_3  = 3,
    ID_TFDH_4  = 4,
    ID_TDUH_5  = 5,
    ID_TNOH_6  = 6,
    ID_TTIM_9  = 7,
    ID_TUPG_10 = 8,
    ID_TVER_11 = 9,
    ID_TFCR_15 = 10,
    ID_TFCR_16 = 11,
    ID_TCN2_20 = 12,

    ID_PDUH_5  = 13,
    ID_PFST_7  = 14,
    ID_PSEP_8  = 15,
    ID_PTIM_9  = 16,
    ID_PUPG_10 = 17,
    ID_PVER_11 = 18,
    ID_PSET_12 = 19,
    ID_PFCC_13 = 20,
    ID_PAST_14 = 21,
    ID_PFCR_15 = 22,
    ID_PFRS_16 = 23,
    ID_PRSI_17 = 24,
    ID_PDAT_18 = 25,
    ID_PODT_19 = 26,
    ID_PCN2_20  = 27,
    ID_PRBT_22 = 28,


} CMD_E;
/*  			enum end  				*/



/*  			stuct start  			*/
typedef struct
{
	uint8_t rxBuff[160];
	uint8_t passingBuff[160];
	uint8_t passingCnt;
	uint8_t rxCnt;
	uint8_t txCnt;
	uint32_t rxTimeStamp;
	uint32_t txMsgTimeStamp;
	uint8_t txMsgFlag;
	uint32_t txAckTimeStamp;
	uint8_t txAckFlag;
	uint8_t eotTx;
	char txAllBuff[40];
	uint8_t ID;
	uint8_t txCmd;
	uint8_t TOFH_2_Ack;
	uint8_t TDDH_3_Ack;
	uint8_t TFDH_4_Ack;
	uint8_t TDUH_5_Ack;
	uint8_t tofhDone;

	uint8_t stepNoneAck;
	uint8_t stepNoneMsg;
	uint8_t stepNoneNck;
}CMD_T;


typedef struct
{
	uint32_t facCode;//시설코드 [1][4][5][6][13]
	uint8_t itemCode;//항목코드 [1][4][5][6]
	float measureValue;//측정값 [1][4][5]
	uint8_t measureStatus;//자료상태 [1][4][5]
	uint8_t operStatus;//가동상태 [1][4][5][6]
	uint8_t protectStatus;//배출시설 정상여부 [1][4][5][6]

	uint16_t nomalCnt;//정상건수[3]
	uint16_t FultCnt;//비정상건수[3]
	uint16_t commuErrCnt;//통신불량[3]
	uint16_t fixCnt;//점검중건수[3]
	float measureMin;//측정범위 최소값[14]
	float measureMax;//측정범위 최대값[14]
	float measureStandard;//측정범위 기준값[14]
} PART_T;

typedef struct
{

	uint32_t workPlaceCode;//사업장코드[공통]
	uint8_t chimCode;//굴뚝코드[공통]
	uint16_t allLan;//전체길이[공통]
	uint32_t measureTime;//측정시간[공통] // YYMMDDhhmm
	uint8_t measureQty;//항목수[1][4][5]
	PART_T part[5];//항목

	uint32_t powerOffDay;//전원단절 기준일자 yyyyMMDD[2]
	uint16_t powerOffCnt;//전원단절 건수[2][3]
	uint16_t powerOffTime[290];//전원단절시간 hhmm[2]
	uint32_t closeDate;
	uint16_t dayCnt;
	uint16_t TDAHcnt;//TDAH건수[3]
	uint16_t TOFHcnt;//TOFH건수[3]

	uint16_t noTxTime;//미전송시간[7] hhmm
	uint32_t passWard;//비밀번호 10자리 [8]

	uint8_t FTPtype;// [10]
	char FTPipDomain[40]; // [10]
	uint32_t FTPport; // [10]
	char road[50]; //경로 // [10]
	char FTPid[10]; // [10]
	char FTPpwd[10]; // [10]
	uint8_t IP[4]; // [10][11][17]

	uint8_t GWip[4]; // [10][11]
	uint8_t manuCode;// [10][11]
	char GWmodel[20];// [10][11]
	char fwVer[20];
	char heshCode[32];// [10][11]
	uint8_t protectRelyCnt;//방지시설 정상여부 관계정보수 [15][16]
	uint16_t disposDelTime;//배출시설 가동유예시간(분)[19]
	uint16_t protectDelTime;//방지시설 정지유예시간(분)[19]
    uint16_t disposBuff[10];
    uint16_t protectBuff[10];

	uint8_t transferMode;//[21][공통][18]
	float valueMin;//[14]
	float valueMax;//[14]
	float valueSdrd;//[14]
	uint32_t sevrDay; // 서버 날짜 6자리 YYMMDD
	uint32_t sevrTime;// 서버시간 6자리 hhmmss [9][12]
    uint32_t startTime;//시작일시[5]
    uint32_t endTime;//끝일시[5]
} CHIMNEY_T;


typedef struct
{
	uint8_t rxViewBuff[RX_BUFF_SIZE];
	uint16_t rxViewCnt;
	uint8_t rs485En;//필요시
	int rxRingBuff[10][2];
	int rxCmdAdd;
	int rxCmdData;
	uint8_t rxRingCnt;
	uint16_t rxCmdChk;
	uint8_t rxStep;

} UART_T;

/*  			stuct end  				*/



/*  			function start  		*/
void TxTest();
void Uart_Init();

void UartRx1DataProcess();
void UartRx2DataProcess();
void UartRx3DataProcess();


/*  			function end  			*/
void Uart_Gulobal();
void Test_Config();
void Testfunction();


/*  			extern start  			*/



/*  			extern end  			*/

#endif
//remocon.h




