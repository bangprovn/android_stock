#ifndef __CUST_PANTECH_CAMERA_H__
#define __CUST_PANTECH_CAMERA_H__


/*------------------------------------------------------------------------------

(1)  하드웨어 구성
   
EF39S   : APQ8060, CE1612(8M ISP), S5K6AAFX13(1.3M)
EF40S/40K/65L   : APQ8060, CE1612(8M ISP), MT9D113(1.3M)
PRESTO  : APQ8060, MT9P111(5M SOC), MT9V113(VGA)
EF44S   : MSM8960, CE1502(13M ISP), YACD5C1SBDBC(2M)
MAGNUS   : MSM8960, CE1502(13M ISP), YACD5C1SBDBC(2M)
EF48S/49K/50L   : APQ8064, CE1502(13M ISP), YACD5C1SBDBC(2M)
EF56S/57K/58L   : MSM8974, IMX135(13M bayer), S5K6B2YX(2M FHD bayer)


(2)  카메라 관련 모든 kernel/userspace/android 로그를 제거

kernel/arch/arm/config/msm8660-perf-(모델명)_(보드버전)_defconfig 를 수정한다.

	# CONFIG_MSM_CAMERA_DEBUG is not set (default)

CUST_PANTECH_CAMERA.h 에서 F_PANTECH_CAMERA_LOG_PRINTK 을 #undef 한다.

	#define F_PANTECH_CAMERA_LOG_PRINTK (default)

모든 소스 파일에서 F_PANTECH_CAMERA_LOG_OEM 을 찾아 주석 처리한다.
	선언 된 경우, 해당 파일에 사용된 SKYCDBG/SKYCERR 매크로를 이용한
	메시지들을 활성화 시킨다. (user-space only)

모든 소스 파일에서 F_PANTECH_CAMERA_LOG_CDBG 를 찾아 주석 처리한다.
	선언 된 경우, 해당 파일에 사용된 CDBG 매크로를 이용한 메시지들을
	활성화 시킨다. (user-space only)

모든 소스 파일에서 F_PANTECH_CAMERA_LOG_VERBOSE 를 찾아 주석 처리한다.
	선언 된 경우, 해당 파일에 사용된 LOGV/LOGD/LOGI 매크로를 이용한
	메시지들을 활성화 시킨다. (user-space only)


(4)  안면인식 관련 기능 빌드 환경

vendor/qcom/android-open/libcamera2/Android.mk 를 수정한다.
	3rd PARTY 솔루션 사용 여부를 결정한다.

	PANTECH_CAMERA_FD_ENGINE := 0		미포함
	PANTECH_CAMERA_FD_ENGINE := 1		올라웍스 솔루션 사용
	PANTECH_CAMERA_FD_ENGINE := 2		기타 솔루션 사용

CUST_PANTECH_CAMERA.h 에서 F_PANTECH_CAMERA_ADD_CFG_FACE_FILTER 를 #undef 한다.
	안면인식 기능 관련 인터페이스 포함 여부를 결정한다.

libOlaEngine.so 를 기존 libcamera.so 에 링크하므로 기존 대비 libcamera.so 의
크기가 증가하여 링크 오류가 발생 가능하며, 이 경우 아래 파일들에서
liboemcamera.so 의 영역을 줄여 libcamera.so 의 영역을 확보할 수 있다.

build/core/prelink-linux-arm-2G.map (for 2G/2G)
build/core/prelink-linux-arm.map (for 1G/3G)

------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*  MODEL-SPECIFIC                                                            */
/*  해당 모델에만 적용되는 또는 해당 모델에서만 검증된 FEATURE 목록           */
/*----------------------------------------------------------------------------*/
#if defined(CONFIG_SKY_EF39S_BOARD)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF39S
#define F_PANTECH_CAMERA_SKT
#elif defined(CONFIG_SKY_EF40S_BOARD)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF40S
#define F_PANTECH_CAMERA_SKT
#elif defined(CONFIG_SKY_EF40K_BOARD)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF40K
#elif defined(CONFIG_PANTECH_PRESTO_BOARD)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_PRESTO
#define F_PANTECH_CAMERA_PRESTO
/* AT&T모델의 경우 추가되어야 하는 Featrue */
#define F_PANTECH_CAMERA_ATT
#elif defined(T_EF45K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF45K
#define F_PANTECH_CAMERA_EF47S_45K_46L
#elif defined(T_EF46L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF46L
#define F_PANTECH_CAMERA_EF47S_45K_46L
#elif defined(T_EF47S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF47S
#define F_PANTECH_CAMERA_EF47S_45K_46L
#elif defined(T_SVLTE)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_SVLTE
#elif defined(T_CSFB)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_CSFB
#elif defined(T_CHEETAH)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_CHEETAH
#elif defined(T_STARQ)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_STARQ
#define F_PANTECH_CAMERA_ATT
#elif defined(T_OSCAR)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_OSCAR
#define F_PANTECH_CAMERA_ATT
#elif defined(T_VEGAPVW)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_VEGAPVW
/* AT&T모델의 경우 추가되어야 하는 Featrue */
#define F_PANTECH_CAMERA_ATT
#elif defined(T_ZEPPLIN)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_ZEPPLIN
#elif defined(T_RACERJ)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_RACERJ
#elif defined(T_SIRIUSLTE)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_SIRIUSLTE
#elif defined(T_EF44S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF44S
#define F_PANTECH_CAMERA_SKT
#elif defined(T_MAGNUS)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_MAGNUS
#elif defined(T_EF48S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF48S
#define F_PANTECH_CAMERA_EF48S_49K_50L
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF49K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF49K
#define F_PANTECH_CAMERA_EF48S_49K_50L
#define F_PANTECH_CAMERA_KT
#elif defined(T_EF50L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF50L
#define F_PANTECH_CAMERA_EF48S_49K_50L
#define F_PANTECH_CAMERA_LGT
#elif defined(T_EF51S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF51S
#define F_PANTECH_CAMERA_EF51S_51K_51L
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF51K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF51K
#define F_PANTECH_CAMERA_EF51S_51K_51L
#define F_PANTECH_CAMERA_KT
#elif defined(T_EF51L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF51L
#define F_PANTECH_CAMERA_EF51S_51K_51L
#define F_PANTECH_CAMERA_LGT
#elif defined(T_EF52S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52S
#define F_PANTECH_CAMERA_EF52S_52K_52L
/* #define F_PANTECH_CAMERA_SKT */
#elif defined(T_EF52K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52K
#define F_PANTECH_CAMERA_EF52S_52K_52L
#define F_PANTECH_CAMERA_KT
#elif defined(T_EF52L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52L
#define F_PANTECH_CAMERA_EF52S_52K_52L
#define F_PANTECH_CAMERA_LGT
#elif defined(T_EF52W)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52W
#define F_PANTECH_CAMERA_EF52S_52K_52L
/* #define F_PANTECH_CAMERA_SKT */
#elif defined(T_EF52W)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF52W
#define F_PANTECH_CAMERA_EF52S_52K_52L
/* #define F_PANTECH_CAMERA_SKT */
#elif defined(T_EF56S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF56S
#define F_PANTECH_CAMERA_EF56S_57K_58L
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF57K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF57K
#define F_PANTECH_CAMERA_EF56S_57K_58L
#elif defined(T_EF58L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF58L
#define F_PANTECH_CAMERA_EF56S_57K_58L
#elif defined(T_EF59S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF59S
#define F_PANTECH_CAMERA_EF59S_59K_59L
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF59K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF59K
#define F_PANTECH_CAMERA_EF59S_59K_59L
#elif defined(T_EF59L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF59L
#define F_PANTECH_CAMERA_EF59S_59K_59L
#elif defined(T_EF60S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF60S
#define F_PANTECH_CAMERA_EF60S_61K_62L
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF61K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF61K
#define F_PANTECH_CAMERA_EF60S_61K_62L
#elif defined(T_EF62L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF62L
#define F_PANTECH_CAMERA_EF60S_61K_62L
#elif defined(T_NAMI)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_NAMI
#elif defined(T_EF63S)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF63S
#define F_PANTECH_CAMERA_EF63SS
#define F_PANTECH_CAMERA_SKT
#elif defined(T_EF63K)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF63K
#define F_PANTECH_CAMERA_EF63SS
#elif defined(T_EF63L)
#define F_PANTECH_CAMERA
#define F_PANTECH_CAMERA_TARGET_EF63L
#define F_PANTECH_CAMERA_EF63SS
#endif

