#include "my_state_machine.c"
#include <zephyr/smf.h>
#include "BTN.h"
#include "LED.h"


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
    uint16_t ubinary_code[8];
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

void reset_ubinary(smf_obj_t obj) {
    memset(obj.ubinary_code,0,sizeof(obj.ubinary_code)); // Set binary code to zeroes so far.
}

// ----------------------------------


static smf_obj_t smf_obj;

void state_machine_init() {
    reset_ubinary(smf_obj);
}

int state_machine_run() {
    return smf_run_state(SMF_CTX(&smf_obj));
}
