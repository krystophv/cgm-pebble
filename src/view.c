#include "pebble.h"
#include "stddef.h"
#include "string.h"

// CONSTANTS

enum CgmKey {
    CGM_ICON_KEY = 0x0,   // TUPLE_CSTRING, MAX 2 BYTES (10)
    CGM_BG_KEY = 0x1,     // TUPLE_CSTRING, MAX 4 BYTES (253 OR 22.2)
    CGM_TCGM_KEY = 0x2,   // TUPLE_INT, 4 BYTES (CGM TIME)
    CGM_TAPP_KEY = 0x3,   // TUPLE_INT, 4 BYTES (APP / PHONE TIME)
    CGM_DLTA_KEY = 0x4,   // TUPLE_CSTRING, MAX 5 BYTES (BG DELTA, -100 or -10.0)
    CGM_UBAT_KEY = 0x5,   // TUPLE_CSTRING, MAX 3 BYTES (UPLOADER BATTERY, 100)
    CGM_NAME_KEY = 0x6,   // TUPLE_CSTRING, MAX 9 BYTES (Christine)
    CGM_VALS_KEY = 0x7,   // TUPLE_CSTRING, MAX 25 BYTES (0,000,000,000,000,0,0,0,0)
    CGM_CLRW_KEY = 0x8,   // TUPLE_CSTRING, MAX 4 BYTES (253 OR 22.2)
    CGM_RWUF_KEY = 0x9,   // TUPLE_CSTRING, MAX 4 BYTES (253 OR 22.2)
    CGM_NOIZ_KEY = 0xA    // TUPLE_INT, 4 BYTES (1-4)
};
// TOTAL MESSAGE DATA 4x6+2+5+3+9+25 = 68 BYTES
// TOTAL KEY HEADER DATA (STRINGS) 4x10+2 = 42 BYTES
// TOTAL MESSAGE 110 BYTES

// ICON ASSIGNMENTS OF ARROW DIRECTIONS
static const char NO_ARROW[] = "0";
static const char DOUBLEUP_ARROW[] = "1";
static const char SINGLEUP_ARROW[] = "2";
static const char UP45_ARROW[] = "3";
static const char FLAT_ARROW[] = "4";
static const char DOWN45_ARROW[] = "5";
static const char SINGLEDOWN_ARROW[] = "6";
static const char DOUBLEDOWN_ARROW[] = "7";
static const char NOTCOMPUTE_ICON[] = "8";
static const char OUTOFRANGE_ICON[] = "9";

// ARRAY OF ARROW ICON IMAGES
static const uint8_t ARROW_ICONS[] = {
    RESOURCE_ID_IMAGE_SPECVALUE_NONE,  //0
    RESOURCE_ID_IMAGE_UPUP,            //1
    RESOURCE_ID_IMAGE_UP,              //2
    RESOURCE_ID_IMAGE_UP45,            //3
    RESOURCE_ID_IMAGE_FLAT,            //4
    RESOURCE_ID_IMAGE_DOWN45,          //5
    RESOURCE_ID_IMAGE_DOWN,            //6
    RESOURCE_ID_IMAGE_DOWNDOWN,        //7
    RESOURCE_ID_IMAGE_LOGO             //8
};

// ARRAY OF SPECIAL VALUE ICONS
static const uint8_t SPECIAL_VALUE_ICONS[] = {
    RESOURCE_ID_IMAGE_SPECVALUE_NONE,   //0
    RESOURCE_ID_IMAGE_BROKEN_ANTENNA,   //1
    RESOURCE_ID_IMAGE_BLOOD_DROP,       //2
    RESOURCE_ID_IMAGE_STOP_LIGHT,       //3
    RESOURCE_ID_IMAGE_HOURGLASS,        //4
    RESOURCE_ID_IMAGE_QUESTION_MARKS,   //5
    RESOURCE_ID_IMAGE_LOGO              //6
};

// INDEX FOR ARRAY OF ARROW ICON IMAGES
enum ARROWICON_KEY {
    NONE_ARROW_ICON_INDX = 0,
    UPUP_ICON_INDX = 1,
    UP_ICON_INDX = 2,
    UP45_ICON_INDX = 3,
    FLAT_ICON_INDX = 4,
    DOWN45_ICON_INDX = 5,
    DOWN_ICON_INDX = 6,
    DOWNDOWN_ICON_INDX = 7,
    LOGO_ARROW_ICON_INDX = 8
};

// INDEX FOR ARRAY OF SPECIAL VALUE ICONS
enum SPECICON_KEY {
    NONE_SPECVALUE_ICON_INDX = 0,
    BROKEN_ANTENNA_ICON_INDX = 1,
    BLOOD_DROP_ICON_INDX = 2,
    STOP_LIGHT_ICON_INDX = 3,
    HOURGLASS_ICON_INDX = 4,
    QUESTION_MARKS_ICON_INDX = 5,
    LOGO_SPECVALUE_ICON_INDX = 6
};

// ARRAY OF TIMEAGO ICONS
static const uint8_t TIMEAGO_ICONS[] = {
    RESOURCE_ID_IMAGE_RCVRNONE,   //0
    RESOURCE_ID_IMAGE_RCVRON,     //1
    RESOURCE_ID_IMAGE_RCVROFF     //2
};

// INDEX FOR ARRAY OF TIMEAGO ICONS
static const uint8_t RCVRNONE_ICON_INDX = 0;
static const uint8_t RCVRON_ICON_INDX = 1;
static const uint8_t RCVROFF_ICON_INDX = 2;

// INDEX FOR ARRAYS OF BG CONSTANTS
enum BG_KEY {
    SPECVALUE_BG_INDX = 0,
    SHOWLOW_BG_INDX = 1,
    HYPOLOW_BG_INDX = 2,
    BIGLOW_BG_INDX = 3,
    MIDLOW_BG_INDX = 4,
    LOW_BG_INDX = 5,
    HIGH_BG_INDX = 6,
    MIDHIGH_BG_INDX = 7,
    BIGHIGH_BG_INDX = 8,
    SHOWHIGH_BG_INDX = 9
};

// INDEX FOR ARRAYS OF SPECIAL VALUES CONSTANTS
enum SPECVAL_BG_KEY {
    SENSOR_NOT_ACTIVE_VALUE_INDX = 0,
    MINIMAL_DEVIATION_VALUE_INDX = 1,
    NO_ANTENNA_VALUE_INDX = 2,
    SENSOR_NOT_CALIBRATED_VALUE_INDX = 3,
    STOP_LIGHT_VALUE_INDX = 4,
    HOURGLASS_VALUE_INDX = 5,
    QUESTION_MARKS_VALUE_INDX = 6,
    BAD_RF_VALUE_INDX = 7
};

// MG/DL SPECIAL VALUE CONSTANTS ACTUAL VALUES
// mg/dL = mmol / .0555 OR mg/dL = mmol * 18.0182
enum MGDL_SPECVAL_CONSTS {
    SENSOR_NOT_ACTIVE_VALUE_MGDL = 1,     // show stop light, ?SN
    MINIMAL_DEVIATION_VALUE_MGDL = 2,     // show stop light, ?MD
    NO_ANTENNA_VALUE_MGDL = 3,            // show broken antenna, ?NA
    SENSOR_NOT_CALIBRATED_VALUE_MGDL = 5, // show blood drop, ?NC
    STOP_LIGHT_VALUE_MGDL = 6,            // show stop light, ?CD
    HOURGLASS_VALUE_MGDL = 9,             // show hourglass, hourglass
    QUESTION_MARKS_VALUE_MGDL = 10,       // show ???, ???
    BAD_RF_VALUE_MGDL = 12                // show broken antenna, ?RF
};

// MMOL SPECIAL VALUE CONSTANTS ACTUAL VALUES
// mmol = mg/dL / 18.0182 OR mmol = mg/dL * .0555
enum MMOL_SPECIAL_CONSTS {
    SENSOR_NOT_ACTIVE_VALUE_MMOL = 1,     // show stop light, ?SN (.06 -> .1)
    MINIMAL_DEVIATION_VALUE_MMOL = 1,     // show stop light, ?MD (.11 -> .1)
    NO_ANTENNA_VALUE_MMOL = 2,            // show broken antenna, ?NA (.17 -> .2)
    SENSOR_NOT_CALIBRATED_VALUE_MMOL = 3, // show blood drop, ?NC (.28 -> .3)
    STOP_LIGHT_VALUE_MMOL = 4,            // show stop light, ?CD (.33 -> .3, set to .4 here)
    HOURGLASS_VALUE_MMOL = 5,             // show hourglass, hourglass (.50 -> .5)
    QUESTION_MARKS_VALUE_MMOL = 6,        // show ???, ??? (.56 -> .6)
    BAD_RF_VALUE_MMOL = 7                 // show broken antenna, ?RF (.67 -> .7)
};

// ARRAY OF SPECIAL VALUES CONSTANTS; MGDL
uint8_t SPECVALUE_MGDL[8];

// ARRAY OF SPECIAL VALUES CONSTANTS; MMOL
uint8_t SPECVALUE_MMOL[8];

enum NOISE_KEY {
    NO_NOISE = 0,
    CLEAN_NOISE = 1,
    LIGHT_NOISE = 2,
    MEDIUM_NOISE = 3,
    HEAVY_NOISE = 4,
    WARMUP_NOISE = 5,
    OTHER_NOISE = 6
};

// ** START OF CONSTANTS THAT CAN BE CHANGED; DO NOT CHANGE IF YOU DO NOT KNOW WHAT YOU ARE DOING **
// ** FOR MMOL, ALL VALUES ARE STORED AS INTEGER; LAST DIGIT IS USED AS DECIMAL **
// ** BE EXTRA CAREFUL OF CHANGING SPECIAL VALUES OR TIMERS; DO NOT CHANGE WITHOUT EXPERT HELP **

// FOR BG RANGES
// DO NOT SET ANY BG RANGES EQUAL TO ANOTHER; LOW CAN NOT EQUAL MIDLOW
// LOW BG RANGES MUST BE IN ASCENDING ORDER; SPECVALUE < HYPOLOW < BIGLOW < MIDLOW < LOW
// HIGH BG RANGES MUST BE IN ASCENDING ORDER; HIGH < MIDHIGH < BIGHIGH
// DO NOT ADJUST SPECVALUE UNLESS YOU HAVE A VERY GOOD REASON
// DO NOT USE NEGATIVE NUMBERS OR DECIMAL POINTS OR ANYTHING OTHER THAN A NUMBER

// BG Ranges, MG/DL
uint16_t SPECVALUE_BG_MGDL = 20;
uint16_t SHOWLOW_BG_MGDL = 40;
uint16_t HYPOLOW_BG_MGDL = 55;
uint16_t BIGLOW_BG_MGDL = 60;
uint16_t MIDLOW_BG_MGDL = 70;
uint16_t LOW_BG_MGDL = 80;

uint16_t HIGH_BG_MGDL = 180;
uint16_t MIDHIGH_BG_MGDL = 240;
uint16_t BIGHIGH_BG_MGDL = 300;
uint16_t SHOWHIGH_BG_MGDL = 400;

// BG Ranges, MMOL
// VALUES ARE IN INT, NOT FLOATING POINT, LAST DIGIT IS DECIMAL
// FOR EXAMPLE : SPECVALUE IS 1.1, BIGHIGH IS 16.6
// ALWAYS USE ONE AND ONLY ONE DECIMAL POINT FOR LAST DIGIT
// GOOD : 5.0, 12.2 // BAD : 7 , 14.44
uint16_t SPECVALUE_BG_MMOL = 11;
uint16_t SHOWLOW_BG_MMOL = 23;
uint16_t HYPOLOW_BG_MMOL = 30;
uint16_t BIGLOW_BG_MMOL = 33;
uint16_t MIDLOW_BG_MMOL = 39;
uint16_t LOW_BG_MMOL = 44;

uint16_t HIGH_BG_MMOL = 100;
uint16_t MIDHIGH_BG_MMOL = 133;
uint16_t BIGHIGH_BG_MMOL = 166;
uint16_t SHOWHIGH_BG_MMOL = 222;

// BG Snooze Times, in Minutes; controls when vibrate again
// RANGE 0-240
static const uint8_t SPECVALUE_SNZ_MIN = 30;
static const uint8_t HYPOLOW_SNZ_MIN = 5;
static const uint8_t BIGLOW_SNZ_MIN = 5;
static const uint8_t MIDLOW_SNZ_MIN = 10;
static uint8_t LOW_SNZ_MIN = 15;
static uint8_t HIGH_SNZ_MIN = 30;
static const uint8_t MIDHIGH_SNZ_MIN = 30;
static const uint8_t BIGHIGH_SNZ_MIN = 30;

// Vibration Levels; 0 = NONE; 1 = LOW; 2 = MEDIUM; 3 = HIGH
// IF YOU DO NOT WANT A SPECIFIC VIBRATION, SET TO 0
static const uint8_t SPECVALUE_VIBE = 2;
static const uint8_t HYPOLOWBG_VIBE = 3;
static const uint8_t BIGLOWBG_VIBE = 3;
static uint8_t LOWBG_VIBE = 3;
static uint8_t HIGHBG_VIBE = 2;
static const uint8_t BIGHIGHBG_VIBE = 2;
static const uint8_t DOUBLEDOWN_VIBE = 3;
static const uint8_t APPSYNC_ERR_VIBE = 1;
static const uint8_t BTOUT_VIBE = 1;
static const uint8_t CGMOUT_VIBE = 1;
static const uint8_t PHONEOUT_VIBE = 1;
static const uint8_t LOWBATTERY_VIBE = 1;
static const uint8_t DATAOFFLINE_VIBE = 1;

// Icon Cross Out & Vibrate Once Wait Times, in Minutes
// RANGE 0-240
// IF YOU WANT TO WAIT LONGER TO GET CONDITION, INCREASE NUMBER
static const uint8_t CGMOUT_WAIT_MIN = 15;
static const uint8_t CGMOUT_INIT_WAIT_MIN = 6;
static const uint8_t PHONEOUT_WAIT_MIN = 5;

// Control Messages
// IF YOU DO NOT WANT A SPECIFIC MESSAGE, SET TO true
static const bool TurnOff_NOBLUETOOTH_Msg = false;
static const bool TurnOff_CHECKCGM_Msg = false;
static const bool TurnOff_CHECKPHONE_Msg = false;

