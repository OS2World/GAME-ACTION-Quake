/****************************************************************************/
/*                                                                          */
/* Module Name: MEERROR.H                                                   */
/*                                                                          */
/* OS/2 2.0 Multimedia Extensions Error Return Codes                        */
/*                                                                          */
/* Copyright (c) International Business Machines Corporation 1991, 1992     */
/*                        All Rights Reserved                               */
/*                                                                          */
/****************************************************************************/
/* NOINC */
#ifdef __cplusplus
   extern "C" {
#endif
/* INC */


#ifndef _MEERROR_H_
#define _MEERROR_H_
#endif

#ifndef NO_ERROR
#define NO_ERROR                          0
#endif

/*****************************************/
/* MCI Device Manager Error Return codes */
/*****************************************/

#define MCIERR_BASE                      5000
#define MCIERR_SUCCESS                   0
#define MCIERR_INVALID_DEVICE_ID         (MCIERR_BASE + 1)
#define MCIERR_NO_MASTER                 (MCIERR_BASE + 2)
#define MCIERR_UNRECOGNIZED_KEYWORD      (MCIERR_BASE + 3)
#define MCIERR_MASTER_CONFLICT           (MCIERR_BASE + 4)
#define MCIERR_UNRECOGNIZED_COMMAND      (MCIERR_BASE + 5)
#define MCIERR_HARDWARE                  (MCIERR_BASE + 6)
#define MCIERR_INVALID_DEVICE_NAME       (MCIERR_BASE + 7)
#define MCIERR_OUT_OF_MEMORY             (MCIERR_BASE + 8)
#define MCIERR_DEVICE_OPEN               (MCIERR_BASE + 9)
#define MCIERR_CANNOT_LOAD_DRIVER        (MCIERR_BASE + 10)
#define MCIERR_MISSING_COMMAND_STRING    (MCIERR_BASE + 11)
#define MCIERR_PARAM_OVERFLOW            (MCIERR_BASE + 12)
#define MCIERR_MISSING_STRING_ARGUMENT   (MCIERR_BASE + 13)
#define MCIERR_BAD_INTEGER               (MCIERR_BASE + 14)
#define MCIERR_PARSER_INTERNAL           (MCIERR_BASE + 15)
#define MCIERR_DRIVER_INTERNAL           (MCIERR_BASE + 16)
#define MCIERR_MISSING_PARAMETER         (MCIERR_BASE + 17)
#define MCIERR_UNSUPPORTED_FUNCTION      (MCIERR_BASE + 18)
#define MCIERR_FILE_NOT_FOUND            (MCIERR_BASE + 19)
#define MCIERR_DEVICE_NOT_READY          (MCIERR_BASE + 20)
#define MCIERR_INTERNAL                  (MCIERR_BASE + 21)
#define MCIERR_DRIVER                    (MCIERR_BASE + 22)
#define MCIERR_CANNOT_USE_ALL            (MCIERR_BASE + 23)
#define MCIERR_MULTIPLE                  (MCIERR_BASE + 24)
#define MCIERR_EXTENSION_NOT_FOUND       (MCIERR_BASE + 25)
#define MCIERR_OUTOFRANGE                (MCIERR_BASE + 26)
#define MCIERR_CANNOT_ADD_ALIAS          (MCIERR_BASE + 27)
#define MCIERR_FLAGS_NOT_COMPATIBLE      (MCIERR_BASE + 28)
#define MCIERR_CANNOT_USE_NOUNLOAD       (MCIERR_BASE + 29)
#define MCIERR_FILE_NOT_SAVED            (MCIERR_BASE + 30)
#define MCIERR_DEVICE_TYPE_REQUIRED      (MCIERR_BASE + 31)
#define MCIERR_DEVICE_LOCKED             (MCIERR_BASE + 32)
#define MCIERR_DUPLICATE_ALIAS           (MCIERR_BASE + 33)
#define MCIERR_INSTANCE_INACTIVE         (MCIERR_BASE + 34)

#define MCIERR_COMMAND_TABLE             (MCIERR_BASE + 35)
#define MCIERR_INI_FILE_LOCKED           (MCIERR_BASE + 37)

#define MCIERR_NO_AUDIO_SUPPORT          (MCIERR_BASE + 40)
#define MCIERR_NOT_IN_PM_SESSION         (MCIERR_BASE + 41)
#define MCIERR_DUPLICATE_KEYWORD         (MCIERR_BASE + 42)
#define MCIERR_COMMAND_STRING_OVERFLOW   (MCIERR_BASE + 43)
#define MCIERR_DRIVER_PROC_NOT_FOUND     (MCIERR_BASE + 44)
#define MCIERR_INVALID_DEVICE_TYPE       (MCIERR_BASE + 45)
#define MCIERR_INVALID_DEVICE_ORDINAL    (MCIERR_BASE + 46)
#define MCIERR_HEADPHONES_NOT_SET        (MCIERR_BASE + 47)
#define MCIERR_SPEAKERS_NOT_SET          (MCIERR_BASE + 48)
#define MCIERR_SOUND_NOT_SET             (MCIERR_BASE + 49)
#define MCIERR_INVALID_BUFFER            (MCIERR_BASE + 50)
#define MCIERR_INVALID_MEDIA_TYPE        (MCIERR_BASE + 51)
#define MCIERR_INVALID_CONNECTOR_INDEX   (MCIERR_BASE + 52)
#define MCIERR_NO_CONNECTION             (MCIERR_BASE + 53)
#define MCIERR_INVALID_FLAG              (MCIERR_BASE + 54)
#define MCIERR_CANNOT_LOAD_DSP_MOD       (MCIERR_BASE + 55)
#define MCIERR_ALREADY_CONNECTED         (MCIERR_BASE + 56)
#define MCIERR_INVALID_CALLBACK_HANDLE   (MCIERR_BASE + 57)
#define MCIERR_DRIVER_NOT_FOUND          (MCIERR_BASE + 58)
#define MCIERR_DUPLICATE_DRIVER          (MCIERR_BASE + 59)
#define MCIERR_INI_FILE                  (MCIERR_BASE + 60)
#define MCIERR_INVALID_GROUP_ID          (MCIERR_BASE + 61)
#define MCIERR_ID_ALREADY_IN_GROUP       (MCIERR_BASE + 62)
#define MCIERR_MEDIA_CHANGED             (MCIERR_BASE + 63)
#define MCIERR_MISSING_FLAG              (MCIERR_BASE + 64)
#define MCIERR_UNSUPPORTED_FLAG          (MCIERR_BASE + 65)
#define MCIERR_DRIVER_NOT_LOADED         (MCIERR_BASE + 66)
#define MCIERR_INVALID_MODE              (MCIERR_BASE + 67)
#define MCIERR_INVALID_ITEM_FLAG         (MCIERR_BASE + 68)
#define MCIERR_INVALID_TIME_FORMAT_FLAG  (MCIERR_BASE + 69)
#define MCIERR_SPEED_FORMAT_FLAG         (MCIERR_BASE + 70)
#define MCIERR_INVALID_AUDIO_FLAG        (MCIERR_BASE + 71)
#define MCIERR_NODEFAULT_DEVICE          (MCIERR_BASE + 72)
#define MCIERR_DUPLICATE_EXTENSION       (MCIERR_BASE + 73)
#define MCIERR_FILE_ATTRIBUTE            (MCIERR_BASE + 74)
#define MCIERR_DUPLICATE_CUEPOINT        (MCIERR_BASE + 75)
#define MCIERR_INVALID_CUEPOINT          (MCIERR_BASE + 76)
#define MCIERR_CUEPOINT_LIMIT_REACHED    (MCIERR_BASE + 77)
#define MCIERR_MISSING_ITEM              (MCIERR_BASE + 78)
#define MCIERR_MISSING_TIME_FORMAT       (MCIERR_BASE + 79)
#define MCIERR_MISSING_SPEED_FORMAT      (MCIERR_BASE + 80)
#define MCIERR_INVALID_CONNECTOR_TYPE    (MCIERR_BASE + 81)
#define MCIERR_TARGET_DEVICE_FULL        (MCIERR_BASE + 82)
#define MCIERR_UNSUPPORTED_CONN_TYPE     (MCIERR_BASE + 83)
#define MCIERR_CANNOT_MODIFY_CONNECTOR   (MCIERR_BASE + 84)
#define MCIERR_RECORD_ABORTED            (MCIERR_BASE + 85)
#define MCIERR_GROUP_COMMAND             (MCIERR_BASE + 86)
#define MCIERR_DEVICE_NOT_FOUND          (MCIERR_BASE + 87)
#define MCIERR_RESOURCE_NOT_AVAILABLE    (MCIERR_BASE + 88)
#define MCIERR_INVALID_IO_PROC           (MCIERR_BASE + 89)

#define MCIERR_WAVE_OUTPUTSINUSE         (MCIERR_BASE + 90)
#define MCIERR_WAVE_SETOUTPUTINUSE       (MCIERR_BASE + 91)
#define MCIERR_WAVE_INPUTSINUSE          (MCIERR_BASE + 92)
#define MCIERR_WAVE_SETINPUTINUSE        (MCIERR_BASE + 93)
#define MCIERR_WAVE_OUTPUTUNSPECIFIED    (MCIERR_BASE + 94)
#define MCIERR_WAVE_INPUTUNSPECIFIED     (MCIERR_BASE + 95)
#define MCIERR_WAVE_OUTPUTSUNSUITABLE    (MCIERR_BASE + 96)
#define MCIERR_WAVE_SETOUTPUTUNSUITABLE  (MCIERR_BASE + 97)
#define MCIERR_WAVE_INPUTSUNSUITABLE     (MCIERR_BASE + 98)
#define MCIERR_WAVE_SETINPUTUNSUITABLE   (MCIERR_BASE + 99)

#define MCIERR_SEQ_DIV_INCOMPATIBLE      (MCIERR_BASE + 100)
#define MCIERR_SEQ_PORT_INUSE            (MCIERR_BASE + 101)
#define MCIERR_SEQ_PORT_NONEXISTENT      (MCIERR_BASE + 102)
#define MCIERR_SEQ_PORT_MAPNODEVICE      (MCIERR_BASE + 103)
#define MCIERR_SEQ_PORT_MISCERROR        (MCIERR_BASE + 104)
#define MCIERR_SEQ_TIMER                 (MCIERR_BASE + 105)

#define MCIERR_VDP_COMMANDCANCELLED      (MCIERR_BASE + 106)
#define MCIERR_VDP_COMMANDFAILURE        (MCIERR_BASE + 107)
#define MCIERR_VDP_NOTSPUNUP             (MCIERR_BASE + 108)
#define MCIERR_VDP_NOCHAPTER             (MCIERR_BASE + 109)
#define MCIERR_VDP_NOSIDE                (MCIERR_BASE + 110)
#define MCIERR_VDP_NOSIZE                (MCIERR_BASE + 111)
#define MCIERR_VDP_INVALID_TIMEFORMAT    (MCIERR_BASE + 112)

#define MCIERR_CLIPBOARD_ERROR           (MCIERR_BASE + 114)
#define MCIERR_CANNOT_CONVERT            (MCIERR_BASE + 115)
#define MCIERR_CANNOT_REDO               (MCIERR_BASE + 116)
#define MCIERR_CANNOT_UNDO               (MCIERR_BASE + 117)
#define MCIERR_CLIPBOARD_EMPTY           (MCIERR_BASE + 118)

#define MCIERR_INVALID_WORKPATH          (MCIERR_BASE + 119)
#define MCIERR_INDETERMINATE_LENGTH      (MCIERR_BASE + 120)
#define MCIERR_DUPLICATE_EA              (MCIERR_BASE + 121)
#define MCIERR_INVALID_CONNECTION        (MCIERR_BASE + 122)
#define MCIERR_CHANNEL_OFF               (MCIERR_BASE + 123)
#define MCIERR_CANNOT_CHANGE_CHANNEL     (MCIERR_BASE + 124)
#define MCIERR_FILE_IO                   (MCIERR_BASE + 125)
#define MCIERR_SYSTEM_FILE               (MCIERR_BASE + 126)
#define MCIERR_DISPLAY_RESOLUTION        (MCIERR_BASE + 127)
#define MCIERR_NO_ASYNC_PLAY_ACTIVE      (MCIERR_BASE + 128)

#define MCIERR_UNSUPP_FORMAT_TAG         (MCIERR_BASE + 129)
#define MCIERR_UNSUPP_SAMPLESPERSEC      (MCIERR_BASE + 130)
#define MCIERR_UNSUPP_BITSPERSAMPLE      (MCIERR_BASE + 131)
#define MCIERR_UNSUPP_CHANNELS           (MCIERR_BASE + 132)
#define MCIERR_UNSUPP_FORMAT_MODE        (MCIERR_BASE + 133)
#define MCIERR_NO_DEVICE_DRIVER          (MCIERR_BASE + 134)
#define MCIERR_CODEC_NOT_SUPPORTED       (MCIERR_BASE + 135)

#define MCIERR_TUNER_NO_HW               (MCIERR_BASE + 136)
#define MCIERR_TUNER_NO_AFC              (MCIERR_BASE + 137)
#define MCIERR_TUNER_AFC_ON              (MCIERR_BASE + 138)
#define MCIERR_TUNER_CHANNEL_SKIPPED     (MCIERR_BASE + 139)
#define MCIERR_TUNER_CHANNEL_TOO_LOW     (MCIERR_BASE + 140)
#define MCIERR_TUNER_CHANNEL_TOO_HIGH    (MCIERR_BASE + 141)
#define MCIERR_AUD_CHANNEL_OUTOFRANGE    (MCIERR_BASE + 142)
#define MCIERR_TUNER_INVALID_REGION      (MCIERR_BASE + 143)
#define MCIERR_SIGNAL_INVALID            (MCIERR_BASE + 144)
#define MCIERR_TUNER_MODE                (MCIERR_BASE + 145)
#define MCIERR_TUNER_REGION_NOT_SET      (MCIERR_BASE + 146)
#define MCIERR_TUNER_CHANNEL_NOT_SET     (MCIERR_BASE + 147)
#define MCIERR_UNSUPP_CLASS              (MCIERR_BASE + 148)
#define MCIERR_UNSUPPORTED_ATTRIBUTE     (MCIERR_BASE + 149)

#define MCIERR_CUSTOM_DRIVER_BASE        (MCIERR_BASE + 256)


/******************************************/
/* Sync/Stream Manager Error Return codes */
/******************************************/

#define MEBASE                           (MCIERR_BASE + 500)
#define ERROR_INVALID_STREAM             (MEBASE + 1)
#define ERROR_INVALID_HID                (MEBASE + 2)
#define ERROR_INVALID_NETWORK            (MEBASE + 3)
#define ERROR_INVALID_OBJTYPE            (MEBASE + 4)
#define ERROR_INVALID_FLAG               (MEBASE + 5)
#define ERROR_INVALID_EVCB               (MEBASE + 6)
#define ERROR_INVALID_EVENT              (MEBASE + 7)
#define ERROR_INVALID_MMTIME             (MEBASE + 8)
#define ERROR_INVALID_NUMSLAVES          (MEBASE + 9)
#define ERROR_INVALID_REQUEST            (MEBASE + 10)
#define ERROR_INVALID_SPCBKEY            (MEBASE + 11)
#define ERROR_INVALID_HNDLR_NAME         (MEBASE + 12)
#define ERROR_INVALID_PROTOCOL           (MEBASE + 13)
#define ERROR_INVALID_BUFFER_SIZE        (MEBASE + 14)
#define ERROR_INVALID_BUFFER_RETURNED    (MEBASE + 15)
#define ERROR_INVALID_ACB                (MEBASE + 16)
#define ERROR_INVALID_RECORD_RETURNED    (MEBASE + 17)
#define ERROR_INVALID_MESSAGE            (MEBASE + 18)

#define ERROR_STREAM_NOT_OWNER           (MEBASE + 99)
#define ERROR_STREAM_USED                (MEBASE + 100)
#define ERROR_STREAM_CREATION            (MEBASE + 101)
#define ERROR_STREAM_NOTMASTER           (MEBASE + 102)
#define ERROR_STREAM_NOT_STOP            (MEBASE + 103)
#define ERROR_STREAM_OPERATION           (MEBASE + 104)
#define ERROR_STREAM_STOP_PENDING        (MEBASE + 105)
#define ERROR_STREAM_ALREADY_STOP        (MEBASE + 106)
#define ERROR_STREAM_ALREADY_PAUSE       (MEBASE + 107)
#define ERROR_STREAM_NOT_STARTED         (MEBASE + 108)
#define ERROR_STREAM_NOT_ACTIVE          (MEBASE + 109)
#define ERROR_START_STREAM               (MEBASE + 110)
#define ERROR_MASTER_USED                (MEBASE + 111)
#define ERROR_SPCBKEY_MISMATCH           (MEBASE + 112)
#define ERROR_INSUFF_BUFFER              (MEBASE + 113)
#define ERROR_ALLOC_RESOURCES            (MEBASE + 114)
#define ERROR_ACCESS_OBJECT              (MEBASE + 115)
#define ERROR_HNDLR_REGISTERED           (MEBASE + 116)
#define ERROR_DATA_ITEM_NOT_SPECIFIED    (MEBASE + 117)
#define ERROR_INVALID_SEQUENCE           (MEBASE + 118)
#define ERROR_INITIALIZATION             (MEBASE + 119)
#define ERROR_READING_INI                (MEBASE + 120)
#define ERROR_LOADING_HNDLR              (MEBASE + 121)
#define ERROR_HNDLR_NOT_FOUND            (MEBASE + 122)
#define ERROR_SPCB_NOT_FOUND             (MEBASE + 123)
#define ERROR_DEVICE_NOT_FOUND           (MEBASE + 124)
#define ERROR_TOO_MANY_EVENTS            (MEBASE + 125)
#define ERROR_DEVICE_OVERRUN             (MEBASE + 126)
#define ERROR_DEVICE_UNDERRUN            (MEBASE + 127)
#define ERROR_HNDLR_NOT_IN_INI           (MEBASE + 128)
#define ERROR_QUERY_STREAM_TIME          (MEBASE + 129)
#define ERROR_DATA_ITEM_NOT_SEEKABLE     (MEBASE + 130)
#define ERROR_NOT_SEEKABLE_BY_TIME       (MEBASE + 131)
#define ERROR_NOT_SEEKABLE_BY_BYTES      (MEBASE + 132)
#define ERROR_STREAM_NOT_SEEKABLE        (MEBASE + 133)
#define ERROR_PLAYLIST_STACK_OVERFLOW    (MEBASE + 135)
#define ERROR_PLAYLIST_STACK_UNDERFLOW   (MEBASE + 136)
#define ERROR_LOCKING_BUFFER             (MEBASE + 137)
#define ERROR_UNLOCKING_BUFFER           (MEBASE + 138)
#define ERROR_SEEK_PAST_END              (MEBASE + 139)
#define ERROR_SEEK_BACK_NOT_SUPPORTED    (MEBASE + 140)
#define ERROR_INTERNAL_ERROR             (MEBASE + 141)
#define ERROR_INTERNAL_CORRUPT           (MEBASE + 142)
#define ERROR_INSUFF_MEM                 (MEBASE + 143)
#define ERROR_LARGE_SEEK_BY_TIME         (MEBASE + 144)
#define ERROR_STREAM_PREROLLING          (MEBASE + 145)
#define ERROR_INI_FILE                   (MEBASE + 146)
#define ERROR_SEEK_BEFORE_BEGINNING      (MEBASE + 147)
#define ERROR_TOO_MANY_HANDLERS          (MEBASE + 148)
#define ERROR_ALLOC_HEAP                 (MEBASE + 149)
#define ERROR_END_OF_PLAYLIST            (MEBASE + 150)
#define ERROR_TOO_MANY_STREAMS           (MEBASE + 151)
#define ERROR_FILE_FORMAT_INCORRECT      (MEBASE + 152)
#define ERROR_DESTROY_STREAM             (MEBASE + 153)
#define ERROR_INVALID_NUMMASTERS         (MEBASE + 154)
#define ERROR_MASTER_CONFLICT            (MEBASE + 155)
#define ERROR_NO_MASTER                  (MEBASE + 156)
#define ERROR_NO_SYNC                    (MEBASE + 157)
#define ERROR_STREAM_ALREADY_IN_NETWORK  (MEBASE + 158)
#define ERROR_NO_STREAMS_IN_NETWORK      (MEBASE + 159)
#define ERROR_MISSING_EVENT_ROUTINE      (MEBASE + 160)
#define ERROR_CAN_NOT_REMOVE_STREAM      (MEBASE + 161)

#define ERROR_BUFFER_NOT_AVAILABLE       (MEBASE + 400)
#define ERROR_TOO_MANY_BUFFERS           (MEBASE + 401)
#define ERROR_TOO_MANY_RECORDS           (MEBASE + 402)


/*----- ERROR_INVALID_PROTOCOL ulErrorStatus defines -----*/
/*----- Refer to SHC_NEGOTIATE_RESULT api.*/
#define PROTOCOL_SPCBLENGTH             1
#define PROTOCOL_SPCBKEY                2
#define PROTOCOL_DATAFLAG               3
#define PROTOCOL_NUMRECORDS             4
#define PROTOCOL_BLOCKSIZE              5
#define PROTOCOL_BUFFERSIZE             6
#define PROTOCOL_MINNUMBUFFERS          7
#define PROTOCOL_MAXNUMBUFFERS          8
#define PROTOCOL_SOURCESTART            9
#define PROTOCOL_TARGETSTART            10
#define PROTOCOL_BUFFERFLAG             11
#define PROTOCOL_HANDLERFLAG            12
#define PROTOCOL_SYNCTOLERANCE          13
#define PROTOCOL_SYNCINTERVAL           14
#define PROTOCOL_INTERNALERROR          -1

/***********************************/
/* MMIO Manager Error Return codes */
/***********************************/

#define MMIOERR_BASE                    (MEBASE + 1000)
#define MMIOERR_UNBUFFERED              (MMIOERR_BASE + 1L)
#define MMIOERR_CANNOTWRITE             (MMIOERR_BASE + 2L)
#define MMIOERR_CHUNKNOTFOUND           (MMIOERR_BASE + 3L)

#define MMIOERR_INVALID_HANDLE          (MMIOERR_BASE + 4L)
#define MMIOERR_INVALID_PARAMETER       (MMIOERR_BASE + 5L)
#define MMIOERR_INTERNAL_SYSTEM         (MMIOERR_BASE + 6L)
#define MMIOERR_NO_CORE                 (MMIOERR_BASE + 7L)

#define MMIOERR_INI_OPEN                (MMIOERR_BASE + 8L)
#define MMIOERR_INI_READ                (MMIOERR_BASE + 9L)

#define MMIOERR_INVALID_BUFFER_LENGTH   (MMIOERR_BASE + 10L)
#define MMIOERR_NO_BUFFER_ALLOCATED     (MMIOERR_BASE + 11L)
#define MMIOERR_NO_FLUSH_FOR_MEM_FILE   (MMIOERR_BASE + 12L)
#define MMIOERR_NO_FLUSH_NEEDED         (MMIOERR_BASE + 13L)
#define MMIOERR_READ_ONLY_FILE          (MMIOERR_BASE + 14L)
#define MMIOERR_WRITE_ONLY_FILE         (MMIOERR_BASE + 15L)
#define MMIOERR_INSTALL_PROC_FAILED     (MMIOERR_BASE + 16L)
#define MMIOERR_READ_FAILED             (MMIOERR_BASE + 17L)
#define MMIOERR_WRITE_FAILED            (MMIOERR_BASE + 18L)
#define MMIOERR_SEEK_FAILED             (MMIOERR_BASE + 19L)
#define MMIOERR_CANNOTEXPAND            (MMIOERR_BASE + 20L)
#define MMIOERR_FREE_FAILED             (MMIOERR_BASE + 21L)
#define MMIOERR_EOF_SEEN                (MMIOERR_BASE + 22L)
#define MMIOERR_INVALID_ACCESS_FLAG     (MMIOERR_BASE + 23L)
#define MMIOERR_INVALID_STRUCTURE       (MMIOERR_BASE + 24L)
#define MMIOERR_INVALID_SIZE            (MMIOERR_BASE + 25L)
#define MMIOERR_INVALID_FILENAME        (MMIOERR_BASE + 26L)

#define MMIOERR_CF_DUPLICATE_SEEN       (MMIOERR_BASE + 27L)
#define MMIOERR_CF_ENTRY_NO_CORE        (MMIOERR_BASE + 28L)
#define MMIOERR_CF_WO_UNSUPPORTED       (MMIOERR_BASE + 29L)
#define MMIOERR_CF_ELEMENTS_OPEN        (MMIOERR_BASE + 30L)
#define MMIOERR_CF_NON_BND_FILE         (MMIOERR_BASE + 31L)
#define MMIOERR_CF_ENTRY_NOT_FOUND      (MMIOERR_BASE + 32L)

#define MMIOERR_DELETE_FAILED           (MMIOERR_BASE + 33L)
#define MMIOERR_OUTOFMEMORY             (MMIOERR_BASE + 34L)

#define MMIOERR_INVALID_DLLNAME         (MMIOERR_BASE + 35L)
#define MMIOERR_INVALID_PROCEDURENAME   (MMIOERR_BASE + 36L)
#define MMIOERR_MATCH_NOT_FOUND         (MMIOERR_BASE + 37L)

#define MMIOERR_SEEK_BEFORE_BEGINNING   (MMIOERR_BASE + 38L)
#define MMIOERR_INVALID_FILE            (MMIOERR_BASE + 39L)
#define MMIOERR_QOSUNAVAILABLE          (MMIOERR_BASE + 40L)
#define MMIOERR_MEDIA_NOT_FOUND         (MMIOERR_BASE + 41L)

#define MMIOERR_ERROR_IN_FRAME_DATA     (MMIOERR_BASE + 42L)
#define MMIOERR_INVALID_DIM_ALIGN       (MMIOERR_BASE + 43L)
#define MMIOERR_CODEC_NOT_SUPPORTED     (MMIOERR_BASE + 44L)

#define  MMIOERR_UNSUPPORTED_FUNCTION    (MMIOERR_BASE + 45L)
#define  MMIOERR_CLIPBRD_ERROR           (MMIOERR_BASE + 46L)
#define  MMIOERR_CLIPBRD_ACTIVE          (MMIOERR_BASE + 47L)
#define  MMIOERR_CLIPBRD_EMPTY           (MMIOERR_BASE + 48L)
#define  MMIOERR_NEED_NEW_FILENAME       (MMIOERR_BASE + 49L)
#define  MMIOERR_INVALID_TRACK_OPERATION (MMIOERR_BASE + 50L)
#define  MMIOERR_INCOMPATIBLE_DATA       (MMIOERR_BASE + 51L)
#define  MMIOERR_ACCESS_DENIED           (MMIOERR_BASE + 52L)
#define  MMIOERR_MISSING_FLAG            (MMIOERR_BASE + 53L)
#define  MMIOERR_INVALID_ITEM_FLAG       (MMIOERR_BASE + 54L)

/*************************************/
/* Real-Time MIDI Error Return Codes */
/*************************************/

#define MIDIERR_BASE                      (MMIOERR_BASE + 500)

#define MIDIERR_DUPLICATE_INSTANCE_NAME   (MIDIERR_BASE + 1L)
#define MIDIERR_HARDWARE_FAILED           (MIDIERR_BASE + 2L)
#define MIDIERR_INTERNAL_SYSTEM           (MIDIERR_BASE + 3L)
#define MIDIERR_INVALID_BUFFER_LENGTH     (MIDIERR_BASE + 4L)
#define MIDIERR_INVALID_CLASS_NUMBER      (MIDIERR_BASE + 5L)
#define MIDIERR_INVALID_CONFIG_DATA       (MIDIERR_BASE + 6L)
#define MIDIERR_INVALID_FLAG              (MIDIERR_BASE + 7L)
#define MIDIERR_INVALID_INSTANCE_NAME     (MIDIERR_BASE + 8L)
#define MIDIERR_INVALID_INSTANCE_NUMBER   (MIDIERR_BASE + 9L)
#define MIDIERR_INVALID_PARAMETER         (MIDIERR_BASE + 10L)
#define MIDIERR_INVALID_SETUP             (MIDIERR_BASE + 11L)
#define MIDIERR_NO_DRIVER                 (MIDIERR_BASE + 12L)
#define MIDIERR_NO_DEFAULT_HW_NODE        (MIDIERR_BASE + 13L)
#define MIDIERR_NOT_ALLOWED               (MIDIERR_BASE + 14L)
#define MIDIERR_NOTIFY_MISSED             (MIDIERR_BASE + 15L)
#define MIDIERR_RESOURCE_NOT_AVAILABLE    (MIDIERR_BASE + 16L)
#define MIDIERR_SENDONLY                  (MIDIERR_BASE + 17L)
#define MIDIERR_RECEIVEONLY               (MIDIERR_BASE + 18L)

#define TIMERERR_BASE                     (MIDIERR_BASE + 100)

#define TIMERERR_INVALID_PARAMETER        (TIMERERR_BASE + 1L)
#define TIMERERR_INTERNAL_SYSTEM          (TIMERERR_BASE + 2L)


/***********************************/
/* User defined Error Return codes */
/***********************************/

#define USERERR_BASE                      (MMIOERR_BASE + 1000)

/* NOINC */
#ifdef __cplusplus
}
#endif
/* INC */