#ifdef F_PANTECH_CAMERA

#ifndef CONFIG_PANTECH_CAMERA
#define CONFIG_PANTECH_CAMERA
/* #define CONFIG_PANTECH_CAMERA_TUNER */
#endif

/*******************************************************************
 * 
 *           SENSOR ( sensor name or external ISP name)
 *
 *******************************************************************/

#ifdef F_PANTECH_CAMERA_TARGET_EF39S
/* ISP backend camera ISP */
#if (CONFIG_BOARD_VER == CONFIG_PT10)
#define F_PANTECH_CAMERA_ICP_HD
#else
#define F_PANTECH_CAMERA_CE1612
#endif
/* 1.3M front camera sensor */
#define F_PANTECH_CAMERA_S5K6AAFX13
#endif

#ifdef F_PANTECH_CAMERA_TARGET_PRESTO
#if (CONFIG_BOARD_VER >= CONFIG_WS20)
#define F_PANTECH_CAMERA_S5K4ECGX
#else
#define F_PANTECH_CAMERA_MT9P111
#endif
#define F_PANTECH_CAMERA_MT9V113
#endif

#if defined(F_PANTECH_CAMERA_TARGET_EF40S) || \
    defined(F_PANTECH_CAMERA_TARGET_EF40K)
#if (CONFIG_BOARD_VER >= CONFIG_WS20)
#define F_PANTECH_CAMERA_MT9D113
#else
#define F_PANTECH_CAMERA_S5K6AAFX13
#endif
#define F_PANTECH_CAMERA_CE1612
#endif

#if defined(F_PANTECH_CAMERA_TARGET_EF45K) || \
    	defined(F_PANTECH_CAMERA_TARGET_EF46L) || \
	defined(F_PANTECH_CAMERA_TARGET_EF47S) || \
	defined(F_PANTECH_CAMERA_TARGET_OSCAR) || \
	defined(F_PANTECH_CAMERA_TARGET_VEGAPVW)
#define F_PANTECH_CAMERA_OV8820	
#define F_PANTECH_CAMERA_YACD5C1SBDBC
#endif	

#if defined(F_PANTECH_CAMERA_TARGET_CHEETAH) || \
	defined(F_PANTECH_CAMERA_TARGET_ZEPPLIN) 
#define F_PANTECH_CAMERA_S5K4ECGX
#define F_PANTECH_CAMERA_S5K6AAFX13	
#endif

#ifdef F_PANTECH_CAMERA_TARGET_STARQ
#define F_PANTECH_CAMERA_S5K4ECGX
#define F_PANTECH_CAMERA_MT9V113	
#endif

#if defined(F_PANTECH_CAMERA_TARGET_SVLTE) || \
    defined(F_PANTECH_CAMERA_TARGET_CSFB)
#define F_PANTECH_CAMERA_CE1612	
#define F_PANTECH_CAMERA_S5K6AAFX13	
#endif

#if defined(F_PANTECH_CAMERA_TARGET_EF44S) || defined(F_PANTECH_CAMERA_EF48S_49K_50L)
#define F_PANTECH_CAMERA_CE1502
#define F_PANTECH_CAMERA_YACD5C1SBDBC
#endif

#if defined(F_PANTECH_CAMERA_TARGET_EF51S) || defined(F_PANTECH_CAMERA_TARGET_EF51K) || defined(F_PANTECH_CAMERA_TARGET_EF51L) || defined(F_PANTECH_CAMERA_TARGET_EF52S) || defined(F_PANTECH_CAMERA_TARGET_EF52K) || defined(F_PANTECH_CAMERA_TARGET_EF52L) || defined(F_PANTECH_CAMERA_TARGET_EF52W)
#define F_PANTECH_CAMERA_CE1502
#define F_PANTECH_CAMERA_AS0260
#endif