// Control Vibrations
// SPECIAL FLAG TO HARD CODE VIBRATIONS OFF; If you want no vibrations, SET TO true
// Use for Sleep Face or anyone else for a custom load
bool HardCodeNoVibrations = false;

// Control Animations
// SPECIAL FLAG TO HARD CODE ANIMATIONS OFF; If you want no animations, SET TO true
// SPECIAL FLAG TO HARD CODE ANIMATIONS ALL ON; If you want all animations, SET TO true
// This is for people who want old ones too
// Use for a custom load
bool HardCodeNoAnimations = false;
bool HardCodeAllAnimations = false;

// Control Vibrations for Config File
// IF YOU WANT NO VIBRATIONS, SET TO true
bool TurnOffAllVibrations = false;
// IF YOU WANT LESS INTENSE VIBRATIONS, SET TO true
bool TurnOffStrongVibrations = false;

// Control Raw data
// If you want to turn off vibrations for calculated raw, set to true
bool TurnOffVibrationsCalcRaw = false;
// If you want to see unfiltered raw, set to true
bool TurnOnUnfilteredRaw = false;

// ** END OF CONSTANTS THAT CAN BE CHANGED; DO NOT CHANGE IF YOU DO NOT KNOW WHAT YOU ARE DOING **

// ARRAY OF BG CONSTANTS; MGDL
static uint16_t BG_MGDL[10];

// ARRAY OF BG CONSTANTS; MMOL
static uint16_t BG_MMOL[10];

// global constants for time durations; seconds
static const uint8_t MINUTEAGO = 60;
static const uint16_t HOURAGO = 60 * (60);
static const uint32_t DAYAGO = 24 * (60 * 60);
static const uint32_t WEEKAGO = 7 * (24 * 60 * 60);
static const uint16_t MS_IN_A_SECOND = 1000;

// Constants for string buffers
// If add month to date, buffer size needs to increase to 12; also need to reformat date_app_text init string
static const uint8_t TIME_TEXTBUFF_SIZE = 6;
static const uint8_t DATE_TEXTBUFF_SIZE = 8;
static const uint8_t LABEL_BUFFER_SIZE = 6;
static const uint8_t TIMEAGO_BUFFER_SIZE = 10;
static const uint8_t BATTLEVEL_FORMAT_SIZE = 12;

// END CONSTANTS

static Window *s_main_window;

static TextLayer *s_tophalf_layer;
static TextLayer *s_bg_layer;
static TextLayer *s_cgmtime_layer;
static TextLayer *s_message_layer;
static TextLayer *s_rig_battlevel_layer;
static TextLayer *s_watch_battlevel_layer;
static TextLayer *s_t1dname_layer;
static TextLayer *s_time_watch_layer;
static TextLayer *s_date_app_layer;
static TextLayer *s_happymsg_layer;
static TextLayer *s_raw_calc_layer;
static TextLayer *s_raw_unfilt_layer;
static TextLayer *s_noise_layer;
static TextLayer *s_calcraw_last1_layer;
static TextLayer *s_calcraw_last2_layer;
static TextLayer *s_calcraw_last3_layer;

static BitmapLayer *s_icon_layer;
static BitmapLayer *s_cgmicon_layer;
static BitmapLayer *s_perfectbg_layer;

static GBitmap *s_icon_bitmap;
static GBitmap *s_cgmicon_bitmap;
static GBitmap *s_specialvalue_bitmap;
static GBitmap *s_perfectbg_bitmap;

static GFont s_res_gothic_28_bold;
static GFont s_res_gothic_24_bold;
static GFont s_res_gothic_18_bold;
static GFont s_res_bitham_42_bold;

static PropertyAnimation *perfectbg_animation ;
static PropertyAnimation *happymsg_animation;

static char s_time_watch_text[] = "00:00";
static char s_date_app_text[] = "Wed 13 ";

static AppSync s_sync;
static bool app_sync_err_alert = false;
// CGM message is 110 bytes
// Pebble needs additional 62 Bytes?!? Pad with additional 60 bytes
static uint8_t s_sync_buffer[212];

// global indicators
static bool s_bluetooth_connected;
static bool s_current_bg_is_mmol;
static bool s_specialvalue_alert;
// global BG snooze timer
static uint8_t last_alert_time = 0;

// variables for timers and time
static AppTimer *fetch_timer;
static time_t app_time_now = 0;
int timeformat = 0;
static bool init_loading_cgm_timeago = false;
static bool cleared_outage = false;

// global variables for sync tuple functions
static char current_icon[2] = {0};
static char current_bg_str[6] = {0};
static char last_battlevel[4] = {0};
static char t1name[15] = {0};

static uint32_t current_cgm_time = 0;
static uint32_t stored_cgm_time = 0;
static uint32_t current_cgm_timeago = 0;
static time_t next_fetch_time = 0;

static uint32_t current_app_time = 0;
static char current_bg_delta[10] = {0};
static char last_calc_raw[6] = {0};
static char last_raw_unfilt[6] = {0};
static uint8_t current_noise_value = 0;
static char last_calc_raw1[6] = {0};
static char last_calc_raw2[6] = {0};
static char last_calc_raw3[6] = {0};
static int current_bg = 0;
static int current_calc_raw = 0;
static int current_calc_raw1 = 0;
static bool current_bg_is_MMOL = false;
static int converted_bg_delta = 0;
static char current_values[25] = {0};
static bool have_calc_raw = false;

// global retries counters for timeout problems
static uint8_t appsyncandmsg_retries_counter = 0;
static uint8_t dataoffline_retries_counter = 0;

// global overwrite variables for vibrating when hit a specific BG if already in a snooze
static bool specvalue_overwrite = false;
static bool hypolow_overwrite = false;
static bool biglow_overwrite = false;
static bool midlow_overwrite = false;
static bool low_overwrite = false;
static bool high_overwrite = false;
static bool midhigh_overwrite = false;
static bool bighigh_overwrite = false;

// global variables for vibrating in special conditions
static bool BluetoothAlert = false;
static bool BT_timer_pop = false;
static bool data_offline_alert = false;
static bool cgm_off_alert = false;
static bool phone_off_alert = false;

// pointers to be used to MGDL or MMOL values for parsing
// will be either BG_MMOL and SPECVALUE_MMOL or BG_MGDL and SPECVALUE_MGDL
static uint16_t *bg_ptr;
static uint8_t *specvalue_ptr;

// Message Timer & Animate Wait Times, in Seconds
static const uint8_t WATCH_MSGSEND_SECS = 60;
static const uint8_t LOADING_MSGSEND_SECS = 10;
static const uint8_t PERFECTBG_ANIMATE_SECS = 10;
static const uint8_t HAPPYMSG_ANIMATE_SECS = 10;

// App Sync / Message retries, for timeout / busy problems
// Change to see if there is a temp or long term problem
// This is approximately number of seconds, so if set to 50, timeout is at 50 seconds
// However, this can vary widely - can be up to 6 seconds .... for 50, timeout can be up to 3 minutes
static const uint8_t APPSYNCANDMSG_RETRIES_MAX = 50;

// HTML Request retries, for timeout / busy problems
// Change to see if there is a temp or long term problem
// This is number of minutes, so if set to 11 timeout is at 11 minutes
static const uint8_t DATAOFFLINE_RETRIES_MAX = 14;

// forward declarations
static void process_icon();
static void process_bg();
static void process_cgmtime();
static void process_apptime();
static void process_bg_delta();
static void process_rig_battlevel();
static void process_noise();
static void process_values();
static void animate_happymsg();
static void animate_perfectbg();
static void bg_vibrator (uint16_t BG_BOTTOM_INDX, uint16_t BG_TOP_INDX, uint8_t BG_SNOOZE, bool *bg_overwrite, uint8_t BG_VIBE);
static void fetch_timer_callback(void *data);
static void alert_handler(uint8_t alertValue);

static char *translate_app_error(AppMessageResult result) {
    switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "APP UNKNOWN ERROR";
    }
}

static char *translate_dict_error(DictionaryResult result) {
    switch (result) {
    case DICT_OK: return "DICT_OK";
    case DICT_NOT_ENOUGH_STORAGE: return "DICT_NOT_ENOUGH_STORAGE";
    case DICT_INVALID_ARGS: return "DICT_INVALID_ARGS";
    case DICT_INTERNAL_INCONSISTENCY: return "DICT_INTERNAL_INCONSISTENCY";
    case DICT_MALLOC_FAILED: return "DICT_MALLOC_FAILED";
    default: return "DICT UNKNOWN ERROR";
    }
}

/**
 * Handles switching out one bitmap for another based on a resource.
 * modified from: https://github.com/pebble/pebble-sdk-examples/blob/master/watchfaces/ninety_one_dub/src/ninety_one_dub.c#L80
 * @param bmp_image   GBitmap
 * @param bmp_layer   BitmapLayer
 * @param resource_id the new resource id to use to switch out
 */
static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id) {
    GBitmap *old_image = *bmp_image;

    *bmp_image = gbitmap_create_with_resource(resource_id);
    bitmap_layer_set_bitmap(bmp_layer, *bmp_image);

    if (old_image != NULL) {
        gbitmap_destroy(old_image);
    }
}

char *strtok(s, delim)
register char *s;
register const char *delim;
{
    register char *spanp;
    register int c, sc;
    char *tok;
    static char *last;


    if (s == NULL && (s = last) == NULL)
        return (NULL);

    /*
     * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
     */
cont:
    c = *s++;
    for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
        if (c == sc)
            goto cont;
    }

    if (c == 0) {   /* no non-delimiter characters */
        last = NULL;
        return (NULL);
    }
    tok = s - 1;

    /*
     * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     * Note that delim must have one NUL; we stop if we see that, too.
     */
    for (;;) {
        c = *s++;
        spanp = (char *)delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                last = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}

/**
 * Converts a string representation of the BG to an integer, will also set s_current_bg_is_mmol flag
 * @param  str BG string to convert
 * @return     int value of BG
 */
static int myBGAtoi(char *str) {
    int res = 0;

    // initialize flag
    s_current_bg_is_mmol = false;

    //APP_LOG(APP_LOG_LEVEL_DEBUG, "myBGAtoi, START currentBG is MMOL: %i", s_current_bg_is_mmol );

    // Iterate through all characters of input string and update result
    for (int i = 0; str[i] != '\0'; ++i) {

        //APP_LOG(APP_LOG_LEVEL_DEBUG, "myBGAtoi, STRING IN: %s", &str[i] );

        if (str[i] == ('.')) {
            s_current_bg_is_mmol = true;
            bg_ptr = BG_MMOL;
            specvalue_ptr = SPECVALUE_MMOL;
        } else if ( (str[i] >= ('0')) && (str[i] <= ('9')) ) {
            res = res * 10 + str[i] - '0';
            bg_ptr = BG_MGDL;
            specvalue_ptr = SPECVALUE_MGDL;
        }

        //APP_LOG(APP_LOG_LEVEL_DEBUG, "myBGAtoi, FOR RESULT OUT: %i", res );
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "myBGAtoi, currentBG is MMOL: %i", s_current_bg_is_mmol );
    }
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "myBGAtoi, FINAL RESULT OUT: %i", res );
    return res;
}

/**
 * Determines and executes a type of vibration alert
 * @param alertValue the type of alert to send
 */
static void alert_handler(uint8_t alertValue) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "ALERT HANDLER");
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "ALERT CODE: %d", alertValue);

    // CONSTANTS
    // constants for vibrations patterns; has to be uint32_t, measured in ms, maximum duration 10000ms
    // Vibe pattern: ON, OFF, ON, OFF; ON for 500ms, OFF for 100ms, ON for 100ms;

    // CURRENT PATTERNS
    const uint32_t highalert_fast[] = { 300, 100, 50, 100, 300, 100, 50, 100, 300, 100, 50, 100, 300, 100, 50, 100, 300, 100, 50, 100, 300, 100, 50, 100, 300, 100, 50, 100, 300, 100, 50, 100, 300 };
    const uint32_t medalert_long[] = { 500, 100, 100, 100, 500, 100, 100, 100, 500, 100, 100, 100, 500, 100, 100, 100, 500 };
    const uint32_t lowalert_beebuzz[] = { 75, 50, 50, 50, 75, 50, 50, 50, 75, 50, 50, 50, 75, 50, 50, 50, 75, 50, 50, 50, 75, 50, 50, 50, 75 };

    // PATTERN DURATION
    const uint8_t HIGHALERT_FAST_STRONG = 33;
    const uint8_t HIGHALERT_FAST_SHORT = (33 / 2);
    const uint8_t MEDALERT_LONG_STRONG = 17;
    const uint8_t MEDALERT_LONG_SHORT = (17 / 2);
    const uint8_t LOWALERT_BEEBUZZ_STRONG = 25;
    const uint8_t LOWALERT_BEEBUZZ_SHORT = (25 / 2);

    // CODE START

    if ( (TurnOffAllVibrations == true) || (HardCodeNoVibrations == true) ) {
        //turn off all vibrations is set, return out here
        return;
    }

    switch (alertValue) {

    case 0:
        //No alert
        //Normal (new data, in range, trend okay)
        break;

    case 1:;
        //Low
        //APP_LOG(APP_LOG_LEVEL_INFO, "ALERT HANDLER: LOW ALERT");
        VibePattern low_alert_pat = {
            .durations = lowalert_beebuzz,
            .num_segments = LOWALERT_BEEBUZZ_STRONG,
        };
        if (TurnOffStrongVibrations == true) {
            low_alert_pat.num_segments = LOWALERT_BEEBUZZ_SHORT;
        };
        vibes_enqueue_custom_pattern(low_alert_pat);
        break;

    case 2:;
        // Medium Alert
        //APP_LOG(APP_LOG_LEVEL_INFO, "ALERT HANDLER: MEDIUM ALERT");
        VibePattern med_alert_pat = {
            .durations = medalert_long,
            .num_segments = MEDALERT_LONG_STRONG,
        };
        if (TurnOffStrongVibrations == true) {
            med_alert_pat.num_segments = MEDALERT_LONG_SHORT;
        };
        vibes_enqueue_custom_pattern(med_alert_pat);
        break;

    case 3:;
        // High Alert
        //APP_LOG(APP_LOG_LEVEL_INFO, "ALERT HANDLER: HIGH ALERT");
        VibePattern high_alert_pat = {
            .durations = highalert_fast,
            .num_segments = HIGHALERT_FAST_STRONG,
        };
        if (TurnOffStrongVibrations == true) {
            high_alert_pat.num_segments = HIGHALERT_FAST_SHORT;
        };
        vibes_enqueue_custom_pattern(high_alert_pat);
        break;
    }
}

