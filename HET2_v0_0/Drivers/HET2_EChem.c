/*
 * HET2_EChem.c
 *
 *  Created on: May 28, 2020
 *      Author: tsong
 */
#include <Drivers/HET2_EChem.h>

#define ECHEM_PERIOD 0.5
#define ECHEM_SAMPLES 20

static float LFOSCFreq;
extern uint32_t IntCount1;
extern uint8_t  CHAR1data[];


/* Initialize AD5940 basic blocks like clock */
int32_t AD5940PlatformCfg(void)
{
  CLKCfg_Type clk_cfg;
  FIFOCfg_Type fifo_cfg;
  AGPIOCfg_Type gpio_cfg;
  LFOSCMeasure_Type LfoscMeasure;


  ADCBaseCfg_Type adc_base;
  ADCFilterCfg_Type adc_filter;
  StatCfg_Type stat_cfg;

/* Use hardware reset */
  AD5940_HWReset();

  /* Platform configuration */
  AD5940_Initialize();
  /* Step1. Configure clock */
  clk_cfg.ADCClkDiv = ADCCLKDIV_1;
  clk_cfg.ADCCLkSrc = ADCCLKSRC_HFOSC;
  clk_cfg.SysClkDiv = SYSCLKDIV_1;
  clk_cfg.SysClkSrc = SYSCLKSRC_HFOSC;
  clk_cfg.HfOSC32MHzMode = bFALSE;
  clk_cfg.HFOSCEn = bTRUE;
  clk_cfg.HFXTALEn = bFALSE;
  clk_cfg.LFOSCEn = bTRUE;
  AD5940_CLKCfg(&clk_cfg);
  /* Step2. Configure FIFO and Sequencer*/
  fifo_cfg.FIFOEn = bFALSE;
  fifo_cfg.FIFOMode = FIFOMODE_FIFO;
  fifo_cfg.FIFOSize = FIFOSIZE_4KB;                       /* 4kB for FIFO, The reset 2kB for sequencer */
  fifo_cfg.FIFOSrc = FIFOSRC_DFT;
  fifo_cfg.FIFOThresh = 4;//AppAMPCfg.FifoThresh;        /* DFT result. One pair for RCAL, another for Rz. One DFT result have real part and imaginary part */
  AD5940_FIFOCfg(&fifo_cfg);                             /* Disable to reset FIFO. */
    fifo_cfg.FIFOEn = bTRUE;
  AD5940_FIFOCfg(&fifo_cfg);                             /* Enable FIFO here */

  /* Step3. Interrupt controller */
  AD5940_INTCCfg(AFEINTC_1, AFEINTSRC_ALLINT, bTRUE);           /* Enable all interrupt in Interrupt Controller 1, so we can check INTC flags */
  AD5940_INTCClrFlag(AFEINTSRC_ALLINT);
  AD5940_INTCCfg(AFEINTC_0, AFEINTSRC_DATAFIFOTHRESH|AFEINTSRC_ENDSEQ, bTRUE);   /* Interrupt Controller 0 will control GP0 to generate interrupt to MCU */
  AD5940_INTCClrFlag(AFEINTSRC_ALLINT);
  /* Step4: Reconfigure GPIO */
  gpio_cfg.FuncSet = GP6_SYNC|GP5_SYNC|GP4_SYNC|GP2_TRIG|GP1_SYNC|GP0_INT;
  gpio_cfg.InputEnSet = AGPIO_Pin2;
  gpio_cfg.OutputEnSet = AGPIO_Pin0|AGPIO_Pin1|AGPIO_Pin4|AGPIO_Pin5|AGPIO_Pin6;
  gpio_cfg.OutVal = 0;
  gpio_cfg.PullEnSet = 0;
  AD5940_AGPIOCfg(&gpio_cfg);

    AD5940_SleepKeyCtrlS(SLPKEY_UNLOCK);  /* Enable AFE to enter sleep mode. */
  /* Measure LFOSC frequency */
  LfoscMeasure.CalDuration = 1000.0;  /* 1000ms used for calibration. */
  LfoscMeasure.CalSeqAddr = 0;
  LfoscMeasure.SystemClkFreq = 16000000.0f; /* 16MHz in this firmware. */
  AD5940_LFOSCMeasure(&LfoscMeasure, &LFOSCFreq);
  printf("Freq:%f\n", LFOSCFreq);
  return 0;
}

/* !!Change the application parameters here if you want to change it to non-default value */
void AD5940AMPStructInit()
{
  AppCHRONOAMPCfg_Type *pAMPCfg;
  AppCAPHGetCfg(&pAMPCfg);
  /* Configure general parameters */
    pAMPCfg->WuptClkFreq = LFOSCFreq;                   /* Use measured 32kHz clock freq for accurate wake up timer */
  pAMPCfg->SeqStartAddr = 0;
  pAMPCfg->MaxSeqLen = 512;                                 /* @todo add checker in function */
  pAMPCfg->RcalVal = 10000.0;
  pAMPCfg->NumOfData = -1;                              /* Never stop until you stop it mannually by AppAMPCtrl() function */

    pAMPCfg->AmpODR = conv_period(CHAR1data[5]);
    pAMPCfg->FifoThresh = ECHEM_SAMPLES;
    pAMPCfg->ADCRefVolt = 1.82;                         /* Measure voltage on VREF_1V8 pin and add here */

    pAMPCfg->ExtRtia = bFALSE;          /* Set to true if using external Rtia */
    pAMPCfg->ExtRtiaVal = 10000000; /* Enter external Rtia value here is using one */
    pAMPCfg->LptiaRtiaSel = (uint32_t) CHAR1data[4];       /* Select TIA gain resistor. */
    uint8_t uint_bias = CHAR1data[3]; // type conversion
    float float_bias = uint_bias - 128.0;
    pAMPCfg->SensorBias =  float_bias;   /* Sensor bias voltage between reference and sense electrodes*/
    pAMPCfg->Vzero = 1100;
    /* Configure Pulse*/
    pAMPCfg->pulseAmplitude = 500;                      /* Pulse amplitude on counter electrode (mV) */
    pAMPCfg->pulseLength = 500;                             /* Length of voltage pulse in ms */
}


float conv_period(uint8_t t_code){
    static const float period_lookup[15] = {0.05, 0.1, 0.125, 0.1667, 0.25,
                                            0.5, 1, 2, 2.5, 5,
                                            10, 20, 25, 30, 50,
                                            60, 120, 150, 300, 600};
    float period;
    period = period_lookup[t_code]/2; //sampling between channels
    return period;
}





int32_t AMPShowResult(float *pData, uint32_t DataCount)
{
  for(int i=0;i<DataCount;i++)
  {
  //Display_printf(dispHandle, SP_ROW_RSSI, 0, "Current:%f uA\n", &pData[i]);
     //printf("mem: %i\n", &pData[i]);
     printf("val: %f\n", pData[i]);
  }
  return 0;
}