#if defined(F_PANTECH_CAMERA_TARGET_MAGNUS)
#define F_PANTECH_CAMERA_CE1502
#define F_PANTECH_CAMERA_YACD5C1SBDBC
#endif

#if defined(F_PANTECH_CAMERA_TARGET_SIRIUSLTE)
#define F_PANTECH_CAMERA_CE1612
#define F_PANTECH_CAMERA_YACD5C1SBDBC
#endif

#if defined(F_PANTECH_CAMERA_EF56S_57K_58L) || defined(F_PANTECH_CAMERA_EF59S_59K_59L) || defined(F_PANTECH_CAMERA_EF60S_61K_62L) || (defined(F_PANTECH_CAMERA_EF63SS) && (CONFIG_BOARD_VER == CONFIG_PT10))
#define F_PANTECH_CAMERA_IMX135
#endif

#if defined(F_PANTECH_CAMERA_EF56S_57K_58L) || defined(F_PANTECH_CAMERA_EF59S_59K_59L) || defined(F_PANTECH_CAMERA_EF60S_61K_62L) || defined(F_PANTECH_CAMERA_EF63SS)
#define F_PANTECH_CAMERA_S5K6B2YX
#endif

#if (defined(F_PANTECH_CAMERA_EF63SS) && (CONFIG_BOARD_VER > CONFIG_PT10))
#define F_PANTECH_CAMERA_IMX214
#endif


/*******************************************************************
 * 
 *           ACTUATOR
 *
 *******************************************************************/
 
#if defined(F_PANTECH_CAMERA_EF56S_57K_58L) || defined(F_PANTECH_CAMERA_EF59S_59K_59L) || defined(F_PANTECH_CAMERA_EF60S_61K_62L) || (defined(F_PANTECH_CAMERA_EF63SS) && (CONFIG_BOARD_VER == CONFIG_PT10))
#define F_PANTECH_CAMERA_ACT_WV560
#endif

#if defined(F_PANTECH_CAMERA_EF63SS)
#define F_PANTECH_CAMERA_ACT_RUMBA_SA
#endif

/*******************************************************************
 * 
 *           SENSOR TYPE
 *
 *******************************************************************/

#if defined(F_PANTECH_CAMERA_IMX135) || defined(F_PANTECH_CAMERA_S5K6B2YX) || defined(F_PANTECH_CAMERA_IMX214)
#define F_PANTECH_CAMERA_SENSOR_BAYER
#endif

/*******************************************************************
 * 
 *           SENSOR SPECIFICs
 *
 *******************************************************************/

#ifdef F_PANTECH_CAMERA_CE1612
#define F_PANTECH_CAMERA_CFG_WDR
#define F_PANTECH_CAMERA_ADD_CFG_UPDATE_ISP
#define F_PANTECH_CAMERA_ADD_CFG_READ_REG

#define F_PANTECH_CAMERA_CFG_STOP_CAPTURE
#endif

#if !defined(F_PANTECH_CAMERA_OV8820)
#define F_PANTECH_CAMERA_BACKFACE_YUV
#endif

#ifdef F_PANTECH_CAMERA_CE1502
#define F_PANTECH_CAMERA_CFG_GET_FRAME_INFO
#define F_PANTECH_CAMERA_CFG_YUV_ZSL_FLASH
#define F_PANTECH_CAMERA_ADD_CFG_OJT
#endif

#if defined(F_PANTECH_CAMERA_IMX135) || defined(F_PANTECH_CAMERA_IMX214)
/* 
 * EEPROM feature code 
*/
#define F_PANTECH_CAMERA_EEPROM
#endif

#if defined(F_PANTECH_CAMERA_IMX214)
#define F_PANTECH_CAMERA_ADD_OIS
#ifdef F_PANTECH_CAMERA_ADD_OIS
#define CONFIG_PANTECH_CAMERA_ADD_OIS
#endif
#endif

#ifndef T_BUILD_USER
/*#ifndef FEATURE_PANTECH_RELEASE */
/* 커널 영역 코드에서 SKYCDBG/SKYCERR 매크로를 사용하여 출력되는 메시지들을
 * 활성화 시킨다. 
 * kernel/arch/arm/mach-msm/include/mach/camera.h 
 * 위 파일에서 #define F_PANTECH_CAMERA_LOG_PRINTK을 정의하여 로그를 열 수 있다.
*/
/* 유저 영역(vendor)의 SKYCDBG/SKYCERR 메세지 on off */
#define F_PANTECH_CAMERA_LOG_OEM		

/* vendor/qcom/proprietary/mm-camera/mm-camera2/include/camera_dbg.h 
 * vendor/qcom/proprietary/mm-camera/mm-camera2/media-controller/modules/sensors/module/sensor_dbg.h 
 * vendor/qcom/proprietary/mm-camera/mm-camera2/media-controller/modules/imglib/components/include/img_dbg.h 
 * 위 파일들에서 #define F_PANTECH_CAMERA_LOG_PRINTK을 정의하여 로그를 열 수 있다.
 * 유저 영역(vendor)의 CDBG 메세지 on off 
 * 디버깅을 위해 각 파일에서 로그를 열어 사용하며, 각 파일의 로그 레벨에 따른 로그를 출력 할 수 있다.
 * camera_dbg.h : CDBG_LEVEL (0 ~ 2)
 * sensor_dbg.h : SLOG_HIGH, SLOG_LOW
 * img_dbg.h : IDBG_LOG_LEVEL (1 ~ 4)
*/
/*#define F_PANTECH_CAMERA_LOG_CDBG*/

/* 유저 영역의 LOGV/LOGD/LOGI 메세지 on off */
#define F_PANTECH_CAMERA_LOG_VERBOSE
#endif
/*
 * 카메라 Preview 중 유저 영역의 반복 되는 로그를 삭제한다.
 * preview 중 불필요한 반복 로그로 인해 다른 로그를 보기가 어렵고 디버깅을 어렵게 하므로 삭제한다.
*/
#define F_PANTECH_CAMERA_ERASE_PREVIEW_LOGS