/**
 * Handles bluetooth connect/disconnect events
 * @param bt_connected true if connect, false if disconnect
 */
static void bt_handler(bool bt_connected) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "HANDLE BT: ENTER CODE");

    if (bt_connected == false) {
        s_bluetooth_connected = false;
        alert_handler(BTOUT_VIBE);
    } else {
        //APP_LOG(APP_LOG_LEVEL_INFO, "HANDLE BT: BLUETOOTH ON");
        s_bluetooth_connected = true;
    }

    process_bg_delta();
    process_cgmtime();

    //APP_LOG(APP_LOG_LEVEL_INFO, "BluetoothAlert: %i", BluetoothAlert);
}

/**
 * Fires everytime the minute value of clock time changes, updates the clock and
 * tells the cgmtime to update
 * @param tick_time_cgm int time
 * @param units_changed enum of units changed
 */
static void minute_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    if (units_changed & MINUTE_UNIT) {
        //APP_LOG(APP_LOG_LEVEL_INFO, "TICK TIME MINUTE CODE");

        clock_copy_time_string(s_time_watch_text, TIME_TEXTBUFF_SIZE);
        text_layer_set_text(s_time_watch_layer, s_time_watch_text);

        //APP_LOG(APP_LOG_LEVEL_DEBUG, "last_alert_time IN:  %i", last_alert_time);
        // increment BG snooze
        ++last_alert_time;
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "last_alert_time OUT:  %i", last_alert_time);
    }

    if (units_changed & DAY_UNIT) {
        strftime(s_date_app_text, DATE_TEXTBUFF_SIZE, "%a %d", tick_time);
        text_layer_set_text(s_date_app_layer, s_date_app_text);
    }

    // update the cgmtime here
    process_cgmtime();

    if(next_fetch_time <= mktime(tick_time)){
      fetch_timer_callback(NULL);
    }
}

/**
 * Handles battery change events
 * @param watch_charge_state current battery state
 */
static void watch_battery_handler(BatteryChargeState watch_charge_state) {
    static char watch_battery_text[] = "Wch 100%";

    if (watch_charge_state.is_charging) {
        snprintf(watch_battery_text, BATTLEVEL_FORMAT_SIZE, "CHARGING");
    } else {
        snprintf(watch_battery_text, BATTLEVEL_FORMAT_SIZE, "Wch %d%%", watch_charge_state.charge_percent);
    }
    text_layer_set_text(s_watch_battlevel_layer, watch_battery_text);
}

static void draw_date_from_app() {
    time_t d_app = time(NULL);
    struct tm *current_d_app = localtime(&d_app);
    size_t draw_return = 0;

    // format current date from app
    if (strcmp(s_time_watch_text, "00:00") == 0) {
        clock_copy_time_string(s_time_watch_text, TIME_TEXTBUFF_SIZE);
        text_layer_set_text(s_time_watch_layer, s_time_watch_text);
    }

    draw_return = strftime(s_date_app_text, DATE_TEXTBUFF_SIZE, "%a %d", current_d_app);
    if (draw_return != 0) {
        text_layer_set_text(s_date_app_layer, s_date_app_text);
    }
}

/**
 * Setter for s_specialvalue_alert
 * @param new_val the new value to set
 */
static void setSpecialAlert(bool new_val) {
    s_specialvalue_alert = new_val;
    process_icon();
}

/**
 * When the cgmtime changes, we set the next fetch for when it might be available
 */
static void set_next_fetch() {
    int next_tick = 0;
    int time_till_next_tick = 0;
    time_t cgm_time_now = 0;

    cgm_time_now = time(NULL);
    // set next poll for data to 6 minutes and 30 seconds after the current data or,
    // if the time to next poll is less than 30 seconds, wait one minute
    next_tick = current_cgm_time + 60 * 6 + 30;
    time_till_next_tick = next_tick - cgm_time_now;
    if (time_till_next_tick < 30) {
        time_till_next_tick = 60;
    }

    next_fetch_time = (time_t) cgm_time_now + time_till_next_tick;

    // if timer already exists, just reschedule it
    /*
    if (fetch_timer != NULL) {
        app_timer_reschedule(fetch_timer, time_till_next_tick * MS_IN_A_SECOND);
    } else {
        fetch_timer = app_timer_register((time_till_next_tick * MS_IN_A_SECOND), fetch_timer_callback, NULL);
    }
    */
}

// ANIMATION CODE

static void destroy_perfectbg_animation(PropertyAnimation **perfectbg_animation) {
    if (*perfectbg_animation == NULL) {
        return;
    }

    if (animation_is_scheduled((Animation *) *perfectbg_animation)) {
        animation_unschedule((Animation *) *perfectbg_animation);
    }

    if (perfectbg_animation != NULL) {
        property_animation_destroy(*perfectbg_animation);
    }
    *perfectbg_animation = NULL;
} // end destroy_perfectbg_animation

// PERFECTBG ANIMATION
static void perfectbg_animation_started(Animation *animation, void *data) {

    //APP_LOG(APP_LOG_LEVEL_INFO, "PERFECT BG ANIMATE, ANIMATION STARTED ROUTINE");

    // clear out BG and icon
    text_layer_set_text(s_bg_layer, " ");
    text_layer_set_text(s_message_layer, "FIST BUMP!\0");

} // end perfectbg_animation_started

static void perfectbg_animation_stopped(Animation *animation, bool finished, void *data) {

    //APP_LOG(APP_LOG_LEVEL_INFO, "PERFECT BG ANIMATE, ANIMATION STOPPED ROUTINE");

    // reset bg and icon
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "PERFECT BG ANIMATE, ANIMATION STOPPED, SET TO BG: %s ", current_bg_str);
    text_layer_set_text(s_bg_layer, current_bg_str);
    process_icon();
    process_bg_delta();
    destroy_perfectbg_animation(&perfectbg_animation);

} // end perfectbg_animation_stopped

static void animate_perfectbg() {

    // ARRAY OF ICONS FOR PERFECT BG
    const uint8_t PERFECTBG_ICONS[] = {
        RESOURCE_ID_IMAGE_CLUB100,         //0
        RESOURCE_ID_IMAGE_CLUB55           //1
    };

    // INDEX FOR ARRAY OF PERFECT BG ICONS
    static const uint8_t CLUB100_ICON_INDX = 0;
    static const uint8_t CLUB55_ICON_INDX = 1;

    Layer *animate_perfectbg_layer = NULL;

    // for animation
    GRect from_perfectbg_rect = GRect(0, 0, 0, 0);
    GRect to_perfectbg_rect = GRect(0, 0, 0, 0);

    // CODE START

    if (current_bg_is_MMOL) {
        set_container_image(&s_perfectbg_bitmap, s_perfectbg_layer, PERFECTBG_ICONS[CLUB55_ICON_INDX]);
    } else {
        set_container_image(&s_perfectbg_bitmap, s_perfectbg_layer, PERFECTBG_ICONS[CLUB100_ICON_INDX]);
    }
    animate_perfectbg_layer = bitmap_layer_get_layer(s_perfectbg_layer);
    from_perfectbg_rect = GRect(0, 3, 95, 47);
    to_perfectbg_rect = GRect(144, 3, 95, 47);
    destroy_perfectbg_animation(&perfectbg_animation);
    //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, ANIMATE BG, CREATE FRAME");
    perfectbg_animation = property_animation_create_layer_frame(animate_perfectbg_layer, &from_perfectbg_rect, &to_perfectbg_rect);
    //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, ANIMATE BG, SET DURATION AND CURVE");
    animation_set_duration((Animation *) perfectbg_animation, PERFECTBG_ANIMATE_SECS * MS_IN_A_SECOND);
    animation_set_curve((Animation *) perfectbg_animation, AnimationCurveLinear);

    //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, ANIMATE BG, SET HANDLERS");
    animation_set_handlers((Animation *) perfectbg_animation, (AnimationHandlers) {
        .started = (AnimationStartedHandler) perfectbg_animation_started,
         .stopped = (AnimationStoppedHandler) perfectbg_animation_stopped,
    }, NULL /* callback data */);

    //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, ANIMATE BG, SCHEDULE");
    animation_schedule((Animation *) perfectbg_animation);
}

static void destroy_happymsg_animation(PropertyAnimation **happymsg_animation) {
    if (*happymsg_animation == NULL) {
        return;
    }

    if (animation_is_scheduled((Animation *) *happymsg_animation)) {
        animation_unschedule((Animation *) *happymsg_animation);
    }

    if (happymsg_animation != NULL) {
        property_animation_destroy(*happymsg_animation);
    }
    *happymsg_animation = NULL;
}

static void happymsg_animation_started(Animation *animation, void *data) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "HAPPY MSG ANIMATE, ANIMATION STARTED ROUTINE, CLEAR OUT BG DELTA");

    // clear out BG delta / message layer
    text_layer_set_text(s_message_layer, "");
    text_layer_set_text(s_cgmtime_layer, "");
    text_layer_set_text(s_rig_battlevel_layer, "");
    set_container_image(&s_cgmicon_bitmap, s_cgmicon_layer, TIMEAGO_ICONS[RCVRNONE_ICON_INDX]);
}

static void happymsg_animation_stopped(Animation *animation, bool finished, void *data) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "HAPPY MSG ANIMATE, ANIMATION STOPPED ROUTINE");

    // set BG delta / message layer
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "HAPPY MSG ANIMATE, ANIMATION STOPPED, SET TO BG DELTA");
    process_bg_delta();
    process_cgmtime();
    process_apptime();
    process_rig_battlevel();
    destroy_happymsg_animation(&happymsg_animation);
}

static void animate_happymsg(char *happymsg_to_display) {
    const uint8_t HAPPYMSG_BUFFER_SIZE = 30;

    Layer *animate_happymsg_layer = NULL;

    // for animation
    GRect from_happymsg_rect = GRect(0, 0, 0, 0);
    GRect to_happymsg_rect = GRect(0, 0, 0, 0);

    static char animate_happymsg_buffer[30] = {0};

    //APP_LOG(APP_LOG_LEVEL_DEBUG, "ANIMATE HAPPY MSG, STRING PASSED: %s", happymsg_to_display);
    strncpy(animate_happymsg_buffer, happymsg_to_display, HAPPYMSG_BUFFER_SIZE);
    text_layer_set_text(s_happymsg_layer, animate_happymsg_buffer);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "ANIMATE HAPPY MSG, MSG IN BUFFER: %s", animate_happymsg_buffer);
    animate_happymsg_layer = text_layer_get_layer(s_happymsg_layer);
    from_happymsg_rect = GRect(-10, 33, 144, 55);
    to_happymsg_rect = GRect(144, 33, 144, 55);
    destroy_happymsg_animation(&happymsg_animation);
    //APP_LOG(APP_LOG_LEVEL_INFO, "ANIMATE HAPPY MSG, CREATE FRAME");
    happymsg_animation = property_animation_create_layer_frame(animate_happymsg_layer, &from_happymsg_rect, &to_happymsg_rect);
    //APP_LOG(APP_LOG_LEVEL_INFO, "ANIMATE HAPPY MSG, SET DURATION AND CURVE");
    animation_set_duration((Animation *) happymsg_animation, HAPPYMSG_ANIMATE_SECS * MS_IN_A_SECOND);
    animation_set_curve((Animation *) happymsg_animation, AnimationCurveLinear);

    //APP_LOG(APP_LOG_LEVEL_INFO, "ANIMATE HAPPY MSG, SET HANDLERS");
    animation_set_handlers((Animation *) happymsg_animation, (AnimationHandlers) {
        .started = (AnimationStartedHandler) happymsg_animation_started,
         .stopped = (AnimationStoppedHandler) happymsg_animation_stopped,
    }, NULL /* callback data */);

    //APP_LOG(APP_LOG_LEVEL_INFO, "ANIMATE HAPPY MSG, SCHEDULE");
    animation_schedule((Animation *) happymsg_animation);
}

static void bg_vibrator (uint16_t BG_BOTTOM_INDX, uint16_t BG_TOP_INDX, uint8_t BG_SNOOZE, bool *bg_overwrite, uint8_t BG_VIBE) {
    uint16_t conv_vibrator_bg = 180;

    conv_vibrator_bg = current_bg;

    // adjust high bg for comparison, if needed
    if ( ((!current_bg_is_MMOL) && (current_bg >= HIGH_BG_MGDL))
            || ((current_bg_is_MMOL) && (current_bg >= HIGH_BG_MMOL)) ) {
        conv_vibrator_bg = current_bg + 1;
    }

    // check BG and vibrate if needed
    //APP_LOG(APP_LOG_LEVEL_INFO, "BG VIBRATOR, CHECK TO SEE IF WE NEED TO VIBRATE");
    if ( ( ((conv_vibrator_bg > BG_BOTTOM_INDX) && (conv_vibrator_bg <= BG_TOP_INDX))
            && ((last_alert_time == 0) || (last_alert_time > BG_SNOOZE)) )
            || ( ((conv_vibrator_bg > BG_BOTTOM_INDX) && (conv_vibrator_bg <= BG_TOP_INDX)) && (*bg_overwrite == false) ) ) {

        //APP_LOG(APP_LOG_LEVEL_DEBUG, "last_alert_time SNOOZE VALUE IN: %i", last_alert_time);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "bg_overwrite IN: %i", *bg_overwrite);

        // send alert and handle a bouncing connection
        if ((last_alert_time == 0) || (*bg_overwrite == false)) {
            //APP_LOG(APP_LOG_LEVEL_INFO, "BG VIBRATOR: VIBRATE");
            alert_handler(BG_VIBE);
            // don't know where we are coming from, so reset last alert time no matter what
            // set to 1 to prevent bouncing connection
            last_alert_time = 1;
            if (*bg_overwrite == false) {
                *bg_overwrite = true;
            }
        }

        // if hit snooze, reset snooze counter; will alert next time around
        if (last_alert_time > BG_SNOOZE) {
            last_alert_time = 0;
            specvalue_overwrite = false;
            hypolow_overwrite = false;
            biglow_overwrite = false;
            midlow_overwrite = false;
            low_overwrite = false;
            midhigh_overwrite = false;
            bighigh_overwrite = false;
            //APP_LOG(APP_LOG_LEVEL_INFO, "BG VIBRATOR, OVERWRITE RESET");
        }

        //APP_LOG(APP_LOG_LEVEL_DEBUG, "BG VIBRATOR, last_alert_time SNOOZE VALUE OUT: %i", last_alert_time);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "BG VIBRATOR, bg_overwrite OUT: %i", *bg_overwrite);
    }

}

