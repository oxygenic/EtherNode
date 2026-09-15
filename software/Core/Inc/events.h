#ifndef EVENTS_H
#define EVENTS_H

#define MAX_TIM_NUM 10
#define MAX_VAR_NUM 20


typedef enum {evtUnused=0,
              evtDIn0=1,evtDIn1,evtDIn2,evtDIn3,evtDIn4,evtDIn5,evtDIn6,evtDIn7,
              evtAIn0=17,evtAIn1,
              evtEnc0Pos=33,evtEnc0Spd,evtEnc0Acc,
              evtTim0=50,evtTim1,evtTim2,evtTim3,evtTim4,evtTim5,evtTim6,evtTim7,evtTim8,evtTim9,
              evtVar0=70,evtVar1,evtVar2,evtVar3,evtVar4,evtVar5,evtVar6,evtVar7,evtVar8,evtVar9,
              evtVar10,evtVar11,evtVar12,evtVar13,evtVar14,evtVar15,evtVar16,evtVar17,evtVar18,evtVar19,
              evtMPos0=100,evtMPos1,evtMPos2,evtMPos3,evtMPos4,evtMPos5,evtMPos6,
              evtMSpd=130,
             } evtType_t;

typedef enum {trgUnused=0,
              trgDOut0=1,trgDOut1,trgDOut2,trgDOut3,trgDOut4,trgDOut5,trgDOut6,trgDOut7,
              trgTim0=50,trgTim1,trgTim2,trgTim3,trgTim4,trgTim5,trgTim6,trgTim7,trgTim8,trgTim9,
              trgVar0=70,trgVar1,trgVar2,trgVar3,trgVar4,trgVar5,trgVar6,trgVar7,trgVar8,trgVar9,
              trgVar10,trgVar11,trgVar12,trgVar13,trgVar14,trgVar15,trgVar16,trgVar17,trgVar18,trgVar19,
              trgPWM0Frq=90,trgPWM0Pul,trgPWM1Frq,trgPWM1Pul,
              trgMPos0=100,trgMPos1,trgMPos2,trgMPos3,trgMPos4,trgMPos5,trgMPos6,
              trgSPos0=110,trgSPos1,trgSPos2,trgSPos3,trgSPos4,trgSPos5,trgSPos6,
              trgHPos0=120,trgHPos1,trgHPos2,trgHPos3,trgHPos4,trgHPos5,trgHPos6,
              trgMSpd0=130,trgMSpd1,trgMSpd2,trgMSpd3,trgMSpd4,trgMSpd5,trgMSpd6,
              trgMAcc0=140,trgMAcc1,trgMAcc2,trgMAcc3,trgMAcc4,trgMAcc5,trgMAcc6,
              trgJump=250,
             } trgType_t;

typedef enum {cmpNone=0,
              cmpLT=1, // <
              cmpLE,   // <=
              cmpGE,   // >=
              cmpGT,   // >
              cmpEQ,   // ==
              cmpNE    // !=
             } cmpType_t;
typedef enum {opNone=0,
              opSet,   // =
              opPlus,  // +
              opMinus, // -
              opMul,   // *
              opDiv,   // /
              opToggle,// #
              opOR,    // |
              opNOT    // ~
              } opType_t;

struct __attribute__((packed)) event_def
{
   uint8_t onSrc;     // evtType_t;
   uint8_t compOp;    // cmpType_t
   uint8_t dCompareVal; // dynamic evtType_t value to compare event with
   int32_t cCompareVal; // constant value to compare event with

   uint8_t doTrg;     // trgType_t;
   uint8_t withOp;    // opType_t;
   uint8_t dval;      // dynamic evtType_t value to set to doTrg by using withOp out of evt-source; if not used, dynamic value is assumed
   int32_t cval;      // constant value to set to doTrg by using withOp

   uint8_t else_doTrg;     // trgType_t;
   uint8_t else_withOp;    // opType_t;
   uint8_t else_dval;      // dynamic evtType_t value to set to doTrg by using withOp out of evt-source; if not used, dynamic value is assumed
   int32_t else_cval;      // constant value to set to doTrg by using withOp
};
// usage:
//     onSrc compOp   compareVal          doTrg                          withOp    dval/cval
// if source compared with value is true, change doTrg by using operator withOp to dval or cval
// for future use:
//                                        else change else_doTrg by using operator else_withOp to else_dval or else_cval

extern char *handleSetEventCommand(char *cmd);
extern char *handleGetEventCommand(char *cmd);
extern char *handleListEvent(const int32_t evtNum);
extern char *handleDeleteEventCommand(char *cmd);
extern void handleEvents(const uint32_t DInnew);

#endif // EVENTS_H