/*
 * Bayer tuning을 위한 chromatix 파일을 load 해서 적용하도록 한 기능
 * Tuning 완료 후 제거 필요
*/
#ifdef F_PANTECH_CAMERA_SENSOR_BAYER
#define LOAD_CHROMATIX_DATA_FROM_FILE
#define F_PANTECH_CAMERA_TUNING

/*
 * 2013.11.01.
 * Bayer Camera sensor 사용시 analog gain 과 함께 digital gain을 사용 할 수 있도록 코드 적용.
 * real gain으로 부터 실제 analog gain과 함께 digital gain은 sensor의 user driver에서 설정 하도록 하여야 한다.
 * EF56/59/60 계열에서는 IMX135 sensor에서만 digigal gain을 적용 하였다.
*/
#define F_PANTECH_CAMERA_TUNING_CFG_DIGITAL_GAIN
#endif



/*----------------------------------------------------------------------------*/
/*  PANTECH CUSTOMIZATION                                                             */
/*  팬택 Device 또는 모델에 맞게 수정 또는 최적화                             */
/*----------------------------------------------------------------------------*/

/* 지원 가능한 촬영 해상도 테이블을 수정한다. 
 *
 * HAL 에서는 유효한 촬영 해상도를 테이블 형태로 관리하고 테이블에 포함된 
 * 해상도 이외의 설정 요청은 오류로 처리한다. */
#define F_PANTECH_CAMERA_CUST_PICTURE_SIZES

/* 단말에서 촬영된 사진의 EXIF TAG 정보 중 제조사 관련 정보를 수정한다. */
#define F_PANTECH_CAMERA_CUST_OEM_EXIF_TAG

/* UI에서 "num-snaps-per-shutter" parameter를 set 함으로써 burst shot을 설정한 후 
 * takePicture를 통해 burst shot을 촬영 할 수 있도록 한다.
 *
 * burst shot 촬영시 촬영음은 single shot과 마찬가지로 Camera Service에서 play 하
 * 도록 한다.
 * 해상도 이외의 설정 요청은 오류로 처리한다. */
#define F_PANTECH_CAMERA_CUST_BURSTSHOT

#ifdef F_PANTECH_CAMERA_CUST_BURSTSHOT
/* 
 * burst shot 중 UI로 부터 stop command를 받아 처리하도록 한다.
 * UI는 Camera.java의 stopBurstShot(int numBurst) 를 호출 할 수 있다.
 * StopBurstShot command를 받으면 numBurst 만큼 jpeg callback을 완료
 * 한 후 cancelPicture를 호출 하여 BurstShot을 종료 할 수 있다.
*/
#define F_PANTECH_CAMERA_CUST_STOP_BURSTSHOT

/* 
 * burst shot인 경우 촬영음을 CameraService에서 발생하지 않고,
 * UI에서 촬영음을 재생하도록 한다.
 * "pantech-burst-sound" 을 "disable" 로 설정을 하면 burst shot
 * 인 경우 촬영음을 재생하지 않는다. 단, shutter callback을 발생
 * 한다.
*/
#define F_PANTECH_CAMERA_CUST_BURST_UI_SOUND
#endif

/* 
 * 공정 command flash test를 위해 추가한 flash mode : FLASH_MODE_TORCH_FLASH
 * tourch mode 뿐 아니라 flash mode를 테스트 할 수 있다.
 * flash mode로 임의 발광 한 후에는 일정 시간 후 자동으로 flash off 되도록 한다.
*/
#define F_PANTECH_CAMERA_CUST_TORCH_FLASH

/* 
 * CameraHAL에서 ISP(VFE) 관련 초기 값을 설정한다.
 * initDefaultParameters() 의 설정 중 Pantech의 default 사양에 맞게 수정한 부분 임
 * 정확한 동작 및 카메라 진입 속도 최적화를 위해 UI의 default 값 그리고 ISP(VFE)의
 * default 설정 값이 일치하도록 하여야 한다.
 * 특히 ISP(VFE)의 초기값과 일치하는지 확인 하여 진행 한다. */
#define F_PANTECH_CAMERA_CUST_INIT_DEFAULT_PARAM

 /* 
 * Android Gingerbread 이전 버전에서는 "preview-frame-rate" 으로 fixed fps를 설정 하였다.
 * Android Gingerbread 버전 부터는 "preview-fps-range-values" 가 추가 되었으며, min_fps 
 * 와 max_fps를 설정 하도록 한다. 
 * "preview-fps-range-values"는 가변 frame rate을 설정 할 수 있으며, min, max가 같은 경우
 * 에는 fixed fps로 설정이 된다.
 * 따라서 "preview-fps-range-values"를 사용 할 것을 권장 하지만 기존의 "preview-frame-rate"
 * 역시 지원을 하고 두 파라미터를 같이 쓰는데 충돌이 없도록 설정 부분을 수정 한다. */
#define F_PANTECH_CAMERA_CUST_FPS


#define F_PANTECH_CAMERA_CUST_VT_TUNING

/* 2014.01.08. jjhwang
 * AF와 플래쉬가 연동되도록 하는 기능을 추가하기 위한 feature
 * 터치 AF를 비롯하여 일반 AF시에는 플래쉬 모드에 따라 AF 동작을 위해 플래쉬가 켜진다.
 * continuous AF 시에는 플래쉬 모드랑 상관없이 AF search시에 플래쉬를 사용하지 않는다.
 * AF mode가 continuous AF인 경우에는 촬영시에 flash mode에 따라 flash+AF 를 수행하여
 * AF를 다시 잡고 촬영 한다.
 * 해당 기능은 MSM8974 모델 부터 가능하다. (EF56 이후)
*/
/* #define F_PANTECH_CAMERA_CUST_AF_FLASH */
#ifdef F_PANTECH_CAMERA_CUST_AF_FLASH
#define F_PANTECH_CAMERA_CUST_AF_FLASH_QBUG
#endif

 /* 
 *  VT의 프리뷰에서 사용을 하는 preview 해상도를 추가 정의 한다.
 *  VT에서만 LCD와 직각 방향의 해상도를 사용하도록 기존의 preview 해상도에 추가 하지
 *  않고 따로 table을 만들어 사용한다.
 *  따라서 pantech의 VT app에서만 해상도를 사용 한다. */