/**
 * Handles figuring out if an animation is called for and calls it
 */
static void animate_bg() {
    // happy message; max message 24 characters
    // DO NOT GO OVER 24 CHARACTERS, INCLUDING SPACES OR YOU WILL CRASH
    // YOU HAVE BEEN WARNED
    char happymsg_buffer65[26] = "SUUGAR! YES PLZ FOR*MY*D\0";
    char happymsg_buffer83[26] = "PEDAL TO THE MEDAL! CK83\0";
    char happymsg_buffer143[26] = "YOUR PEBBLE LOVES U TOO\0";
    char happymsg_buffer123[26] = "ABC EASY AS D*CAN*BE\0";
    char happymsg_buffer116[26] = "VICTORY LANE! RYAN REED\0";
    char happymsg_buffer222[26] = "UR BG'S MOVIN NO*LIE*LIE\0";
    char happymsg_buffer321[26] = "BLAST OFF! HOUSTON?\0";
    //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, FUNCTION START");

    if (HardCodeNoAnimations == false) {
        if ( ((s_current_bg_is_mmol == false) && (current_bg == 100)) || ((s_current_bg_is_mmol == true) && (current_bg == 55)) ) {
            //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, ANIMATE PERFECT BG");
            animate_perfectbg();
        } // perfect bg club, animate BG

        if ((s_current_bg_is_mmol == false) && (current_bg == 143)) {
            //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, ANIMATE HAPPY MSG LAYER");
            animate_happymsg(happymsg_buffer143);
        } // animate happy msg layer @ 143

        if ((s_current_bg_is_mmol == false) && (current_bg == 116)) {
            //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, ANIMATE HAPPY MSG LAYER");
            animate_happymsg(happymsg_buffer116);
        } // animate happy msg layer @ 116

        if ( ((s_current_bg_is_mmol == false) && (current_bg == 222)) || ((s_current_bg_is_mmol == true) && (current_bg == 120)) ) {
            //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, ANIMATE HAPPY MSG LAYER");
            animate_happymsg(happymsg_buffer222);
        } // animate happy msg layer @ 222

        if ( ((s_current_bg_is_mmol == false) && (current_bg == 83)) || ((s_current_bg_is_mmol == true) && (current_bg == 83)) ) {
            //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, ANIMATE HAPPY MSG LAYER");
            animate_happymsg(happymsg_buffer83);
        } // animate happy msg layer @ 83

        if (HardCodeAllAnimations == true) {
            // extra animations for those that want them, these are the old ones
            if ((s_current_bg_is_mmol == false) && (current_bg == 321)) {
                //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, ANIMATE HAPPY MSG LAYER");
                animate_happymsg(happymsg_buffer321);
            } // animate happy msg layer @ 321

            if ((s_current_bg_is_mmol == false) && (current_bg == 123)) {
                //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, ANIMATE HAPPY MSG LAYER");
                animate_happymsg(happymsg_buffer123);
            }  // animate happy msg layer @ 123

            if ( ((s_current_bg_is_mmol == false) && (current_bg == 65)) || ((s_current_bg_is_mmol == true) && (current_bg == 35)) ) {
                //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, ANIMATE HAPPY MSG LAYER");
                animate_happymsg(happymsg_buffer65);
            } // animate happy msg layer @ 65
        }
    }
}

/**
 * Handles interpretation of the current BG and sets the s_bg_layer text or the s_calcraw layers
 * appropriately
 */
static void process_bg() {
    //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, FUNCTION START");

    // CONSTANTS
    const uint8_t BG_BUFFER_SIZE = 6;

    // set special value alert to false no matter what
    setSpecialAlert(false);

    //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD BG, BGATOI IN, CURRENT_BG: %d LAST_BG: %s ", current_bg, current_bg_str);
    current_bg = myBGAtoi(current_bg_str);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD BG, BG ATOI OUT, CURRENT_BG: %d LAST_BG: %s ", current_bg, current_bg_str);

    //APP_LOG(APP_LOG_LEVEL_DEBUG, "LAST BG: %s", current_bg_str);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "CURRENT BG: %i", current_bg);

    // check for init code or error code
    if ((current_bg <= 0) || (current_bg_str[0] == '-')) {
        last_alert_time = 0;

        //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, UNEXPECTED BG: SET ERR ICON");
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD BG, UNEXP BG, CURRENT_BG: %d LAST_BG: %s ", current_bg, current_bg_str);

        text_layer_set_text(s_bg_layer, "ERR");

        setSpecialAlert(true);
    } // if current_bg <= 0

    else {
        // valid BG

        // check for special value, if special value, then replace icon and blank BG; else send current BG
        //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, BEFORE CREATE SPEC VALUE BITMAP");
        if ((current_bg == specvalue_ptr[NO_ANTENNA_VALUE_INDX]) || (current_bg == specvalue_ptr[BAD_RF_VALUE_INDX])) {
            //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, SPECIAL VALUE: SET BROKEN ANTENNA");
            text_layer_set_text(s_bg_layer, "");
            setSpecialAlert(true);
        } else if (current_bg == specvalue_ptr[SENSOR_NOT_CALIBRATED_VALUE_INDX]) {
            //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, SPECIAL VALUE: SET BLOOD DROP");
            text_layer_set_text(s_bg_layer, "");
            setSpecialAlert(true);
        } else if ((current_bg == specvalue_ptr[SENSOR_NOT_ACTIVE_VALUE_INDX]) || (current_bg == specvalue_ptr[MINIMAL_DEVIATION_VALUE_INDX])
                   || (current_bg == specvalue_ptr[STOP_LIGHT_VALUE_INDX])) {
            //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, SPECIAL VALUE: SET STOP LIGHT");
            text_layer_set_text(s_bg_layer, "");
            setSpecialAlert(true);
        } else if (current_bg == specvalue_ptr[HOURGLASS_VALUE_INDX]) {
            //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, SPECIAL VALUE: SET HOUR GLASS");
            text_layer_set_text(s_bg_layer, "");
            setSpecialAlert(true);
        } else if (current_bg == specvalue_ptr[QUESTION_MARKS_VALUE_INDX]) {
            //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, SPECIAL VALUE: SET QUESTION MARKS, CLEAR TEXT");
            text_layer_set_text(s_bg_layer, "");
            //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, SPECIAL VALUE: SET QUESTION MARKS, SET BITMAP");
            //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, SPECIAL VALUE: SET QUESTION MARKS, DONE");
            setSpecialAlert(true);
        } else if (current_bg < bg_ptr[SPECVALUE_BG_INDX]) {
            //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, UNEXPECTED SPECIAL VALUE: SET LOGO ICON");
            text_layer_set_text(s_bg_layer, "");
            setSpecialAlert(true);
        } // end special value checks

        //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, AFTER CREATE SPEC VALUE BITMAP");

        if (s_specialvalue_alert == false) {
            // we didn't find a special value, so set BG instead
            // arrow icon already set separately
            if (current_bg < bg_ptr[SHOWLOW_BG_INDX]) {
                //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG: SET TO LO");
                text_layer_set_text(s_bg_layer, "LO");
            } else if (current_bg > bg_ptr[SHOWHIGH_BG_INDX]) {
                //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG: SET TO HI");
                text_layer_set_text(s_bg_layer, "HI");
            } else {
                // else update with current BG
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD BG, SET TO BG: %s ", current_bg_str);
                text_layer_set_text(s_bg_layer, current_bg_str);
            }
        } // end bg checks (if special_value_bitmap)

        // see if we're going to use the current bg or the calculated raw bg for vibrations
        if ( ((current_bg > 0) && (current_bg < bg_ptr[SPECVALUE_BG_INDX])) && (have_calc_raw == true) ) {

            current_calc_raw = myBGAtoi(last_calc_raw);

            //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD BG, TurnOffVibrationsCalcRaw: %d", TurnOffVibrationsCalcRaw);

            if (TurnOffVibrationsCalcRaw == false) {
                // set current_bg to calculated raw so we can vibrate on that instead
                current_bg = current_calc_raw;
                if (s_current_bg_is_mmol == false) {
                    bg_ptr = BG_MGDL;
                } else {
                    bg_ptr = BG_MMOL;
                }
            } // TurnOffVibrationsCalcRaw

            // use calculated raw values in BG field; if different cascade down so we have last three values
            if (current_calc_raw != current_calc_raw1) {
                strncpy(last_calc_raw3, last_calc_raw2, BG_BUFFER_SIZE);
                strncpy(last_calc_raw2, last_calc_raw1, BG_BUFFER_SIZE);
                strncpy(last_calc_raw1, last_calc_raw, BG_BUFFER_SIZE);
                current_calc_raw1 = current_calc_raw;
            }
        }

        else {
            // if not in special values or don't have calculated raw, blank out the fields
            strncpy(last_calc_raw1, " ", BG_BUFFER_SIZE);
            strncpy(last_calc_raw2, " ", BG_BUFFER_SIZE);
            strncpy(last_calc_raw3, " ", BG_BUFFER_SIZE);
        }

        // set bg field accordingly for calculated raw layer
        text_layer_set_text(s_calcraw_last1_layer, last_calc_raw1);
        text_layer_set_text(s_calcraw_last2_layer, last_calc_raw2);
        text_layer_set_text(s_calcraw_last3_layer, last_calc_raw3);

        //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD BG, START VIBRATE, CURRENT_BG: %d LAST_BG: %s ", current_bg, current_bg_str);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD BG, START VIBRATE, CURRENT_CALC_RAW: %d LAST_CALC_RAW: %s ", current_calc_raw, last_calc_raw);
        //        //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD BG, START VIBRATE, CALC_RAW 2: %d FORMAT CALC RAW 2: %s ", current_calc_raw2, formatted_calc_raw2);
        //        //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD BG, START VIBRATE, CALC_RAW 3: %d FORMAT CALC RAW 2: %s ", current_calc_raw3, formatted_calc_raw3);

        bg_vibrator (0, bg_ptr[SPECVALUE_BG_INDX], SPECVALUE_SNZ_MIN, &specvalue_overwrite, SPECVALUE_VIBE);
        bg_vibrator (bg_ptr[SPECVALUE_BG_INDX], bg_ptr[HYPOLOW_BG_INDX], HYPOLOW_SNZ_MIN, &hypolow_overwrite, HYPOLOWBG_VIBE);
        bg_vibrator (bg_ptr[HYPOLOW_BG_INDX], bg_ptr[BIGLOW_BG_INDX], BIGLOW_SNZ_MIN, &biglow_overwrite, BIGLOWBG_VIBE);
        bg_vibrator (bg_ptr[BIGLOW_BG_INDX], bg_ptr[MIDLOW_BG_INDX], MIDLOW_SNZ_MIN, &midlow_overwrite, LOWBG_VIBE);
        bg_vibrator (bg_ptr[MIDLOW_BG_INDX], bg_ptr[LOW_BG_INDX], LOW_SNZ_MIN, &low_overwrite, LOWBG_VIBE);
        bg_vibrator (bg_ptr[HIGH_BG_INDX], bg_ptr[MIDHIGH_BG_INDX], HIGH_SNZ_MIN, &high_overwrite, HIGHBG_VIBE);
        bg_vibrator (bg_ptr[MIDHIGH_BG_INDX], bg_ptr[BIGHIGH_BG_INDX], MIDHIGH_SNZ_MIN, &midhigh_overwrite, HIGHBG_VIBE);
        bg_vibrator (bg_ptr[BIGHIGH_BG_INDX], 1000, BIGHIGH_SNZ_MIN, &bighigh_overwrite, BIGHIGHBG_VIBE);

        // else "normal" range or init code
        if ( ((current_bg > bg_ptr[LOW_BG_INDX]) && (current_bg < bg_ptr[HIGH_BG_INDX]))
                || (current_bg <= 0) ) {

            // do nothing; just reset snooze counter
            last_alert_time = 0;
        } // else if "NORMAL RANGE" BG

    } // else if current bg <= 0

    animate_bg();
    //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG, FUNCTION OUT");
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD BG, FUNCTION OUT, SNOOZE VALUE: %d", last_alert_time);
}

