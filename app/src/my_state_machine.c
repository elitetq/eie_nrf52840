#include "my_state_machine.h"
#include <zephyr/smf.h>
#include "BTN.h"
#include "LED.h"

#define MAX_CHAR 50

// State Prototypes and Enums
static void s0_state_entry(void *o);
static enum smf_state_result s0_state_run(void *o);
static void s0_state_exit(void *o);
static void s1_state_entry(void *o);
static enum smf_state_result s1_state_run(void *o);
static void s1_state_exit(void *o);
static void s2_state_entry(void *o);
static enum smf_state_result s2_state_run(void *o);
static void s2_state_exit(void *o);

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
    uint8_t ubinary_code[8]; // Binary code per character
    char char_seq[MAX_CHAR]; // String of characters
    uint8_t char_index; // used to track where we are in the current sequence
    char cur_char;
} smf_obj_t;

// SMF States array
static const struct smf_state smf_states[] = {
    [S0] = SMF_CREATE_STATE(s0_state_entry,s0_state_run,s0_state_exit,NULL,NULL),
    [S1] = SMF_CREATE_STATE(s1_state_entry,s1_state_run,s1_state_exit,NULL,NULL),
    [S2] = SMF_CREATE_STATE(s2_state_entry,s2_state_run,s2_state_exit,NULL,NULL),
};

// -------- USEFUL FUNCTIONS --------

/// @brief Reset the binary array of an SMF_OBJ_T object.
/// @param obj The SMF_OBJ_T object you want to reset the binary array of.
void reset_ubinary(smf_obj_t *obj) {
    memset(obj->ubinary_code,0,8); // Set binary code to zeroes so far.
}
/// @brief Reset the char array of an SMF_OBJ_T object.
/// @param obj The SMF_OBJ_T object you want to reset the char array of.
void reset_char(smf_obj_t *obj) {
    memset(obj->char_seq,0,50); // Set binary code to zeroes so far.
    obj->char_index = 0;
}

/// @brief Converts a binary array of 1's and 0's into a character code
/// @param binary Binary array (Size MUST be 8 array elements).
/// @return character of the binary array provided.
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
    for(int i = 0; i < 2; i++) {
        LED_set(i,0);
    }
    switch(id) {
        case 0: // success
            LED_set(LED0,1);
            k_msleep(50);
            LED_set(LED1,1);
            k_msleep(100);
            LED_set(LED0,0);
            k_msleep(50);
            LED_set(LED1,0);
            k_msleep(50);
            LED_set(LED0,1);
            k_msleep(50);
            LED_set(LED1,1);
            k_msleep(100);
            LED_set(LED0,0);
            k_msleep(50);
            LED_set(LED1,0);
            break;
        case 1: // cool
            LED_set(LED0,1);
            k_msleep(100);
            LED_set(LED0,0);
            LED_set(LED1,1);
            k_msleep(100);
            LED_set(LED1,0);
            break;
        case 2: // confirmation
            LED_set(LED0,1);
            LED_set(LED1,1);
            k_msleep(100);
            LED_set(LED0,0);
            LED_set(LED1,0);
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
    smf_obj.cur_char = 0;
    smf_set_initial(SMF_CTX(&smf_obj),&smf_states[S0]);
}

int state_machine_run() {
    return smf_run_state(SMF_CTX(&smf_obj));
}


/*
 * S0: Enter/Save/Reset Char Code
 */

static void s0_state_entry(void* o) {
    printk("Entering 8-bit ASCII sequence...\n");
    LED_blink(LED2,LED_1HZ);
}

static enum smf_state_result s0_state_run(void* o) {
    int i = 0;
    while(i < 8) {
        if(BTN_check_clear_pressed(BTN0)) { // Set 1 bit
            smf_obj.ubinary_code[i] = 1;
            light_fx(2);
            i++;
        } else if(BTN_check_clear_pressed(BTN1)) { // Set 0 bit
            smf_obj.ubinary_code[i] = 0;
            light_fx(2);
            i++;
        } else if(BTN_check_clear_pressed(BTN2)) { // clear
            reset_ubinary(&smf_obj);
            reset_char(&smf_obj);
            i = 0;
            printk("Cleared binary buffer...\n");
            light_fx(2);
        } else if(BTN_check_clear_pressed(BTN3)) { // write character
            if(smf_obj.char_index != MAX_CHAR) {
                smf_obj.char_seq[smf_obj.char_index++] = smf_obj.cur_char;
            }
            light_fx(1);
            smf_set_state(SMF_CTX(&smf_obj),&smf_states[S1]);
            return SMF_EVENT_HANDLED; // Important since in loop.
        }
        k_msleep(1);
    }
    smf_obj.cur_char = cnv_binary_char(smf_obj.ubinary_code);
    
    printk("Current Character | [%c]\n",smf_obj.cur_char);

    k_msleep(500);
    light_fx(0);
    return SMF_EVENT_HANDLED;
}

static void s0_state_exit(void* o) {
    printk("Saving character %c...\n",smf_obj.cur_char);
}

/*
* S1: Edit string
*/

static void s1_state_entry(void* o) {
    LED_blink(LED2,LED_4HZ);
}

static enum smf_state_result s1_state_run(void* o) {
    if(BTN_check_clear_pressed(BTN0) || BTN_check_clear_pressed(BTN1)) {
        light_fx(2);
        smf_set_state(SMF_CTX(&smf_obj),&smf_states[S0]);
    } else if (BTN_check_clear_pressed(BTN2)) { // Clear string
        reset_char(&smf_obj);
        reset_ubinary(&smf_obj);
        printk("Clearing string...\n");
        smf_set_state(SMF_CTX(&smf_obj),&smf_states[S0]);
    } else if(BTN_check_clear_pressed(BTN3)) { // Save string
        light_fx(2);
        smf_set_state(SMF_CTX(&smf_obj),&smf_states[S2]);
    }
    k_msleep(1);
    return SMF_EVENT_HANDLED;
}

static void s1_state_exit(void* o) {
}

/*
*   S2: View string
*/

static void s2_state_entry(void* o) {
    printk("Saving string...\n");
    LED_blink(LED2,LED_16HZ);
}

static enum smf_state_result s2_state_run(void* o) {
    if(BTN_check_clear_pressed(BTN0) || BTN_check_clear_pressed(BTN1)) {
        light_fx(2);
        smf_set_state(SMF_CTX(&smf_obj),&smf_states[S0]);
        return SMF_EVENT_HANDLED;
    } else if (BTN_check_clear_pressed(BTN2)) { // Clear string
        reset_char(&smf_obj);
        reset_ubinary(&smf_obj);
        printk("Clearing string...\n");
        smf_set_state(SMF_CTX(&smf_obj),&smf_states[S0]);
        return SMF_EVENT_HANDLED;
    } else if(BTN_check_clear_pressed(BTN3)) { // Print string
        light_fx(2);
        printk("%s\n",smf_obj.char_seq);
    }
    k_msleep(1);
    return SMF_EVENT_HANDLED;
}

static void s2_state_exit(void* o) {
}