#define F_PANTECH_CAMERA_FIX_VT_PREVIEW

/* 
 * stagefright로 인코딩 된 동영상이 KT에서 전송이 안되는 현상
 * 동영상 트랙 헤더의 "pasp" 부분을 KT서버에서 파싱을 못하는 것으로 보여지며 SKT나 LG향에서는 정상적으로 동작이 되어짐
 * KT향의 경우 서버 수정이 불가한 상황을 대비하여 헤더의 해당 부분을 넣지 않도록 함. 향 후 KT에서 수정 될 수 있는 사항
 * pasp 가 없어도 문제 없으므로 통신사 상관 없이 빠지도록 한다.
 */
#define F_PANTECH_CAMERA_FIX_MMS_PASP

/***********************************************************
2014.04.09 
AE issue when FLASH ON TO OFF
Added by H/W Platform
************************************************************/
#define F_PANTECH_CAMERA_CUST_TUNING_FLASH_ON_TO_OFF

/*
 * Moving the location calling releaseSound() from ~Client() to removeClientByRemote().
 * The pid of caller on Client is different from ~Client. so Sound pool can't remove the shutter sound object because pids are different.
 * the pid of Client is application's but the pid of ~Client is MediaServer's.
*/
#define F_PANTECH_CAMERA_FIX_MEDIAPLAYER_CALLING_PID

/* SKY캠코더 녹화파일이 Qparser로 확인시 에러발생.(deocde thumnail할수없음)
 * 캠코더 레코딩시 구글캠코더와 SKY캠코더의 차이중 하??가
 * app에서 내려오는 stagefrightrecorder의 mMaxFileDurationUs 값이다.
 * (SKY캠코더: 3600000000(us)=1시간 / 구글캠코더: 600000000(us)=10분.)
 * mMaxFileDurationUs의 차이로인해 Mpeg4write에서
 * SKY캠코더는 64bitfileoffset / 구글캠코더는 32bitfileoffset를 사용하게 된다.
 * 이를 32bitfileoffset으로 동일하게 설정하기 위해서 해당부분을 수정한다.
 * 임시로 수정되는 부분이므로 추가 검토 및 지속적인 모니터링이 필요함.
 * EF52 이후에서는 max recording size를 4GB로 늘리기 위해 64bitfileoffset으로 변경
 * 하였으며, 다만, MMS인 경우에는 32bitfileoffset 으로 recording 하도록 한다.
*/
#define F_PANTECH_CAMERA_VIDEO_REC_FILEOFFSET


/* 촬영 속도 개선을 위해 sound file load 시점을 camera client 생성시가 아니라 
 * camera service 생성시로 옮김
*/
#define F_PANTECH_CAMERA_MOVE_LOAD_SOUND

/*
 * WFD(wifi-display)를 사용을 하는 경우 적용 한다.
 * WFD 사용하는 경우 모든 음원이 리모트로 전송되어 이 경우 카메라 촬영음도 실제 촬영을 한
 * 로컬 폰에서 소리가 나지 않고 리모트 단말에서 소리가 남
 * 카메라 셔터음은 항상 촬영한 폰에서 소리가 나야 하므로, 로컬에서 소리나도록 관련 코드 적용
*/
#define F_PANTECH_CAMERA_PLAYSOUND_IN_WFD


/* MDM 정책에 따라 카메라 차단 여부를 결정한다 */
#define F_PANTECH_CAMERA_MDM_CHECK


/*----------------------------------------------------------------------------*/
/*  PANTECH ADD FEATURE                                                       */
/*  팬택 Device 또는 모델에 특성에 따라 기능 또는 API 추가                    */
/*----------------------------------------------------------------------------*/

/* 동영상 녹화 중 recordig file size 를 app 으로 전달해 준다. */
#define F_PANTECH_CAMERA_ADD_EVT_RECSIZE

/*
 * jpeg file exif 정보 추가
*/
#define F_PANTECH_CAMERA_ADD_EXIF_DATA

/* 2014.01.02. manual focus 
 * focus-mode에 manual mode를 추가 하여 UI에서 lens 위치를 특정 위치로 이동 시킬 수 있도록 한다.
 * focus-mode는 manual로 설정하고 pantech-focus-step를 0 ~ 9 까지 설정 하여 lens를 이동시킨다.
 * focus step을 max 80기반으로 10단계로 나누었다.
 * 추가된 manaul focus 기능을 이용하여 EF60 계열의 범퍼 케이스 카메라를 위한 focus를 UI에서 추가 한다.
*/
#define F_PANTECH_CAMERA_ADD_MANUAL_FOCUS

/*Morpho HDR에서 사용하는 feature
 *
 * HDR on으로 설정할 경우 3 장의 영상을 사용하여 HDR processing을 한다.
*/
#ifndef F_PANTECH_CAMERA_TARGET_NAMI
#define F_PANTECH_CAMERA_CFG_HDR

/*
 *HDR checker
 *evaluate preview and set aebracket
*/
#define F_PANTECH_CAMERA_HDR_CHECKER

/*
 * HDR 촬영 시 thumbnail 을 main image 로 부터 만든다.
 * 간혹 main image 와 thumbnail 이 다른 경우가 생겨서 이와 같이 수정 함.
 */
#define F_PANTECH_CAMERA_HDR_THUMBNAIL

/*
 * recording pause 후에 취소키를 누르면 카메라가 종료되는 문제 수정
*/
#define F_PANTECH_CAMERA_RECORDING_PAUSE

/*
 * MSM8974 KK 에서 90,270 rotation 으로 HDR 촬영 시 thumbnail 이 밝게 저장되는 문제 수정
*/
#define F_PANTECH_CAMERA_HDR_THUMBNAIL_KK

#if defined(F_PANTECH_CAMERA_EF63SS)
/****************************************************************
 * Description : 저조도 촬영을 지원한다. (LOW LIGHT SHOT)
                        5장 촬영 Multi Processing 처리.
 * Creation Date : 2014/2/14
 * Added By : Leehwanhee
 ****************************************************************/