static void process_icon() {
    //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD ICON ARROW FUNCTION START");

    static bool double_down_alert = false;
    static uint8_t current_icon_resource;

    uint8_t new_icon_resource = ARROW_ICONS[LOGO_ARROW_ICON_INDX];

    //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD ARROW ICON, BEFORE CHECK SPEC VALUE BITMAP");
    // check if special value set
    if (s_specialvalue_alert == false) {
        // no special value, set arrow
        // check for arrow direction, set proper arrow icon
        if ( (strcmp(current_icon, NO_ARROW) == 0) || (strcmp(current_icon, NOTCOMPUTE_ICON) == 0) || (strcmp(current_icon, OUTOFRANGE_ICON) == 0) ) {
            new_icon_resource = ARROW_ICONS[NONE_ARROW_ICON_INDX];
        } else if (strcmp(current_icon, DOUBLEUP_ARROW) == 0) {
            new_icon_resource = ARROW_ICONS[UPUP_ICON_INDX];
        } else if (strcmp(current_icon, SINGLEUP_ARROW) == 0) {
            new_icon_resource = ARROW_ICONS[UP_ICON_INDX];
        } else if (strcmp(current_icon, UP45_ARROW) == 0) {
            new_icon_resource = ARROW_ICONS[UP45_ICON_INDX];
        } else if (strcmp(current_icon, FLAT_ARROW) == 0) {
            new_icon_resource = ARROW_ICONS[FLAT_ICON_INDX];
        } else if (strcmp(current_icon, DOWN45_ARROW) == 0) {
            new_icon_resource = ARROW_ICONS[DOWN45_ICON_INDX];
        } else if (strcmp(current_icon, SINGLEDOWN_ARROW) == 0) {
            new_icon_resource = ARROW_ICONS[DOWN_ICON_INDX];
        } else if (strcmp(current_icon, DOUBLEDOWN_ARROW) == 0) {
            new_icon_resource = ARROW_ICONS[DOWNDOWN_ICON_INDX];
        } else if (strcmp(current_bg_delta, "NOEP") == 0 || strcmp(current_bg_delta, "LOAD") == 0) {
            new_icon_resource = SPECIAL_VALUE_ICONS[LOGO_SPECVALUE_ICON_INDX];
        }
    } else {
        // special values
        if ((current_bg <= 0) || (current_bg_str[0] == '-')) {
            new_icon_resource = SPECIAL_VALUE_ICONS[NONE_SPECVALUE_ICON_INDX];
        } else if ((current_bg == specvalue_ptr[NO_ANTENNA_VALUE_INDX]) || (current_bg == specvalue_ptr[BAD_RF_VALUE_INDX])) {
            new_icon_resource = SPECIAL_VALUE_ICONS[BROKEN_ANTENNA_ICON_INDX];
        } else if (current_bg == specvalue_ptr[SENSOR_NOT_CALIBRATED_VALUE_INDX]) {
            new_icon_resource = SPECIAL_VALUE_ICONS[BLOOD_DROP_ICON_INDX];
        } else if ((current_bg == specvalue_ptr[SENSOR_NOT_ACTIVE_VALUE_INDX]) ||
                   (current_bg == specvalue_ptr[MINIMAL_DEVIATION_VALUE_INDX]) ||
                   (current_bg == specvalue_ptr[STOP_LIGHT_VALUE_INDX])) {
            new_icon_resource = SPECIAL_VALUE_ICONS[STOP_LIGHT_ICON_INDX];
        } else if (current_bg == specvalue_ptr[HOURGLASS_VALUE_INDX]) {
            new_icon_resource = SPECIAL_VALUE_ICONS[HOURGLASS_ICON_INDX];
        } else if (current_bg == specvalue_ptr[QUESTION_MARKS_VALUE_INDX]) {
            new_icon_resource = SPECIAL_VALUE_ICONS[QUESTION_MARKS_ICON_INDX];
        } else if (current_bg < bg_ptr[SPECVALUE_BG_INDX]) {
            new_icon_resource = SPECIAL_VALUE_ICONS[LOGO_SPECVALUE_ICON_INDX];
        }
    }

    // handle the double down arrow alert
    if (new_icon_resource == ARROW_ICONS[DOWNDOWN_ICON_INDX] && !double_down_alert) {
        alert_handler(DOUBLEDOWN_VIBE);
        double_down_alert = true;
    } else {
        double_down_alert = false;
    }

    // only create and set a new icon if necessary
    if (current_icon_resource != new_icon_resource) {
        set_container_image(&s_icon_bitmap, s_icon_layer, new_icon_resource);
        current_icon_resource = new_icon_resource;
    }
}

static void process_cgmtime() {
    //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD CGMTIME FUNCTION START");

    // VARIABLES
    uint32_t current_cgm_timeago = 0;
    int cgm_timeago_diff = 0;
    int new_cgmicon_index;

    static time_t cgm_time_now = 0;
    static char formatted_cgm_timeago[10] = {0};

    char cgm_label_buffer[6] = {0};
    /* wait... they get the bluetooth disconnected warning, why clear this out?
    if (!s_bluetooth_connected) {
        text_layer_set_text(s_cgmtime_layer, "");
        set_container_image(&s_cgmicon_bitmap, s_cgmicon_layer, TIMEAGO_ICONS[RCVRNONE_ICON_INDX]);
        return;
    }
    */

    // initialize label buffer
    strncpy(cgm_label_buffer, "", LABEL_BUFFER_SIZE);

    //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD CGMTIME, NEW CGM TIME: %lu", current_cgm_time);

    if (current_cgm_time == 0) {
        // Init code or error code; set text layer & icon to empty value
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD CGMTIME, CGM TIME AGO INIT OR ERROR CODE: %s", cgm_label_buffer);
        text_layer_set_text(s_cgmtime_layer, "");
        set_container_image(&s_cgmicon_bitmap, s_cgmicon_layer, TIMEAGO_ICONS[RCVRNONE_ICON_INDX]);
    } else {

        cgm_time_now = time(NULL);
        current_cgm_timeago = abs(cgm_time_now - current_cgm_time);

        //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD CGMTIME, CURRENT CGM TIME: %lu", current_cgm_time);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD CGMTIME, STORED CGM TIME: %lu", stored_cgm_time);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD CGMTIME, TIME NOW IN CGM: %lu", cgm_time_now);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD CGMTIME, CURRENT CGM TIMEAGO: %lu", current_cgm_timeago);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD CGMTIME, INIT LOADING BOOL: %d", init_loading_cgm_timeago);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD CGMTIME, GM TIME AGO LABEL IN: %s", cgm_label_buffer);
        new_cgmicon_index = TIMEAGO_ICONS[RCVRON_ICON_INDX];

        //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD CGMTIME, GM TIME AGO LABEL IN: %s", cgm_label_buffer);

        if (current_cgm_timeago < MINUTEAGO) {
            cgm_timeago_diff = 0;
            strncpy (formatted_cgm_timeago, "now", TIMEAGO_BUFFER_SIZE);
        } else if (current_cgm_timeago < HOURAGO) {
            cgm_timeago_diff = (current_cgm_timeago / MINUTEAGO);
            snprintf(formatted_cgm_timeago, TIMEAGO_BUFFER_SIZE, "%i", cgm_timeago_diff);
            strncpy(cgm_label_buffer, "m", LABEL_BUFFER_SIZE);
            strcat(formatted_cgm_timeago, cgm_label_buffer);
        } else if (current_cgm_timeago < DAYAGO) {
            cgm_timeago_diff = (current_cgm_timeago / HOURAGO);
            snprintf(formatted_cgm_timeago, TIMEAGO_BUFFER_SIZE, "%i", cgm_timeago_diff);
            strncpy(cgm_label_buffer, "h", LABEL_BUFFER_SIZE);
            strcat(formatted_cgm_timeago, cgm_label_buffer);
        } else if (current_cgm_timeago < WEEKAGO) {
            cgm_timeago_diff = (current_cgm_timeago / DAYAGO);
            snprintf(formatted_cgm_timeago, TIMEAGO_BUFFER_SIZE, "%i", cgm_timeago_diff);
            strncpy(cgm_label_buffer, "d", LABEL_BUFFER_SIZE);
            strcat(formatted_cgm_timeago, cgm_label_buffer);
        } else {
            strncpy (formatted_cgm_timeago, "ERR", TIMEAGO_BUFFER_SIZE);
            new_cgmicon_index = TIMEAGO_ICONS[RCVRNONE_ICON_INDX];
        }

        text_layer_set_text(s_cgmtime_layer, formatted_cgm_timeago);

        // check to see if we need to show receiver off icon
        if ( ((cgm_timeago_diff >= CGMOUT_WAIT_MIN) ||
                ((strcmp(cgm_label_buffer, "") != 0) && (strcmp(cgm_label_buffer, "m") != 0))) ||
                ((current_cgm_timeago < HOURAGO) &&
                 ((current_cgm_timeago / MINUTEAGO) >= CGMOUT_INIT_WAIT_MIN) &&
                 (init_loading_cgm_timeago == true)) ) {
            // set receiver off icon
            //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD CGMTIME, SET RCVR OFF ICON, CGM TIMEAGO DIFF: %d", cgm_timeago_diff);
            //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD CGMTIME, SET RCVR OFF ICON, LABEL: %s", cgm_label_buffer);
            new_cgmicon_index = TIMEAGO_ICONS[RCVROFF_ICON_INDX];
            // Vibrate if we need to
            if ((cgm_off_alert == false) && (phone_off_alert == false)) {
                //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD CGMTIME, CGM TIMEAGO: VIBRATE");
                alert_handler(CGMOUT_VIBE);
                cgm_off_alert = true;
            } // if cgm_off_alert
        } else {
            if (cgm_off_alert == true) {
                cleared_outage = true;
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD CGMTIME, SET CLEARED OUTAGE: %i ", cleared_outage);
            }
            cgm_off_alert = false;
        }
        set_container_image(&s_cgmicon_bitmap, s_cgmicon_layer, new_cgmicon_index);
    }
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD CGMTIME, CGM TIMEAGO LABEL OUT: %s", cgm_label_buffer);
}

static void process_apptime() {
    //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD APPTIME, READ APP TIME FUNCTION START");
    /*

    I genuinely can't figure out what would be useful here that isn't happening in
    process_cgmtime() or the minute tick handler every minute anyway...

      uint32_t current_app_timeago = 0;
      int app_timeago_diff = 0;

      app_time_now = time(NULL);

      //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD APPTIME, TIME NOW: %lu", app_time_now);

      current_app_timeago = abs(app_time_now - current_app_time);

      //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD APPTIME, CURRENT APP TIMEAGO: %lu", current_app_timeago);

      app_timeago_diff = (current_app_timeago / MINUTEAGO);
      if ( (current_app_timeago < HOURAGO) && (app_timeago_diff >= PHONEOUT_WAIT_MIN) ) {

          // erase cgm ago times and cgm icon
          text_layer_set_text(cgmtime_layer, "");
          set_container_image(&cgmicon_bitmap, cgmicon_layer, TIMEAGO_ICONS[RCVRNONE_ICON_INDX]);
          init_loading_cgm_timeago = 111;
          //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD APPTIME, SET init_loading_cgm_timeago: %i", init_loading_cgm_timeago);

          //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD APPTIME, CHECK IF HAVE TO VIBRATE");
          // Vibrate if we need to
          if (phone_off_alert == 100) {
              //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD APPTIME, READ APP TIMEAGO: VIBRATE");
              alert_handler(PHONEOUT_VIBE);
              phone_off_alert = 111;
          }
      } else {
          // reset phone_off_alert
          if (phone_off_alert == 111) {
              cleared_outage = 111;
              stored_cgm_time = current_cgm_time;
              //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD APPTIME, SET CLEARED OUTAGE: %i ", cleared_outage);
          }
          phone_off_alert = 100;
      }
      //} // else init code
    */
    //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD APPTIME, FUNCTION OUT");
}

static void process_bg_delta() {
    //APP_LOG(APP_LOG_LEVEL_INFO, "BG DELTA FUNCTION START");

    // CONSTANTS
    const uint8_t MSGLAYER_BUFFER_SIZE = 14;
    const uint8_t BGDELTA_LABEL_SIZE = 14;
    const uint8_t BGDELTA_FORMATTED_SIZE = 14;

    // VARIABLES
    static char formatted_bg_delta[14] = {0};

    char delta_label_buffer[14] = {0};

    if (!s_bluetooth_connected && !TurnOff_NOBLUETOOTH_Msg) {
        text_layer_set_text(s_message_layer, "NO BLUETOOTH");
        return;
    }

    // check for CHECK PHONE condition, if true set message
    if ((phone_off_alert == true) && (TurnOff_CHECKPHONE_Msg == false)) {
        text_layer_set_text(s_message_layer, "CHECK PHONE");
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD BG DELTA MSG, init_loading_cgm_timeago: %i", init_loading_cgm_timeago);
        return;
    }

    // check for CHECK CGM condition, if true set message
    if ((cgm_off_alert == true) && (TurnOff_CHECKCGM_Msg == false)) {
        text_layer_set_text(s_message_layer, "CHECK RIG");
        return;
    }

    // check for special messages; if no string, set no message
    if (strcmp(current_bg_delta, "") == 0) {
        strncpy(formatted_bg_delta, "", MSGLAYER_BUFFER_SIZE);
        text_layer_set_text(s_message_layer, formatted_bg_delta);
        return;
    }

    // check for NO ENDPOINT condition, if true set message
    // put " " (space) in bg field so logo continues to show
    if (strcmp(current_bg_delta, "NOEP") == 0) {
        strncpy(formatted_bg_delta, "NO ENDPOINT", MSGLAYER_BUFFER_SIZE);
        text_layer_set_text(s_message_layer, formatted_bg_delta);
        text_layer_set_text(s_bg_layer, " ");
        setSpecialAlert(false);
        return;
    }

    // check for COMPRESSION (compression low) condition, if true set message
    if (strcmp(current_bg_delta, "PRSS") == 0) {
        strncpy(formatted_bg_delta, "COMPRESSION?", MSGLAYER_BUFFER_SIZE);
        text_layer_set_text(s_message_layer, formatted_bg_delta);
        return;
    }

    // check for DATA OFFLINE condition, if true set message to fix condition
    if (strcmp(current_bg_delta, "OFF") == 0) {
        if (dataoffline_retries_counter >= DATAOFFLINE_RETRIES_MAX) {
            strncpy(formatted_bg_delta, "ATTN: NO DATA", MSGLAYER_BUFFER_SIZE);
            text_layer_set_text(s_message_layer, formatted_bg_delta);
            text_layer_set_text(s_bg_layer, " ");
            if (data_offline_alert == false) {
                //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG DELTA, DATA OFFLINE, VIBRATE");
                alert_handler(DATAOFFLINE_VIBE);
                data_offline_alert = true;
            } // data_offline_alert
            // NOTE: data_offline_alert is cleared in load_icon because that means we got a good message again
            // NOTE: dataoffline_retries_counter is cleared in load_icon because that means we got a good message again
        } else {
            dataoffline_retries_counter++;
        }
        return;
    } // strcmp "OFF"

    // check if LOADING.., if true set message
    // put " " (space) in bg field so logo continues to show
    if (strcmp(current_bg_delta, "LOAD") == 0) {
        strncpy(formatted_bg_delta, "LOADING 7.1", MSGLAYER_BUFFER_SIZE);
        text_layer_set_text(s_message_layer, formatted_bg_delta);
        text_layer_set_text(s_bg_layer, " ");
        setSpecialAlert(false);
        return;
    }

    // check for zero delta here; if get later then we know we have an error instead
    if (strcmp(current_bg_delta, "0") == 0) {
        strncpy(formatted_bg_delta, "0", BGDELTA_FORMATTED_SIZE);
        strncpy(delta_label_buffer, " mg/dL", BGDELTA_LABEL_SIZE);
        strcat(formatted_bg_delta, delta_label_buffer);
        text_layer_set_text(s_message_layer, formatted_bg_delta);
        return;
    }

    if (strcmp(current_bg_delta, "0.0") == 0) {
        strncpy(formatted_bg_delta, "0.0", BGDELTA_FORMATTED_SIZE);
        strncpy(delta_label_buffer, " mmol", BGDELTA_LABEL_SIZE);
        strcat(formatted_bg_delta, delta_label_buffer);
        text_layer_set_text(s_message_layer, formatted_bg_delta);
        return;
    }

    // check to see if we have MG/DL or MMOL
    // get current_bg_is_MMOL in myBGAtoi
    converted_bg_delta = myBGAtoi(current_bg_delta);

    // zero here, means we have an error instead; set error message
    if (converted_bg_delta == 0) {
        strncpy(formatted_bg_delta, "BG DELTA ERR", BGDELTA_FORMATTED_SIZE);
        text_layer_set_text(s_message_layer, formatted_bg_delta);
        return;
    }

    if (!s_current_bg_is_mmol) {
        // set mg/dL string
        //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG DELTA: FOUND MG/DL, SET STRING");
        if (converted_bg_delta >= 100) {
            // bg delta too big, set zero instead
            strncpy(formatted_bg_delta, "0", BGDELTA_FORMATTED_SIZE);
        } else {
            strncpy(formatted_bg_delta, current_bg_delta, BGDELTA_FORMATTED_SIZE);
        }
        strncpy(delta_label_buffer, " mg/dL", BGDELTA_LABEL_SIZE);
        strcat(formatted_bg_delta, delta_label_buffer);
    } else {
        // set mmol string
        //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BG DELTA: FOUND MMOL, SET STRING");
        if (converted_bg_delta >= 55) {
            // bg delta too big, set zero instead
            strncpy(formatted_bg_delta, "0.0", BGDELTA_FORMATTED_SIZE);
        } else {
            strncpy(formatted_bg_delta, current_bg_delta, BGDELTA_FORMATTED_SIZE);
        }
        strncpy(delta_label_buffer, " mmol", BGDELTA_LABEL_SIZE);
        strcat(formatted_bg_delta, delta_label_buffer);
    }

    text_layer_set_text(s_message_layer, formatted_bg_delta);
}

