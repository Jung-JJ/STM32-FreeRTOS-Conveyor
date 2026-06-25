/*
 * belt.c
 *
 * Created on: Jul 30, 2025
 * Author: jjj
 */

#include "belt.h"
#include "state.h"
#include "events.h"
#include "app_os.h"
#include "stm32f1xx_hal.h"

volatile SystemState_t g_state = STATE_IDLE; //일단 기본값은 초기
volatile FaultCode_t   g_fault = FAULT_NONE; //fault도 NONE 상태

// 사용자 인터록: START로만 1, STOP/ESTOP/RESET은 항상 0
static volatile uint8_t g_run_enable = 0;

enum {
  IC_BLOCK_FAULT_MS        = 10000,  // 10초 감지 지속 → FAULT_SERVO
  SERVO_ACTION_TIMEOUT_MS  = 6000,   // 서보 완료 미도착 타임아웃
  SERVO_AUTOCLEAR_MS       = 100     // 감지 해제 후 유지되면 자동 복구(옵션)
};

static uint32_t ic_high_since_ms    = 0;
static uint32_t servo_cmd_ts_ms     = 0;
static uint32_t area_clear_since_ms = 0;

static uint8_t act_done_latched   = 0;
static uint8_t area_clear_latched = 0;

static inline uint8_t ir_is_detected_now(void){
    return (HAL_GPIO_ReadPin(IR_SENSOR_Pin_GPIO_Port, IR_SENSOR_Pin_Pin) == GPIO_PIN_RESET) ? 1u : 0u; //엑티브 로우 IR 센서 0이면 1 1이면 0
}

static inline void belt_apply_conveyor(uint16_t duty_if_run){ //컨베이어 벨트 허용 플레그 설정 코드.
    if (g_run_enable) {
        ConvCmd c = {.type = CONV_CMD_SET, .duty_pct = duty_if_run}; //구조체 생성(플레그 + 듀티 값)
        xQueueSend(gConvQ, &c, 0); //큐에 넣기
    } else {
        ConvCmd c = {.type = CONV_CMD_STOP}; //구조체 생성 STOP플래그
        xQueueSend(gConvQ, &c, 0); //큐에 보내기(중요 메시지여서 0으로 설정)
    }
}

