#include <zephyr/smf.h>

#include "LED.h"
#include "BTN.h"
#include "my_state_machine.h"

// Function Prototypes
static void s0_state_entry(void* o);
static enum smf_state_result s0_state_run(void* o);
static void s0_state_exit(void* o);

static void s1_state_entry(void* o);
static enum smf_state_result s1_state_run(void* o);
static void s1_state_exit(void* o);

static void s2_state_entry(void* o);
static enum smf_state_result s2_state_run(void* o);
static void s2_state_exit(void* o);

static void s3_state_entry(void* o);
static enum smf_state_result s3_state_run(void* o);
static void s3_state_exit(void* o);

static void s4_state_entry(void* o);
static enum smf_state_result s4_state_run(void* o);
static void s4_state_exit(void* o);


// Typedefs
enum led_state_machine_states {
    S0,
    S1,
    S2,
    S3,
    S4
};

typedef struct {
    struct smf_ctx ctx;

    uint16_t count;
} s_object_t;

// Local Variables
static const struct smf_state s_states[] = {
    [S0] = SMF_CREATE_STATE(s0_state_entry,s0_state_run,s0_state_exit,NULL,NULL),
    [S1] = SMF_CREATE_STATE(s1_state_entry,s1_state_run,s1_state_exit,NULL,NULL),
    [S2] = SMF_CREATE_STATE(s2_state_entry,s2_state_run,s2_state_exit,NULL,NULL),
    [S3] = SMF_CREATE_STATE(s3_state_entry,s3_state_run,s3_state_exit,NULL,NULL),
    [S4] = SMF_CREATE_STATE(s4_state_entry,s4_state_run,s4_state_exit,NULL,NULL),
};


//Functions and stuff
static s_object_t s_object;

void reset_LED() {
    LED_set(LED0,LED_OFF);
    LED_set(LED1,LED_OFF);
    LED_set(LED2,LED_OFF);
    LED_set(LED3,LED_OFF);
}

void state_machine_init() {
    s_object.count = 0;
    smf_set_initial(SMF_CTX(&s_object),&s_states[S0]);
}

int state_machine_run() {
    return smf_run_state(SMF_CTX(&s_object));
}

/*
 * S0: All LED's off (B1 -> S1)
 */

static void s0_state_entry(void* o) {
    printk("Entering s0.\n");
    reset_LED();
}

static enum smf_state_result s0_state_run(void* o) {
    if(BTN_check_clear_pressed(BTN0)) {
        smf_set_state(SMF_CTX(&s_object),&s_states[S1]);
    }

    return SMF_EVENT_HANDLED;
}

static void s0_state_exit(void* o) {
    printk("Exiting s0.\n");
}

/*
 * S1: LED0 blinks at 4Hz (B1 -> S2, B2 -> S4, B3 -> S0)
 */

static void s1_state_entry(void* o) {
    printk("Entering s1.\n");
    reset_LED();
}

static enum smf_state_result s1_state_run(void* o) {
    // code refreshes every ms, so 125 is 125ms, and so it turns on and off every 250ms, which is 4 Hz.
    if(s_object.count > 125) {
        s_object.count = 0;
        LED_toggle(LED0);
    }

    if(BTN_check_clear_pressed(BTN1)) {
        smf_set_state(SMF_CTX(&s_object),&s_states[S2]);
    } else if(BTN_check_clear_pressed(BTN2)) {
        smf_set_state(SMF_CTX(&s_object),&s_states[S4]);
    } else if(BTN_check_clear_pressed(BTN3)) {
        smf_set_state(SMF_CTX(&s_object),&s_states[S0]);
    }

    s_object.count++;

    return SMF_EVENT_HANDLED;
}

static void s1_state_exit(void* o) {
    printk("Exiting s1.\n");
}

/*
 * S2: LED0 and LED2 on, LED1 and LED3 off. (B3 -> S0) After 1 second will switch to S3
 */

static void s2_state_entry(void* o) {
    printk("Entering s2.\n");
    reset_LED();
    LED_set(LED0,LED_ON);
    LED_set(LED2,LED_ON);
}

static enum smf_state_result s2_state_run(void* o) {
    if(s_object.count > 1000) {
        s_object.count = 0;
        smf_set_state(SMF_CTX(&s_object),&s_states[S3]);
    }
    if(BTN_check_clear_pressed(BTN3)) {
        s_object.count = 0;
        smf_set_state(SMF_CTX(&s_object),&s_states[S0]);
    }

    s_object.count++;

    return SMF_EVENT_HANDLED;
}

static void s2_state_exit(void* o) {
    printk("Exiting s2.\n");
}

/*
 * S3: LED0 and LED2 off, LED1 and LED3 on. (B3 -> S0) After 2 second will switch to S2
 */

static void s3_state_entry(void* o) {
    printk("Entering s3.\n");
    reset_LED();
    LED_set(LED1,LED_ON);
    LED_set(LED3,LED_ON);
}

static enum smf_state_result s3_state_run(void* o) {
    if(s_object.count > 2000) {
        s_object.count = 0;
        smf_set_state(SMF_CTX(&s_object),&s_states[S2]);
    }
    if(BTN_check_clear_pressed(BTN3)) {
        s_object.count = 0;
        smf_set_state(SMF_CTX(&s_object),&s_states[S0]);
    }

    s_object.count++;

    return SMF_EVENT_HANDLED;
}

static void s3_state_exit(void* o) {
    printk("Exiting s3.\n");
}

/*
 * S4: All LED's blink at 16Hz (B3 -> S0) 
 */


static void s4_state_entry(void* o) {
    printk("Entering s4.\n");
    reset_LED();
}

static enum smf_state_result s4_state_run(void* o) {
    if(s_object.count > 31) {
        s_object.count = 0;
        LED_toggle(LED0);
        LED_toggle(LED1);
        LED_toggle(LED2);
        LED_toggle(LED3);
    }
    if(BTN_check_clear_pressed(BTN3)) {
        s_object.count = 0;
        smf_set_state(SMF_CTX(&s_object),&s_states[S0]);
    }

    s_object.count++;

    return SMF_EVENT_HANDLED;
}

static void s4_state_exit(void* o) {
    printk("Exiting s4.\n");
}