static void process_rig_battlevel() {
    //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BATTLEVEL, FUNCTION START");

    const uint8_t BATTLEVEL_LABEL_SIZE = 5;
    const uint8_t BATTLEVEL_PERCENT_SIZE = 6;

    static char formatted_battlevel[10] = {0};
    static uint8_t LowBatteryAlert = false;

    uint8_t current_battlevel = 0;
    char battlevel_percent[6] = {0};

    //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD BATTLEVEL, LAST BATTLEVEL: %s", last_battlevel);

    if (strcmp(last_battlevel, " ") == 0) {
        // Init code or no battery, can't do battery; set text layer to empty value
        //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BATTLEVEL, NO BATTERY");
        text_layer_set_text(s_rig_battlevel_layer, "");
        LowBatteryAlert = false;
        return;
    }

    if (strcmp(last_battlevel, "0") == 0) {
        // Zero battery level; set here, so if we get zero later we know we have an error instead
        //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BATTLEVEL, ZERO BATTERY, SET STRING");
        text_layer_set_text(s_rig_battlevel_layer, "0%");
        //layer_set_hidden((Layer *)inv_rig_battlevel_layer, false);
        if (LowBatteryAlert == false) {
            //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BATTLEVEL, ZERO BATTERY, VIBRATE");
            alert_handler(LOWBATTERY_VIBE);
            LowBatteryAlert = true;
        }
        return;
    }

    current_battlevel = atoi(last_battlevel);

    //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD BATTLEVEL, CURRENT BATTLEVEL: %i", current_battlevel);

    if ((current_battlevel <= 0) || (current_battlevel > 100) || (last_battlevel[0] == '-')) {
        // got a negative or out of bounds or error battery level
        //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BATTLEVEL, UNKNOWN, ERROR BATTERY");
        text_layer_set_text(s_rig_battlevel_layer, "ERR");
        //layer_set_hidden((Layer *)inv_rig_battlevel_layer, false);
        return;
    }
    // initialize formatted battlevel
    strncpy(formatted_battlevel, " ", BATTLEVEL_FORMAT_SIZE);

    // get current battery level and set battery level text with percent
    if (current_battlevel < 10) {
        strncpy(formatted_battlevel, "Rig  ", BATTLEVEL_LABEL_SIZE);
    } else {
        strncpy(formatted_battlevel, "Rig ", BATTLEVEL_LABEL_SIZE);
    }
    snprintf(battlevel_percent, BATTLEVEL_PERCENT_SIZE, "%i%%", current_battlevel);
    strcat(formatted_battlevel, battlevel_percent);
    text_layer_set_text(s_rig_battlevel_layer, formatted_battlevel);

    if ( (current_battlevel > 10) && (current_battlevel <= 20) ) {
        //layer_set_hidden((Layer *)inv_rig_battlevel_layer, false);
        if (LowBatteryAlert == false) {
            //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BATTLEVEL, LOW BATTERY, 20 OR LESS, VIBRATE");
            alert_handler(LOWBATTERY_VIBE);
            LowBatteryAlert = true;
        }
    }

    if ( (current_battlevel > 5) && (current_battlevel <= 10) ) {
        //layer_set_hidden((Layer *)inv_rig_battlevel_layer, false);
        if (LowBatteryAlert == false) {
            //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BATTLEVEL, LOW BATTERY, 10 OR LESS, VIBRATE");
            alert_handler(LOWBATTERY_VIBE);
            LowBatteryAlert = true;
        }
    }

    if ( (current_battlevel > 0) && (current_battlevel <= 5) ) {
        //layer_set_hidden((Layer *)inv_rig_battlevel_layer, false);
        if (LowBatteryAlert == false) {
            //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BATTLEVEL, LOW BATTERY, 5 OR LESS, VIBRATE");
            alert_handler(LOWBATTERY_VIBE);
            LowBatteryAlert = true;
        }
    }

    //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD BATTLEVEL, END FUNCTION");
}

static void process_values() {
    //APP_LOG(APP_LOG_LEVEL_DEBUG,"Loaded Values: %s", current_values);

    int num_a_items = 0;
    char *o;
    int mgormm = 0;
    int vibes = 0;
    int rawvibrate = 0;
    if (current_values == NULL) {
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "vals == NULL");
        return;
    } else {
        o = strtok(current_values, ",");
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "mg or mm: %s", o);
        mgormm = atoi(o);

        while (o != NULL) {
            num_a_items++;
            switch (num_a_items) {
            case 2:
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "lowbg: %s", o);
                if (mgormm == 0) {
                    LOW_BG_MGDL = atoi(o);
                    if (LOW_BG_MGDL < 60) {
                        MIDLOW_BG_MGDL = 55;
                        BIGLOW_BG_MGDL = 50;
                        HYPOLOW_BG_MGDL = 45;
                    } else if (LOW_BG_MGDL < 70) {
                        MIDLOW_BG_MGDL = 60;
                        BIGLOW_BG_MGDL = 55;
                        HYPOLOW_BG_MGDL = 50;
                    }
                } else {
                    LOW_BG_MMOL = atoi(o);
                    if (LOW_BG_MMOL < 33) {
                        MIDLOW_BG_MMOL = 31;
                        BIGLOW_BG_MMOL = 28;
                        HYPOLOW_BG_MMOL = 25;
                    } else if (LOW_BG_MMOL < 39) {
                        MIDLOW_BG_MMOL = 33;
                        BIGLOW_BG_MMOL = 31;
                        HYPOLOW_BG_MMOL = 28;
                    }
                }
                break;
            case 3:
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "highbg: %s", o);
                if (mgormm == 0) {
                    HIGH_BG_MGDL = atoi(o);
                    if (HIGH_BG_MGDL > 239) {
                        MIDHIGH_BG_MGDL = 300;
                        BIGHIGH_BG_MGDL = 350;
                    }
                } else {
                    HIGH_BG_MMOL = atoi(o);
                    if (HIGH_BG_MMOL > 132) {
                        MIDHIGH_BG_MMOL = 166;
                        BIGHIGH_BG_MMOL =  200;
                    }
                }
                break;
            case 4:
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "lowsnooze: %s", o);
                LOW_SNZ_MIN = atoi(o);
                break;
            case 5:
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "highsnooze: %s", o);
                HIGH_SNZ_MIN = atoi(o);
                break;
            case 6:
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "lowvibe: %s", o);
                LOWBG_VIBE = atoi(o);
                break;
            case 7:
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "highvibe: %s", o);
                HIGHBG_VIBE = atoi(o);
                break;
            case 8:
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "vibepattern: %s", o);
                vibes = atoi(o);
                if (vibes == 0) {
                    TurnOffAllVibrations = true;
                    TurnOffStrongVibrations = true;
                } else if (vibes == 1) {
                    TurnOffAllVibrations = false;
                    TurnOffStrongVibrations = true;
                } else if (vibes == 2) {
                    TurnOffAllVibrations = false;
                    TurnOffStrongVibrations = false;
                }
                break;
            case 9:
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "timeformat: %s", o);
                timeformat = atoi(o);
                break;
            case 10:
                //APP_LOG(APP_LOG_LEVEL_DEBUG, "rawvibrate: %s", o);
                rawvibrate = atoi(o);
                if (rawvibrate == 0) {
                    TurnOffVibrationsCalcRaw = true;
                } else {
                    TurnOffVibrationsCalcRaw = false;
                }
                break;
            }
            o = strtok(NULL, ",");
        }
    }
}

static void process_noise() {
    //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD NOISE, FUNCTION START");

    const uint8_t NOISE_FORMATTED_SIZE = 8;

    static char formatted_noise[8] = {0};

    //APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD NOISE, CURRENT NOISE VALUE: %i ", current_noise_value);

    switch (current_noise_value) {

    case NO_NOISE:;
        strncpy(formatted_noise, " ", NOISE_FORMATTED_SIZE);
        break;
    case CLEAN_NOISE:;
        strncpy(formatted_noise, "Cln", NOISE_FORMATTED_SIZE);
        break;
    case LIGHT_NOISE:;
        strncpy(formatted_noise, "Lgt", NOISE_FORMATTED_SIZE);
        break;
    case MEDIUM_NOISE:;
        strncpy(formatted_noise, "Med", NOISE_FORMATTED_SIZE);
        break;
    case HEAVY_NOISE:;
        strncpy(formatted_noise, "Hvy", NOISE_FORMATTED_SIZE);
        break;
    case WARMUP_NOISE:;
        strncpy(formatted_noise, " ", NOISE_FORMATTED_SIZE);
        break;
    case OTHER_NOISE:;
        strncpy(formatted_noise, " ", NOISE_FORMATTED_SIZE);
        break;
    default:;
        strncpy(formatted_noise, "ERR", NOISE_FORMATTED_SIZE);
    }

    //APP_LOG(APP_LOG_LEVEL_DEBUG, "SYNC TUPLE, NOISE: %s ", formatted_noise);

    text_layer_set_text(s_noise_layer, formatted_noise);

    //APP_LOG(APP_LOG_LEVEL_INFO, "LOAD NOISE, END FUNCTION");
}

/**
 * App sync changed handler callback. This checks for changes in stored values and runs the appropriate
 * processing functions if necessary.
 * @param key       [description]
 * @param new_tuple [description]
 * @param old_tuple [description]
 * @param context   [description]
 */