/*1. 현재 이벤트 확인
2. 현재 상태 확인
3. 상태 변경
4. 컨베이어/서보 명령 전송
5. 처리한 이벤트 삭제*/
void Belt_StateMachine_Step(void)
{
    EventBits_t ev = xEventGroupGetBits(gEvent); //현재 이벤트 그룹 읽기
    EventBits_t used = 0; //이벤트 used는 즉 사용한 비트를 기록하기 위해서(지우기 위함) 초기화.
    uint32_t now = HAL_GetTick(); //현제 시간을 받아오기.

    /* ────────────────────────────────────────────────────────────────
       FAULT 하드 게이트:
       - FAULT에서는 RESET으로만 해제 가능
       - ESTOP/STOP/START/센서 이벤트는 모두 무시(모터는 정지 유지)
       ──────────────────────────────────────────────────────────────── */
    if (g_state == STATE_FAULT) { //FAULT상태일 때
        if (ev & EV_RESET) { //RESET비트가 켜지면 (리셋버튼 누르면)
            // FAULT 해제는 RESET으로만
            g_run_enable = 0; //정지
            g_fault = FAULT_NONE; //Fault 상태 초기화
            g_state = STATE_IDLE; //상태는 초기값으로
            used |= EV_RESET; //리셋 부분이 1이면 used도 1로 (RESET비트를 지워야 해서)

            // 안전: 모터 정지 유지
            ConvCmd c = {.type = CONV_CMD_STOP}; //컨베이어 상태는 정지.
            xQueueSend(gConvQ, &c, 0); //큐에 정지 메시지 보내기

            // 내부 상태 정리
            ic_high_since_ms = servo_cmd_ts_ms = area_clear_since_ms = 0;
            act_done_latched = area_clear_latched = 0;
        } else {
            // 다른 이벤트들은 모두 무시(필요시 큐에서만 비워준다)
            used |= (ev & (EV_STOP | EV_START | EV_OBJ_DETECTED | EV_ACT_DONE | EV_AREA_CLEAR | EV_FAULT_SERVO | EV_ESTOP)); //RESET이 안 켜지면 현제 상태를 USED에 넣기(CLEAR 위함)

            // 안전: 모터 정지 유지
            ConvCmd c = {.type = CONV_CMD_STOP};
            xQueueSend(gConvQ, &c, 0);
        }

        if (used) xEventGroupClearBits(gEvent, used); //used에 기록된 비트가 gEvent에 있는 비트를 지움(Fault에서는 아무것도 못 하게)
        return;  // ★ FAULT에서는 여기서 함수 종료
    }

    /* FAULT이 아닐 때: 안전/FAULT/사용자 이벤트를 우선 처리
       (ESTOP/FAULT_SERVO/RESET/STOP/START 순)*/
    if (ev & EV_ESTOP){ //ESTOP일때
        g_run_enable = 0; //컨베이어 중지
        g_state = STATE_FAULT; //상태 FAULT
        g_fault = FAULT_ESTOP; //실패는 ESTOP
        used |= EV_ESTOP; //used에 ESTOP SET

        ConvCmd c = {.type = CONV_CMD_STOP}; //컨베이어 STOP
        xQueueSend(gConvQ, &c, 0); //큐에 넣기

        ic_high_since_ms = servo_cmd_ts_ms = area_clear_since_ms = 0;
        act_done_latched = area_clear_latched = 0;
    }
    else if (ev & EV_FAULT_SERVO){  //서보 실패시
        g_run_enable = 0; //중지
        g_state = STATE_FAULT;
        g_fault = FAULT_SERVO; //실패는 서보떄문에
        used |= EV_FAULT_SERVO;

        ConvCmd c = {.type = CONV_CMD_STOP};
        xQueueSend(gConvQ, &c, 0);

        ic_high_since_ms = servo_cmd_ts_ms = area_clear_since_ms = 0;
        act_done_latched = area_clear_latched = 0;
    }
    else if (ev & EV_RESET){ //리셋을 눌르면 (Fault 상태 X)
        g_run_enable = 0;
        g_fault = FAULT_NONE; //실패 상태 off
        g_state = STATE_IDLE; //초기 상태로
        used |= EV_RESET; //used에 리셋 켜기.

        ConvCmd c = {.type = CONV_CMD_STOP};
        xQueueSend(gConvQ, &c, 0);

        ic_high_since_ms = servo_cmd_ts_ms = area_clear_since_ms = 0;
        act_done_latched = area_clear_latched = 0;
    }
    else if (ev & EV_STOP){ //STOP일때
        g_run_enable = 0;
        g_state = STATE_IDLE; //상태는초기
        used |= EV_STOP;

        ConvCmd c = {.type = CONV_CMD_STOP};
        xQueueSend(gConvQ, &c, 0);

        ic_high_since_ms = servo_cmd_ts_ms = area_clear_since_ms = 0;
        act_done_latched = area_clear_latched = 0;
    }
    else if ((ev & EV_START) && g_state==STATE_IDLE){ //상태가 초기상태면
        g_run_enable = 1; //컨베이어 EN
        g_state = STATE_RUN; //상태는 시작
        used |= EV_START;

        ic_high_since_ms = servo_cmd_ts_ms = area_clear_since_ms = 0;
        act_done_latched = area_clear_latched = 0;

        belt_apply_conveyor(40);   // 컨베이어 시작 듀티
    }
    // RUN 중 감지 → PROCESSING
    else if ((ev & EV_OBJ_DETECTED) && g_state==STATE_RUN){ //상태가 RUN 이고 물체 감지 이벤트가 1이면
        g_state = STATE_PROCESSING; //상태 처리 상태
        used |= EV_OBJ_DETECTED; //used에 물체감지 이벤트 기록

        ConvCmd c1 = {.type = CONV_CMD_STOP};
        xQueueSend(gConvQ,&c1,0);

        ActCmd a  = {.type = ACT_CMD_KICK, .arg1 = 90, .arg2 = 250}; //서보 90도 유지는 250ms
        xQueueSend(gActQ,&a,0);

        ic_high_since_ms    = now; //물체 감지시간
        servo_cmd_ts_ms     = now; //서보 명렬 보낸 시각
        area_clear_since_ms = 0;

        act_done_latched   = 0;
        area_clear_latched = 0;
    }
    // PROCESSING 상태
    else if (g_state==STATE_PROCESSING){ //처리 상태
        if (ev & EV_ACT_DONE){ //행동 끝
            used |= EV_ACT_DONE; //used에 행동 끝 저장
            act_done_latched = 1; //행동 끝 래치에 1
        }
        if (ev & EV_AREA_CLEAR){ //영역에 물건이 없으면
            used |= EV_AREA_CLEAR; //기록
            if (!area_clear_latched && area_clear_since_ms==0) area_clear_since_ms = now; //만약 clear가 0이거나 area_clear_since_ms이 0이면 area_clear_since_ms는 지금 시간
            area_clear_latched = 1; //클리어 1
            ic_high_since_ms = 0;
        }

        uint8_t area_clear_live = (ir_is_detected_now() == 0) ? 1u : 0u; //물체가 없으면 1 있으면 0
        if (!area_clear_live){ //만약 물체가 있으면
            area_clear_since_ms = 0;
            area_clear_latched  = 0;
            if (ic_high_since_ms == 0) ic_high_since_ms = now;
        }

        // 정상 복귀
        if (act_done_latched && area_clear_latched){ //행동끝 래치와 클리어 래치가 1이면
            if (g_run_enable){ //컨베이어 EN상태이면
                g_state = STATE_RUN; //상태는 RUN
                belt_apply_conveyor(40); //컨베이어 듀티 40
            } else {//처리 중일떄 STOP을 눌렀을 떄
                g_state = STATE_IDLE;// 초기상태
                belt_apply_conveyor(0); //컨베이어 정지.
            }
            act_done_latched   = 0;
            area_clear_latched = 0;
            ic_high_since_ms = servo_cmd_ts_ms = area_clear_since_ms = 0;
        } else {
            // 잘 처리 못했을 때 Fault
            if (ic_high_since_ms && ir_is_detected_now() &&
                (now - ic_high_since_ms) > IC_BLOCK_FAULT_MS){ //물체 감지 시작 시간이 있고 지금도 물체가 있고 감지된지 10초가 지났을 때
                g_state = STATE_FAULT;
                g_fault = FAULT_SERVO;
                ic_high_since_ms = servo_cmd_ts_ms = 0;
            }
            if ((g_state==STATE_PROCESSING) && servo_cmd_ts_ms &&
                !act_done_latched &&
                (now - servo_cmd_ts_ms) > SERVO_ACTION_TIMEOUT_MS){ //현재 PROCESSING 중이고 서보 명령을 보낸 적 있고 아직 EV_ACT_DONE이 안 왔고 서보 명령 후 6초가 넘었으면
                g_state = STATE_FAULT;
                g_fault = FAULT_SERVO;
                servo_cmd_ts_ms = 0;
            }
        }
    }

#ifdef SERVO_FAULT_AUTOCLEAR
    if (g_state==STATE_FAULT && g_fault==FAULT_SERVO){
        if ((ev & EV_AREA_CLEAR)){
            if (area_clear_since_ms==0) area_clear_since_ms = now;
            if (area_clear_since_ms && (now - area_clear_since_ms) > SERVO_AUTOCLEAR_MS){
                g_fault = FAULT_NONE;
                g_state = STATE_RUN;
                belt_apply_conveyor(40);
                ic_high_since_ms = servo_cmd_ts_ms = area_clear_since_ms = 0;
                act_done_latched   = 0;
                area_clear_latched = 0;
                used |= EV_AREA_CLEAR;
            }
        }else{
            area_clear_since_ms = 0;
        }
    }
#endif

    if (used) xEventGroupClearBits(gEvent, used); //여태까지 처리한 used플레그 된거 이벤트 그룹에서 다 지우기
}