#define F_PANTECH_CAMERA_CFG_LLS 
#endif
#endif

/* F_PANTECH_CAMERA_TODO, SKT FOTA DCMO (Device Control Management Object)
 * SKT 향에만 적용되며, UI VOB에서만 define을 연다.
 * "pantech/development/sky_fota/sky_fota.h" 파일이 있어야 한다.
*/
/* #define F_PANTECH_CAMERA_FOTA_DCMO_CAMERA_LOCK */

/* AOT(Always On Top) CAMERA 추가 사항.
 * 카메라 동작상태를 체크하도록 CameraService::isRunning() 함수 추가.
*/
#define F_PANTECH_CAMERA_AOT_ISRUNNING

/* F_PANTECH_CAMERA_TODO
 * 동작인식 관련 Feature.
 * Preview buffer에 frame 들어가지 않도록 구현
*/
#if defined(F_PANTECH_CAMERA_EF59S_59K_59L) || defined(F_PANTECH_CAMERA_EF60S_61K_62L) || defined(F_PANTECH_CAMERA_EF63SS)
#define F_PANTECH_CAMERA_VTS
#endif

/*
  *raw capture 가 dual isp 에서 동작하지 않아 raw capture 시에는 single isp 로 모드전환을 한다.
*/
#define F_PANTECH_CAMERA_SINGLE_ISP

 /*
  * MSM8974 에서 전면 카메라 촬영 시 thumbnail 이 원본과 다르게 반전되어 저장되는 문제 수정
  */
#define F_PANTECH_CAMERA_THUMBNAIL_FLIP

/* 
 * Flip Cover의 window에서 실행 되는 SmartFlipCamera를 지원하기 위한 기능.
 * window의 size 및 비율이 일반적으로 사용 되는 것이 아니므로, preview/snapshot/recording 
 * table에 해상도를 추가 하지 않고 SmartFlipCamera 어플에서만 사용 할 수 있도록 한다.
 * window의 size는 모델별로 또는 개발 중 변경 될 수가 있으므로 사전에 협의 된 해상도를 UI
 * 에서 정상적으로 설정을 했다고 가정을 하고 해상도를 따로 check 하지 않도록 한다.
 * 설정 파라미터 - "pantech-window-camera" : "enable"/"disable"
*/
#define F_PANTECH_CAMERA_CUST_FLIP_COVER_CAMERA

 /*
  * 동일한 카메라 진입시 이전 카메라 종료 하도록하고 
  * flash app의 경우 카메라 진입시 flash 종료 하도록 수정.
 */
#define F_PANTECH_CAMERA_DEVICE_CONNECT_FAIL_FIXED

/*----------------------------------------------------------------------------*/
/*  Qualcomm BUGS                                                             */
/*  Qualcomm bug로 주로 SR, 패치로 수정.                             */
/*  이 후 Qualcomm main stream에 반영사항 주시가 필요 함.                     */
/*----------------------------------------------------------------------------*/

/* 2014.01.28.
 * sensor mode 선택시에 Qualcomm source bug로 인해 최적화된 mode가 선택 되어지지 않음.
 * bug 수정하여 설정에 따른 최적화된 mode로 선택 되어지도록 수정.
*/
#define F_PANTECH_CAMERA_QBUG_SENSOR_PICK_RES

/* case#01169476 
 * 전/후면 카메라를 동시에 사용 하기 위해 MSM8974에서 Dual VFE를 적용한다. 
 * Dual VFE 기능 중 simultaneous camera feature가 추가 되었으나, 개발 중 
 * 1023 release에 bug가 있어 퀄컴으로 부터 패치를 받아 적용 하였다.
 * 커널 영역의 수정 사항은 다음으로 확인 가능하다.
 * #if(n)def CONFIG_PANTECH_CAMERA // case#01169476 simultaneous camera
 */
#define F_PANTECH_CAMERA_QBUG_SIMULTANEOUS_CAM

/* case#01187023 
 * 13M@24fps ZSL을 적용하기 위해 8974의 highperformance(dual vfe)를
 * 적용하면서 발생한 문제를 수정하기 위해 적용한 코드
 * 이슈 : ZSL OFF/ON 설정시 VFE0와 VFE1의 frame sync가 맞지 않는 문제
 *
 * 1 : 2013-05-14 : 이슈 재현은 되지만 CASE로 부터 받은 코드
 */
#define F_PANTECH_CAMERA_QBUG_HIGH_PERFORMANCE

/* 
 * ZSL 캡쳐된 이미지가 깨지는 현상에 대한 수정.
 * 이전 프레임과 다음 프레임이 중간에 잘려서 하나의 frame으로 저장 되는 현상 임.
*/
#define F_PANTECH_CAMERA_FIX_QBUG_BROCKEN_IMAGE


/*
 * 깨진 PNG file 을 decoding 할 때, skia decoder 에서 fail이 발생 하고,
 * 이로 인해 겔러리에서 비정상 종료 되는 문제
*/
#define F_PANTECH_CAMERA_FIX_PNG_DECORDING_ERROR_WIH_BROKEN_DATA

/*
 * Snapshot dump img 가 세로로 촬영 시 정상적으로 보이지 않던 문제 수정
*/
#define F_PANTECH_CAMERA_QBUG_DUMP_IMAGE

/*
 *fix wrong image capture problem, the cause was wrong buffer count
 *case # 01466765
*/
#define F_PANTECH_CAMERA_QBUG_WRONG_IMAGE_CAPTURE

/*
 * stop preview error during dual camera recording stop
 *fix SIGSEGV in mct_pipeline_check_stream, case#01473686
*/
#define F_PANTECH_CAMERA_QBUG_STOP_PREVIEW

/*
 * 2014/03/11
 * case#01473540
 * increasing meta buffer cnt from 8 to 10
*/
#define F_PANTECH_CAMERA_QBUG_PREPARE_SNAPSHOT

/****************************************************************
 * Description : case#01504004
fix prepare snapshot done problem
 * Creation Date : 2014/04/21
 * Added By : Ha Dong Jin
 ****************************************************************/