static void sync_changed_handler(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "starting sync_changed_handler");
    const uint8_t ICON_MSGSTR_SIZE = 4;
    const uint8_t BG_MSGSTR_SIZE = 6;
    const uint8_t BGDELTA_MSGSTR_SIZE = 6;
    const uint8_t BATTLEVEL_MSGSTR_SIZE = 4;
    const uint8_t VALUE_MSGSTR_SIZE = 25;

    // reset appsync retries counter
    appsyncandmsg_retries_counter = 0;

    switch (key) {
    case CGM_ICON_KEY:;
        //APP_LOG(APP_LOG_LEVEL_INFO, "SYNC TUPLE: ICON ARROW");
        //TODO: check for data offline here?
        if (strcmp(current_icon, new_tuple->value->cstring) != 0) {
            strncpy(current_icon, new_tuple->value->cstring, ICON_MSGSTR_SIZE);
            process_icon();
        }
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "SYNC TUPLE, ICON VALUE: %s ", current_icon);
        break;

    case CGM_BG_KEY:;
        //APP_LOG(APP_LOG_LEVEL_INFO, "SYNC TUPLE: BG CURRENT");
        if (strcmp(current_bg_str, new_tuple->value->cstring) != 0) {
            strncpy(current_bg_str, new_tuple->value->cstring, BG_MSGSTR_SIZE);
            process_bg();
        }
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "SYNC TUPLE, BG VALUE: %s ", current_bg_str);
        break;
    case CGM_TCGM_KEY:;
        //APP_LOG(APP_LOG_LEVEL_INFO, "SYNC TUPLE: READ CGM TIME");
        if (current_cgm_time != new_tuple->value->uint32) {
            current_cgm_time = new_tuple->value->uint32;
            process_cgmtime();
            set_next_fetch();
        }
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "SYNC TUPLE, CLEARED OUTAGE IN: %i ", cleared_outage);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "SYNC TUPLE, CURRENT CGM TIME: %lu ", current_cgm_time);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "SYNC TUPLE, CLEARED OUTAGE OUT: %i ", cleared_outage);
        //APP_LOG(APP_LOG_LEVEL_INFO, "SYNC TUPLE: READ CGM TIME OUT");
        break;

    case CGM_TAPP_KEY:;
        //APP_LOG(APP_LOG_LEVEL_INFO, "SYNC TUPLE: READ APP TIME NOW");
        if (current_app_time != new_tuple->value->uint32) {
            current_app_time = new_tuple->value->uint32;
            process_apptime();
        }
        break;

    case CGM_DLTA_KEY:;
        //APP_LOG(APP_LOG_LEVEL_INFO, "SYNC TUPLE: BG DELTA");
        if (strcmp(current_bg_delta, new_tuple->value->cstring) != 0) {
            strncpy(current_bg_delta, new_tuple->value->cstring, BGDELTA_MSGSTR_SIZE);
            process_bg_delta();
        }

        //APP_LOG(APP_LOG_LEVEL_DEBUG, "SYNC TUPLE, BG DELTA VALUE: %s ", current_bg_delta);
        break;

    case CGM_UBAT_KEY:;
        //APP_LOG(APP_LOG_LEVEL_INFO, "SYNC TUPLE: UPLOADER BATTERY LEVEL");
        if (strcmp(last_battlevel, new_tuple->value->cstring) != 0) {
            strncpy(last_battlevel, new_tuple->value->cstring, BATTLEVEL_MSGSTR_SIZE);
            process_rig_battlevel();
        }

        //APP_LOG(APP_LOG_LEVEL_DEBUG, "SYNC TUPLE, BATTERY LEVEL VALUE: %s ", last_battlevel);
        break;

    case CGM_NAME_KEY:;
        //APP_LOG(APP_LOG_LEVEL_INFO, "SYNC TUPLE: T1D NAME");
        if (strcmp(t1name, new_tuple->value->cstring) != 0 && strcmp("", new_tuple->value->cstring) != 0) {
            text_layer_set_text(s_t1dname_layer, new_tuple->value->cstring);
            strncpy(t1name, new_tuple->value->cstring, sizeof(t1name));
        }
        break;

    case CGM_VALS_KEY:;
        //APP_LOG(APP_LOG_LEVEL_INFO, "SYNC TUPLE: VALUES");
        if (strcmp(current_values, new_tuple->value->cstring) != 0) {
            strncpy(current_values, new_tuple->value->cstring, VALUE_MSGSTR_SIZE);
            process_values();
        }
        break;

    case CGM_CLRW_KEY:;
        //APP_LOG(APP_LOG_LEVEL_INFO, "SYNC TUPLE: CALCULATED RAW");
        if (strcmp(last_calc_raw, new_tuple->value->cstring) != 0 ||
                ((((strcmp(last_calc_raw, "0") == 0) || (strcmp(last_calc_raw, "0.0") == 0)) && strcmp(last_calc_raw, " ") != 0)) ) {
            strncpy(last_calc_raw, new_tuple->value->cstring, BG_MSGSTR_SIZE);
            if ( (strcmp(last_calc_raw, "0") == 0) || (strcmp(last_calc_raw, "0.0") == 0) ) {
                strncpy(last_calc_raw, " ", BG_MSGSTR_SIZE);
                have_calc_raw = false;
            } else {
                have_calc_raw = true;
            }
            text_layer_set_text(s_raw_calc_layer, last_calc_raw);
            process_bg();
        }
        break;

    case CGM_RWUF_KEY:;
        //APP_LOG(APP_LOG_LEVEL_INFO, "SYNC TUPLE: RAW UNFILTERED");
        if (strcmp(last_raw_unfilt, new_tuple->value->cstring) != 0) {
            strncpy(last_raw_unfilt, new_tuple->value->cstring, BG_MSGSTR_SIZE);
            if ( (strcmp(last_raw_unfilt, "0") == 0) || (strcmp(last_raw_unfilt, "0.0") == 0) || (TurnOnUnfilteredRaw == false) ) {
                strncpy(last_raw_unfilt, " ", BG_MSGSTR_SIZE);
            }
            text_layer_set_text(s_raw_unfilt_layer, last_raw_unfilt);
        }
        break;

    case CGM_NOIZ_KEY:;
        //APP_LOG(APP_LOG_LEVEL_INFO, "SYNC TUPLE: NOISE");
        if (current_noise_value != new_tuple->value->uint8) {
            current_noise_value = new_tuple->value->uint8;
            process_noise();
        }
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "SYNC TUPLE, NOISE: %i ", current_noise_value);
        break;
    }
}

static void sync_error_handler(DictionaryResult appsync_dict_error, AppMessageResult appsync_error, void *context) {

    DictionaryIterator *iter = NULL;
    AppMessageResult appsync_err_openerr = APP_MSG_OK;
    AppMessageResult appsync_err_senderr = APP_MSG_OK;

    if (!s_bluetooth_connected) {
        // bluetooth is out, BT message already set; return out
        return;
    }

    // increment app sync retries counter
    appsyncandmsg_retries_counter++;

    // if hit max counter, skip resend and flag user
    if (appsyncandmsg_retries_counter < APPSYNCANDMSG_RETRIES_MAX) {

        // APPSYNC ERROR debug logs
        //APP_LOG(APP_LOG_LEVEL_INFO, "APP SYNC ERROR");
        APP_LOG(APP_LOG_LEVEL_DEBUG, "APPSYNC ERR, MSG: %i RES: %s DICT: %i RES: %s RETRIES: %i",
                appsync_error, translate_app_error(appsync_error), appsync_dict_error, translate_dict_error(appsync_dict_error), appsyncandmsg_retries_counter);

        // try to resend the message; open app message outbox
        appsync_err_openerr = app_message_outbox_begin(&iter);
        if (appsync_err_openerr == APP_MSG_OK) {
            // could open app message outbox; send message
            appsync_err_senderr = app_message_outbox_send();
            if (appsync_err_senderr == APP_MSG_OK) {
                // everything OK, reset app_sync_err_alert so no vibrate
                if (app_sync_err_alert == true) {
                    cleared_outage = true;
                    //APP_LOG(APP_LOG_LEVEL_DEBUG, "APPSYNC ERR, SET CLEARED OUTAGE: %i ", cleared_outage);
                }
                app_sync_err_alert = false;
                // sent message OK; return
                return;
            } // if appsync_err_senderr
        } // if appsync_err_openerr
    } // if appsyncandmsg_retries_counter

    // flag error
    if (appsyncandmsg_retries_counter > APPSYNCANDMSG_RETRIES_MAX) {
        //APP_LOG(APP_LOG_LEVEL_INFO, "APP SYNC TOO MANY MESSAGES ERROR");
        APP_LOG(APP_LOG_LEVEL_DEBUG, "APPSYNC ERR, MSG: %i RES: %s DICT: %i RES: %s RETRIES: %i",
                appsync_error, translate_app_error(appsync_error), appsync_dict_error, translate_dict_error(appsync_dict_error), appsyncandmsg_retries_counter);
    } else {
        //APP_LOG(APP_LOG_LEVEL_INFO, "APP SYNC RESEND ERROR");
        APP_LOG(APP_LOG_LEVEL_DEBUG, "APPSYNC RESEND ERR, OPEN: %i RES: %s SEND: %i RES: %s RETRIES: %i",
                appsync_err_openerr, translate_app_error(appsync_err_openerr), appsync_err_senderr, translate_app_error(appsync_err_senderr), appsyncandmsg_retries_counter);
        return;
    }

    // check bluetooth again
    if (!s_bluetooth_connected) {
        // bluetooth is out, BT message already set; return out
        return;
    }

    // set message to RESTART WATCH -> PHONE
    text_layer_set_text(s_message_layer, "√APP/RESTRT");

    // reset appsync retries counter
    appsyncandmsg_retries_counter = 0;

    // erase cgm and app ago times (WHY? they're not invalid.)
    //text_layer_set_text(s_cgmtime_layer, "");

    // erase cgm icon
    //create_update_bitmap(&cgmicon_bitmap, cgmicon_layer, TIMEAGO_ICONS[RCVRNONE_ICON_INDX]);

    // check if need to vibrate
    if (app_sync_err_alert == false) {
        //APP_LOG(APP_LOG_LEVEL_INFO, "APPSYNC ERROR: VIBRATE");
        alert_handler(APPSYNC_ERR_VIBE);
        app_sync_err_alert = true;
    }
}

static void send_cmd_cgm(void) {
    DictionaryIterator *iter = NULL;
    AppMessageResult sendcmd_openerr = APP_MSG_OK;
    AppMessageResult sendcmd_senderr = APP_MSG_OK;

    //APP_LOG(APP_LOG_LEVEL_INFO, "SEND CMD IN, ABOUT TO OPEN APP MSG OUTBOX");
    sendcmd_openerr = app_message_outbox_begin(&iter);

    //APP_LOG(APP_LOG_LEVEL_INFO, "SEND CMD, MSG OUTBOX OPEN, CHECK FOR ERROR");
    if (sendcmd_openerr != APP_MSG_OK) {
        //APP_LOG(APP_LOG_LEVEL_INFO, "WATCH SENDCMD OPEN ERROR");
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "WATCH SENDCMD OPEN ERR CODE: %i RES: %s", sendcmd_openerr, translate_app_error(sendcmd_openerr));
        return;
    }

    //APP_LOG(APP_LOG_LEVEL_INFO, "SEND CMD, MSG OUTBOX OPEN, NO ERROR, ABOUT TO SEND MSG TO APP");
    sendcmd_senderr = app_message_outbox_send();

    if (sendcmd_senderr != APP_MSG_OK) {
        //APP_LOG(APP_LOG_LEVEL_INFO, "WATCH SENDCMD SEND ERROR");
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "WATCH SENDCMD SEND ERR CODE: %i RES: %s", sendcmd_senderr, translate_app_error(sendcmd_senderr));
    }

    //APP_LOG(APP_LOG_LEVEL_INFO, "SEND CMD OUT, SENT MSG TO APP");
}

static void fetch_timer_callback(void *data) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "TIMER CALLBACK IN, TIMER POP, ABOUT TO CALL SEND CMD");

    // send message
    send_cmd_cgm();

    //APP_LOG(APP_LOG_LEVEL_INFO, "TIMER CALLBACK, SEND CMD DONE, ABOUT TO REGISTER TIMER");
    //fetch_timer = app_timer_register((WATCH_MSGSEND_SECS * MS_IN_A_SECOND), fetch_timer_callback, NULL);
    next_fetch_time = time(NULL) + WATCH_MSGSEND_SECS;

    //APP_LOG(APP_LOG_LEVEL_INFO, "TIMER CALLBACK, REGISTER TIMER DONE");
}

static void inbox_dropped_handler(AppMessageResult appmsg_indrop_error, void *context) {
    // incoming appmessage send back from Pebble app dropped; no data received
    // have never seen handler get called, think because AppSync is always used
    // just set log now to avoid crash, if see log then can go back to old handler

    // APPMSG IN DROP debug logs
    //APP_LOG(APP_LOG_LEVEL_INFO, "APPMSG IN DROP ERROR");
    APP_LOG(APP_LOG_LEVEL_DEBUG, "APPMSG IN DROP ERR, CODE: %i RES: %s",
            appmsg_indrop_error, translate_app_error(appmsg_indrop_error));

}

static void outbox_failed_handler(DictionaryIterator *failed, AppMessageResult appmsg_outfail_error, void *context) {
    // outgoing appmessage send failed to deliver to Pebble
    // have never seen handler get called, think because AppSync is always used
    // just set log now to avoid crash, if see log then can go back to old handler

    // APPMSG OUT FAIL debug logs
    //APP_LOG(APP_LOG_LEVEL_INFO, "APPMSG OUT FAIL ERROR");
    APP_LOG(APP_LOG_LEVEL_DEBUG, "APPMSG OUT FAIL ERR, CODE: %i RES: %s",
            appmsg_outfail_error, translate_app_error(appmsg_outfail_error));
}

