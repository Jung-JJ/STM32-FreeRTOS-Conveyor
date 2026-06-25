/*
 * events.h
 *
 *  Created on: Aug 22, 2025
 *      Author: jjj
 */

#ifndef INC_EVENTS_H_
#define INC_EVENTS_H_

#pragma once
//이벤트 정의.
#define EV_START        (1u<<0) //시작
#define EV_STOP         (1u<<1) //정지
#define EV_ESTOP        (1u<<2) //이머전시 스톱
#define EV_RESET        (1u<<3) //리셋
#define EV_OBJ_DETECTED (1u<<4) //물체 감지
#define EV_ACT_DONE     (1u<<5) //행동 끝
#define EV_AREA_CLEAR   (1u<<6) //물건 치움.
#define EV_FAULT_SERVO  (1u<<7) //서보 실패
#endif /* INC_EVENTS_H_ */