#define F_PANTECH_CAMERA_QBUG_PREPARE_SNAPSHOT_DONE

/*
* 2013/12/02, Ha Dong Jin
* EF61K PLM#664, CASE#1349270, CR#522939
* Clean-up the msm camera generic buff queue on userspace
* crash to avoid invalid memory access in next session
* which might lead to corruption in other modules
* and system stability issues and also avoid memory leak.
*/
#define F_PANTECH_CAMERA_QBUG_CLEANUP_MSM_GENERIC_BUF


/*
 * Effect Recording 시작하면 Effect 가 사라지고 녹화되는 현상 SR 로 수정 case# 01215481
*/
#define F_PANTECH_CAMERA_QBUG_EFFECT_RECORDING

/*
 * focus mode를 "continuous-video" 에서 "auto"로 변경 하였을 때,
 * continuous af가 stop 되지 않는 문제 수정.
 * camcorder caf file로 부터 algo type을 잘못 load 하게 되며, 이
 * 로인해 caf_state 오류로 caf가 stop 되지 않는 문제 임.
*/
#define F_PANTECH_CAMERA_QBUG_CONTINUOUS_VIDEO_STOP


#ifdef F_PANTECH_CAMERA_IMX135
/*
 * IMX135 sensor에서 지원하는 동영상HDR 기능 적용시 imx135_fill_awb_hdr_array
 * 호출 되지 않는 문제 수정
*/
#define F_PANTECH_CAMERA_QBUG_MOVIE_HDR

/*
 * IMX135 sensor에서 지원하는 동영상HDR 기능 최적화를 위해
 * customizing
*/
#define F_PANTECH_CAMERA_CUST_MOVIE_HDR
#endif

/*
 * 카메라 캡쳐시 shutter callback 관련하여 미흡한 코드 부분 구현
 * shutter callback시 sound play 여부를 판단 하여 1회에 한하여 
 * callback 이 되도록 수정 한다.
*/
#define F_PANTECH_CAMERA_QBUG_SHUTTER_CALLBACK

/*
 * 카메라 촬영 중 Home Key를 눌러 stopPreview command 발생시 카메라 halt 되는
 * 문제 수정
 * OMX Jpeg encoder release 중 halt 되는 문제
*/
#define F_PANTECH_CAMERA_QBUG_STOP_ON_SNAPSHOT

/*
 *Start preview 전에 flash or movie mode 설정시 적용 안되는
 *문제 workaround 수정
 */
#define F_PANTECH_CAMERA_QBUG_LED_MODE

/* 동영상 녹화 시작/종료를 빠르게 반복하거나, 이어잭을 장착한 상태에서 연속촬영
 * 모드로 촬영할 경우, MediaPlayer 가 오동작하면서 HALT 발생한다.
 *
 * MediaPlayer 의 경우, 동일한 음원을 재생 중에 또 다시 재생 시도할 경우 100%
 * 오동작하므로 동일 음원을 연속하여 재생해야 할 경우, 반드시 이전 재생이 완료
 * 되었는지 여부를 확인 후 재생해야 한다. 
 * 
 * 2014/02/19, sangwon, prevented this feature to resolve a PLM(61K_KK,#00654) issue that is related to burst shot in case of using SD storage.
 * this issue is occured when takePicture command rapidly because the duration time of calling playSound() is shorter than play time of shutter sound.
 * to replay shutter sound we needed to add calling pause() because of the bug of mediaPlayer but it was very long time ago and MediaPlayer don't have to be called pause CameraService included in MediaServer.
 */
//#define F_PANTECH_CAMERA_QBUG_SKIP_CAMERA_SOUND  

/*
* 초기 진입시 Sharpness 적용되지 않는 문제 수정.
*/
#define F_PANTECH_CAMERA_QBUG_SHARPNESS

/*
 *MSM8974 KK 에서 발생하는 thumbnail rotation 문제를 수정하기 위한 QC patch code 적용, case# 01438139
*/
#define F_PANTECH_CAMERA_QBUG_THUMB_ROTATION


#define F_PANTECH_CAMERA_QBUG_FIX_GOOFY_FACE_RECORD

/*
* Recording 시작시 Effect 적용되지 않는 문제 수정.
*/
#define F_PANTECH_CAMERA_QBUG_EFFECT


/*
* CPL patch후 camera start/stop preview에서 map/unmap 중 hold 되는 문제
*/
#define F_PANTECH_CAMERA_QBUG_HOLD_DURING_MAP_UNMAP

/*
* The camera mode is picked up according to resolution and fps but the snapshot chromatix in the orignal code is just set with MSM_SENSOR_RES_FULL is 0.
* So, modified code is selecte snapshot chromatix according to camera mode selected.
*/
#define F_PANTECH_CAMERA_QBUG_WRONG_SNAPSHOT_CHROMATIX

/*
 * live shot 촬영시 live shot 해상도를 picture size가 아니라, recording size를 사용 하도록 수정
 */
 #define F_PANTECH_CAMERA_CUST_LIVESNAPSHOT

/*
* [2014/01/09, CASE : 01413457]
* VFE MAX clock 설정 값을 잘못 받아오는 문제 수정
*/
#define F_PANTECH_CAMERA_QBUG_VFE_MAX_CLOCK_SETTING

/*
* 2013/12/19/ case : 01388126
* Qualcomm fixes for known FD leakage issues in camera side
*/
#define F_PANTECH_CAMERA_QBUG_FD_LEAKAGE

/*
* 2013/12/19/ case : 01388126
* Qualcomm fix memory leak in luma adaptation 
*/
/* #define F_PANTECH_CAMERA_QBUG_LUMA_LEAKAGE */

/*
* 2014.01.23. - case#01428626
* 카메라 stability 이슈, stream on 할때 fail 발생. 아래 error log 출력
*       mm-camera stats_port_event: Failure posting to the bus!
*       mm-camera isp_ch_util_streamon_ack: error, ISP_HW_ACTION_CODE_STREAM_START_ACK, sessid = 1, vfe_id = 0, rc = -1
* ISP Buffer manager 에서 mutex 관련 handling에 문제가 있어 퀄컴 patch code 적용
* MSM8974 JB에서 동일 이슈로 patch 수정 하였으나, KK에서 일부 적용 및 코드 변경으로 인해 JB patch 적용 못하고
* LINUX build ID LNX.LA.3.5-07510-8x74.0-1 base로 patch code 받아서 적용.
*/
#define F_PANTECH_CAMERA_QBUG_MUTEX_FOR_ISP_BUFMGR

