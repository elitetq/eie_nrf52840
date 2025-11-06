#include "my_state_machine.h"
#include <zephyr/smf.h>
#include "BTN.h"
#include "LED.h"

#define MAX_CHAR 50

// State Prototypes and Enums
static void s0_state_entry(void *o);
static enum smf_state_result s0_state_run(void *o);
static void s0_state_exit(void *o);

enum MACHINE_STATES {
    S0,
    S1,
    S2,
    S3,
    S4
};

// Struct object, holds static variables
typedef struct {
    struct smf_ctx ctx;
    uint8_t ubinary_code[8];
    char char_seq[MAX_CHAR];
    uint8_t seq_index; // used to track where we are in the current sequence
} smf_obj_t;

// SMF States array
static const struct smf_state smf_states[] = {
    [S0] = SMF_CREATE_STATE(s0_state_entry,s0_state_run,s0_state_exit,NULL,NULL),
    [S1] = SMF_CREATE_STATE(s0_state_entry,s0_state_run,s0_state_exit,NULL,NULL),
    [S2] = SMF_CREATE_STATE(s0_state_entry,s0_state_run,s0_state_exit,NULL,NULL),
    [S3] = SMF_CREATE_STATE(s0_state_entry,s0_state_run,s0_state_exit,NULL,NULL),
    [S4] = SMF_CREATE_STATE(s0_state_entry,s0_state_run,s0_state_exit,NULL,NULL)
};

// -------- USEFUL FUNCTIONS --------

void reset_ubinary(smf_obj_t *obj) {
    memset(obj->ubinary_code,0,8); // Set binary code to zeroes so far.
}

void reset_char(smf_obj_t *obj) {
    memset(obj->char_seq,0,50); // Set binary code to zeroes so far.
}

char cnv_binary_char(const uint8_t* binary) {
    // Assuming the binary array is 8 bytes long.
    char result = 0;
    for(int i = 0; i < 8; i++) {
        result <<= 1;
        result += binary[i];
    }
    return result;
}


// ---------- LIGHT EFFECTS -------------

void light_fx(char id) {
    for(int i = 0; i < 4; i++) {
        LED_set(i,0);
    }
    switch(id) {
        case 0: // light sweep
            LED_set(LED0,1);
            k_msleep(50);
            LED_set(LED1,1);
            LED_set(LED2,1);
            k_msleep(25);
            LED_set(LED0,0);
            k_msleep(25);
            LED_set(LED3,1);
            k_msleep(25);
            LED_set(LED1,0);
            LED_set(LED2,0);
            k_msleep(25);
            LED_set(LED3,0);
            break;
        case 1: // zigzag
            LED_set(LED0,1);
            LED_set(LED3,1);
            k_msleep(100);
            LED_set(LED0,0);
            LED_set(LED3,0);
            LED_set(LED1,1);
            LED_set(LED2,1);
            k_msleep(100);
            LED_set(LED1,0);
            LED_set(LED2,0);
            break;
        case 2: // blink
            LED_set(LED0,1);
            LED_set(LED1,1);
            LED_set(LED2,1);
            LED_set(LED3,1);
            k_msleep(100);
            LED_set(LED0,0);
            LED_set(LED1,0);
            LED_set(LED2,0);
            LED_set(LED3,0);
            break;
        case 3:

            break;
        default:
            printk("Invalid light_fx id: %d",id);
            break;
    }
}

//


static smf_obj_t smf_obj;

void state_machine_init() {
    reset_ubinary(&smf_obj);
    reset_char(&smf_obj);
    smf_obj.seq_index = 0;
    smf_set_initial(SMF_CTX(&smf_obj),&smf_states[S0]);
}

int state_machine_run() {
    return smf_run_state(SMF_CTX(&smf_obj));
}


/*
 * S0: Enter/Save/Reset Char Code
 */

static void s0_state_entry(void* o) {
    printk("Entering s0.\n");
}

static enum smf_state_result s0_state_run(void* o) {
    int i = 0;
    while(i < 8) {
        if(BTN_check_clear_pressed(BTN0)) {
            smf_obj.ubinary_code[i] = 1;
            light_fx(2);
            i++;
        } else if(BTN_check_clear_pressed(BTN1)) {
            smf_obj.ubinary_code[i] = 0;
            light_fx(2);
            i++;
        }
    }
    smf_obj.char_seq[smf_obj.seq_index++] = cnv_binary_char(smf_obj.ubinary_code);
    printk("%s\n",smf_obj.char_seq);
    
    k_msleep(500);
    light_fx(0);
    return SMF_EVENT_HANDLED;
}

static void s0_state_exit(void* o) {
    printk("Exiting s0.\n");
}