static void main_window_load(Window *window) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "win load start");

    Layer *window_layer = window_get_root_layer(window);

    s_res_gothic_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
    s_res_gothic_24_bold = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
    s_res_gothic_18_bold = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    s_res_bitham_42_bold = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);

    // TOPHALF WHITE
    s_tophalf_layer = text_layer_create(GRect(0, 0, 144, 83));
    text_layer_set_text_color(s_tophalf_layer, GColorBlack);
    text_layer_set_background_color(s_tophalf_layer, GColorWhite);
    text_layer_set_font(s_tophalf_layer, s_res_gothic_28_bold);
    text_layer_set_text_alignment(s_tophalf_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_tophalf_layer));

    // HAPPY MSG LAYER
    s_happymsg_layer = text_layer_create(GRect(-10, 33, 144, 55));
    text_layer_set_text_color(s_happymsg_layer, GColorBlack);
    text_layer_set_background_color(s_happymsg_layer, GColorClear);
    text_layer_set_font(s_happymsg_layer, s_res_gothic_24_bold);
    text_layer_set_text_alignment(s_happymsg_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_happymsg_layer));

    // DELTA BG / MESSAGE LAYER
    s_message_layer = text_layer_create(GRect(0, 33, 144, 55));
    text_layer_set_text_color(s_message_layer, GColorBlack);
    text_layer_set_background_color(s_message_layer, GColorClear);
    text_layer_set_font(s_message_layer, s_res_gothic_28_bold);
    text_layer_set_text_alignment(s_message_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_message_layer));

    // ICON, ARROW OR SPECIAL VALUE
    s_icon_layer = bitmap_layer_create(GRect(78, -2, 78, 50));
    bitmap_layer_set_alignment(s_icon_layer, GAlignCenter);
    bitmap_layer_set_background_color(s_icon_layer, GColorClear);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));

    // RIG BATTERY LEVEL
    s_rig_battlevel_layer = text_layer_create(GRect(70, 61, 72, 22));
    text_layer_set_text_color(s_rig_battlevel_layer, GColorBlack);
    text_layer_set_background_color(s_rig_battlevel_layer, GColorClear);
    text_layer_set_font(s_rig_battlevel_layer, s_res_gothic_18_bold);
    text_layer_set_text_alignment(s_rig_battlevel_layer, GTextAlignmentRight);
    layer_add_child(window_layer, text_layer_get_layer(s_rig_battlevel_layer));

    // BG
    s_bg_layer = text_layer_create(GRect(0, -5, 95, 47));
    text_layer_set_text_color(s_bg_layer, GColorBlack);
    text_layer_set_background_color(s_bg_layer, GColorClear);
    text_layer_set_font(s_bg_layer, s_res_bitham_42_bold);
    text_layer_set_text_alignment(s_bg_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_bg_layer));

    // CALCULATED RAW INSTEAD OF BG - LAST VALUE (1)
    s_calcraw_last1_layer = text_layer_create(GRect(0, -7, 40, 25));
    text_layer_set_text_color(s_calcraw_last1_layer, GColorBlack);
    text_layer_set_background_color(s_calcraw_last1_layer, GColorClear);
    text_layer_set_font(s_calcraw_last1_layer, s_res_gothic_24_bold);
    text_layer_set_text_alignment(s_calcraw_last1_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_calcraw_last1_layer));

    // CALCULATED RAW INSTEAD OF BG - 2ND LAST VALUE (2)
    s_calcraw_last2_layer = text_layer_create(GRect(32, 3, 40, 25));
    text_layer_set_text_color(s_calcraw_last2_layer, GColorBlack);
    text_layer_set_background_color(s_calcraw_last2_layer, GColorClear);
    text_layer_set_font(s_calcraw_last2_layer, s_res_gothic_24_bold);
    text_layer_set_text_alignment(s_calcraw_last2_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_calcraw_last2_layer));

    // CALCULATED RAW INSTEAD OF BG - 3RD LAST VALUE (3)
    s_calcraw_last3_layer = text_layer_create(GRect(63, 16, 40, 25));
    text_layer_set_text_color(s_calcraw_last3_layer, GColorBlack);
    text_layer_set_background_color(s_calcraw_last3_layer, GColorClear);
    text_layer_set_font(s_calcraw_last3_layer, s_res_gothic_24_bold);
    text_layer_set_text_alignment(s_calcraw_last3_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_calcraw_last3_layer));

    // PERFECT BG
    s_perfectbg_layer = bitmap_layer_create(GRect(0, -7, 95, 47));
    bitmap_layer_set_alignment(s_perfectbg_layer, GAlignTopLeft);
    bitmap_layer_set_background_color(s_perfectbg_layer, GColorClear);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_perfectbg_layer));

    // CGM TIME AGO ICON
    s_cgmicon_layer = bitmap_layer_create(GRect(2, 63, 40, 19));
    bitmap_layer_set_alignment(s_cgmicon_layer, GAlignLeft);
    bitmap_layer_set_background_color(s_cgmicon_layer, GColorWhite);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_cgmicon_layer));

    // CGM TIME AGO READING
    s_cgmtime_layer = text_layer_create(GRect(26, 56, 40, 24));
    text_layer_set_text_color(s_cgmtime_layer, GColorBlack);
    text_layer_set_background_color(s_cgmtime_layer, GColorClear);
    text_layer_set_font(s_cgmtime_layer, s_res_gothic_24_bold);
    text_layer_set_text_alignment(s_cgmtime_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_cgmtime_layer));

    // T1D NAME
    s_t1dname_layer = text_layer_create(GRect(2, 140, 69, 28));
    text_layer_set_text_color(s_t1dname_layer, GColorWhite);
    text_layer_set_background_color(s_t1dname_layer, GColorClear);
    text_layer_set_font(s_t1dname_layer, s_res_gothic_24_bold);
    text_layer_set_text_alignment(s_t1dname_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_t1dname_layer));

    // INVERTER BATTERY LAYER
    //inv_rig_battlevel_layer = inverter_layer_create(GRect(112, 66, 30, 15));
    //layer_add_child(window_get_root_layer(window_cgm), inverter_layer_get_layer(inv_rig_battlevel_layer));

    // TIME; CURRENT ACTUAL TIME FROM WATCH
    s_time_watch_layer = text_layer_create(GRect(0, 102, 144, 44));
    text_layer_set_text_color(s_time_watch_layer, GColorWhite);
    text_layer_set_background_color(s_time_watch_layer, GColorClear);
    text_layer_set_font(s_time_watch_layer, s_res_bitham_42_bold);
    text_layer_set_text_alignment(s_time_watch_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_time_watch_layer));

    // DATE
    s_date_app_layer = text_layer_create(GRect(39, 80, 72, 28));
    text_layer_set_text_color(s_date_app_layer, GColorWhite);
    text_layer_set_background_color(s_date_app_layer, GColorClear);
    text_layer_set_font(s_date_app_layer, s_res_gothic_28_bold);
    text_layer_set_text_alignment(s_date_app_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_date_app_layer));

    // RAW CALCULATED
    s_raw_calc_layer = text_layer_create(GRect(1, 76, 40, 25));
    text_layer_set_text_color(s_raw_calc_layer, GColorWhite);
    text_layer_set_background_color(s_raw_calc_layer, GColorClear);
    text_layer_set_font(s_raw_calc_layer, s_res_gothic_24_bold);
    text_layer_set_text_alignment(s_raw_calc_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_raw_calc_layer));

    // NOISE
    s_noise_layer = text_layer_create(GRect(85, 76, 58, 27));
    text_layer_set_text_color(s_noise_layer, GColorWhite);
    text_layer_set_background_color(s_noise_layer, GColorClear);
    text_layer_set_font(s_noise_layer, s_res_gothic_24_bold);
    text_layer_set_text_alignment(s_noise_layer, GTextAlignmentRight);
    layer_add_child(window_layer, text_layer_get_layer(s_noise_layer));

    // RAW UNFILT
    s_raw_unfilt_layer = text_layer_create(GRect(1, 92, 40, 25));
    text_layer_set_text_color(s_raw_unfilt_layer, GColorWhite);
    text_layer_set_background_color(s_raw_unfilt_layer, GColorClear);
    text_layer_set_font(s_raw_unfilt_layer, s_res_gothic_24_bold);
    text_layer_set_text_alignment(s_raw_unfilt_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_raw_unfilt_layer));

    // WATCH BATTERY LEVEL
    s_watch_battlevel_layer = text_layer_create(GRect(71, 145, 72, 22));
    text_layer_set_text_color(s_watch_battlevel_layer, GColorWhite);
    text_layer_set_background_color(s_watch_battlevel_layer, GColorBlack);
    text_layer_set_font(s_watch_battlevel_layer, s_res_gothic_18_bold);
    text_layer_set_text_alignment(s_watch_battlevel_layer, GTextAlignmentRight);
    layer_add_child(window_layer, text_layer_get_layer(s_watch_battlevel_layer));
}

static void main_window_unload(Window *window) {
    text_layer_destroy(s_tophalf_layer);
    text_layer_destroy(s_bg_layer);
    text_layer_destroy(s_cgmtime_layer);
    text_layer_destroy(s_message_layer);
    text_layer_destroy(s_rig_battlevel_layer);
    text_layer_destroy(s_watch_battlevel_layer);
    text_layer_destroy(s_t1dname_layer);
    text_layer_destroy(s_time_watch_layer);
    text_layer_destroy(s_date_app_layer);
    text_layer_destroy(s_happymsg_layer);
    text_layer_destroy(s_raw_calc_layer);
    text_layer_destroy(s_raw_unfilt_layer);
    text_layer_destroy(s_noise_layer);
    text_layer_destroy(s_calcraw_last1_layer);
    text_layer_destroy(s_calcraw_last2_layer);
    text_layer_destroy(s_calcraw_last3_layer);

    bitmap_layer_destroy(s_icon_layer);
    bitmap_layer_destroy(s_cgmicon_layer);
    bitmap_layer_destroy(s_perfectbg_layer);

    gbitmap_destroy(s_icon_bitmap);
    gbitmap_destroy(s_cgmicon_bitmap);
    gbitmap_destroy(s_specialvalue_bitmap);
    gbitmap_destroy(s_perfectbg_bitmap);
}

static void init(void) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "init entry");
    // ARRAY OF SPECIAL VALUES CONSTANTS; MGDL
    SPECVALUE_MGDL[0] = SENSOR_NOT_ACTIVE_VALUE_MGDL;     //0
    SPECVALUE_MGDL[1] = MINIMAL_DEVIATION_VALUE_MGDL;     //1
    SPECVALUE_MGDL[2] = NO_ANTENNA_VALUE_MGDL;            //2
    SPECVALUE_MGDL[3] = SENSOR_NOT_CALIBRATED_VALUE_MGDL; //3
    SPECVALUE_MGDL[4] = STOP_LIGHT_VALUE_MGDL;            //4
    SPECVALUE_MGDL[5] = HOURGLASS_VALUE_MGDL;             //5
    SPECVALUE_MGDL[6] = QUESTION_MARKS_VALUE_MGDL;        //6
    SPECVALUE_MGDL[7] = BAD_RF_VALUE_MGDL;                //7

    // ARRAY OF SPECIAL VALUES CONSTANTS; MMOL
    SPECVALUE_MMOL[0] = SENSOR_NOT_ACTIVE_VALUE_MMOL;     //0
    SPECVALUE_MMOL[1] = MINIMAL_DEVIATION_VALUE_MMOL;     //1
    SPECVALUE_MMOL[2] = NO_ANTENNA_VALUE_MMOL;            //2
    SPECVALUE_MMOL[3] = SENSOR_NOT_CALIBRATED_VALUE_MMOL; //3
    SPECVALUE_MMOL[4] = STOP_LIGHT_VALUE_MMOL;            //4
    SPECVALUE_MMOL[5] = HOURGLASS_VALUE_MMOL;             //5
    SPECVALUE_MMOL[6] = QUESTION_MARKS_VALUE_MMOL;        //6
    SPECVALUE_MMOL[7] = BAD_RF_VALUE_MMOL;                //7

    // ARRAY OF BG CONSTANTS; MGDL
    BG_MGDL[0] = SPECVALUE_BG_MGDL;  //0
    BG_MGDL[1] = SHOWLOW_BG_MGDL;    //1
    BG_MGDL[2] = HYPOLOW_BG_MGDL;    //2
    BG_MGDL[3] = BIGLOW_BG_MGDL;     //3
    BG_MGDL[4] = MIDLOW_BG_MGDL;     //4
    BG_MGDL[5] = LOW_BG_MGDL;        //5
    BG_MGDL[6] = HIGH_BG_MGDL;       //6
    BG_MGDL[7] = MIDHIGH_BG_MGDL;    //7
    BG_MGDL[8] = BIGHIGH_BG_MGDL;    //8
    BG_MGDL[9] = SHOWHIGH_BG_MGDL;   //9

    // ARRAY OF BG CONSTANTS; MMOL
    BG_MMOL[0] = SPECVALUE_BG_MMOL;  //0
    BG_MMOL[1] = SHOWLOW_BG_MMOL;    //1
    BG_MMOL[2] = HYPOLOW_BG_MMOL;    //2
    BG_MMOL[3] = BIGLOW_BG_MMOL;     //3
    BG_MMOL[4] = MIDLOW_BG_MMOL;     //4
    BG_MMOL[5] = LOW_BG_MMOL;        //5
    BG_MMOL[6] = HIGH_BG_MMOL;       //6
    BG_MMOL[7] = MIDHIGH_BG_MMOL;    //7
    BG_MMOL[8] = BIGHIGH_BG_MMOL;    //8
    BG_MMOL[9] = SHOWHIGH_BG_MMOL;   //9

    s_main_window = window_create();
    window_set_background_color(s_main_window, GColorBlack);
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
         .unload = main_window_unload,
    });
    window_stack_push(s_main_window, false);

    draw_date_from_app();
    //// set initial BT connection bool
    s_bluetooth_connected = bluetooth_connection_service_peek();
    // subscribe to the tick timer service
    tick_timer_service_subscribe(MINUTE_UNIT, minute_tick_handler);
    // subscribe to the bluetooth connection service
    bluetooth_connection_service_subscribe(bt_handler);

    watch_battery_handler(battery_state_service_peek());
    // subscribe to the watch battery state service
    battery_state_service_subscribe(watch_battery_handler);

    //APP_LOG(APP_LOG_LEVEL_INFO, "INIT CODE, REGISTER APP MESSAGE ERROR HANDLERS");
    app_message_register_inbox_dropped(inbox_dropped_handler);
    app_message_register_outbox_failed(outbox_failed_handler);

    //APP_LOG(APP_LOG_LEVEL_INFO, "INIT CODE, ABOUT TO CALL APP MSG OPEN");
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
    //APP_LOG(APP_LOG_LEVEL_INFO, "INIT CODE, APP MSG OPEN DONE");

    // put " " (space) in bg field so logo continues to show
    // " " (space) also shows these are init values, not bad or null values

    Tuplet initial_values[] = {
        TupletCString(CGM_ICON_KEY, " "),
        TupletCString(CGM_BG_KEY, " "),
        TupletInteger(CGM_TCGM_KEY, 0),
        TupletInteger(CGM_TAPP_KEY, 0),
        TupletCString(CGM_DLTA_KEY, "LOAD"),
        TupletCString(CGM_UBAT_KEY, " "),
        TupletCString(CGM_NAME_KEY, " "),
        TupletCString(CGM_VALS_KEY, " "),
        TupletCString(CGM_CLRW_KEY, " "),
        TupletCString(CGM_RWUF_KEY, " "),
        TupletInteger(CGM_NOIZ_KEY, 0)
    };

    //APP_LOG(APP_LOG_LEVEL_INFO, "WINDOW LOAD, ABOUT TO CALL APP SYNC INIT");
    app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer), initial_values, ARRAY_LENGTH(initial_values), sync_changed_handler, sync_error_handler, NULL);

    // init timer to null if needed, and register timer
    //APP_LOG(APP_LOG_LEVEL_INFO, "WINDOW LOAD, APP INIT DONE, ABOUT TO REGISTER TIMER");
    //fetch_timer_callback(NULL);
    fetch_timer = app_timer_register((10 * MS_IN_A_SECOND), fetch_timer_callback, NULL);

    //APP_LOG(APP_LOG_LEVEL_INFO, "WINDOW LOAD, TIMER REGISTER DONE");
}

static void deinit(void) {
    window_destroy(s_main_window);

    // unsubscribe to the tick timer service
    //APP_LOG(APP_LOG_LEVEL_INFO, "DEINIT, UNSUBSCRIBE TICK TIMER");
    tick_timer_service_unsubscribe();

    // unsubscribe to the bluetooth connection service
    //APP_LOG(APP_LOG_LEVEL_INFO, "DEINIT, UNSUBSCRIBE BLUETOOTH");
    bluetooth_connection_service_unsubscribe();

    // unsubscribe to the watch battery state service
    battery_state_service_unsubscribe();

    // cancel timers if they exist
    if (fetch_timer != NULL) {
        app_timer_cancel(fetch_timer);
    }

    app_sync_deinit(&s_sync);
}

int main(void) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "main entry");

    init();
    app_event_loop();
    deinit();
}