/*
 *2014/2/18 case : 01452485
 *fix module_imglib_clear_session_params fail
*/
#define F_PANTECH_CAMERA_QBUG_IMGLIB_CLEAR

/* 2014/3/18 case : 01484820
 * destroy super buf cmd thread  
*/
#define F_PANTECH_CAMERA_QBUG_RELEASE_THREAD

/*
* 2014/02/13  lee sangwon
* video size 4k x 2k(UHD) support is enabled
*/
#define F_PANTECH_CAMERA_SUPPORT_4K2K_UHD


/*
* 2014.03.27. case:01475905
* stream on fail issue when preview is started
* in issue case, below fail log is occured.
* "QCameraStream Failed to config stream, rc = -1"
* Additionally, you can see only the log for enterance of preview_start without ending of start_preview.
*/
#define F_PANTECH_CAMERA_QBUG_STREM_ON_FAIL

/*
* 2014/02/28  lee sangwon
* Added the QCom Patch about High Speed Recording (CR605405), Case#01460192
*/
#if defined(F_PANTECH_CAMERA_EF63SS)
#define F_PANTECH_CAMERA_SUPPORT_HSR
#endif

/*
* 2014/02/28  lee junhee
* Fix Camcoder Recording File Audio not Sync
* Because Add H/W ACC Encoder use code in StageFrightRecorder.cpp by Post CR Patch
*/
#define F_PANTECH_CAMERA_FIX_CAM_AUDIO_SYNC

/*
 *2014/03/10
 *Fix mms recording problem. The track header size is too big in mms recording case.
 *So set it to 6KB.
*/
#define F_PANTECH_CAMERA_MMS_RECORDING

/****************************************************************
 * Description : fixed issue that jpeg exif data is not seen as flash fired in torch mode.
 * Creation Date : 2014/03/24
 * Added By : lee hwanhee
 ****************************************************************/
#define F_PANTECH_CAMERA_JPEG_EXIF_DATA_IN_TORCH_MODE

/****************************************************************
 * Description : bug fixed for recording stopping during liveshot
 * Creation Date : 2014/04/21
 * Added By : lee hwanhee
 ****************************************************************/
#define F_PANTECH_CAMERA_RECORDING_STOP_DURING_LIVESHOT

/*----------------------------------------------------------------------------*/
/*  MODEL-SPECIFIC CONSTANTS                                                  */
/*  모델 관련 상수 선언                                              */
/*----------------------------------------------------------------------------*/

/* camera select enum */
#define C_PANTECH_CAMERA_SELECT_MAIN_CAM 		0
#define C_PANTECH_CAMERA_SELECT_SUB_CAM 		1

#ifndef F_PANTECH_CAMERA_TARGET_NAMI
//P11232 US4 CAMERA
#define F_PANTECH_CAMERA_CFG_IPL_SKY_PROCESSING
#ifdef F_PANTECH_CAMERA_CFG_IPL_SKY_PROCESSING
#define F_PANTECH_CAMERA_CFG_CAMNOTE
#define F_PANTECH_CAMERA_CFG_MINIATURE
#define F_PANTECH_CAMERA_CFG_COLOREXTRACTION
#endif
#endif

#ifdef F_PANTECH_CAMERA_CUST_OEM_EXIF_TAG
#define C_PANTECH_CAMERA_EXIF_MAKE		"PANTECH"
#define C_PANTECH_CAMERA_EXIF_MAKE_LEN		8		/* including NULL */
#ifdef F_PANTECH_CAMERA_TARGET_EF39S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A800S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF40S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A810S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF40K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A810K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_PRESTO
#define C_PANTECH_CAMERA_EXIF_MODEL		"PRESTO"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		7		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF45K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A830K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF46L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A830L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF47S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A830S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_SVLTE
#define C_PANTECH_CAMERA_EXIF_MODEL		"SVLTE"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		6		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_CSFB
#define C_PANTECH_CAMERA_EXIF_MODEL		"CSFB"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		5		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_CHEETAH
#define C_PANTECH_CAMERA_EXIF_MODEL		"CHEETAH"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		8		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_STARQ
#define C_PANTECH_CAMERA_EXIF_MODEL		"STARQ"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		6		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_OSCAR
#define C_PANTECH_CAMERA_EXIF_MODEL		"OSCAR"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		6		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_VEGAPVW
#define C_PANTECH_CAMERA_EXIF_MODEL		"ADR930L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		8		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_ZEPPLIN
#define C_PANTECH_CAMERA_EXIF_MODEL		"ZEPPLIN"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		8		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_SIRIUSLTE
#define C_PANTECH_CAMERA_EXIF_MODEL		"SIRIUSLTE"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		10		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF44S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A840S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_MAGNUS
#define C_PANTECH_CAMERA_EXIF_MODEL		"P9090"		//MAGNUS
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		6		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF48S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A850S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF49K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A850K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF50L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A850L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF51S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A860S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF51K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A860K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF51L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A860L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF52S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A851L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF52K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A851L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF52L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A851L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF52W
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A851L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF56S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A880S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF57K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A880K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF58L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A880L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF59S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A890S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF59K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A890K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF59L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A890L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF60S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A900S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF61K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A900K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF62L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A900L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_NAMI
#define C_PANTECH_CAMERA_EXIF_MODEL		"NAMI"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		5		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF63S
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A910S"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF63K
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A910K"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#ifdef F_PANTECH_CAMERA_TARGET_EF63L
#define C_PANTECH_CAMERA_EXIF_MODEL		"IM-A910L"
#define C_PANTECH_CAMERA_EXIF_MODEL_LEN		9		/* including NULL */
#endif
#endif

#endif /* F_PANTECH_CAMERA */

#endif /* CUST_PANTECH_CAMERA.h */
